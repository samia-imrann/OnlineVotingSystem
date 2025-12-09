#ifndef VOTER_H
#define VOTER_H
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
#include <cctype>
using namespace std;

struct Voter {
    string voterID;
    string name;
    string cnic;
    char gender;
    string contactNumber;
    string town;
    string pollingStation;
    string password;
    bool hasVoted;

    Voter(string id = "", string n = "", string c = "", char g = 'O',
        string phone = "", string t = "", string station = "",
        string pass = "", bool voted = false) {
        voterID = id;
        name = n;
        cnic = c;
        gender = g;
        contactNumber = phone;
        town = t;
        pollingStation = station;
        password = pass;
        hasVoted = voted;
    }
};

class VoterHashTable {
private:
    int tableSize;
    vector<Voter*>* table;
    const char* FILENAME = "voters.bin";

    struct TownStation {
        string town;
        vector<string> stations;
    };

    vector<TownStation> townStations;

    void initializeTownStations() {
        townStations.clear();
        TownStation ts;

        ts.town = "Karachi";
        ts.stations = { "KHI01", "KHI02", "KHI03", "KHI04", "KHI05" };
        townStations.push_back(ts);

        ts.town = "Lahore";
        ts.stations = { "LHR01", "LHR02", "LHR03", "LHR04" };
        townStations.push_back(ts);

        ts.town = "Islamabad";
        ts.stations = { "ISB01", "ISB02", "ISB03" };
        townStations.push_back(ts);

        ts.town = "Rawalpindi";
        ts.stations = { "RWP01", "RWP02" };
        townStations.push_back(ts);

        ts.town = "Faisalabad";
        ts.stations = { "FSD01", "FSD02", "FSD03" };
        townStations.push_back(ts);

        ts.town = "Multan";
        ts.stations = { "MUL01", "MUL02" };
        townStations.push_back(ts);

        ts.town = "Peshawar";
        ts.stations = { "PES01", "PES02", "PES03" };
        townStations.push_back(ts);

        ts.town = "Quetta";
        ts.stations = { "QTA01", "QTA02" };
        townStations.push_back(ts);

        ts.town = "Default";
        ts.stations = { "DEF01", "DEF02" };
        townStations.push_back(ts);
    }

    vector<string> getStationsForTown(string town) {
        for (const auto& ts : townStations) {
            if (ts.town == town) {
                return ts.stations;
            }
        }
        for (const auto& ts : townStations) {
            if (ts.town == "Default") {
                return ts.stations;
            }
        }
        return { "DEF01" };
    }

    int hashFunc(const string& id) {
        int hash = 0;
        for (char c : id) hash = (hash * 31 + c) % tableSize;
        return hash;
    }

    int simpleHash(const string& str) {
        int hash = 0;
        for (size_t i = 0; i < str.length(); i++) {
            hash = (hash * 31 + str[i]) % 1000000;
        }
        if (hash < 0) hash = -hash;
        return hash;
    }

    string assignPollingStation(string town, string cnic) {
        vector<string> stations = getStationsForTown(town);
        int hashValue = simpleHash(cnic);
        int stationIndex = hashValue % stations.size();
        return stations[stationIndex];
    }

