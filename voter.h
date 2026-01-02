// #ifndef VOTER_H
// #define VOTER_H

// #include <iostream>
// #include <vector>
// #include <iomanip>
// #include <string>
// #include <fstream>
// #include <sstream>
// #include <cctype>
// #include "config.h"
// using namespace std;

// // VOTERHASHTABLE CLASS - MAIN STORAGE & OPERATIONS

// class VoterHashTable {
// private:
//     int tableSize;
//     vector<Voter*>* table;
//     const char* FILENAME = "voters.bin";

//     vector<string> getStationsForTown(string town) {
//         return GeographicConfig::getStationsForTown(town);
//     }

//     // HASH FUNCTION - POLYNOMIAL ROLLING HASH
//     int hashFunc(const string& id) {
//         int hash = 0;
//         for (char c : id) 
//             hash = (hash * 31 + c) % tableSize;
//         return hash;
//     }

//     // SIMPLE HASH FOR STATION ASSIGNMENT
//     int simpleHash(const string& str) {
//         int hash = 0;
//         for (size_t i = 0; i < str.length(); i++) 
//             hash = (hash * 31 + str[i]) % 1000000;
//         if (hash < 0) 
//             hash = -hash;
//         return hash;
//     }

//     string toLower(string str) {
//         for (char& c : str) c = tolower(c);
//         return str;
//     }

//     // AUTO-ASSIGN POLLING STATION BASED ON TOWN & CNIC
//     string assignPollingStation(string town, string cnic) {
//         vector<string> stations = getStationsForTown(town);
//         if (stations.empty()) {
//             cout << "Warning: No stations found for town: " << town << endl;
//             return "UNKNOWN";
//         }
//         int hashValue = simpleHash(cnic);
//         int stationIndex = hashValue % stations.size();
//         return stations[stationIndex];
//     }

//     // GENERATE SEQUENTIAL VOTER ID (VID00001 FORMAT)
//     string generateVoterID() {
//         int maxID = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (v->voterID.length() >= 3 && v->voterID.substr(0, 3) == "VID") {
//                     string numPart = v->voterID.substr(3);
//                     try {
//                         int currentID = stoi(numPart);
//                         if (currentID > maxID) maxID = currentID;
//                     }
//                     catch (...) {}
//                 }
//             }
//         }
//         stringstream ss;
//         ss << "VID" << setw(7) << setfill('0') << (maxID + 1);  // 3 + 7 = 10 chars
//         return ss.str();
//     }

//     // VALIDATION FUNCTIONS
//     bool validateCNIC(const string& cnic) {
//         if (cnic.length() != 13) return false;
//         for (char c : cnic) if (!isdigit(c)) return false;
//         return true;
//     }

//     bool validateContact(const string& contact) {
//         if (contact.length() != 11) return false;
//         if (contact.substr(0, 2) != "03") return false;
//         for (char c : contact) if (!isdigit(c)) return false;
//         return true;
//     }

//     bool validatePassword(const string& password) {
//         return password.length() >= 6;
//     }

//     char validateGenderInput(const string& genderInput) {
//         if (genderInput.length() == 0) return 'O';
//         char genderChar = toupper(genderInput[0]);
//         if (genderChar == 'M' || genderChar == 'F' || genderChar == 'O') return genderChar;
//         return 'O';
//     }

//     // FILE PERSISTENCE - APPEND SINGLE VOTER
//     void saveVoterToFile(Voter* v) {
//         ofstream out(FILENAME, ios::app | ios::binary);
//         if (!out) {
//             cout << "Error: Cannot open voter data file for appending!\n";
//             return;
//         }
        
//         auto writeString = [&out](const string& str) {
//             size_t len = str.size();
//             out.write((char*)&len, sizeof(len));
//             out.write(str.c_str(), len);
//         };
        
//         writeString(v->voterID); 
//         writeString(v->name); 
//         writeString(v->cnic);
//         out.write((char*)&v->gender, sizeof(v->gender));
//         writeString(v->contactNumber); 
//         writeString(v->town);
//         writeString(v->pollingStation); 
//         writeString(v->password);
//         out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
//         out.close();
        
//         if (!out.good()) {
//             cout << "Warning: Error writing voter to file!\n";
//         }
//     }

