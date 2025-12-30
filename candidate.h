#ifndef CANDIDATEBTREE_H
#define CANDIDATEBTREE_H

#include "disk_manager.h"
#include "config.h"
#include <vector>
#include <queue>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <cstring>
#include <map>
using namespace std;

class CandidateBTree {
private:
    DiskManager dm;
    int rootBlock;
    int nextCandidateID;



    void debugRootContents() {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return;

        cout << "DEBUG ROOT CONTENTS:\n";
        cout << "key_count=" << root->key_count << ", is_leaf=" << root->is_leaf << endl;

        for (int i = 0; i < root->key_count; i++) {
            cout << "Candidate " << i << ":\n";
            cout << "  ID: ";
            for (int j = 0; j < ID_SIZE; j++) {
                if (root->candidates[i].candidateID[j] == 0) break;
                cout << root->candidates[i].candidateID[j];
            }
            cout << endl;

            cout << "  Name: ";
            for (int j = 0; j < NAME_SIZE; j++) {
                if (root->candidates[i].name[j] == 0) break;
                cout << root->candidates[i].name[j];
            }
            cout << endl;

            cout << "  Raw bytes (ID): ";
            for (int j = 0; j < ID_SIZE; j++) {
                printf("%02X ", (unsigned char)root->candidates[i].candidateID[j]);
            }
            cout << endl;
        }
        delete root;
    }

    int compareCandidateID(const string& id1, const string& id2) {
        return id1.compare(id2);
    }

