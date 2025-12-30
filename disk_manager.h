#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <limits>
#include <iomanip>
#include <vector>
#include <queue>
#include "config.h"
#include <map>
using namespace std;

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
        if (!file.good()) {
            cerr << "ERROR: Failed to write superblock!\n";
        }
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
            cout << "Created new empty candidates.bin\n";
        }

        // Get file size
        file.seekg(0, ios::end);
        streampos size = file.tellg();
        cout << "DiskManager: File size = " << size << " bytes\n";
        file.seekg(0, ios::beg);

        cout << "DiskManager: BITMAP_SIZE = " << BITMAP_SIZE
            << ", METADATA_SIZE = " << METADATA_SIZE
            << ", sizeof(BTreeNode) = " << sizeof(BTreeNode) << endl;

        file.read(bitmap, BITMAP_SIZE);
        file.read((char*)&rootIndex, sizeof(rootIndex));
        fileOpen = true;

        cout << "DiskManager: Root index from file = " << rootIndex << endl;
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
        if (!fileOpen || node == nullptr || node->disk_index <= 0) {
            return;
        }

        char buffer[sizeof(BTreeNode)];
        serialize_node(node, buffer);

        long long position = METADATA_SIZE + (long long)(node->disk_index - 1) * sizeof(BTreeNode);
        file.seekp(position, ios::beg);

        if (!file.good()) {
            file.clear();
            file.seekp(position, ios::beg);
        }

        file.write(buffer, sizeof(BTreeNode));
        file.flush();

        cout << "Saved node " << node->disk_index << " at position " << position
            << ", key_count=" << node->key_count << endl;
    }

    BTreeNode* createEmptyNode(bool isLeaf) {
        BTreeNode* node = new BTreeNode(isLeaf);
        node->key_count = 0;
        node->disk_index = -1;
        return node;
    }

    BTreeNode* loadNode(int index) {
        if (!fileOpen || index <= 0 || index >= MAX_BLOCKS) {
            cout << "loadNode: Invalid index " << index << endl;
            return nullptr;
        }

        cout << "LOADING node " << index << "..." << endl;

        BTreeNode* node = new BTreeNode(true);
        char buffer[4096];

        long long position = METADATA_SIZE + (long long)(index - 1) * sizeof(BTreeNode);
        cout << "  Position = " << position << endl;

        file.seekg(position, ios::beg);
        if (!file.good()) {
            cout << "  ERROR: Seek failed!" << endl;
            delete node;
            return nullptr;
        }

        file.read(buffer, sizeof(BTreeNode));
        streamsize bytesRead = file.gcount();
        cout << "  Read " << bytesRead << " bytes (expected " << sizeof(BTreeNode) << ")" << endl;

        if (bytesRead != sizeof(BTreeNode)) {
            cout << "  ERROR: Incomplete read!" << endl;
            delete node;
            return nullptr;
        }

        deserialize_node(node, buffer);

        cout << "  Loaded: key_count=" << node->key_count
            << ", is_leaf=" << node->is_leaf
            << ", disk_index=" << node->disk_index << endl;

        if (node->key_count < 0 || node->key_count > MAX_KEYS) {
            cout << "  ERROR: Corrupted key_count=" << node->key_count << endl;
            delete node;
            return nullptr;
        }

        node->disk_index = index;
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

#endif // DISKMANAGER_H