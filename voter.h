#ifndef VOTER_H
#define VOTER_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include"config.h"
using namespace std;

// VOTERHASHTABLE CLASS - MAIN STORAGE & OPERATIONS

class VoterHashTable {
private:
    int tableSize;
    vector<Voter*>* table;
    const char* FILENAME = "voters.bin";

    vector<string> getStationsForTown(string town) {
        return GeographicConfig::getStationsForTown(town);
    }

    // HASH FUNCTION - POLYNOMIAL ROLLING HASH
    int hashFunc(const string& id) {
        int hash = 0;
        for (char c : id) 
            hash = (hash * 31 + c) % tableSize;
        return hash;
    }

    // SIMPLE HASH FOR STATION ASSIGNMENT
    int simpleHash(const string& str) {
        int hash = 0;
        for (size_t i = 0; i < str.length(); i++) 
            hash = (hash * 31 + str[i]) % 1000000;
        if (hash < 0) 
            hash = -hash;
        return hash;
    }

    string toLower(string str) {
        for (char& c : str) c = tolower(c);
        return str;
    }

    // AUTO-ASSIGN POLLING STATION BASED ON TOWN & CNIC
    string assignPollingStation(string town, string cnic) {
        vector<string> stations = getStationsForTown(town);
        int hashValue = simpleHash(cnic);
        int stationIndex = hashValue % stations.size();
        return stations[stationIndex];
    }

    // GENERATE SEQUENTIAL VOTER ID (VID00001 FORMAT)
    string generateVoterID() {
        int maxID = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->voterID.length() >= 3 && v->voterID.substr(0, 3) == "VID") {
                    string numPart = v->voterID.substr(3);
                    try {
                        int currentID = stoi(numPart);
                        if (currentID > maxID) maxID = currentID;
                    }
                    catch (...) {}
                }
            }
        }
        stringstream ss;
        ss << "VID" << setw(7) << setfill('0') << (maxID + 1);  // 3 + 7 = 10 chars
        return ss.str();
    }

    // VALIDATION FUNCTIONS
    bool validateCNIC(const string& cnic) {
        if (cnic.length() != 13) return false;
        for (char c : cnic) if (!isdigit(c)) return false;
        return true;
    }

    bool validateContact(const string& contact) {
        if (contact.length() != 11) return false;
        if (contact.substr(0, 2) != "03") return false;
        for (char c : contact) if (!isdigit(c)) return false;
        return true;
    }

    bool validatePassword(const string& password) {
        return password.length() >= 6;
    }

    char validateGenderInput(const string& genderInput) {
        if (genderInput.length() == 0) return 'O';
        char genderChar = toupper(genderInput[0]);
        if (genderChar == 'M' || genderChar == 'F' || genderChar == 'O') return genderChar;
        return 'O';
    }

    // FILE PERSISTENCE - APPEND SINGLE VOTER
    void saveVoterToFile(Voter* v) {
        ofstream out(FILENAME, ios::app | ios::binary);
        if (!out) return;
        auto writeString = [&out](const string& str) {
            size_t len = str.size();
            out.write((char*)&len, sizeof(len));
            out.write(str.c_str(), len);
            };
        writeString(v->voterID); writeString(v->name); writeString(v->cnic);
        out.write((char*)&v->gender, sizeof(v->gender));
        writeString(v->contactNumber); writeString(v->town);
        writeString(v->pollingStation); writeString(v->password);
        out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
        out.close();
    }

    // LOAD ALL VOTERS FROM BINARY FILE
    void loadFromFile() {
        ifstream in(FILENAME, ios::binary);
        if (!in) {
            cout << "No voter data file found. Starting fresh.\n";
            return;
        }
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) delete v;
            table[i].clear();
        }
        cout << "Loading voter data from file...\n";
        int loadedCount = 0;
        while (true) {
            Voter* v = new Voter();
            size_t len;
            in.read((char*)&len, sizeof(len));
            if (in.eof() || !in.good()) { delete v; break; }
            if (len > 1000) { delete v; cout << "Error: Invalid voterID length\n"; break; }
            v->voterID.resize(len); in.read(&v->voterID[0], len);
            in.read((char*)&len, sizeof(len)); v->name.resize(len); in.read(&v->name[0], len);
            in.read((char*)&len, sizeof(len)); v->cnic.resize(len); in.read(&v->cnic[0], len);
            in.read(&v->gender, sizeof(char));
            in.read((char*)&len, sizeof(len)); v->contactNumber.resize(len); in.read(&v->contactNumber[0], len);
            in.read((char*)&len, sizeof(len)); v->town.resize(len); in.read(&v->town[0], len);
            in.read((char*)&len, sizeof(len)); v->pollingStation.resize(len); in.read(&v->pollingStation[0], len);
            in.read((char*)&len, sizeof(len)); v->password.resize(len); in.read(&v->password[0], len);
            in.read((char*)&v->hasVoted, sizeof(bool));
            if (!in.good()) { delete v; if (in.eof()) break; break; }
            int index = hashFunc(v->voterID);
            table[index].push_back(v);
            loadedCount++;
        }
        in.close();
        cout << "Loaded " << loadedCount << " voters from file.\n";
    }

    // SAVE ALL VOTERS TO FILE (OVERWRITE)
    void saveAllToFile() {
        ofstream out(FILENAME, ios::out | ios::binary | ios::trunc);
        if (!out) {
            cout << "Error: Cannot open voter data file for writing!\n";
            return;
        }
        int savedCount = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                auto writeString = [&out](const string& str) {
                    size_t len = str.size();
                    out.write((char*)&len, sizeof(len));
                    out.write(str.c_str(), len);
                    };
                writeString(v->voterID); writeString(v->name); writeString(v->cnic);
                out.write((char*)&v->gender, sizeof(v->gender));
                writeString(v->contactNumber); writeString(v->town);
                writeString(v->pollingStation); writeString(v->password);
                out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
                savedCount++;
            }
        }
        out.close();
        if (!out.good()) cout << "Warning: There might have been an error writing voter data!\n";
        else cout << "Saved " << savedCount << " voters to file.\n";
    }

    // UTILITY FUNCTIONS
    bool existsInVector(const vector<string>& vec, const string& str) {
        for (const string& s : vec) if (s == str) return true;
        return false;
    }

    void validateAllVoterIDs() {
        cout << "Validating Voter IDs...\n";
        int invalidCount = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->voterID.empty() || v->voterID.substr(0, 3) != "VID" || v->voterID.length() != 10) {
                    cout << "Warning: Invalid Voter ID found: '" << v->voterID
                        << "' for voter: '" << v->name << "'" << endl;
                    invalidCount++;
                }
            }
        }
        if (invalidCount == 0) cout << "All Voter IDs are valid.\n";
    }


    void cleanCorruptedEntries() {
        int removed = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto it = table[i].begin(); it != table[i].end(); ) {
                Voter* v = *it;
                bool isCorrupted = false;
                if (v->voterID.empty()) {  
                    isCorrupted = true;
                }
                else if (v->gender != 'M' && v->gender != 'F' && v->gender != 'O') v->gender = 'O';
                if (isCorrupted) {
                    cout << "Removing corrupted entry: " << v->voterID << endl;
                    delete v; it = table[i].erase(it); removed++;
                }
                else ++it;
            }
        }
        if (removed > 0) cout << "Removed " << removed << " corrupted entries.\n";
    }