//     // LOAD ALL VOTERS FROM BINARY FILE - FIXED VERSION
//     void loadAllFromFile() {
//     cout << "Loading voters from file...\n";
    
//     ifstream in(FILENAME, ios::in | ios::binary);
//     if (!in) {
//         cout << "No existing voter data found.\n";
//         return;
//     }
    
//     in.seekg(0, ios::end);
//     streampos fileSize = in.tellg();
//     in.seekg(0, ios::beg);
    
//     if (fileSize == 0) {
//         cout << "File is empty.\n";
//         in.close();
//         return;
//     }
    
//     cout << "File size: " << fileSize << " bytes\n";
    
//     int loaded = 0;
//     int skipped = 0;
    
//     while (in && in.tellg() < fileSize) {
//         // FIXED readString function
//         auto readString = [&in]() -> string {
//             // Read length as size_t
//             size_t len = 0;
//             in.read((char*)&len, sizeof(size_t));
            
//             if (!in || len == 0 || len > 1000) {
//                 // Return empty string on error
//                 return "";
//             }
            
//             // Read the actual string
//             vector<char> buffer(len);
//             in.read(buffer.data(), len);
            
//             if (!in) {
//                 return "";
//             }
            
//             return string(buffer.begin(), buffer.end());
//         };
        
//         // Read voter record
//         string voterID = readString();
//         if (voterID.empty()) {
//             cout << "Failed to read voterID, stopping.\n";
//             break;
//         }
        
//         string name = readString();
//         string cnic = readString();
        
//         char gender;
//         in.read((char*)&gender, sizeof(char));
        
//         string contact = readString();
//         string town = readString();
//         string station = readString();
//         string password = readString();
        
//         bool hasVoted;
//         in.read((char*)&hasVoted, sizeof(bool));
        
//         // Check for read errors
//         if (!in) {
//             cout << "Read error at record " << loaded + 1 << endl;
//             break;
//         }
        
//         // Validate voterID
//         if (voterID.length() < 3 || voterID.substr(0, 3) != "VID") {
//             skipped++;
//             cout << "Skipping invalid voterID: " << voterID << endl;
//             continue;
//         }
        
//         // Store voter
//         int index = hashFunc(voterID);
//         Voter* v = new Voter(voterID, name, cnic, gender, contact, 
//                            town, station, password, hasVoted);
//         table[index].push_back(v);
//         loaded++;
//     }
    
//     in.close();
//     cout << "Loaded " << loaded << " voters, skipped " << skipped << " invalid records.\n";
// }

//     // SAVE ALL VOTERS TO FILE (OVERWRITE)
//     void saveAllToFile() {
//         ofstream out(FILENAME, ios::binary | ios::trunc);
//         if (!out) {
//             cout << "Error: Cannot open voter data file for writing!\n";
//             return;
//         }
        
//         int savedCount = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 auto writeString = [&out](const string& str) {
//                     size_t len = str.size();
//                     out.write((char*)&len, sizeof(len));
//                     out.write(str.c_str(), len);
//                 };
                
//                 writeString(v->voterID); 
//                 writeString(v->name); 
//                 writeString(v->cnic);
//                 out.write((char*)&v->gender, sizeof(v->gender));
//                 writeString(v->contactNumber); 
//                 writeString(v->town);
//                 writeString(v->pollingStation); 
//                 writeString(v->password);
//                 out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
//                 savedCount++;
//             }
//         }
        
//         out.close();
//         if (!out.good()) {
//             cout << "Warning: There might have been an error writing voter data!\n";
//         } else {
//             cout << "Saved " << savedCount << " voters to file.\n";
//         }
//     }

//     // UTILITY FUNCTIONS
//     bool existsInVector(const vector<string>& vec, const string& str) {
//         for (const string& s : vec) if (s == str) return true;
//         return false;
//     }

//     void validateAllVoterIDs() {
//         cout << "Validating Voter IDs...\n";
//         int invalidCount = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (v->voterID.empty() || v->voterID.substr(0, 3) != "VID" || v->voterID.length() != 10) {
//                     cout << "Warning: Invalid Voter ID found: '" << v->voterID
//                         << "' for voter: '" << v->name << "'" << endl;
//                     invalidCount++;
//                 }
//             }
//         }
//         if (invalidCount == 0) {
//             cout << "All Voter IDs are valid.\n";
//         } else {
//             cout << "Found " << invalidCount << " invalid Voter IDs.\n";
//         }
//     }

