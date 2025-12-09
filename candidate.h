#ifndef CANDIDATE_H
#define CANDIDATE_H
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <limits>
#include <iomanip>
#include <vector>
using namespace std;

const int MAX_BLOCKS = 128;
const char CANDIDATE_DB_FILE[] = "candidates.bin";
const int ID_SIZE = 10;         
const int NAME_SIZE = 30;
const int CNIC_SIZE = 14;         
const int PARTY_SIZE = 20;
const int SYMBOL_SIZE = 15;
const int STATION_SIZE = 10;      
const int CONST_SIZE = 20;      
const int PASS_SIZE = 9;

struct CandidateNode {
    char candidateID[ID_SIZE];     
    char name[NAME_SIZE];
    char cnic[CNIC_SIZE];          
    char party[PARTY_SIZE];        
    char symbol[SYMBOL_SIZE];      
    char pollingStation[STATION_SIZE];
    char constituency[CONST_SIZE]; 
    char password[PASS_SIZE];
    int voteCount;
    int height;
    int leftRef;
    int rightRef;

    CandidateNode(string id = "", string n = "", string c = "",
        string p = "", string sym = "", string station = "",
        string consti = "", string pass = "")
        : voteCount(0), height(1), leftRef(-1), rightRef(-1) {

        // Safe copy with truncation for Visual Studio
        strncpy_s(candidateID, ID_SIZE, id.c_str(), _TRUNCATE);
        candidateID[ID_SIZE - 1] = '\0';

        strncpy_s(name, NAME_SIZE, n.c_str(), _TRUNCATE);
        name[NAME_SIZE - 1] = '\0';

        strncpy_s(cnic, CNIC_SIZE, c.c_str(), _TRUNCATE);
        cnic[CNIC_SIZE - 1] = '\0';

        strncpy_s(party, PARTY_SIZE, p.c_str(), _TRUNCATE);
        party[PARTY_SIZE - 1] = '\0';

        strncpy_s(symbol, SYMBOL_SIZE, sym.c_str(), _TRUNCATE);
        symbol[SYMBOL_SIZE - 1] = '\0';

        strncpy_s(pollingStation, STATION_SIZE, station.c_str(), _TRUNCATE);
        pollingStation[STATION_SIZE - 1] = '\0';

        strncpy_s(constituency, CONST_SIZE, consti.c_str(), _TRUNCATE);
        constituency[CONST_SIZE - 1] = '\0';

        strncpy_s(password, PASS_SIZE, pass.c_str(), _TRUNCATE);
        password[PASS_SIZE - 1] = '\0';
    }
};

class DiskManager {
private:
    fstream file;
    char bitmap[MAX_BLOCKS / 8];
    int rootIndex;
    const size_t BITMAP_SIZE = sizeof(bitmap);
    const size_t METADATA_SIZE = BITMAP_SIZE + sizeof(rootIndex);
    bool fileOpen;

public:
    DiskManager() : rootIndex(-1), fileOpen(false) {
        file.open(CANDIDATE_DB_FILE, ios::in | ios::out | ios::binary);
        if (!file) {
            file.clear();
            file.open(CANDIDATE_DB_FILE, ios::out | ios::binary);
            for (size_t i = 0; i < BITMAP_SIZE; ++i) bitmap[i] = 0;
            rootIndex = -1;
            file.write(bitmap, BITMAP_SIZE);
            file.write((char*)&rootIndex, sizeof(rootIndex));
            file.close();
            file.open(CANDIDATE_DB_FILE, ios::in | ios::out | ios::binary);
        }
        file.seekg(0, ios::beg);
        file.read(bitmap, BITMAP_SIZE);
        file.read((char*)&rootIndex, sizeof(rootIndex));
        fileOpen = true;
    }

    ~DiskManager() {
        if (fileOpen) {
            file.seekp(0, ios::beg);
            file.write(bitmap, BITMAP_SIZE);
            file.write((char*)&rootIndex, sizeof(rootIndex));
            file.close();
        }
    }

    bool isFileOpen() const { return fileOpen; }
    int getRoot() { return rootIndex; }
    void setRoot(int idx) {
        rootIndex = idx;
        file.seekp(BITMAP_SIZE, ios::beg);
        file.write((char*)&rootIndex, sizeof(rootIndex));
        file.flush();
    }

    bool isBitSet(int k) { return (bitmap[k / 8] & (1 << (k % 8))) != 0; }
    void setBit(int k) { bitmap[k / 8] |= (1 << (k % 8)); }
    void clearBit(int k) { bitmap[k / 8] &= ~(1 << (k % 8)); }