    // Generate Voter ID starting with "VID" followed by sequential numbers
    string generateVoterID() {
        int maxID = 0;

        // Scan all voters to find highest VID number
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->voterID.length() >= 3 && v->voterID.substr(0, 3) == "VID") {
                    string numPart = v->voterID.substr(3); // Get "00001" part
                    try {
                        int currentID = stoi(numPart);
                        if (currentID > maxID) {
                            maxID = currentID;
                        }
                    }
                    catch (...) {
                        // If conversion fails, skip this voter
                    }
                }
            }
        }

        stringstream ss;
        ss << "VID" << setw(5) << setfill('0') << (maxID + 1);
        return ss.str();
    }

    bool validateCNIC(const string& cnic) {
        if (cnic.length() != 13) return false;
        for (char c : cnic) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

    bool validateContact(const string& contact) {
        if (contact.length() != 11) return false;
        if (contact.substr(0, 2) != "03") return false;
        for (char c : contact) {
            if (!isdigit(c)) return false;
        }
        return true;
    }

    bool validatePassword(const string& password) {
        // Password must be at least 6 characters
        if (password.length() < 6) return false;
        return true;
    }

    char validateGenderInput(const string& genderInput) {
        if (genderInput.length() == 0) return 'O';

        char genderChar = toupper(genderInput[0]);
        if (genderChar == 'M' || genderChar == 'F' || genderChar == 'O') {
            return genderChar;
        }
        return 'O';  // Default to 'Other' if invalid input
    }

    void saveVoterToFile(Voter* v) {
        ofstream out(FILENAME, ios::app | ios::binary);
        if (!out) return;

        auto writeString = [&out](const string& str) {
            size_t len = str.size();
            out.write((char*)&len, sizeof(len));
            out.write(str.c_str(), len);
            };

        writeString(v->voterID);
        writeString(v->name);
        writeString(v->cnic);
        out.write((char*)&v->gender, sizeof(v->gender));  // Store char directly
        writeString(v->contactNumber);
        writeString(v->town);
        writeString(v->pollingStation);
        writeString(v->password);  // Only password, no username
        out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
        out.close();
    }

    void loadFromFile() {
        ifstream in(FILENAME, ios::binary);
        if (!in) {
            cout << "No voter data file found. Starting fresh.\n";
            return;
        }

        // Clear existing in-memory data first
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                delete v;
            }
            table[i].clear();
        }

        cout << "Loading voter data from file...\n";

        int loadedCount = 0;
        while (true) {
            Voter* v = new Voter();
            size_t len;

            // === READ VOTERID ===
            in.read((char*)&len, sizeof(len));
            if (in.eof() || !in.good()) {
                delete v;
                break;
            }

            if (len > 1000) {
                delete v;
                cout << "Error: Invalid voterID length\n";
                break;
            }

            v->voterID.resize(len);
            in.read(&v->voterID[0], len);

            // === READ NAME ===
            in.read((char*)&len, sizeof(len));
            v->name.resize(len);
            in.read(&v->name[0], len);

            // === READ CNIC ===
            in.read((char*)&len, sizeof(len));
            v->cnic.resize(len);
            in.read(&v->cnic[0], len);

            // === READ GENDER (CHAR) ===
            in.read(&v->gender, sizeof(char));

            // === READ CONTACT ===
            in.read((char*)&len, sizeof(len));
            v->contactNumber.resize(len);
            in.read(&v->contactNumber[0], len);

            // === READ TOWN ===
            in.read((char*)&len, sizeof(len));
            v->town.resize(len);
            in.read(&v->town[0], len);

            // === READ POLLING STATION ===
            in.read((char*)&len, sizeof(len));
            v->pollingStation.resize(len);
            in.read(&v->pollingStation[0], len);

            // === READ PASSWORD ===
            in.read((char*)&len, sizeof(len));
            v->password.resize(len);
            in.read(&v->password[0], len);

            // === READ HASVOTED ===
            in.read((char*)&v->hasVoted, sizeof(bool));

            // Check if read was successful
            if (!in.good()) {
                delete v;
                if (in.eof()) break;
                cout << "Warning: Error reading voter data at record #" << (loadedCount + 1) << endl;
                break;
            }

            // Add to hash table
            int index = hashFunc(v->voterID);
            table[index].push_back(v);
            loadedCount++;
        }

        in.close();
        cout << "Loaded " << loadedCount << " voters from file.\n";
    }

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

                writeString(v->voterID);
                writeString(v->name);
                writeString(v->cnic);
                out.write((char*)&v->gender, sizeof(v->gender));
                writeString(v->contactNumber);
                writeString(v->town);
                writeString(v->pollingStation);
                writeString(v->password);
                out.write((char*)&v->hasVoted, sizeof(v->hasVoted));

                savedCount++;
            }
        }

        out.close();

        if (!out.good()) {
            cout << "Warning: There might have been an error writing voter data!\n";
        }
        else {
            cout << "Saved " << savedCount << " voters to file.\n";
        }
    }

    bool existsInVector(const vector<string>& vec, const string& str) {
        for (const string& s : vec) {
            if (s == str) return true;
        }
        return false;
    }

    void validateAllVoterIDs() {
        cout << "Validating Voter IDs...\n";
        int invalidCount = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->voterID.empty() || v->voterID.substr(0, 3) != "VID") {
                    cout << "Warning: Invalid Voter ID found: '" << v->voterID
                        << "' for voter: '" << v->name << "'" << endl;
                    invalidCount++;
                }
            }
        }
        if (invalidCount == 0) {
            cout << "All Voter IDs are valid.\n";
        }
    }

    void cleanCorruptedEntries() {
        int removed = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto it = table[i].begin(); it != table[i].end(); ) {
                Voter* v = *it;

                // If voter has no name or invalid CNIC, it's corrupted
                bool isCorrupted = false;

                // Check for obvious corruption
                if (v->name.empty() && v->cnic.empty() && v->voterID.empty()) {
                    isCorrupted = true;
                }
                // Check if gender is valid
                else if (v->gender != 'M' && v->gender != 'F' && v->gender != 'O') {
                    v->gender = 'O'; // Fix invalid gender
                }

                if (isCorrupted) {
                    cout << "Removing corrupted entry: " << v->voterID << endl;
                    delete v;
                    it = table[i].erase(it);
                    removed++;
                }
                else {
                    ++it;
                }
            }
        }

        if (removed > 0) {
            cout << "Removed " << removed << " corrupted entries.\n";
        }
    }