public:
    // CONSTRUCTOR - INITIALIZE HASH TABLE & LOAD DATA
    VoterHashTable(int size) {
        tableSize = size;
        table = new vector<Voter*>[tableSize];
        loadFromFile();
        cleanCorruptedEntries();
        validateAllVoterIDs();
    }

    // DESTRUCTOR - SAVE DATA & CLEAN MEMORY
    ~VoterHashTable() {
        cout << "Saving voter data to file...\n";
        saveAllToFile();
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) delete v;
            table[i].clear();
        }
        delete[] table;
    }

    // ====================================================
    // PUBLIC INTERFACE - VOTER MANAGEMENT
    // ====================================================

    // REGISTER NEW VOTER WITH VALIDATION
    void insertVoter(string name, string cnic, string genderInput,string contact, string town, string password) {
        if (searchByCNIC(cnic) != nullptr) { 
            cout << "Error: CNIC '" << cnic << "' is already registered!\n"; 
            return; 
        }
        if (!validateCNIC(cnic)) { 
            cout << "Error: Invalid CNIC! Must be 13 digits.\n"; 
            return; 
        }
        if (!validateContact(contact)) 
        { 
            cout << "Error: Invalid contact number! Must be 11 digits starting with 03.\n"; 
            return; 
        }
        if (!validatePassword(password)) 
        { cout << "Error: Password must be at least 6 characters long.\n"; 
        return; 
        }
        char gender = validateGenderInput(genderInput);
        if (gender == 'O' && (toupper(genderInput[0]) != 'O')) 
            cout << "Warning: Gender set to 'Other' (valid: M/m, F/f, O/o)\n";
        string station = assignPollingStation(town, cnic);
        string voterID = generateVoterID();
        if (searchVoter(voterID) != nullptr) { 
            cout << "Error: Voter ID collision detected! Please try again.\n"; 
            return; }
        int index = hashFunc(voterID);
        Voter* v = new Voter(voterID, name, cnic, gender, contact, town, station, password, false);
        table[index].push_back(v);
        saveVoterToFile(v);
        cout << "\n========================================\n";
        cout << "  VOTER REGISTERED SUCCESSFULLY!\n";
        cout << "========================================\n";
        cout << "Voter ID:" << voterID << "\n";
        cout << "Name:" << name << "\n";
        cout << "CNIC:" << cnic << "\n";
        cout << "Polling Station:" << station << "\n";
        cout << "========================================\n";
    }

    // SEARCH BY VOTER ID
    Voter* searchVoter(string id) {
        int index = hashFunc(id);
        for (auto v : table[index]) 
            if (v->voterID == id) 
                return v;
        return nullptr;
    }

    // SEARCH BY CNIC (LINEAR SEARCH)
    Voter* searchByCNIC(string cnic) {
        for (int i = 0; i < tableSize; i++)
            for (auto v : 
                table[i]) if (v->cnic == cnic) 
                return v;
        return nullptr;
    }

    // SEARCH FOR LOGIN (BY VOTER ID OR CNIC)
    Voter* searchForLogin(string identifier) {
        Voter* v = searchVoter(identifier);
        if (v) 
            return v;
        return searchByCNIC(identifier);
    }

    // DISPLAY ALL VOTERS IN DETAILED FORMAT
    void printTable() {
        cout << "\n" << string(80, '=') << "\n";
        cout << "                       ALL REGISTERED VOTERS\n";
        cout << string(80, '=') << "\n";
        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                count++;
                cout << "Voter #" << count << "\n" << string(40, '-') << "\n";
                cout << "Voter ID: " << v->voterID << "\n";
                cout << "Name: " << v->name << "\n";
                cout << "CNIC: " << v->cnic << "\n";
                cout << "Password: " << v->password << "\n";
                cout << "Gender: ";
                switch (toupper(v->gender)) {
                case 'M': cout << "Male\n"; break;
                case 'F': cout << "Female\n"; break;
                case 'O': cout << "Other\n"; break;
                default: cout << "Unknown\n";
                }
                cout << "Contact: " << v->contactNumber << "\n";
                cout << "Town: " << v->town << "\n";
                cout << "Polling Station: " << v->pollingStation << "\n";
                cout << "Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
                cout << string(40, '-') << "\n\n";
            }
        }
        if (count == 0) cout << "No voters registered yet.\n";
        cout << string(80, '=') << "\n";
    }

    // LIST VOTERS IN SPECIFIC POLLING STATION
    void printVotersByStation(string stationID) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "Voters in Polling Station: " << stationID << "\n";
        cout << string(60, '=') << "\n";
        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->pollingStation == stationID) {
                    count++;
                    cout << count << ". " << v->name << " (" << v->voterID << ")\n";
                    cout << "   CNIC: " << v->cnic << " | Contact: " << v->contactNumber << "\n";
                    cout << "   Gender: ";
                    switch (toupper(v->gender)) {
                    case 'M': cout << "Male"; break;
                    case 'F': cout << "Female"; break;
                    case 'O': cout << "Other"; break;
                    default: cout << "Unknown";
                    }
                    cout << " | Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
                    cout << string(30, '-') << "\n";
                }
            }
        }
        if (count == 0) cout << "No voters assigned to this polling station.\n";
        cout << "Total: " << count << " voters\n";
        cout << string(60, '=') << "\n";
    }

    // LIST VOTERS FROM SPECIFIC TOWN
    void printVotersByTown(string town) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "Voters in Town: " << town << "\n";
        string townLower = toLower(town);
        cout << string(60, '=') << "\n";
        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (toLower(v->town) == townLower) {
                    count++;
                    cout << count << ". " << v->name << " (" << v->voterID << ")\n";
                    cout << "   Station: " << v->pollingStation << " | CNIC: " << v->cnic << "\n";
                    cout << "   Gender: ";
                    switch (toupper(v->gender)) {
                    case 'M': cout << "Male"; break;
                    case 'F': cout << "Female"; break;
                    case 'O': cout << "Other"; break;
                    default: cout << "Unknown";
                    }
                    cout << " | Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
                    cout << string(30, '-') << "\n";
                }
            }
        }
        if (count == 0) cout << "No voters from this town.\n";
        cout << "Total: " << count << " voters\n";
        cout << string(60, '=') << "\n";
    }

    // GET LIST OF ALL UNIQUE POLLING STATIONS
    vector<string> getAllPollingStations() {
        vector<string> stations;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (!existsInVector(stations, v->pollingStation)) stations.push_back(v->pollingStation);
            }
        }
        return stations;
    }

    // GET LIST OF ALL UNIQUE TOWNS
    vector<string> getAllTowns() {
        vector<string> towns;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (!existsInVector(towns, v->town)) towns.push_back(v->town);
            }
        }
        return towns;
    }

    // DISPLAY TOWN-WISE VOTER STATISTICS (TOTAL, VOTED, TURNOUT)
    void printTownStatistics() {
        vector<string> townNames;
        vector<int> townTotal;
        vector<int> townVoted;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                string town = v->town;
                int townIndex = -1;
                for (size_t j = 0; j < townNames.size(); j++) {
                    if (townNames[j] == town) { townIndex = j; break; }
                }
                if (townIndex == -1) {
                    townNames.push_back(town); townTotal.push_back(0); townVoted.push_back(0);
                    townIndex = townNames.size() - 1;
                }
                townTotal[townIndex]++;
                if (v->hasVoted) townVoted[townIndex]++;
            }
        }
        cout << "\n" << string(70, '=') << "\n";
        cout << "                VOTER STATISTICS BY TOWN\n";
        cout << string(70, '=') << "\n";
        cout << left << setw(15) << "TOWN" << setw(10) << "TOTAL" << setw(10) << "VOTED" << setw(12) << "REMAINING" << setw(15) << "TURNOUT %" << "\n";
        cout << string(70, '-') << "\n";
        for (size_t i = 0; i < townNames.size(); i++) {
            string town = townNames[i];
            int total = townTotal[i];
            int voted = townVoted[i];
            int remaining = total - voted;
            float turnout = total > 0 ? (float(voted) / total * 100) : 0;
            cout << left << setw(15) << town << setw(10) << total << setw(10) << voted << setw(12) << remaining << fixed << setprecision(1) << setw(15) << turnout << "%\n";
        }
        cout << string(70, '=') << "\n";
    }

    // GET TOTAL NUMBER OF REGISTERED VOTERS
    int getTotalVoters() {
        int count = 0;
        for (int i = 0; i < tableSize; i++) count += table[i].size();
        return count;
    }

    // COUNT HOW MANY VOTERS HAVE VOTED
    int getVotedCount() {
        int count = 0;
        for (int i = 0; i < tableSize; i++)
            for (auto v : table[i]) if (v->hasVoted) count++;
        return count;
    }

    // COUNT HOW MANY VOTERS HAVEN'T VOTED
    int getNotVotedCount() {
        return getTotalVoters() - getVotedCount();
    }

    // MARK VOTER AS VOTED (ONE-TIME VOTING ENFORCEMENT)
    bool markAsVoted(string voterID) {
        Voter* v = searchVoter(voterID);
        if (v && !v->hasVoted) {
            v->hasVoted = true;
            saveAllToFile();
            return true;
        }
        return false;
    }

    // RESET ALL VOTE FLAGS (ADMIN FUNCTION)
    void resetAllVotes() {
        for (int i = 0; i < tableSize; i++)
            for (auto v : table[i]) v->hasVoted = false;
        saveAllToFile();
        cout << "All votes reset successfully!\n";
    }

    // GET VOTER IDS IN SPECIFIC POLLING STATION
    vector<string> getVotersInStation(string stationID) {
        vector<string> voterIDs;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->pollingStation == stationID) voterIDs.push_back(v->voterID);
            }
        }
        return voterIDs;
    }

    // GET POLLING STATION OF SPECIFIC VOTER
    string getPollingStation(string voterID) {
        Voter* v = searchVoter(voterID);
        return v ? v->pollingStation : "";
    }

    // GET TOWN OF SPECIFIC VOTER
    string getVoterTown(string voterID) {
        Voter* v = searchVoter(voterID);
        return v ? v->town : "";
    }

    // UPDATE VOTER'S PASSWORD
    bool updatePassword(string voterID, string newPassword) {
        Voter* v = searchVoter(voterID);
        if (v && validatePassword(newPassword)) {
            v->password = newPassword;
            saveAllToFile();
            return true;
        }
        return false;
    }

    // CONVERT GENDER CHAR TO DISPLAY STRING
    string getGenderString(char gender) {
        switch (toupper(gender)) {
        case 'M': return "Male";
        case 'F': return "Female";
        case 'O': return "Other";
        default: return "Unknown";
        }
    }

    void viewProfile(Voter* v) {
        if (!v) {
            cout << "Voter doesn't exist\n";
        }
        cout << "==============================\n";
        cout << "   Y O U R    P R O F I L E   \n";
        cout << "==============================\n";
        cout << "Voter ID: " << v->voterID << endl;
        cout << "Name: " << v->name << endl;
        cout << "CNIC: " << v->cnic << endl;
        cout << "Gender: " << v->gender << endl;
        cout << "Contact Number: " << v->contactNumber << endl;
        cout << "City :" << v->town << endl;
        cout << "Polling Station: " << v->pollingStation << endl;   
    }
};

#endif