//     void cleanCorruptedEntries() {
//         cout<<"ID format wrong so removed.\n";
//         int removed = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto it = table[i].begin(); it != table[i].end(); ) {
//                 Voter* v = *it;
//                 bool isCorrupted = false;
                
//                 // Check for corruption
//                 if (v->voterID.empty() || v->name.empty() || v->cnic.empty()) {  
//                     isCorrupted = true;
//                 }
//                 else if (v->gender != 'M' && v->gender != 'F' && v->gender != 'O') {
//                     v->gender = 'O'; // Fix invalid gender
//                 }
                
//                 if (isCorrupted) {
//                     cout << "Removing corrupted entry: " << v->voterID << " - " << v->name << endl;
//                     delete v; 
//                     it = table[i].erase(it); 
//                     removed++;
//                 }
//                 else {
//                     ++it;
//                 }
//             }
//         }
//         if (removed > 0) {
//             cout << "Removed " << removed << " corrupted entries.\n";
//         }
//     }

// public:
//     // CONSTRUCTOR - INITIALIZE HASH TABLE & LOAD DATA
//     VoterHashTable(int size) {
//         cout << "Initializing VoterHashTable with size: " << size << "\n";
//         tableSize = size;
//         table = new vector<Voter*>[tableSize];
//        loadAllFromFile();
//     }

//     // DESTRUCTOR - SAVE DATA & CLEAN MEMORY
//     ~VoterHashTable() {
//         cout << "Saving voter data to file...\n";
//         saveAllToFile();
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) delete v;
//             table[i].clear();
//         }
//         delete[] table;
//     }

//     // ====================================================
//     // PUBLIC INTERFACE - VOTER MANAGEMENT
//     // ====================================================

//     // REGISTER NEW VOTER WITH VALIDATION
//     void insertVoter(string name, string cnic, string genderInput, string contact, string town, string password) {
//         if (searchByCNIC(cnic) != nullptr) { 
//             cout << "Error: CNIC '" << cnic << "' is already registered!\n"; 
//             return; 
//         }
//         if (!validateCNIC(cnic)) { 
//             cout << "Error: Invalid CNIC! Must be 13 digits.\n"; 
//             return; 
//         }
//         if (!validateContact(contact)) { 
//             cout << "Error: Invalid contact number! Must be 11 digits starting with 03.\n"; 
//             return; 
//         }
//         if (!validatePassword(password)) { 
//             cout << "Error: Password must be at least 6 characters long.\n"; 
//             return; 
//         }
        
//         char gender = validateGenderInput(genderInput);
//         if (gender == 'O' && (toupper(genderInput[0]) != 'O')) {
//             cout << "Warning: Gender set to 'Other' (valid: M/m, F/f, O/o)\n";
//         }
        
//         string station = assignPollingStation(town, cnic);
//         string voterID = generateVoterID();
        
//         if (searchVoter(voterID) != nullptr) { 
//             cout << "Error: Voter ID collision detected! Please try again.\n"; 
//             return; 
//         }
        
//         int index = hashFunc(voterID);
//         Voter* v = new Voter(voterID, name, cnic, gender, contact, town, station, password, false);
//         table[index].push_back(v);
//         saveVoterToFile(v);
        
//         cout << "\n========================================\n";
//         cout << "  VOTER REGISTERED SUCCESSFULLY!\n";
//         cout << "========================================\n";
//         cout << "Voter ID: " << voterID << "\n";
//         cout << "Name: " << name << "\n";
//         cout << "CNIC: " << cnic << "\n";
//         cout << "Polling Station: " << station << "\n";
//         cout << "========================================\n";
//     }

//     // SEARCH BY VOTER ID
//     Voter* searchVoter(string id) {
//         int index = hashFunc(id);
//         for (auto v : table[index]) 
//             if (v->voterID == id) 
//                 return v;
//         return nullptr;
//     }

//     // SEARCH BY CNIC (LINEAR SEARCH)
//     Voter* searchByCNIC(string cnic) {
//         for (int i = 0; i < tableSize; i++)
//             for (auto v : table[i]) 
//                 if (v->cnic == cnic) 
//                     return v;
//         return nullptr;
//     }