    int allocateBlock() {
        for (int k = 1; k < MAX_BLOCKS; ++k) {
            if (!isBitSet(k)) {
                setBit(k);
                file.seekp(0, ios::beg);
                file.write(bitmap, BITMAP_SIZE);
                file.flush();
                return k;
            }
        }
        return -1;
    }

    void freeBlock(int index) {
        if (index <= 0 || index >= MAX_BLOCKS) return;
        clearBit(index);
        file.seekp(0, ios::beg);
        file.write(bitmap, BITMAP_SIZE);
        file.flush();
    }

    void saveNode(int index, CandidateNode* node) {
        if (!fileOpen || node == nullptr || index <= 0) return;
        file.seekp(METADATA_SIZE + index * sizeof(CandidateNode), ios::beg);
        file.write((char*)node, sizeof(CandidateNode));
        file.flush();
    }

    CandidateNode* loadNode(int index) {
        if (!fileOpen || index <= 0) return nullptr;
        CandidateNode* node = new CandidateNode();
        file.seekg(METADATA_SIZE + index * sizeof(CandidateNode), ios::beg);
        file.read((char*)node, sizeof(CandidateNode));
        return node;
    }
};

class CandidateBTree {
private:
    DiskManager dm;
    int rootBlock;

    int height(int blockIndex) {
        if (blockIndex == -1) return 0;
        CandidateNode* node = dm.loadNode(blockIndex);
        int h = node ? node->height : 0;
        delete node;
        return h;
    }

    int getBalance(int blockIndex) {
        if (blockIndex == -1) return 0;
        CandidateNode* node = dm.loadNode(blockIndex);
        int balance = height(node->leftRef) - height(node->rightRef);
        delete node;
        return balance;
    }

    int rotateRight(int yBlock) {
        CandidateNode* y = dm.loadNode(yBlock);
        int xBlock = y->leftRef;
        CandidateNode* x = dm.loadNode(xBlock);
        int T2 = x->rightRef;
        x->rightRef = yBlock;
        y->leftRef = T2;
        y->height = 1 + max(height(y->leftRef), height(y->rightRef));
        x->height = 1 + max(height(x->leftRef), height(x->rightRef));
        dm.saveNode(yBlock, y);
        dm.saveNode(xBlock, x);
        delete y; delete x;
        return xBlock;
    }

    int rotateLeft(int xBlock) {
        CandidateNode* x = dm.loadNode(xBlock);
        int yBlock = x->rightRef;
        CandidateNode* y = dm.loadNode(yBlock);
        int T2 = y->leftRef;
        y->leftRef = xBlock;
        x->rightRef = T2;
        x->height = 1 + max(height(x->leftRef), height(x->rightRef));
        y->height = 1 + max(height(y->leftRef), height(y->rightRef));
        dm.saveNode(xBlock, x);
        dm.saveNode(yBlock, y);
        delete x; delete y;
        return yBlock;
    }

    int insertRec(int blockIndex, string id, string name, string cnic,
        string party, string symbol, string station,
        string constituency, string pass) {
        if (blockIndex == -1) {
            int newBlock = dm.allocateBlock();
            if (newBlock == -1) {
                cout << "ERROR: Disk full!\n";
                return -1;
            }
            CandidateNode* node = new CandidateNode(id, name, cnic, party,
                symbol, station,
                constituency, pass);
            dm.saveNode(newBlock, node);
            delete node;
            return newBlock;
        }

        CandidateNode* node = dm.loadNode(blockIndex);
        if (id < node->candidateID)
            node->leftRef = insertRec(node->leftRef, id, name, cnic, party,
                symbol, station, constituency, pass);
        else if (id > node->candidateID)
            node->rightRef = insertRec(node->rightRef, id, name, cnic, party,
                symbol, station, constituency, pass);
        else {
            delete node;
            return blockIndex;
        }

        node->height = 1 + max(height(node->leftRef), height(node->rightRef));
        int balance = getBalance(blockIndex);

        // AVL rotations
        CandidateNode* leftNode = node->leftRef != -1 ? dm.loadNode(node->leftRef) : nullptr;
        CandidateNode* rightNode = node->rightRef != -1 ? dm.loadNode(node->rightRef) : nullptr;

        if (balance > 1 && leftNode && id < leftNode->candidateID) {
            if (leftNode) delete leftNode;
            if (rightNode) delete rightNode;
            delete node;
            return rotateRight(blockIndex);
        }
        if (balance < -1 && rightNode && id > rightNode->candidateID) {
            if (leftNode) delete leftNode;
            if (rightNode) delete rightNode;
            delete node;
            return rotateLeft(blockIndex);
        }
        if (balance > 1 && leftNode && id > leftNode->candidateID) {
            node->leftRef = rotateLeft(node->leftRef);
            dm.saveNode(blockIndex, node);
            if (leftNode) delete leftNode;
            if (rightNode) delete rightNode;
            delete node;
            return rotateRight(blockIndex);
        }
        if (balance < -1 && rightNode && id < rightNode->candidateID) {
            node->rightRef = rotateRight(node->rightRef);
            dm.saveNode(blockIndex, node);
            if (leftNode) delete leftNode;
            if (rightNode) delete rightNode;
            delete node;
            return rotateLeft(blockIndex);
        }

        if (leftNode) delete leftNode;
        if (rightNode) delete rightNode;

        dm.saveNode(blockIndex, node);
        delete node;
        return blockIndex;
    }

