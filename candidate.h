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
#include <algorithm> 
#include <map>
using namespace std;

class CandidateBTree {
private:
    DiskManager dm;
    int rootBlock;


    // Fix the splitChild function (around line 85)
    void splitChild(BTreeNode* parent, int i, BTreeNode* child) {
        BTreeNode* newChild = new BTreeNode(child->is_leaf);
        newChild->disk_index = dm.allocateBlock();

        if (newChild->disk_index == -1) {
            delete newChild;
            return;
        }

        // FIX: MIN_KEYS should be (MIN_DEGREE - 1)
        int t = MIN_DEGREE;
        newChild->key_count = t - 1;

        // Copy the last (t-1) keys from child to newChild
        for (int j = 0; j < t - 1; j++) {
            newChild->candidates[j] = child->candidates[j + t];
        }

        // If not leaf, copy the last t children
        if (!child->is_leaf) {
            for (int j = 0; j < t; j++) {
                newChild->child_indices[j] = child->child_indices[j + t];
            }
        }

        // Reduce child's key count
        child->key_count = t - 1;

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
        parent->candidates[i] = child->candidates[t - 1];
        parent->key_count++;

        // Clear the moved key from child
        // This is important for consistency
        memset(&child->candidates[t - 1], 0, sizeof(CandidateNode));

        // Save all modified nodes
        dm.saveNode(child);
        dm.saveNode(newChild);
        dm.saveNode(parent);

        // Also save the new child's disk_index in parent's children array
        dm.saveNode(parent);

        delete newChild;
    }

    // Fix the getAllCandidatesRec function - add better debugging
    void getAllCandidatesRec(int blockIndex, vector<CandidateNode>& result) {
        if (blockIndex <= 0) return;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) {
            cout << "ERROR: Failed to load node at block " << blockIndex << endl;
            return;
        }

        // For each key in the node
        for (int i = 0; i < node->key_count; i++) {
            // Traverse left child if exists and not leaf
            if (!node->is_leaf && node->child_indices[i] > 0) {
                getAllCandidatesRec(node->child_indices[i], result);
            }

            // Add current candidate
            CandidateNode copy;
            memset(&copy, 0, sizeof(CandidateNode));

            // Copy all fields including town - GCC COMPATIBLE VERSION
            strncpy(copy.candidateID, node->candidates[i].candidateID, ID_SIZE - 1);
            copy.candidateID[ID_SIZE - 1] = '\0';

            strncpy(copy.name, node->candidates[i].name, NAME_SIZE - 1);
            copy.name[NAME_SIZE - 1] = '\0';

            strncpy(copy.cnic, node->candidates[i].cnic, CNIC_SIZE - 1);
            copy.cnic[CNIC_SIZE - 1] = '\0';

            strncpy(copy.party, node->candidates[i].party, PARTY_SIZE - 1);
            copy.party[PARTY_SIZE - 1] = '\0';

            // ADDED: Copy town field
            strncpy(copy.town, node->candidates[i].town, TOWN_SIZE - 1);
            copy.town[TOWN_SIZE - 1] = '\0';

            strncpy(copy.pollingStation, node->candidates[i].pollingStation, STATION_SIZE - 1);
            copy.pollingStation[STATION_SIZE - 1] = '\0';

            strncpy(copy.password, node->candidates[i].password, PASS_SIZE - 1);
            copy.password[PASS_SIZE - 1] = '\0';

            copy.voteCount = node->candidates[i].voteCount;

            result.push_back(copy);
        }

        // Traverse last child if exists and not leaf
        if (!node->is_leaf && node->child_indices[node->key_count] > 0) {
            getAllCandidatesRec(node->child_indices[node->key_count], result);
        }