//     // SEARCH FOR LOGIN (BY VOTER ID OR CNIC)
//     Voter* searchForLogin(string identifier) {
//         Voter* v = searchVoter(identifier);
//         if (v) 
//             return v;
//         return searchByCNIC(identifier);
//     }

//     // DISPLAY ALL VOTERS IN DETAILED FORMAT
//     void printTable() {
//         cout << "\n" << string(80, '=') << "\n";
//         cout << "                       ALL REGISTERED VOTERS\n";
//         cout << string(80, '=') << "\n";
//         int count = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 count++;
//                 cout << "Voter #" << count << "\n" << string(40, '-') << "\n";
//                 cout << "Voter ID: " << v->voterID << "\n";
//                 cout << "Name: " << v->name << "\n";
//                 cout << "CNIC: " << v->cnic << "\n";
//                 cout << "Password: " << v->password << "\n";
//                 cout << "Gender: ";
//                 switch (toupper(v->gender)) {
//                 case 'M': cout << "Male\n"; break;
//                 case 'F': cout << "Female\n"; break;
//                 case 'O': cout << "Other\n"; break;
//                 default: cout << "Unknown\n";
//                 }
//                 cout << "Contact: " << v->contactNumber << "\n";
//                 cout << "Town: " << v->town << "\n";
//                 cout << "Polling Station: " << v->pollingStation << "\n";
//                 cout << "Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
//                 cout << string(40, '-') << "\n\n";
//             }
//         }
//         if (count == 0) cout << "No voters registered yet.\n";
//         cout << string(80, '=') << "\n";
//     }

//     // LIST VOTERS IN SPECIFIC POLLING STATION
//     void printVotersByStation(string stationID) {
//         cout << "\n" << string(60, '=') << "\n";
//         cout << "Voters in Polling Station: " << stationID << "\n";
//         cout << string(60, '=') << "\n";
//         int count = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (v->pollingStation == stationID) {
//                     count++;
//                     cout << count << ". " << v->name << " (" << v->voterID << ")\n";
//                     cout << "   CNIC: " << v->cnic << " | Contact: " << v->contactNumber << "\n";
//                     cout << "   Gender: ";
//                     switch (toupper(v->gender)) {
//                     case 'M': cout << "Male"; break;
//                     case 'F': cout << "Female"; break;
//                     case 'O': cout << "Other"; break;
//                     default: cout << "Unknown";
//                     }
//                     cout << " | Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
//                     cout << string(30, '-') << "\n";
//                 }
//             }
//         }
//         if (count == 0) cout << "No voters assigned to this polling station.\n";
//         cout << "Total: " << count << " voters\n";
//         cout << string(60, '=') << "\n";
//     }

//     // LIST VOTERS FROM SPECIFIC TOWN
//     void printVotersByTown(string town) {
//         cout << "\n" << string(60, '=') << "\n";
//         cout << "Voters in Town: " << town << "\n";
//         string townLower = toLower(town);
//         cout << string(60, '=') << "\n";
//         int count = 0;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (toLower(v->town) == townLower) {
//                     count++;
//                     cout << count << ". " << v->name << " (" << v->voterID << ")\n";
//                     cout << "   Station: " << v->pollingStation << " | CNIC: " << v->cnic << "\n";
//                     cout << "   Gender: ";
//                     switch (toupper(v->gender)) {
//                     case 'M': cout << "Male"; break;
//                     case 'F': cout << "Female"; break;
//                     case 'O': cout << "Other"; break;
//                     default: cout << "Unknown";
//                     }
//                     cout << " | Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
//                     cout << string(30, '-') << "\n";
//                 }
//             }
//         }
//         if (count == 0) cout << "No voters from this town.\n";
//         cout << "Total: " << count << " voters\n";
//         cout << string(60, '=') << "\n";
//     }

//     // GET LIST OF ALL UNIQUE POLLING STATIONS
//     vector<string> getAllPollingStations() {
//         vector<string> stations;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (!existsInVector(stations, v->pollingStation)) 
//                     stations.push_back(v->pollingStation);
//             }
//         }
//         return stations;
//     }

//     // GET LIST OF ALL UNIQUE TOWNS
//     vector<string> getAllTowns() {
//         vector<string> towns;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (!existsInVector(towns, v->town)) 
//                     towns.push_back(v->town);
//             }
//         }
//         return towns;
//     }