    int searchRec(int blockIndex, string id) {
        if (blockIndex == -1) return -1;
        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return -1;

        string nodeID(node->candidateID);
        if (id == nodeID) {
            delete node;
            return blockIndex;
        }
        int nextBlock = (id < nodeID) ? node->leftRef : node->rightRef;
        delete node;
        return searchRec(nextBlock, id);
    }

    // Helper function to get all candidates in a polling station
    void getCandidatesByStationRec(int blockIndex, string stationID, vector<string>& result) {
        if (blockIndex == -1) return;

        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        getCandidatesByStationRec(node->leftRef, stationID, result);

        // If candidate contests from this station, add to result
        if (string(node->pollingStation) == stationID) {
            result.push_back(string(node->candidateID));
        }

        getCandidatesByStationRec(node->rightRef, stationID, result);
        delete node;
    }

    // Helper function to print candidates filtered by polling station
    void printFilteredCandidatesRec(int blockIndex, const vector<string>& allowedIDs) {
        if (blockIndex == -1) return;

        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        printFilteredCandidatesRec(node->leftRef, allowedIDs);

        // Check if this candidate's ID is in the allowed list
        string currentID(node->candidateID);
        bool isAllowed = false;
        for (const string& id : allowedIDs) {
            if (id == currentID) {
                isAllowed = true;
                break;
            }
        }

        if (isAllowed) {
            cout << "Candidate ID: " << node->candidateID << endl;
            cout << "Name: " << node->name << endl;
            cout << "Party: " << node->party << endl;
            cout << "Symbol: " << node->symbol << endl;
            cout << "Constituency: " << node->constituency << endl;
            cout << "Votes: " << node->voteCount << endl;
            cout << "---------------------------\n";
        }

        printFilteredCandidatesRec(node->rightRef, allowedIDs);
        delete node;
    }

    void inorderRec(int blockIndex) {
        if (blockIndex == -1) return;
        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;
        inorderRec(node->leftRef);
        cout << "---------------------------\n";
        cout << "Candidate ID: " << node->candidateID << endl;
        cout << "Name: " << node->name << endl;
        cout << "CNIC: " << node->cnic << endl;
        cout << "Party: " << node->party << endl;
        cout << "Symbol: " << node->symbol << endl;
        cout << "Polling Station: " << node->pollingStation << endl;
        cout << "Constituency: " << node->constituency << endl;
        cout << "Votes: " << node->voteCount << endl;
        cout << "---------------------------\n";
        inorderRec(node->rightRef);
        delete node;
    }

    // Helper for printWinner
    void findMaxVotes(int blockIndex, string& winnerID, string& winnerName, int& maxVotes) {
        if (blockIndex == -1) return;
        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        findMaxVotes(node->leftRef, winnerID, winnerName, maxVotes);

        if (node->voteCount > maxVotes) {
            maxVotes = node->voteCount;
            winnerID = string(node->candidateID);
            winnerName = string(node->name);
        }

        findMaxVotes(node->rightRef, winnerID, winnerName, maxVotes);
        delete node;
    }

    void inorderRecDetailed(int blockIndex) {
        if (blockIndex == -1) return;
        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;
        inorderRecDetailed(node->leftRef);
        cout << left << setw(10) << node->candidateID
            << setw(20) << node->name
            << setw(15) << node->party
            << setw(12) << node->constituency
            << setw(15) << node->pollingStation
            << setw(8) << node->voteCount << "\n";
        inorderRecDetailed(node->rightRef);
        delete node;
    }

    // Helper to count candidates in a constituency
    void countByConstituencyRec(int blockIndex, string constituency, int& count) {
        if (blockIndex == -1) return;
        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        countByConstituencyRec(node->leftRef, constituency, count);

        if (string(node->constituency) == constituency) {
            count++;
        }

        countByConstituencyRec(node->rightRef, constituency, count);
        delete node;
    }

public:
    CandidateBTree() { rootBlock = dm.getRoot(); }