        delete node;
    }

    // Fix the insertNonFull function to handle child indices properly
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
                cout << "ERROR: Invalid index " << i << " in insertNonFull\n";
                return;
            }

            if (node->child_indices[i] <= 0) {
                cout << "ERROR: Invalid child index at position " << i << endl;
                return;
            }

            BTreeNode* child = dm.loadNode(node->child_indices[i]);
            if (!child) {
                cout << "ERROR: Failed to load child at index " << node->child_indices[i] << endl;
                return;
            }

            if (child->key_count == MAX_KEYS) {
                splitChild(node, i, child);

                // After split, determine which child to insert into
                if (compareCandidateID(candidate->candidateID, node->candidates[i].candidateID) > 0) {
                    i++;
                }

                // Reload the correct child
                delete child;
                child = dm.loadNode(node->child_indices[i]);
                if (!child) {
                    cout << "ERROR: Failed to reload child after split\n";
                    return;
                }
            }

            insertNonFull(child, candidate);
            delete child;
        }
    }

    // =============================================
    // NEW HELPER METHODS FOR TOWN-BASED ASSIGNMENT
    // =============================================

    // Check if a party already has a candidate in specific polling station
    bool isPartyInStation(const string& partyName, const string& stationID) {
        vector<CandidateNode> allCandidates;
        getAllCandidatesRec(rootBlock, allCandidates);

        for (const auto& candidate : allCandidates) {
            if (string(candidate.party) == partyName &&
                string(candidate.pollingStation) == stationID) {
                return true;
            }
        }
        return false;
    }

    // Find available polling station for a party in a town
    string findAvailableStation(const string& partyName, const string& town) {
        vector<string> stations = GeographicConfig::getStationsForTown(town);

        if (stations.empty()) {
            cout << "Error: No polling stations found for town: " << town << endl;
            return "";
        }

        cout << "\nChecking availability in " << town << " stations: ";
        for (const string& station : stations) {
            cout << station << " ";
        }
        cout << endl;

        for (const string& station : stations) {
            if (!isPartyInStation(partyName, station)) {
                cout << "  ? Station " << station << " is available for " << partyName << endl;
                return station; // First available station
            }
            cout << "  ? Station " << station << " already has a candidate from " << partyName << endl;
        }

        return ""; // No available station
    }

    int compareCandidateID(const string& id1, const string& id2) {
        return id1.compare(id2);
    }

    string generateCandidateID() {
        vector<CandidateNode> allCandidates;
        getAllCandidatesRec(rootBlock, allCandidates);

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

    // NEW: Get candidates filtered by station from entire B-tree
    void getCandidatesByStationRec(int blockIndex, const string& stationID, vector<CandidateNode>& result) {
        if (blockIndex <= 0) return;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) {
            cout << "ERROR: Failed to load node at block " << blockIndex << " for station search" << endl;
            return;
        }

        for (int i = 0; i < node->key_count; i++) {
            // Traverse left child first
            if (!node->is_leaf && node->child_indices[i] > 0) {
                getCandidatesByStationRec(node->child_indices[i], stationID, result);
            }

            // Check if candidate matches station filter
            if (stationID.empty() || string(node->candidates[i].pollingStation) == stationID) {
                CandidateNode copy;
                memset(&copy, 0, sizeof(CandidateNode));

                // GCC COMPATIBLE VERSION
                strncpy(copy.candidateID, node->candidates[i].candidateID, ID_SIZE - 1);
                copy.candidateID[ID_SIZE - 1] = '\0';

                strncpy(copy.name, node->candidates[i].name, NAME_SIZE - 1);
                copy.name[NAME_SIZE - 1] = '\0';

                strncpy(copy.party, node->candidates[i].party, PARTY_SIZE - 1);
                copy.party[PARTY_SIZE - 1] = '\0';

                // ADDED: Copy town field
                strncpy(copy.town, node->candidates[i].town, TOWN_SIZE - 1);
                copy.town[TOWN_SIZE - 1] = '\0';

                strncpy(copy.pollingStation, node->candidates[i].pollingStation, STATION_SIZE - 1);
                copy.pollingStation[STATION_SIZE - 1] = '\0';

                copy.voteCount = node->candidates[i].voteCount;
                result.push_back(copy);
            }
        }

        // Traverse last child
        if (!node->is_leaf && node->child_indices[node->key_count] > 0) {
            getCandidatesByStationRec(node->child_indices[node->key_count], stationID, result);
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
    CandidateBTree() : rootBlock(-1) {
        rootBlock = dm.getRoot();

        if (rootBlock == -1) {
            // Create new tree
            BTreeNode* root = new BTreeNode(true);
            root->disk_index = dm.allocateBlock();
            if (root->disk_index != -1) {
                dm.saveNode(root);
                dm.setRoot(root->disk_index);
                rootBlock = root->disk_index;
            }
            delete root;
        }
        else {
            // Load existing tree
            BTreeNode* root = dm.loadNode(rootBlock);
            if (root) {
                // Validate the node
                if (root->key_count >= 0 && root->key_count <= MAX_KEYS) {
                    // Test traversal to see how many candidates we can load
                    vector<CandidateNode> allCandidates;
                    getAllCandidatesRec(rootBlock, allCandidates);
                }
                else {
                    // Corrupted data
                    // Create new empty tree
                    dm.setRoot(-1);
                    rootBlock = -1;

                    BTreeNode* newRoot = new BTreeNode(true);
                    newRoot->disk_index = dm.allocateBlock();
                    if (newRoot->disk_index != -1) {
                        dm.saveNode(newRoot);
                        dm.setRoot(newRoot->disk_index);
                        rootBlock = newRoot->disk_index;
                    }
                    delete newRoot;
                }
                delete root;
            }
            else {
                // Create new empty tree
                BTreeNode* newRoot = new BTreeNode(true);
                newRoot->disk_index = dm.allocateBlock();
                if (newRoot->disk_index != -1) {
                    dm.saveNode(newRoot);
                    dm.setRoot(newRoot->disk_index);
                    rootBlock = newRoot->disk_index;
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

    // MODIFIED: Now accepts town instead of pollingStation
    bool registerCandidate(string name, string cnic, string partyName,
        string secretCode, string town, string password) {

        // Validate inputs
        if (name.empty() || cnic.empty() || partyName.empty() ||
            secretCode.empty() || town.empty() || password.empty()) {
            cout << "Error: All fields are required!\n";
            return false;
        }

        // Validate town exists
        if (!GeographicConfig::isValidTown(town)) {
            cout << "Error: Invalid town! Available towns are:\n";
            vector<string> towns = GeographicConfig::getAllTowns();
            for (const auto& t : towns) {
                cout << "  - " << t << "\n";
            }
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

        // Find available polling station based on party and town
        string pollingStation = findAvailableStation(partyName, town);

        if (pollingStation.empty()) {
            cout << "\n" << string(60, '=') << "\n";
            cout << "  REGISTRATION REJECTED!\n";
            cout << string(60, '=') << "\n";
            cout << "All polling stations in " << town << " already have a candidate from " << partyName << ".\n";
            cout << "Better luck next time!\n";
            cout << string(60, '=') << "\n";
            return false;
        }

        string candidateID = generateCandidateID();

        // Create candidate node with assigned polling station
        CandidateNode newCandidate(candidateID, name, cnic, partyName, town, pollingStation, password);
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

        cout << "\n" << string(60, '=') << "\n";
        cout << "  CANDIDATE REGISTERED SUCCESSFULLY!\n";
        cout << string(60, '=') << "\n";
        cout << "ID: " << candidateID << "\n";
        cout << "Name: " << name << "\n";
        cout << "Party: " << partyName << "\n";
        cout << "Town: " << town << "\n";
        cout << "Assigned Polling Station: " << pollingStation << "\n";
        cout << string(60, '=') << "\n";

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
        getAllCandidatesRec(rootBlock, allCandidates);

        if (allCandidates.empty()) {
            cout << "\nNo candidates registered yet.\n";
            return;
        }

        cout << "\n" << string(120, '-') << "\n";
        cout << left << setw(10) << "ID" << setw(25) << "Name"
            << setw(15) << "Party" << setw(15) << "Town" << setw(15) << "Station" << setw(10) << "Votes" << "\n";
        cout << string(120, '-') << "\n";

        // Sort candidates by ID for better display
        sort(allCandidates.begin(), allCandidates.end(),
            [](const CandidateNode& a, const CandidateNode& b) {
                return string(a.candidateID) < string(b.candidateID);
            });

        for (const auto& candidate : allCandidates) {
            // Convert char arrays to proper strings - GCC COMPATIBLE VERSION
            string id(candidate.candidateID);
            string name(candidate.name);
            string party(candidate.party);
            string town(candidate.town);
            string station(candidate.pollingStation);

            cout << left << setw(10) << id
                << setw(25) << name
                << setw(15) << party
                << setw(15) << town
                << setw(15) << station
                << setw(10) << candidate.voteCount << "\n";
        }

        cout << string(120, '-') << "\n";
        cout << "Total: " << allCandidates.size() << " candidates\n";
    }

    // NEW: Get all candidates from specific station (for voters to view)
    vector<CandidateNode> getCandidatesByStation(const string& stationID) {
        vector<CandidateNode> candidates;
        getCandidatesByStationRec(rootBlock, stationID, candidates);
        return candidates;
    }

    void printCandidatesByStation(const string& stationID) {
        vector<CandidateNode> candidates = getCandidatesByStation(stationID);

        cout << "\nCandidates contesting from Station: " << stationID << "\n";
        cout << string(100, '-') << "\n";

        if (candidates.empty()) {
            cout << "No candidates found.\n";
            return;
        }

        // Sort by votes descending
        sort(candidates.begin(), candidates.end(),
            [](const CandidateNode& a, const CandidateNode& b) {
                return a.voteCount > b.voteCount;
            });

        cout << left << setw(10) << "ID" << setw(25) << "Name"
            << setw(15) << "Party" << setw(15) << "Town" << setw(10) << "Votes" << "\n";
        cout << string(100, '-') << "\n";

        for (const auto& c : candidates) {
            // GCC COMPATIBLE VERSION
            string id(c.candidateID);
            string name(c.name);
            string party(c.party);
            string town(c.town);

            cout << left << setw(10) << id
                << setw(25) << name
                << setw(15) << party
                << setw(15) << town
                << setw(10) << c.voteCount << "\n";
        }

        cout << string(100, '-') << "\n";
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

    //// Get all candidates (public version) - traverses entire B-tree
    vector<CandidateNode> getAllCandidates() {
        vector<CandidateNode> allCandidates;
        getAllCandidatesRec(rootBlock, allCandidates);
        return allCandidates;
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
        getAllCandidatesRec(rootBlock, allCandidates);
        return static_cast<int>(allCandidates.size());
    }

    // NEW: Get candidates by town
    vector<CandidateNode> getCandidatesByTown(const string& townName) {
        vector<CandidateNode> allCandidates;
        getAllCandidatesRec(rootBlock, allCandidates);

        vector<CandidateNode> townCandidates;
        for (const auto& candidate : allCandidates) {
            if (string(candidate.town) == townName) {
                townCandidates.push_back(candidate);
            }
        }
        return townCandidates;
    }

    // NEW: Check station availability for a party in a town
    bool checkStationAvailability(const string& partyName, const string& town) {
        string availableStation = findAvailableStation(partyName, town);
        return !availableStation.empty();
    }

    // NEW: Get available stations for a party in a town
    vector<string> getAvailableStations(const string& partyName, const string& town) {
        vector<string> stations = GeographicConfig::getStationsForTown(town);
        vector<string> availableStations;

        for (const string& station : stations) {
            if (!isPartyInStation(partyName, station)) {
                availableStations.push_back(station);
            }
        }

        return availableStations;
    }

    // NEW: Print party-station allocations for a town
    void printPartyAllocationsByTown(const string& town) {
        vector<CandidateNode> allCandidates;
        getAllCandidatesRec(rootBlock, allCandidates);

        cout << "\n" << string(60, '=') << "\n";
        cout << "PARTY ALLOCATIONS IN " << town << "\n";
        cout << string(60, '=') << "\n";

        // Group by polling station
        map<string, vector<string>> stationParties;

        for (const auto& candidate : allCandidates) {
            if (string(candidate.town) == town) {
                string station = string(candidate.pollingStation);
                string party = string(candidate.party);
                stationParties[station].push_back(party);
            }
        }

        if (stationParties.empty()) {
            cout << "No candidates registered in " << town << "\n";
            return;
        }

        for (const auto& entry : stationParties) {
            cout << "Station " << entry.first << ":\n";
            for (const auto& party : entry.second) {
                cout << "  - " << party << "\n";
            }
            cout << string(40, '-') << "\n";
        }

        vector<string> stations = GeographicConfig::getStationsForTown(town);
        cout << "\nTotal Stations in " << town << ": " << stations.size() << "\n";
        cout << "Stations with candidates: " << stationParties.size() << "\n";
        cout << string(60, '=') << "\n";
    }

    int countAllCandidatesRecursive(int blockIndex) {
        if (blockIndex <= 0) return 0;

        BTreeNode* node = dm.loadNode(blockIndex);
        if (!node) return 0;

        int count = node->key_count;

        if (!node->is_leaf) {
            for (int i = 0; i <= node->key_count; i++) {
                if (node->child_indices[i] > 0) {
                    count += countAllCandidatesRecursive(node->child_indices[i]);
                }
            }
        }

        delete node;
        return count;
    }

};

#endif // CANDIDATEBTREE_H