//     // DISPLAY TOWN-WISE VOTER STATISTICS (TOTAL, VOTED, TURNOUT)
//     void printTownStatistics() {
//         vector<string> townNames;
//         vector<int> townTotal;
//         vector<int> townVoted;
        
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 string town = v->town;
//                 int townIndex = -1;
//                 for (size_t j = 0; j < townNames.size(); j++) {
//                     if (townNames[j] == town) { 
//                         townIndex = j; 
//                         break; 
//                     }
//                 }
//                 if (townIndex == -1) {
//                     townNames.push_back(town); 
//                     townTotal.push_back(0); 
//                     townVoted.push_back(0);
//                     townIndex = townNames.size() - 1;
//                 }
//                 townTotal[townIndex]++;
//                 if (v->hasVoted) townVoted[townIndex]++;
//             }
//         }
        
//         cout << "\n" << string(70, '=') << "\n";
//         cout << "                VOTER STATISTICS BY TOWN\n";
//         cout << string(70, '=') << "\n";
//         cout << left << setw(15) << "TOWN" << setw(10) << "TOTAL" << setw(10) << "VOTED" 
//              << setw(12) << "REMAINING" << setw(15) << "TURNOUT %" << "\n";
//         cout << string(70, '-') << "\n";
        
//         for (size_t i = 0; i < townNames.size(); i++) {
//             string town = townNames[i];
//             int total = townTotal[i];
//             int voted = townVoted[i];
//             int remaining = total - voted;
//             float turnout = total > 0 ? (float(voted) / total * 100) : 0;
//             cout << left << setw(15) << town 
//                  << setw(10) << total 
//                  << setw(10) << voted 
//                  << setw(12) << remaining 
//                  << fixed << setprecision(1) << setw(15) << turnout << "%\n";
//         }
//         cout << string(70, '=') << "\n";
//     }

//     // GET TOTAL NUMBER OF REGISTERED VOTERS
//     int getTotalVoters() {
//         int count = 0;
//         for (int i = 0; i < tableSize; i++) 
//             count += table[i].size();
//         return count;
//     }

//     // COUNT HOW MANY VOTERS HAVE VOTED
//     int getVotedCount() {
//         int count = 0;
//         for (int i = 0; i < tableSize; i++)
//             for (auto v : table[i]) 
//                 if (v->hasVoted) 
//                     count++;
//         return count;
//     }

//     // COUNT HOW MANY VOTERS HAVEN'T VOTED
//     int getNotVotedCount() {
//         return getTotalVoters() - getVotedCount();
//     }

//     // MARK VOTER AS VOTED (ONE-TIME VOTING ENFORCEMENT)
//     bool markAsVoted(string voterID) {
//         Voter* v = searchVoter(voterID);
//         if (v && !v->hasVoted) {
//             v->hasVoted = true;
//             saveAllToFile();  // Save changes immediately
//             return true;
//         }
//         return false;
//     }

//     // RESET ALL VOTE FLAGS (ADMIN FUNCTION)
//     void resetAllVotes() {
//         for (int i = 0; i < tableSize; i++)
//             for (auto v : table[i]) 
//                 v->hasVoted = false;
//         saveAllToFile();
//         cout << "All votes reset successfully!\n";
//     }

//     // GET VOTER IDS IN SPECIFIC POLLING STATION
//     vector<string> getVotersInStation(string stationID) {
//         vector<string> voterIDs;
//         for (int i = 0; i < tableSize; i++) {
//             for (auto v : table[i]) {
//                 if (v->pollingStation == stationID) 
//                     voterIDs.push_back(v->voterID);
//             }
//         }
//         return voterIDs;
//     }

//     // GET POLLING STATION OF SPECIFIC VOTER
//     string getPollingStation(string voterID) {
//         Voter* v = searchVoter(voterID);
//         return v ? v->pollingStation : "";
//     }

//     // GET TOWN OF SPECIFIC VOTER
//     string getVoterTown(string voterID) {
//         Voter* v = searchVoter(voterID);
//         return v ? v->town : "";
//     }

//     // UPDATE VOTER'S PASSWORD
//     bool updatePassword(string voterID, string newPassword) {
//         Voter* v = searchVoter(voterID);
//         if (v && validatePassword(newPassword)) {
//             v->password = newPassword;
//             saveAllToFile();
//             return true;
//         }
//         return false;
//     }