    // Enhanced insert with all new fields
    void insertCandidate(string id, string name, string cnic,
        string party, string symbol, string station,
        string constituency, string pass = "") {
        rootBlock = insertRec(rootBlock, id, name, cnic, party, symbol,
            station, constituency, pass);
        dm.setRoot(rootBlock);
        cout << "Candidate " << id << " inserted successfully!\n";
    }

    int searchCandidate(string id) {
        return searchRec(rootBlock, id);
    }

    // Get candidate details
    CandidateNode* getCandidate(string id) {
        int block = searchCandidate(id);
        if (block == -1) return nullptr;
        return dm.loadNode(block);
    }

    // Increment vote and persist
    void voteCandidate(string id) {
        int block = searchCandidate(id);
        if (block == -1) {
            cout << "Candidate not found!\n";
            return;
        }
        CandidateNode* node = dm.loadNode(block);
        if (!node) return;
        node->voteCount++;
        dm.saveNode(block, node);
        delete node;
        cout << "Vote cast for candidate " << id << endl;
    }

    void printCandidates() {
        inorderRec(rootBlock);
    }

    void printCandidatesTable() {
        cout << "\n" << string(90, '-') << "\n";
        cout << left << setw(10) << "ID"
            << setw(20) << "Name"
            << setw(15) << "Party"
            << setw(12) << "Constituency"
            << setw(15) << "Station"
            << setw(8) << "Votes" << "\n";
        cout << string(90, '-') << "\n";
        inorderRecDetailed(rootBlock);
        cout << string(90, '-') << "\n";
    }

    // Print candidates from a specific polling station
    void printCandidatesByStation(string stationID) {
        cout << "\nCandidates contesting from Station: " << stationID << "\n";
        cout << string(60, '-') << "\n";

        vector<string> stationCandidates;
        getCandidatesByStationRec(rootBlock, stationID, stationCandidates);

        if (stationCandidates.empty()) {
            cout << "No candidates found for this polling station.\n";
            return;
        }

        cout << left << setw(10) << "ID"
            << setw(20) << "Name"
            << setw(15) << "Party"
            << setw(15) << "Symbol" << "\n";
        cout << string(60, '-') << "\n";

        for (const string& id : stationCandidates) {
            CandidateNode* node = getCandidate(id);
            if (node) {
                cout << left << setw(10) << node->candidateID
                    << setw(20) << node->name
                    << setw(15) << node->party
                    << setw(15) << node->symbol << "\n";
                delete node;
            }
        }
        cout << string(60, '-') << "\n";
    }

    // Print filtered candidates (for voters to see only their station's candidates)
    void printFilteredCandidates(const vector<string>& allowedIDs) {
        cout << "\nCandidates available in your polling station:\n";
        cout << string(60, '-') << "\n";
        printFilteredCandidatesRec(rootBlock, allowedIDs);
        cout << string(60, '-') << "\n";
    }

    void printWinner() {
        if (rootBlock == -1) {
            cout << "No candidates available.\n";
            return;
        }

        string winnerName, winnerID;
        int maxVotes = -1;
        findMaxVotes(rootBlock, winnerID, winnerName, maxVotes);

        if (maxVotes <= 0)
            cout << "No votes were cast.\n";
        else
            cout << "Winner: " << winnerName << " (ID: " << winnerID
            << ") with " << maxVotes << " votes.\n";
    }

    // Verify candidate password
    bool verifyCandidatePassword(const string& id, const string& pass) {
        int block = searchCandidate(id);
        if (block == -1) return false;
        CandidateNode* node = dm.loadNode(block);
        if (!node) return false;
        bool ok = (string(node->password) == pass);
        delete node;
        return ok;
    }

    // Count candidates in a constituency
    int countCandidatesInConstituency(string constituency) {
        int count = 0;
        countByConstituencyRec(rootBlock, constituency, count);
        return count;
    }

    // Get all candidates in a polling station
    vector<string> getCandidatesInStation(string stationID) {
        vector<string> candidates;
        getCandidatesByStationRec(rootBlock, stationID, candidates);
        return candidates;
    }

    int getTotalVotes() {
        int total = 0;
        // This would require traversal to sum all votes
        // For now, returning -1 to indicate not implemented
        return -1;
    }

    void resetAllVotes() {
        resetVotesRec(rootBlock);
    }

private:
    void resetVotesRec(int blockIndex) {
        if (blockIndex == -1) return;
        CandidateNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        resetVotesRec(node->leftRef);
        node->voteCount = 0;
        dm.saveNode(blockIndex, node);
        resetVotesRec(node->rightRef);

        delete node;
    }
};

#endif // CANDIDATE_H