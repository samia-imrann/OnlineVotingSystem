#ifndef CANDIDATE_H
#define CANDIDATE_H
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <limits>
#include <iomanip>
#include <vector>
#include <queue>
#include <map>
using namespace std;

const int MAX_BLOCKS = 1024;
const char CANDIDATE_DB_FILE[] = "candidates.bin";
const int ID_SIZE = 10;           // CID00001 format
const int NAME_SIZE = 50;         // Increased from 30
const int CNIC_SIZE = 14;         // 13 digits + null
const int PARTY_SIZE = 15;        // Shortened party name size
const int STATION_SIZE = 10;      // KHI01 format
const int PASS_SIZE = 20;         // Increased from 9
// Note: Removed constituency as it wasn't being used consistently

// Party secret codes - could be stored in a separate file or hardcoded
const map<string, string> PARTY_SECRET_CODES = {
    {"PTI", "PTI12345"},
    {"PMLN", "PMLN5678"},
    {"PPP", "PPP91011"},
    {"MQM", "MQM13141"},
    {"ANP", "ANP16171"},
    {"JUI", "JUI19202"},
    {"IND", "INDEP1234"}
};

// B-tree parameters
const int MIN_DEGREE = 3;
const int MAX_KEYS = 2 * MIN_DEGREE - 1;
const int MIN_KEYS = MIN_DEGREE - 1;

struct CandidateNode {
    char candidateID[ID_SIZE];
    char name[NAME_SIZE];
    char cnic[CNIC_SIZE];
    char party[PARTY_SIZE];
    char pollingStation[STATION_SIZE];
    char password[PASS_SIZE];
    int voteCount;

    CandidateNode() : voteCount(0) {
        memset(candidateID, 0, ID_SIZE);
        memset(name, 0, NAME_SIZE);
        memset(cnic, 0, CNIC_SIZE);
        memset(party, 0, PARTY_SIZE);
        memset(pollingStation, 0, STATION_SIZE);
        memset(password, 0, PASS_SIZE);
    }

    CandidateNode(string id, string n, string c,
        string p, string station,
        string pass)
        : voteCount(0) {

        strncpy_s(candidateID, ID_SIZE, id.c_str(), _TRUNCATE);
        candidateID[ID_SIZE - 1] = '\0';

        strncpy_s(name, NAME_SIZE, n.c_str(), _TRUNCATE);
        name[NAME_SIZE - 1] = '\0';

        strncpy_s(cnic, CNIC_SIZE, c.c_str(), _TRUNCATE);
        cnic[CNIC_SIZE - 1] = '\0';

        strncpy_s(party, PARTY_SIZE, p.c_str(), _TRUNCATE);
        party[PARTY_SIZE - 1] = '\0';

        strncpy_s(pollingStation, STATION_SIZE, station.c_str(), _TRUNCATE);
        pollingStation[STATION_SIZE - 1] = '\0';

        strncpy_s(password, PASS_SIZE, pass.c_str(), _TRUNCATE);
        password[PASS_SIZE - 1] = '\0';
    }
};

// B-tree Node structure for disk storage
struct BTreeNode {
    bool is_leaf;
    int key_count;
    int disk_index;
    int child_indices[MAX_KEYS + 1];
    CandidateNode candidates[MAX_KEYS];