//     // CONVERT GENDER CHAR TO DISPLAY STRING
//     string getGenderString(char gender) {
//         switch (toupper(gender)) {
//         case 'M': return "Male";
//         case 'F': return "Female";
//         case 'O': return "Other";
//         default: return "Unknown";
//         }
//     }

//     void viewProfile(Voter* v) {
//         if (!v) {
//             cout << "Voter doesn't exist\n";
//             return;
//         }
//         cout << "==============================\n";
//         cout << "   Y O U R    P R O F I L E   \n";
//         cout << "==============================\n";
//         cout << "Voter ID: " << v->voterID << endl;
//         cout << "Name: " << v->name << endl;
//         cout << "CNIC: " << v->cnic << endl;
//         cout << "Gender: " << v->gender << endl;
//         cout << "Contact Number: " << v->contactNumber << endl;
//         cout << "City: " << v->town << endl;
//         cout << "Polling Station: " << v->pollingStation << endl;   
//         cout << "Voted: " << (v->hasVoted ? "Yes" : "No") << endl;
//         cout << "==============================\n";
//     }
// };

// #endif

#ifndef VOTER_H
#define VOTER_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstring>
#include "config.h"
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
        if (stations.empty()) {
            cout << "Warning: No stations found for town: " << town << endl;
            return "UNKNOWN";
        }
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
        ss << "VID" << setw(7) << setfill('0') << (maxID + 1);
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

    // FIXED: Improved file reading with better error handling
    void loadAllFromFile() {
        cout << "Loading voters from " << FILENAME << "...\n";
        
        ifstream in(FILENAME, ios::in | ios::binary);
        if (!in) {
            cout << "No existing voter data found. Creating new file.\n";
            return;
        }
        
        in.seekg(0, ios::end);
        streampos fileSize = in.tellg();
        in.seekg(0, ios::beg);
        
        if (fileSize == 0) {
            cout << "File is empty.\n";
            in.close();
            return;
        }
        
        cout << "File size: " << fileSize << " bytes\n";
        
        int loaded = 0;
        int skipped = 0;
        
        while (in && in.tellg() < fileSize) {
            // Try to read each field
            string voterID, name, cnic, contact, town, station, password;
            char gender;
            bool hasVoted;
            
            try {
                // Read voterID
                size_t len;
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100 || len == 0) {
                    // Skip corrupted entry
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                char* buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                voterID = string(buffer);
                delete[] buffer;
                
                // Read name
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100) {
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                name = string(buffer);
                delete[] buffer;
                
                // Read cnic
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100) {
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                cnic = string(buffer);
                delete[] buffer;
                
                // Read gender
                if (!in.read(&gender, sizeof(char))) break;
                
                // Read contact
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100) {
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                contact = string(buffer);
                delete[] buffer;
                
                // Read town
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100) {
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                town = string(buffer);
                delete[] buffer;
                
                // Read station
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100) {
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                station = string(buffer);
                delete[] buffer;
                
                // Read password
                if (!in.read((char*)&len, sizeof(size_t))) break;
                if (len > 100) {
                    in.seekg(fileSize, ios::beg);
                    break;
                }
                buffer = new char[len + 1];
                in.read(buffer, len);
                buffer[len] = '\0';
                password = string(buffer);
                delete[] buffer;
                
                // Read hasVoted
                if (!in.read((char*)&hasVoted, sizeof(bool))) break;
                
            } catch (...) {
                cout << "Error reading voter record. Stopping.\n";
                break;
            }
            
            // Validate basic data
            if (voterID.empty() || name.empty() || cnic.empty()) {
                skipped++;
                continue;
            }
            
            // Store voter
            int index = hashFunc(voterID);
            Voter* v = new Voter(voterID, name, cnic, gender, contact, 
                               town, station, password, hasVoted);
            table[index].push_back(v);
            loaded++;
            
            if (loaded % 100 == 0) {
                cout << "Loaded " << loaded << " voters...\n";
            }
        }
        
        in.close();
        cout << "Successfully loaded " << loaded << " voters, skipped " << skipped << " invalid records.\n";
        
        // Display summary
        int total = getTotalVoters();
        int voted = getVotedCount();
        cout << "Total voters in system: " << total << endl;
        cout << "Voters who have voted: " << voted << endl;
        if (total > 0) {
            float turnout = (float)voted / total * 100;
            cout << "Current turnout: " << fixed << setprecision(1) << turnout << "%\n";
        }
    }

    // SAVE ALL VOTERS TO FILE (OVERWRITE)
    void saveAllToFile() {
        ofstream out(FILENAME, ios::binary | ios::trunc);
        if (!out) {
            cout << "Error: Cannot open voter data file for writing!\n";
            return;
        }
        
        int savedCount = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                // Write voterID
                size_t len = v->voterID.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->voterID.c_str(), len);
                
                // Write name
                len = v->name.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->name.c_str(), len);
                
                // Write cnic
                len = v->cnic.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->cnic.c_str(), len);
                
                // Write gender
                out.write(&v->gender, sizeof(char));
                
                // Write contact
                len = v->contactNumber.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->contactNumber.c_str(), len);
                
                // Write town
                len = v->town.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->town.c_str(), len);
                
                // Write station
                len = v->pollingStation.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->pollingStation.c_str(), len);
                
                // Write password
                len = v->password.size();
                out.write((char*)&len, sizeof(size_t));
                out.write(v->password.c_str(), len);
                
                // Write hasVoted
                out.write((char*)&v->hasVoted, sizeof(bool));
                
                savedCount++;
            }
        }
        
        out.close();
        if (!out.good()) {
            cout << "Warning: There might have been an error writing voter data!\n";
        } else {
            cout << "Saved " << savedCount << " voters to file.\n";
        }
    }

    // APPEND SINGLE VOTER TO FILE
    void saveVoterToFile(Voter* v) {
        ofstream out(FILENAME, ios::app | ios::binary);
        if (!out) {
            cout << "Error: Cannot open voter data file for appending!\n";
            return;
        }
        
        // Write voterID
        size_t len = v->voterID.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->voterID.c_str(), len);
        
        // Write name
        len = v->name.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->name.c_str(), len);
        
        // Write cnic
        len = v->cnic.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->cnic.c_str(), len);
        
        // Write gender
        out.write(&v->gender, sizeof(char));
        
        // Write contact
        len = v->contactNumber.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->contactNumber.c_str(), len);
        
        // Write town
        len = v->town.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->town.c_str(), len);
        
        // Write station
        len = v->pollingStation.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->pollingStation.c_str(), len);
        
        // Write password
        len = v->password.size();
        out.write((char*)&len, sizeof(size_t));
        out.write(v->password.c_str(), len);
        
        // Write hasVoted
        out.write((char*)&v->hasVoted, sizeof(bool));
        
        out.close();
    }

    // UTILITY FUNCTIONS
    bool existsInVector(const vector<string>& vec, const string& str) {
        for (const string& s : vec) if (s == str) return true;
        return false;
    }