    string generateCandidateID() {
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        int maxID = 0;
        for (const auto& candidate : allCandidates) {
            string idStr = string(candidate.candidateID);
            if (idStr.length() >= 8 && idStr.substr(0, 3) == "CID") {
                string numPart = idStr.substr(3);
                try {
                    int currentID = stoi(numPart);
                    if (currentID > maxID) maxID = currentID;
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

    bool verifyPartySecretCode(const string& partyName, const string& secretCode) {
        auto it = PARTY_SECRET_CODES.find(partyName);
        if (it != PARTY_SECRET_CODES.end())
            return it->second == secretCode;
        return false;
    }

    void splitChild(BTreeNode* parent, int i, BTreeNode* child) {
        BTreeNode* newChild = new BTreeNode(child->is_leaf);
        newChild->disk_index = dm.allocateBlock();

        if (newChild->disk_index == -1) {
            delete newChild;
            return;
        }

        newChild->key_count = MIN_KEYS;

        // Copy the last MIN_KEYS keys from child to newChild
        for (int j = 0; j < MIN_KEYS; j++) {
            newChild->candidates[j] = child->candidates[j + MIN_DEGREE];
        }

        // If not leaf, copy the last MIN_DEGREE children
        if (!child->is_leaf) {
            for (int j = 0; j < MIN_DEGREE; j++) {
                newChild->child_indices[j] = child->child_indices[j + MIN_DEGREE];
                child->child_indices[j + MIN_DEGREE] = -1; // Clear in old child
            }
        }

        // Reduce child's key count (removes the keys moved to newChild)
        child->key_count = MIN_KEYS;

        // Make space in parent for the new child
        for (int j = parent->key_count; j >= i + 1; j--) {
            parent->child_indices[j + 1] = parent->child_indices[j];
        }
        parent->child_indices[i + 1] = newChild->disk_index;

        // Make space in parent for the new key
        for (int j = parent->key_count - 1; j >= i; j--) {
            parent->candidates[j + 1] = parent->candidates[j];
        }

        // Move the middle key from child to parent
        parent->candidates[i] = child->candidates[MIN_KEYS];
        parent->key_count++;

        // Save all modified nodes
        dm.saveNode(child);
        dm.saveNode(newChild);
        dm.saveNode(parent);

        delete newChild;
    }

    void insertNonFull(BTreeNode* node, CandidateNode* candidate) {
        int i = node->key_count - 1;

        if (node->is_leaf) {
            // Find position to insert
            while (i >= 0 && compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) < 0) {
                node->candidates[i + 1] = node->candidates[i];
                i--;
            }

            // Insert at position i+1
            node->candidates[i + 1] = *candidate;
            node->key_count++;
            dm.saveNode(node);
        }
        else {
            // Find child to insert into
            while (i >= 0 && compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) < 0) {
                i--;
            }
            i++;

            if (i < 0 || i > MAX_KEYS) {
                return; // Invalid index
            }

            BTreeNode* child = dm.loadNode(node->child_indices[i]);
            if (!child) {
                return; // Child not found
            }

            if (child->key_count == MAX_KEYS) {
                splitChild(node, i, child);

                // After split, determine which child to insert into
                if (compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) > 0) {
                    i++;
                }

                // Load the correct child
                delete child;
                child = dm.loadNode(node->child_indices[i]);
                if (!child) return;
            }

            insertNonFull(child, candidate);
            dm.saveNode(child);
            delete child;
        }
    }

    void inorderDetailedRec(int blockIndex, vector<CandidateNode>& result) {
        if (blockIndex <= 0) return;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            // Traverse left child if exists
            if (!node->is_leaf && node->child_indices[i] > 0) {
                inorderDetailedRec(node->child_indices[i], result);
            }

            // Add current candidate
            CandidateNode copy;
            memset(&copy, 0, sizeof(CandidateNode));

            // FIXED: Use strncpy_s with proper null termination
            strncpy_s(copy.candidateID, ID_SIZE, node->candidates[i].candidateID, _TRUNCATE);
            copy.candidateID[ID_SIZE - 1] = '\0';

            strncpy_s(copy.name, NAME_SIZE, node->candidates[i].name, _TRUNCATE);
            copy.name[NAME_SIZE - 1] = '\0';

            strncpy_s(copy.cnic, CNIC_SIZE, node->candidates[i].cnic, _TRUNCATE);
            copy.cnic[CNIC_SIZE - 1] = '\0';

            strncpy_s(copy.party, PARTY_SIZE, node->candidates[i].party, _TRUNCATE);
            copy.party[PARTY_SIZE - 1] = '\0';

            strncpy_s(copy.pollingStation, STATION_SIZE, node->candidates[i].pollingStation, _TRUNCATE);
            copy.pollingStation[STATION_SIZE - 1] = '\0';

            strncpy_s(copy.password, PASS_SIZE, node->candidates[i].password, _TRUNCATE);
            copy.password[PASS_SIZE - 1] = '\0';

            copy.voteCount = node->candidates[i].voteCount;

            result.push_back(copy);
        }

        // Traverse last child
        if (!node->is_leaf && node->child_indices[node->key_count] > 0) {
            inorderDetailedRec(node->child_indices[node->key_count], result);
        }

        delete node;
    }

    void inorderStationRec(int blockIndex, const string& stationID, vector<CandidateNode>& result) {
        if (blockIndex <= 0) return;
        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf && node->child_indices[i] > 0) {
                inorderStationRec(node->child_indices[i], stationID, result);
            }