    BTreeNode(bool leaf = true) {
        is_leaf = leaf;
        key_count = 0;
        disk_index = -1;
        for (int i = 0; i <= MAX_KEYS; ++i) child_indices[i] = -1;
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

    void write_superblock() {
        file.seekp(0, ios::beg);
        file.write(bitmap, BITMAP_SIZE);
        file.write((char*)&rootIndex, sizeof(rootIndex));
        file.flush();
    }

    void serialize_node(BTreeNode* node, char* buffer) {
        memset(buffer, 0, sizeof(BTreeNode));
        int offset = 0;

        memcpy(buffer + offset, &node->is_leaf, sizeof(node->is_leaf));
        offset += sizeof(node->is_leaf);

        memcpy(buffer + offset, &node->key_count, sizeof(node->key_count));
        offset += sizeof(node->key_count);

        memcpy(buffer + offset, &node->disk_index, sizeof(node->disk_index));
        offset += sizeof(node->disk_index);

        for (int i = 0; i < MAX_KEYS; ++i) {
            memcpy(buffer + offset, &node->candidates[i], sizeof(CandidateNode));
            offset += sizeof(CandidateNode);
        }

        for (int i = 0; i <= MAX_KEYS; ++i) {
            memcpy(buffer + offset, &node->child_indices[i], sizeof(int));
            offset += sizeof(int);
        }
    }

    void deserialize_node(BTreeNode* node, const char* buffer) {
        int offset = 0;

        memcpy(&node->is_leaf, buffer + offset, sizeof(node->is_leaf));
        offset += sizeof(node->is_leaf);

        memcpy(&node->key_count, buffer + offset, sizeof(node->key_count));
        offset += sizeof(node->key_count);

        memcpy(&node->disk_index, buffer + offset, sizeof(node->disk_index));
        offset += sizeof(node->disk_index);

        for (int i = 0; i < MAX_KEYS; ++i) {
            memcpy(&node->candidates[i], buffer + offset, sizeof(CandidateNode));
            offset += sizeof(CandidateNode);
        }

        for (int i = 0; i <= MAX_KEYS; ++i) {
            memcpy(&node->child_indices[i], buffer + offset, sizeof(int));
            offset += sizeof(int);
        }
    }

public:
    DiskManager() : rootIndex(-1), fileOpen(false) {
        file.open(CANDIDATE_DB_FILE, ios::in | ios::out | ios::binary);
        if (!file) {
            file.clear();
            file.open(CANDIDATE_DB_FILE, ios::out | ios::binary);
            for (size_t i = 0; i < BITMAP_SIZE; ++i) bitmap[i] = 0;
            rootIndex = -1;
            write_superblock();
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
            write_superblock();
            file.close();
        }
    }

    bool isFileOpen() const { return fileOpen; }
    int getRoot() { return rootIndex; }
    void setRoot(int idx) {
        rootIndex = idx;
        write_superblock();
    }

    bool isBitSet(int k) { return (bitmap[k / 8] & (1 << (k % 8))) != 0; }
    void setBit(int k) { bitmap[k / 8] |= (1 << (k % 8)); }
    void clearBit(int k) { bitmap[k / 8] &= ~(1 << (k % 8)); }

    int allocateBlock() {
        for (int k = 1; k < MAX_BLOCKS; ++k) {
            if (!isBitSet(k)) {
                setBit(k);
                write_superblock();
                return k;
            }
        }
        return -1;
    }

    void freeBlock(int index) {
        if (index <= 0 || index >= MAX_BLOCKS) return;
        clearBit(index);
        write_superblock();
    }

    void saveNode(BTreeNode* node) {
        if (!fileOpen || node == nullptr || node->disk_index <= 0) return;
        char buffer[sizeof(BTreeNode)];
        serialize_node(node, buffer);
        file.seekp(METADATA_SIZE + (long long)node->disk_index * sizeof(BTreeNode), ios::beg);
        file.write(buffer, sizeof(BTreeNode));
        file.flush();
    }

    BTreeNode* loadNode(int index) {
        if (!fileOpen || index <= 0) return nullptr;
        BTreeNode* node = new BTreeNode();
        char buffer[sizeof(BTreeNode)];
        file.seekg(METADATA_SIZE + (long long)index * sizeof(BTreeNode), ios::beg);
        file.read(buffer, sizeof(BTreeNode));
        if (!file.good()) {
            delete node;
            return nullptr;
        }
        deserialize_node(node, buffer);
        return node;
    }

    void saveCandidate(int blockIndex, int keyIndex, CandidateNode* candidate) {
        BTreeNode* node = loadNode(blockIndex);
        if (!node) return;
        node->candidates[keyIndex] = *candidate;
        saveNode(node);
        delete node;
    }
};

class CandidateBTree {
private:
    DiskManager dm;
    int rootBlock;
    int nextCandidateID; // For generating sequential IDs