public:
    VoterHashTable(int size) {
        tableSize = size;
        table = new vector<Voter*>[tableSize];
        initializeTownStations();
        loadFromFile();
        cleanCorruptedEntries();
        validateAllVoterIDs();
    }

    ~VoterHashTable() {
        cout << "Saving voter data to file...\n";
        saveAllToFile();

        // Clean up memory
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                delete v;
            }
            table[i].clear();
        }
        delete[] table;
    }

    void insertVoter(string name, string cnic, string genderInput,
        string contact, string town, string password) {

        // Check for duplicate CNIC
        if (searchByCNIC(cnic) != nullptr) {
            cout << "Error: CNIC '" << cnic << "' is already registered!\n";
            return;
        }

        if (!validateCNIC(cnic)) {
            cout << "Error: Invalid CNIC! Must be 13 digits.\n";
            return;
        }

        if (!validateContact(contact)) {
            cout << "Error: Invalid contact number! Must be 11 digits starting with 03.\n";
            return;
        }

        if (!validatePassword(password)) {
            cout << "Error: Password must be at least 6 characters long.\n";
            return;
        }

        // Validate and convert gender input
        char gender = validateGenderInput(genderInput);
        if (gender == 'O' && (toupper(genderInput[0]) != 'O')) {
            cout << "Warning: Gender set to 'Other' (valid: M/m, F/f, O/o)\n";
        }

        string station = assignPollingStation(town, cnic);
        string voterID = generateVoterID();  // Generate new VID-based ID

        // Check for duplicate Voter ID (should not happen with sequential IDs)
        if (searchVoter(voterID) != nullptr) {
            cout << "Error: Voter ID collision detected! Please try again.\n";
            return;
        }

        int index = hashFunc(voterID);
        Voter* v = new Voter(voterID, name, cnic, gender, contact,
            town, station, password, false);  // No username
        table[index].push_back(v);

        saveVoterToFile(v);

        cout << "\n========================================\n";
        cout << "  VOTER REGISTERED SUCCESSFULLY!\n";
        cout << "========================================\n";
        cout << "Voter ID:      " << voterID << "\n";
        cout << "Name:          " << name << "\n";
        cout << "Polling Station: " << station << "\n";
        cout << "Town:          " << town << "\n";
        cout << "Password:      " << password << "\n";
        cout << "========================================\n";
    }

    Voter* searchVoter(string id) {
        int index = hashFunc(id);
        for (auto v : table[index]) {
            if (v->voterID == id)
                return v;
        }
        return nullptr;
    }

    Voter* searchByCNIC(string cnic) {
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->cnic == cnic)
                    return v;
            }
        }
        return nullptr;
    }

    // Updated: Search by CNIC or Voter ID for login
    Voter* searchForLogin(string identifier) {
        // Try as Voter ID first
        Voter* v = searchVoter(identifier);
        if (v) return v;

        // Try as CNIC
        return searchByCNIC(identifier);
    }

    void printTable() {
        cout << "\n" << string(80, '=') << "\n";
        cout << "                       ALL REGISTERED VOTERS\n";
        cout << string(80, '=') << "\n";

        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                count++;
                cout << "Voter #" << count << "\n";
                cout << string(40, '-') << "\n";
                cout << "Voter ID:      " << v->voterID << "\n";
                cout << "Name:          " << v->name << "\n";
                cout << "CNIC:          " << v->cnic << "\n";
                cout << "Gender:        ";
                switch (toupper(v->gender)) {
                case 'M': cout << "Male\n"; break;
                case 'F': cout << "Female\n"; break;
                case 'O': cout << "Other\n"; break;
                default: cout << "Unknown\n";
                }
                cout << "Contact:       " << v->contactNumber << "\n";
                cout << "Town:          " << v->town << "\n";
                cout << "Polling Station: " << v->pollingStation << "\n";
                cout << "Voted:         " << (v->hasVoted ? "Yes" : "No") << "\n";
                cout << string(40, '-') << "\n\n";
            }
        }

        if (count == 0) {
            cout << "No voters registered yet.\n";
        }
        cout << string(80, '=') << "\n";
    }

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

        if (count == 0) {
            cout << "No voters assigned to this polling station.\n";
        }
        cout << "Total: " << count << " voters\n";
        cout << string(60, '=') << "\n";
    }

    void printVotersByTown(string town) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "Voters in Town: " << town << "\n";
        cout << string(60, '=') << "\n";

        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->town == town) {
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

        if (count == 0) {
            cout << "No voters from this town.\n";
        }
        cout << "Total: " << count << " voters\n";
        cout << string(60, '=') << "\n";
    }

    vector<string> getAllPollingStations() {
        vector<string> stations;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (!existsInVector(stations, v->pollingStation)) {
                    stations.push_back(v->pollingStation);
                }
            }
        }
        return stations;
    }

    vector<string> getAllTowns() {
        vector<string> towns;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (!existsInVector(towns, v->town)) {
                    towns.push_back(v->town);
                }
            }
        }
        return towns;
    }

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
                if (v->hasVoted) {
                    townVoted[townIndex]++;
                }
            }
        }

        cout << "\n" << string(70, '=') << "\n";
        cout << "                VOTER STATISTICS BY TOWN\n";
        cout << string(70, '=') << "\n";
        cout << left << setw(15) << "TOWN"
            << setw(10) << "TOTAL"
            << setw(10) << "VOTED"
            << setw(12) << "REMAINING"
            << setw(15) << "TURNOUT %" << "\n";
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

    int getTotalVoters() {
        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            count += table[i].size();
        }
        return count;
    }

    int getVotedCount() {
        int count = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->hasVoted) count++;
            }
        }
        return count;
    }

    int getNotVotedCount() {
        return getTotalVoters() - getVotedCount();
    }

    bool markAsVoted(string voterID) {
        Voter* v = searchVoter(voterID);
        if (v && !v->hasVoted) {
            v->hasVoted = true;
            saveAllToFile();
            return true;
        }
        return false;
    }

    void resetAllVotes() {
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                v->hasVoted = false;
            }
        }
        saveAllToFile();
        cout << "All votes reset successfully!\n";
    }

    vector<string> getVotersInStation(string stationID) {
        vector<string> voterIDs;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->pollingStation == stationID) {
                    voterIDs.push_back(v->voterID);
                }
            }
        }
        return voterIDs;
    }

    string getPollingStation(string voterID) {
        Voter* v = searchVoter(voterID);
        return v ? v->pollingStation : "";
    }

    string getVoterTown(string voterID) {
        Voter* v = searchVoter(voterID);
        return v ? v->town : "";
    }

    bool updatePassword(string voterID, string newPassword) {
        Voter* v = searchVoter(voterID);
        if (v && validatePassword(newPassword)) {
            v->password = newPassword;
            saveAllToFile();
            return true;
        }
        return false;
    }

    // Helper function to convert gender char to string for display
    string getGenderString(char gender) {
        switch (toupper(gender)) {
        case 'M': return "Male";
        case 'F': return "Female";
        case 'O': return "Other";
        default: return "Unknown";
        }
    }
};

#endif