            if (stationID.empty() || string(node->candidates[i].pollingStation) == stationID) {
                CandidateNode copy;
                memset(&copy, 0, sizeof(CandidateNode));

                strncpy_s(copy.candidateID, ID_SIZE, node->candidates[i].candidateID, _TRUNCATE);
                copy.candidateID[ID_SIZE - 1] = '\0';

                strncpy_s(copy.name, NAME_SIZE, node->candidates[i].name, _TRUNCATE);
                copy.name[NAME_SIZE - 1] = '\0';

                strncpy_s(copy.party, PARTY_SIZE, node->candidates[i].party, _TRUNCATE);
                copy.party[PARTY_SIZE - 1] = '\0';

                copy.voteCount = node->candidates[i].voteCount;
                result.push_back(copy);
            }
        }

        if (!node->is_leaf && node->child_indices[node->key_count] > 0) {
            inorderStationRec(node->child_indices[node->key_count], stationID, result);
        }

        delete node;
    }

    void resetVotesRec(int blockIndex) {
        if (blockIndex <= 0) return;
        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf && node->child_indices[i] > 0) {
                resetVotesRec(node->child_indices[i]);
            }
            node->candidates[i].voteCount = 0;
            dm.saveCandidate(blockIndex, i, &node->candidates[i]);
        }

        if (!node->is_leaf && node->child_indices[node->key_count] > 0) {
            resetVotesRec(node->child_indices[node->key_count]);
        }

        delete node;
    }

    void findMaxVotesRec(int blockIndex, string& winnerID, string& winnerName, int& maxVotes) {
        if (blockIndex <= 0) return;
        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return;

        for (int i = 0; i < node->key_count; i++) {
            if (!node->is_leaf && node->child_indices[i] > 0) {
                findMaxVotesRec(node->child_indices[i], winnerID, winnerName, maxVotes);
            }

            if (node->candidates[i].voteCount > maxVotes) {
                maxVotes = node->candidates[i].voteCount;
                winnerID = node->candidates[i].candidateID;
                winnerName = node->candidates[i].name;
            }
        }

        if (!node->is_leaf && node->child_indices[node->key_count] > 0) {
            findMaxVotesRec(node->child_indices[node->key_count], winnerID, winnerName, maxVotes);
        }

        delete node;
    }

    pair<BTreeNode*, int> searchRec(BTreeNode* node, const string& id) {
        if (!node) return { nullptr, -1 };

        int i = 0;
        while (i < node->key_count && compareCandidateID(id, node->candidates[i].candidateID) > 0) {
            i++;
        }

        if (i < node->key_count && compareCandidateID(id, node->candidates[i].candidateID) == 0) {
            return { node, i };
        }

        if (node->is_leaf) return { nullptr, -1 };

        if (i < 0 || i > MAX_KEYS || node->child_indices[i] <= 0) {
            return { nullptr, -1 };
        }

        BTreeNode* child = dm.loadNode(node->child_indices[i]);
        if (!child) return { nullptr, -1 };

        auto result = searchRec(child, id);
        if (child != result.first) delete child;
        return result;
    }