    // Helper function to compare candidate IDs
    int compareCandidateID(const string& id1, const string& id2) {
        return id1.compare(id2);
    }

    // Generate next candidate ID
    string generateCandidateID() {
        // Find max ID from existing candidates
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        int maxID = 0;
        for (const auto& candidate : allCandidates) {
            string idStr = string(candidate.candidateID);
            if (idStr.length() >= 8 && idStr.substr(0, 3) == "CID") {
                string numPart = idStr.substr(3);
                try {
                    int currentID = stoi(numPart);
                    if (currentID > maxID) {
                        maxID = currentID;
                    }
                }
                catch (...) {
                    continue;
                }
            }
        }

        stringstream ss;
        ss << "CID" << setw(5) << setfill('0') << (maxID + 1);
        return ss.str();
    }

    // Verify party secret code
    bool verifyPartySecretCode(const string& partyName, const string& secretCode) {
        auto it = PARTY_SECRET_CODES.find(partyName);
        if (it != PARTY_SECRET_CODES.end()) {
            return it->second == secretCode;
        }
        return false;
    }

    // Search recursively in B-tree
    pair<BTreeNode*, int> searchRec(BTreeNode* node, const string& id) {
        if (!node) return { nullptr, -1 };

        int i = 0;
        while (i < node->key_count && compareCandidateID(id, node->candidates[i].candidateID) > 0) {
            i++;
        }

        if (i < node->key_count && compareCandidateID(id, node->candidates[i].candidateID) == 0) {
            cout << "DEBUG: Found at index " << i << " ID: " << node->candidates[i].candidateID << endl;
            return { node, i };
        }

        if (node->is_leaf) {
            return { nullptr, -1 };
        }

        BTreeNode* child = dm.loadNode(node->child_indices[i]);
        if (!child) return { nullptr, -1 };

        auto result = searchRec(child, id);

        // Only delete child if it's NOT the node we found (which we need to return)
        if (child != result.first) {
            delete child;
        }

        return result;
    }

    // Split child in B-tree
    void splitChild(BTreeNode* parent, int i, BTreeNode* child) {
        // Create new child node
        BTreeNode* newChild = new BTreeNode(child->is_leaf);
        newChild->disk_index = dm.allocateBlock();

        if (newChild->disk_index == -1) {
            cout << "Error: No free blocks available for split!\n";
            delete newChild;
            return;
        }

        newChild->key_count = MIN_KEYS;

        // Copy last MIN_KEYS keys from child to newChild
        for (int j = 0; j < MIN_KEYS; j++) {
            newChild->candidates[j] = child->candidates[j + MIN_DEGREE];
        }

        // If not leaf, copy children too
        if (!child->is_leaf) {
            for (int j = 0; j < MIN_DEGREE; j++) {
                newChild->child_indices[j] = child->child_indices[j + MIN_DEGREE];
                // Reset old child indices
                child->child_indices[j + MIN_DEGREE] = -1;
            }
        }

        // Reduce child's key count
        child->key_count = MIN_KEYS;

        // Make space in parent for new child
        for (int j = parent->key_count; j > i; j--) {
            parent->child_indices[j + 1] = parent->child_indices[j];
        }
        parent->child_indices[i + 1] = newChild->disk_index;

        // Make space in parent for the middle key
        for (int j = parent->key_count - 1; j >= i; j--) {
            parent->candidates[j + 1] = parent->candidates[j];
        }

        // Move middle key from child to parent
        parent->candidates[i] = child->candidates[MIN_KEYS];
        parent->key_count++;

        // Save all nodes
        dm.saveNode(child);
        dm.saveNode(newChild);
        dm.saveNode(parent);

        delete newChild;
    }