public:
    // CONSTRUCTOR - INITIALIZE HASH TABLE & LOAD DATA
    VoterHashTable(int size) {
        cout << "Initializing VoterHashTable with size: " << size << "\n";
        tableSize = size;
        table = new vector<Voter*>[tableSize];
        loadAllFromFile();
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
    bool insertVoter(string name, string cnic, string genderInput, string contact, string town, string password) {
        if (searchByCNIC(cnic) != nullptr) { 
            cout << "Error: CNIC '" << cnic << "' is already registered!\n"; 
            return false; 
        }
        if (!validateCNIC(cnic)) { 
            cout << "Error: Invalid CNIC! Must be 13 digits.\n"; 
            return false; 
        }
        if (!validateContact(contact)) { 
            cout << "Error: Invalid contact number! Must be 11 digits starting with 03.\n"; 
            return false; 
        }
        if (!validatePassword(password)) { 
            cout << "Error: Password must be at least 6 characters long.\n"; 
            return false; 
        }
        
        char gender = validateGenderInput(genderInput);
        if (gender == 'O' && (toupper(genderInput[0]) != 'O')) {
            cout << "Warning: Gender set to 'Other' (valid: M/m, F/f, O/o)\n";
        }
        
        string station = assignPollingStation(town, cnic);
        string voterID = generateVoterID();
        
        if (searchVoter(voterID) != nullptr) { 
            cout << "Error: Voter ID collision detected! Please try again.\n"; 
            return false; 
        }
        
        int index = hashFunc(voterID);
        Voter* v = new Voter(voterID, name, cnic, gender, contact, town, station, password, false);
        table[index].push_back(v);
        saveVoterToFile(v);
        
        cout << "\n========================================\n";
        cout << "  VOTER REGISTERED SUCCESSFULLY!\n";
        cout << "========================================\n";
        cout << "Voter ID: " << voterID << "\n";
        cout << "Name: " << name << "\n";
        cout << "CNIC: " << cnic << "\n";
        cout << "Polling Station: " << station << "\n";
        cout << "========================================\n";
        
        return true;
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
            for (auto v : table[i]) 
                if (v->cnic == cnic) 
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

    // GET TOTAL NUMBER OF REGISTERED VOTERS
    int getTotalVoters() {
        int count = 0;
        for (int i = 0; i < tableSize; i++) 
            count += table[i].size();
        return count;
    }

    // COUNT HOW MANY VOTERS HAVE VOTED
    int getVotedCount() {
        int count = 0;
        for (int i = 0; i < tableSize; i++)
            for (auto v : table[i]) 
                if (v->hasVoted) 
                    count++;
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
            saveAllToFile();  // Save changes immediately
            return true;
        }
        return false;
    }

    // RESET ALL VOTE FLAGS (ADMIN FUNCTION)
    void resetAllVotes() {
        for (int i = 0; i < tableSize; i++)
            for (auto v : table[i]) 
                v->hasVoted = false;
        saveAllToFile();
        cout << "All votes reset successfully!\n";
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

    void viewProfile(Voter* v) {
        if (!v) {
            cout << "Voter doesn't exist\n";
            return;
        }
        cout << "==============================\n";
        cout << "   Y O U R    P R O F I L E   \n";
        cout << "==============================\n";
        cout << "Voter ID: " << v->voterID << endl;
        cout << "Name: " << v->name << endl;
        cout << "CNIC: " << v->cnic << endl;
        cout << "Gender: " << v->gender << endl;
        cout << "Contact Number: " << v->contactNumber << endl;
        cout << "City: " << v->town << endl;
        cout << "Polling Station: " << v->pollingStation << endl;   
        cout << "Voted: " << (v->hasVoted ? "Yes" : "No") << endl;
        cout << "==============================\n";
    }
    
    // Get all polling stations
    vector<string> getAllPollingStations() {
        vector<string> stations;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (!existsInVector(stations, v->pollingStation)) 
                    stations.push_back(v->pollingStation);
            }
        }
        return stations;
    }
    
    // Get all towns
    vector<string> getAllTowns() {
        vector<string> towns;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (!existsInVector(towns, v->town)) 
                    towns.push_back(v->town);
            }
        }
        return towns;
    }
    
    // Print town statistics
    void printTownStatistics() {
        vector<string> townNames;
        vector<int> townTotal;
        vector<int> townVoted;
        
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                string town = v->town;
                int townIndex = -1;
                for (size_t j = 0; j < townNames.size(); j++) {
                    if (townNames[j] == town) { 
                        townIndex = j; 
                        break; 
                    }
                }
                if (townIndex == -1) {
                    townNames.push_back(town); 
                    townTotal.push_back(0); 
                    townVoted.push_back(0);
                    townIndex = townNames.size() - 1;
                }
                townTotal[townIndex]++;
                if (v->hasVoted) townVoted[townIndex]++;
            }
        }
        
        cout << "\n" << string(70, '=') << "\n";
        cout << "                VOTER STATISTICS BY TOWN\n";
        cout << string(70, '=') << "\n";
        cout << left << setw(15) << "TOWN" << setw(10) << "TOTAL" << setw(10) << "VOTED" 
             << setw(12) << "REMAINING" << setw(15) << "TURNOUT %" << "\n";
        cout << string(70, '-') << "\n";
        
        for (size_t i = 0; i < townNames.size(); i++) {
            string town = townNames[i];
            int total = townTotal[i];
            int voted = townVoted[i];
            int remaining = total - voted;
            float turnout = total > 0 ? (float(voted) / total * 100) : 0;
            cout << left << setw(15) << town 
                 << setw(10) << total 
                 << setw(10) << voted 
                 << setw(12) << remaining 
                 << fixed << setprecision(1) << setw(15) << turnout << "%\n";
        }
        cout << string(70, '=') << "\n";
    }
    
    // Print voters by station
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
    
    // Print voters by town
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
};

#endif