public:
    CandidateBTree() : rootBlock(-1), nextCandidateID(1) {
        rootBlock = dm.getRoot();
        debugRootContents();

        if (rootBlock == -1) {
            // Create new tree
            BTreeNode* root = new BTreeNode(true);
            root->disk_index = dm.allocateBlock();
            if (root->disk_index != -1) {
                dm.saveNode(root);
                dm.setRoot(root->disk_index);
                rootBlock = root->disk_index;
                cout << "Created new B-tree with empty root at block: " << rootBlock << endl;
            }
            delete root;
        }
        else {
            // Load existing tree
            BTreeNode* root = dm.loadNode(rootBlock);
            if (root) {
                // Validate the node
                if (root->key_count >= 0 && root->key_count <= MAX_KEYS) {
                    cout << "Loaded existing B-tree from block: " << rootBlock
                        << " with " << root->key_count << " keys" << endl;
                }
                else {
                    // Corrupted data
                    cout << "WARNING: Corrupted B-tree detected (key_count="
                        << root->key_count << "). Creating new tree..." << endl;

                    // Create new empty tree
                    dm.setRoot(-1);
                    rootBlock = -1;

                    BTreeNode* newRoot = new BTreeNode(true);
                    newRoot->disk_index = dm.allocateBlock();
                    if (newRoot->disk_index != -1) {
                        dm.saveNode(newRoot);
                        dm.setRoot(newRoot->disk_index);
                        rootBlock = newRoot->disk_index;
                        cout << "Created new empty root at block: " << rootBlock << endl;
                    }
                    delete newRoot;
                }
                delete root;
            }
            else {
                cout << "Failed to load root node. Creating new tree..." << endl;
                // Create new empty tree
                BTreeNode* newRoot = new BTreeNode(true);
                newRoot->disk_index = dm.allocateBlock();
                if (newRoot->disk_index != -1) {
                    dm.saveNode(newRoot);
                    dm.setRoot(newRoot->disk_index);
                    rootBlock = newRoot->disk_index;
                    cout << "Created new root at block: " << rootBlock << endl;
                }
                delete newRoot;
            }
        }
    }

    bool verifyCandidatePassword(const string& id, const string& password) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return false;

        auto result = searchRec(root, id);
        bool verified = false;

        if (result.first && result.second != -1) {
            if (string(result.first->candidates[result.second].password) == password) {
                verified = true;
            }
        }

        if (root != result.first) delete root;
        if (result.first && result.first != root) delete result.first;
        return verified;
    }

    int searchCandidate(const string& id) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return -1;

        auto result = searchRec(root, id);
        int found = (result.first && result.second != -1) ? 1 : -1;

        if (root != result.first) delete root;
        if (result.first && result.first != root) delete result.first;

        return found;
    }

    bool registerCandidate(string name, string cnic, string partyName,
        string secretCode, string pollingStation, string password) {

        // Validate inputs
        if (name.empty() || cnic.empty() || partyName.empty() ||
            secretCode.empty() || pollingStation.empty() || password.empty()) {
            cout << "Error: All fields are required!\n";
            return false;
        }

        if (!verifyPartySecretCode(partyName, secretCode)) {
            cout << "Invalid party/secret code\n";
            return false;
        }

        if (searchByCNIC(cnic) != -1) {
            cout << "CNIC already registered\n";
            return false;
        }

        string candidateID = generateCandidateID();

        // Create candidate node
        CandidateNode newCandidate(candidateID, name, cnic, partyName, pollingStation, password);
        newCandidate.voteCount = 0;

        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) {
            cout << "Failed to load root node\n";
            return false;
        }

        if (root->key_count == MAX_KEYS) {
            // Root is full, need to split
            BTreeNode* newRoot = new BTreeNode(false);
            newRoot->disk_index = dm.allocateBlock();

            if (newRoot->disk_index == -1) {
                delete newRoot;
                delete root;
                cout << "Failed to allocate block for new root\n";
                return false;
            }

            // Set old root as first child of new root
            newRoot->child_indices[0] = root->disk_index;

            // Split the old root
            splitChild(newRoot, 0, root);

            // Save new root and update
            dm.saveNode(newRoot);
            dm.setRoot(newRoot->disk_index);
            rootBlock = newRoot->disk_index;

            // Determine which child to insert into
            int i = 0;
            while (i < newRoot->key_count &&
                compareCandidateID(candidateID, newRoot->candidates[i].candidateID) > 0) {
                i++;
            }

            BTreeNode* child = dm.loadNode(newRoot->child_indices[i]);
            if (child) {
                insertNonFull(child, &newCandidate);
                dm.saveNode(child);
                delete child;
            }

            delete newRoot;
        }
        else {
            // Root has space, insert directly
            insertNonFull(root, &newCandidate);
            dm.saveNode(root);
        }

        delete root;

        cout << "\nCandidate Registered Successfully!\n";
        cout << "ID: " << candidateID << "\n";
        cout << "Name: " << name << "\n";
        cout << "Party: " << partyName << "\n";
        cout << "Station: " << pollingStation << "\n";

        return true;
    }

    int searchByCNIC(string cnic) {
        // Use BFS to search by CNIC
        queue<int> q;
        q.push(rootBlock);

        while (!q.empty()) {
            int idx = q.front();
            q.pop();

            if (idx <= 0) continue;

            BTreeNode* node = dm.loadNode(idx);
            if (!node) continue;

            // Check keys in current node
            for (int i = 0; i < node->key_count; i++) {
                if (string(node->candidates[i].cnic) == cnic) {
                    delete node;
                    return 1;
                }
            }

            // Add children to queue if not leaf
            if (!node->is_leaf) {
                for (int i = 0; i <= node->key_count; i++) {
                    if (node->child_indices[i] > 0) {
                        q.push(node->child_indices[i]);
                    }
                }
            }

            delete node;
        }

        return -1;
    }

    CandidateNode* getCandidate(string id) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return nullptr;

        auto result = searchRec(root, id);
        CandidateNode* candidate = nullptr;

        if (result.first && result.second != -1) {
            candidate = new CandidateNode();
            *candidate = result.first->candidates[result.second];
        }

        if (root != result.first) delete root;
        if (result.first && result.first != root) delete result.first;

        return candidate;
    }

    void printCandidatesTable() {
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);

        if (allCandidates.empty()) {
            cout << "\nNo candidates registered yet.\n";
            return;
        }

        cout << "\n" << string(100, '-') << "\n";
        cout << left << setw(10) << "ID" << setw(25) << "Name"
            << setw(15) << "Party" << setw(15) << "Station" << setw(10) << "Votes" << "\n";
        cout << string(100, '-') << "\n";

        for (const auto& candidate : allCandidates) {
            // Convert char arrays to proper strings
            string id(candidate.candidateID, strnlen(candidate.candidateID, ID_SIZE));
            string name(candidate.name, strnlen(candidate.name, NAME_SIZE));
            string party(candidate.party, strnlen(candidate.party, PARTY_SIZE));
            string station(candidate.pollingStation, strnlen(candidate.pollingStation, STATION_SIZE));

            cout << left << setw(10) << id
                << setw(25) << name
                << setw(15) << party
                << setw(15) << station
                << setw(10) << candidate.voteCount << "\n";
        }

        cout << string(100, '-') << "\n";
        cout << "Total: " << allCandidates.size() << " candidates\n";
    }

    void printCandidatesByStation(const string& stationID) {
        vector<CandidateNode> candidates;
        inorderStationRec(rootBlock, stationID, candidates);

        cout << "\nCandidates contesting from Station: " << stationID << "\n";
        cout << string(80, '-') << "\n";

        if (candidates.empty()) {
            cout << "No candidates found.\n";
            return;
        }

        cout << left << setw(10) << "ID" << setw(25) << "Name"
            << setw(15) << "Party" << setw(10) << "Votes" << "\n";
        cout << string(80, '-') << "\n";

        for (const auto& c : candidates) {
            string id(c.candidateID, strnlen(c.candidateID, ID_SIZE));
            string name(c.name, strnlen(c.name, NAME_SIZE));
            string party(c.party, strnlen(c.party, PARTY_SIZE));

            cout << left << setw(10) << id
                << setw(25) << name
                << setw(15) << party
                << setw(10) << c.voteCount << "\n";
        }

        cout << string(80, '-') << "\n";
        cout << "Total: " << candidates.size() << " candidates\n";
    }

    void voteCandidate(string id) {
        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) {
            cout << "Error: Could not load root node\n";
            return;
        }

        auto result = searchRec(root, id);
        if (result.first && result.second != -1) {
            result.first->candidates[result.second].voteCount++;
            dm.saveCandidate(result.first->disk_index, result.second,
                &result.first->candidates[result.second]);
            cout << "Vote cast successfully for candidate: " << id << endl;
        }
        else {
            cout << "Candidate not found: " << id << endl;
        }

        if (root != result.first) delete root;
        if (result.first && result.first != root) delete result.first;
    }

    void resetAllVotes() {
        resetVotesRec(rootBlock);
        cout << "All votes have been reset to zero." << endl;
    }

    void printWinner() {
        string winnerID, winnerName;
        int maxVotes = -1;

        findMaxVotesRec(rootBlock, winnerID, winnerName, maxVotes);

        if (maxVotes <= 0) {
            cout << "No votes cast.\n";
        }
        else {
            cout << "Winner: " << winnerName << " (ID: " << winnerID
                << ") with " << maxVotes << " votes.\n";
        }
    }

    // Debug and utility methods
    void debugPrintTree() {
        cout << "\n=== B-TREE DEBUG INFO ===\n";
        cout << "Root block index: " << rootBlock << endl;

        if (rootBlock == -1) {
            cout << "Tree is empty\n";
            return;
        }

        queue<int> q;
        q.push(rootBlock);
        int level = 0;

        while (!q.empty()) {
            int size = static_cast<int>(q.size());
            cout << "\nLevel " << level++ << " (" << size << " nodes): \n";

            for (int i = 0; i < size; i++) {
                int idx = q.front();
                q.pop();

                if (idx <= 0) continue;

                BTreeNode* node = dm.loadNode(idx);
                if (!node) {
                    cout << "  [Failed to load node " << idx << "]\n";
                    continue;
                }

                cout << "  Node " << idx << ": keys=" << node->key_count
                    << ", leaf=" << (node->is_leaf ? "yes" : "no") << "\n";

                if (node->key_count > 0) {
                    cout << "    Keys: [";
                    for (int j = 0; j < node->key_count; j++) {
                        // Convert char array to string properly
                        string candidateID(node->candidates[j].candidateID,
                            strnlen(node->candidates[j].candidateID, ID_SIZE));
                        cout << "'" << candidateID << "'";
                        if (j < node->key_count - 1) cout << ", ";
                    }
                    cout << "]\n";
                }

                if (!node->is_leaf) {
                    cout << "    Children: [";
                    bool first = true;
                    for (int j = 0; j <= node->key_count; j++) {
                        if (node->child_indices[j] > 0) {
                            if (!first) cout << ", ";
                            cout << node->child_indices[j];
                            q.push(node->child_indices[j]);
                            first = false;
                        }
                    }
                    cout << "]\n";
                }
                delete node;
            }
        }
        cout << "\n=== END DEBUG INFO ===\n";
    }

    vector<CandidateNode> getAllCandidatesBruteForce() {
        vector<CandidateNode> allCandidates;

        for (int blockIdx = 1; blockIdx < MAX_BLOCKS; blockIdx++) {
            if (dm.isBitSet(blockIdx)) {
                BTreeNode* node = dm.loadNode(blockIdx);
                if (node) {
                    for (int i = 0; i < node->key_count; i++) {
                        allCandidates.push_back(node->candidates[i]);
                    }
                    delete node;
                }
            }
        }

        return allCandidates;
    }

    void printAllCandidatesBruteForce() {
        auto candidates = getAllCandidatesBruteForce();

        cout << "\n" << string(100, '-') << "\n";
        cout << left << setw(10) << "ID" << setw(25) << "Name"
            << setw(15) << "Party" << setw(15) << "Station" << setw(10) << "Votes" << "\n";
        cout << string(100, '-') << "\n";

        for (const auto& candidate : candidates) {
            string id(candidate.candidateID, strnlen(candidate.candidateID, ID_SIZE));
            string name(candidate.name, strnlen(candidate.name, NAME_SIZE));
            string party(candidate.party, strnlen(candidate.party, PARTY_SIZE));
            string station(candidate.pollingStation, strnlen(candidate.pollingStation, STATION_SIZE));

            cout << left << setw(10) << id
                << setw(25) << name
                << setw(15) << party
                << setw(15) << station
                << setw(10) << candidate.voteCount << "\n";
        }

        cout << string(100, '-') << "\n";
        cout << "Total: " << candidates.size() << " candidates\n";
    }

    // Simple method to check if tree is empty
    bool isEmpty() {
        if (rootBlock == -1) return true;

        BTreeNode* root = dm.loadNode(rootBlock);
        if (!root) return true;

        bool empty = (root->key_count == 0);
        delete root;
        return empty;
    }

    // Get total candidate count
    int getCandidateCount() {
        vector<CandidateNode> allCandidates;
        inorderDetailedRec(rootBlock, allCandidates);
        return static_cast<int>(allCandidates.size());
    }
};

#endif // CANDIDATEBTREE_H