    // Insert non-full
    void insertNonFull(BTreeNode* node, CandidateNode* candidate) {
        int i = node->key_count - 1;

        if (node->is_leaf) {
            // Find the correct position to insert
            while (i >= 0 && compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) < 0) {
                node->candidates[i + 1] = node->candidates[i];
                i--;
            }

            // Check bounds before writing
            if (i + 1 < MAX_KEYS) {
                node->candidates[i + 1] = *candidate;
                node->key_count++;
                dm.saveNode(node);
            }
            else {
                cout << "Error: Node is full! This should not happen in insertNonFull.\n";
            }
        }
        else {
            // Find the child to go to
            while (i >= 0 && compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) < 0) {
                i--;
            }
            i++;  // i is now the index of child to go to

            // Validate child index
            if (i < 0 || i > node->key_count) {
                cout << "Error: Invalid child index " << i << " (key_count=" << node->key_count << ")\n";
                return;
            }

            // Load child node
            BTreeNode* child = dm.loadNode(node->child_indices[i]);

            // If child doesn't exist, create a new leaf node
            if (!child) {
                child = new BTreeNode(true);
                child->disk_index = dm.allocateBlock();
                if (child->disk_index == -1) {
                    cout << "Error: No free blocks available!\n";
                    delete child;
                    return;
                }
                node->child_indices[i] = child->disk_index;
                dm.saveNode(node);  // Save parent with updated child index
            }

            // Check if child is full
            if (child->key_count == MAX_KEYS) {
                splitChild(node, i, child);

                // After split, the candidate might go to a different child
                if (compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) > 0) {
                    i++;

                    // Delete old child pointer and load new one
                    delete child;

                    // Validate new child index
                    if (i < 0 || i > node->key_count) {
                        cout << "Error: Invalid child index after split " << i << "\n";
                        return;
                    }

                    child = dm.loadNode(node->child_indices[i]);
                    if (!child) {
                        cout << "Error: Child not found after split!\n";
                        return;
                    }
                }
            }

            // Recursively insert into child
            insertNonFull(child, candidate);

            // Save child and cleanup
            dm.saveNode(child);
            delete child;
        }
    }

    // Inorder traversal helpers
    void inorderRec(int blockIndex, vector<string>& result, const string& stationID = "") {
        if (blockIndex == -1) return;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf) {
                inorderRec(node->child_indices[i], result, stationID);
            }

            if (stationID.empty() || string(node->candidates[i].pollingStation) == stationID) {
                result.push_back(string(node->candidates[i].candidateID));
            }
        }

        if (!node->is_leaf) {
            inorderRec(node->child_indices[node->key_count], result, stationID);
        }

        delete node;
    }

    void inorderDetailedRec(int blockIndex, vector<CandidateNode>& result) {
        if (blockIndex == -1) return;
        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;
        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf) {
                inorderDetailedRec(node->child_indices[i], result);
            }
            // Create a COPY of the candidate, not a reference
            CandidateNode copy;
            strncpy_s(copy.candidateID, ID_SIZE, node->candidates[i].candidateID, ID_SIZE - 1);
            strncpy_s(copy.name, NAME_SIZE, node->candidates[i].name, NAME_SIZE - 1);
            strncpy_s(copy.cnic, CNIC_SIZE, node->candidates[i].cnic, CNIC_SIZE - 1);
            strncpy_s(copy.party, PARTY_SIZE, node->candidates[i].party, PARTY_SIZE - 1);
            strncpy_s(copy.pollingStation, STATION_SIZE, node->candidates[i].pollingStation, STATION_SIZE - 1);
            strncpy_s(copy.password, PASS_SIZE, node->candidates[i].password, PASS_SIZE - 1);
            copy.voteCount = node->candidates[i].voteCount;
            // Ensure null termination
            copy.candidateID[ID_SIZE - 1] = '\0';
            copy.name[NAME_SIZE - 1] = '\0';
            copy.cnic[CNIC_SIZE - 1] = '\0';
            copy.party[PARTY_SIZE - 1] = '\0';
            copy.pollingStation[STATION_SIZE - 1] = '\0';
            copy.password[PASS_SIZE - 1] = '\0';
            result.push_back(copy);
        }
        if (!node->is_leaf) {
            inorderDetailedRec(node->child_indices[node->key_count], result);
        }
        delete node;
    }

    // Find candidate with max votes
    void findMaxVotesRec(int blockIndex, string& winnerID, string& winnerName, int& maxVotes) {
        if (blockIndex == -1) return;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf) {
                findMaxVotesRec(node->child_indices[i], winnerID, winnerName, maxVotes);
            }

            if (node->candidates[i].voteCount > maxVotes) {
                maxVotes = node->candidates[i].voteCount;
                winnerID = string(node->candidates[i].candidateID);
                winnerName = string(node->candidates[i].name);
            }
        }

        if (!node->is_leaf) {
            findMaxVotesRec(node->child_indices[node->key_count], winnerID, winnerName, maxVotes);
        }

        delete node;
    }

    // Reset votes recursively
    void resetVotesRec(int blockIndex) {
        if (blockIndex == -1) return;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf) {
                resetVotesRec(node->child_indices[i]);
            }

            node->candidates[i].voteCount = 0;
            dm.saveCandidate(blockIndex, i, &node->candidates[i]);
        }

        if (!node->is_leaf) {
            resetVotesRec(node->child_indices[node->key_count]);
        }

        delete node;
    }

public:
    CandidateBTree() {
        rootBlock = dm.getRoot();

        // If no root exists in disk, create an empty one
        if (rootBlock == -1) {
            BTreeNode* root = new BTreeNode(true);
            root->disk_index = dm.allocateBlock();
            if (root->disk_index != -1) {
                dm.saveNode(root);
                dm.setRoot(root->disk_index);
                rootBlock = root->disk_index;
            }
            delete root;
        }

        nextCandidateID = 1;
    }

    // Public method to register candidate with party secret code
    bool registerCandidate(string name, string cnic, string partyName, string secretCode, string pollingStation, string password) {

        // Verify party secret code
        if (!verifyPartySecretCode(partyName, secretCode)) {
            cout << "Error: Invalid party name or secret code!\n";
            return false;
        }

        // Check if CNIC already registered
        if (searchByCNIC(cnic) != -1) {
            cout << "Error: This CNIC is already registered as a candidate!\n";
            return false;
        }

        // Generate candidate ID
        string candidateID = generateCandidateID();

        // Create and insert candidate
        CandidateNode* newCandidate = new CandidateNode(candidateID, name, cnic,
            partyName, pollingStation, password);

        // Load root node
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) {
            // Create new root if it doesn't exist
            root = new BTreeNode(true);
            root->disk_index = dm.allocateBlock();
            if (root->disk_index == -1) {
                cout << "Error: No free blocks available!\n";
                delete root;
                delete newCandidate;
                return false;
            }
            rootBlock = root->disk_index;
            dm.setRoot(rootBlock);
        }

        if (root->key_count == MAX_KEYS) {
            // Root is full, create new root
            BTreeNode* newRoot = new BTreeNode(false);
            newRoot->disk_index = dm.allocateBlock();
            if (newRoot->disk_index == -1) {
                cout << "Error: No free blocks available for new root!\n";
                delete newRoot;
                delete root;
                delete newCandidate;
                return false;
            }

            newRoot->child_indices[0] = root->disk_index;
            splitChild(newRoot, 0, root);

            // Update root
            dm.setRoot(newRoot->disk_index);
            rootBlock = newRoot->disk_index;

            // Insert into new root
            insertNonFull(newRoot, newCandidate);
            dm.saveNode(newRoot);
            delete newRoot;
        }
        else {
            insertNonFull(root, newCandidate);
            dm.saveNode(root);
        }

        delete root;
        delete newCandidate;

        cout << "\n" << string(50, '=') << "\n";
        cout << "  CANDIDATE REGISTERED SUCCESSFULLY!\n";
        cout << string(50, '=') << "\n";
        cout << "Candidate ID: " << candidateID << "\n";
        cout << "Name: " << name << "\n";
        cout << "Party: " << partyName << "\n";
        cout << "Polling Station: " << pollingStation << "\n";
        cout << string(50, '=') << "\n";

        return true;
    }

    // Helper method for admin to insert candidate directly (for testing/backup)
    void insertCandidate(string id, string name, string cnic,
        string party, string station,
        string pass = "") {

        CandidateNode* newCandidate = new CandidateNode(id, name, cnic, party,
            station, pass);

        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return;

        if (root->key_count == MAX_KEYS) {
            BTreeNode* newRoot = new BTreeNode(false);
            newRoot->disk_index = dm.allocateBlock();
            newRoot->child_indices[0] = root->disk_index;

            splitChild(newRoot, 0, root);
            dm.setRoot(newRoot->disk_index);
            rootBlock = newRoot->disk_index;

            insertNonFull(newRoot, newCandidate);
            dm.saveNode(newRoot);
            delete newRoot;
        }
        else {
            insertNonFull(root, newCandidate);
            dm.saveNode(root);
        }

        delete root;
        delete newCandidate;
        cout << "Candidate " << id << " inserted successfully!\n";
    }

    // Search for candidate by ID
    int searchCandidate(string id) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return -1;

        auto result = searchRec(root, id);
        delete root;

        if (result.first && result.second != -1) {
            return result.first->disk_index;
        }
        return -1;
    }

    // Search for candidate by CNIC
    int searchByCNIC(string cnic) {
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        for (const auto& candidate : allCandidates) {
            if (string(candidate.cnic) == cnic) {
                return 1; // Found
            }
        }
        return -1; // Not found
    }

    // Get candidate details
    CandidateNode* getCandidate(string id) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return nullptr;
        auto result = searchRec(root, id);
        CandidateNode* candidate = nullptr;

        if (result.first && result.second != -1) {
            // Get reference to found candidate
            const CandidateNode& found = result.first->candidates[result.second];

            // Create new candidate with proper initialization
            candidate = new CandidateNode();

            // Copy all fields safely
            strncpy_s(candidate->candidateID, ID_SIZE, found.candidateID, ID_SIZE - 1);
            candidate->candidateID[ID_SIZE - 1] = '\0';

            strncpy_s(candidate->name, NAME_SIZE, found.name, NAME_SIZE - 1);
            candidate->name[NAME_SIZE - 1] = '\0';

            strncpy_s(candidate->cnic, CNIC_SIZE, found.cnic, CNIC_SIZE - 1);
            candidate->cnic[CNIC_SIZE - 1] = '\0';

            strncpy_s(candidate->party, PARTY_SIZE, found.party, PARTY_SIZE - 1);
            candidate->party[PARTY_SIZE - 1] = '\0';

            strncpy_s(candidate->pollingStation, STATION_SIZE, found.pollingStation, STATION_SIZE - 1);
            candidate->pollingStation[STATION_SIZE - 1] = '\0';

            strncpy_s(candidate->password, PASS_SIZE, found.password, PASS_SIZE - 1);
            candidate->password[PASS_SIZE - 1] = '\0';

            candidate->voteCount = found.voteCount;
        }
        if (root != result.first) {
            delete root;
        }
        if (result.first && result.first != root) {
            delete result.first;
        }
        return candidate;
    }

    // Vote for candidate
    void voteCandidate(string id) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return;

        auto result = searchRec(root, id);
        delete root;

        if (result.first && result.second != -1) {
            result.first->candidates[result.second].voteCount++;
            dm.saveCandidate(result.first->disk_index, result.second,
                &result.first->candidates[result.second]);
        }
    }

    // Print all candidates
    void printCandidates() {
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        for (const auto& candidate : allCandidates) {
            cout << "---------------------------\n";
            cout << "Candidate ID: " << candidate.candidateID << endl;
            cout << "Name: " << candidate.name << endl;
            cout << "CNIC: " << candidate.cnic << endl;
            cout << "Party: " << candidate.party << endl;
            cout << "Polling Station: " << candidate.pollingStation << endl;
            cout << "Votes: " << candidate.voteCount << endl;
            cout << "---------------------------\n";
        }
    }

    // Print candidates in table format
    void printCandidatesTable() {
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        cout << "\n" << string(100, '-') << "\n";
        cout << left << setw(10) << "ID"
            << setw(25) << "Name"
            << setw(15) << "Party"
            << setw(15) << "Station"
            << setw(10) << "Votes" << "\n";
        cout << string(100, '-') << "\n";

        for (const auto& candidate : allCandidates) {
            cout << left << setw(10) << candidate.candidateID
                << setw(25) << candidate.name
                << setw(15) << candidate.party
                << setw(15) << candidate.pollingStation
                << setw(10) << candidate.voteCount << "\n";
        }
        cout << string(100, '-') << "\n";
    }

    // Print candidates by station
    void printCandidatesByStation(string stationID) {
        vector<string> stationCandidates;
        inorderRec(rootBlock, stationCandidates, stationID);

        cout << "\nCandidates contesting from Station: " << stationID << "\n";
        cout << string(80, '-') << "\n";

        if (stationCandidates.empty()) {
            cout << "No candidates found for this polling station.\n";
            return;
        }

        cout << left << setw(10) << "ID"
            << setw(25) << "Name"
            << setw(15) << "Party"
            << setw(10) << "Votes" << "\n";
        cout << string(80, '-') << "\n";

        for (const string& id : stationCandidates) {
            CandidateNode* node = getCandidate(id);
            if (node) {
                cout << left << setw(10) << node->candidateID
                    << setw(25) << node->name
                    << setw(15) << node->party
                    << setw(10) << node->voteCount << "\n";
                delete node;
            }
        }
        cout << string(80, '-') << "\n";
    }

    // Print filtered candidates
    void printFilteredCandidates(const vector<string>& allowedIDs) {
        cout << "\nCandidates available in your polling station:\n";
        cout << string(80, '-') << "\n";

        for (const string& id : allowedIDs) {
            CandidateNode* node = getCandidate(id);
            if (node) {
                cout << "Candidate ID: " << node->candidateID << endl;
                cout << "Name: " << node->name << endl;
                cout << "Party: " << node->party << endl;
                cout << "Votes: " << node->voteCount << endl;
                cout << "---------------------------\n";
                delete node;
            }
        }
        cout << string(80, '-') << "\n";
    }

    // Print winner
    void printWinner() {
        if (rootBlock == -1) {
            cout << "No candidates available.\n";
            return;
        }

        string winnerName, winnerID;
        int maxVotes = -1;
        findMaxVotesRec(rootBlock, winnerID, winnerName, maxVotes);

        if (maxVotes <= 0)
            cout << "No votes were cast.\n";
        else
            cout << "Winner: " << winnerName << " (ID: " << winnerID
            << ") with " << maxVotes << " votes.\n";
    }

    // Verify candidate password
    bool verifyCandidatePassword(const string& id, const string& pass) {
        CandidateNode* node = getCandidate(id);
        if (!node) return false;
        bool ok = (string(node->password) == pass);
        delete node;
        return ok;
    }

    // Get all candidates in a polling station
    vector<string> getCandidatesInStation(string stationID) {
        vector<string> candidates;
        inorderRec(rootBlock, candidates, stationID);
        return candidates;
    }

    // Get list of all parties
    vector<string> getAllParties() {
        vector<string> parties;
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        for (const auto& candidate : allCandidates) {
            string party = string(candidate.party);
            bool found = false;
            for (const auto& p : parties) {
                if (p == party) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                parties.push_back(party);
            }
        }
        return parties;
    }

    // Display available parties and their symbols
    void displayAvailableParties() {
        cout << "\n" << string(60, '=') << "\n";
        cout << "          AVAILABLE POLITICAL PARTIES\n";
        cout << string(60, '=') << "\n";

        for (const auto& party : PARTY_SECRET_CODES) {
            cout << "Party: " << left << setw(10) << party.first;
            cout << " | Secret Code: " << party.second;
            cout << "\n";
        }
        cout << string(60, '=') << "\n\n";
    }

    // Reset all votes
    void resetAllVotes() {
        resetVotesRec(rootBlock);
    }
};

#endif // CANDIDATE_H