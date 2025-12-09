#ifndef VOTER_H
#define VOTER_H
#include <iostream>
#include <vector>
#include <iomanip>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;

struct Voter {
    string voterID;
    string name;
    string cnic;
    string gender;
    string contactNumber;
    string town;
    string pollingStation;
    string username;
    string password;
    bool hasVoted;

    Voter(string id = "", string n = "", string c = "", string g = "",
        string phone = "", string t = "", string station = "",
        string user = "", string pass = "", bool voted = false) {
        voterID = id;
        name = n;
        cnic = c;
        gender = g;
        contactNumber = phone;
        town = t;
        pollingStation = station;
        username = user;
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

    string generateVoterID(int seqNumber, string town, string station) {
        string townCode = "";
        if (town.length() >= 2) {
            townCode += toupper(town[0]);
            townCode += toupper(town[1]);
        }
        else {
            townCode = "XX";
        }

        string stationCode = "";
        if (station.length() >= 2) {
            stationCode = station.substr(station.length() - 2);
        }
        else {
            stationCode = "01";
        }

        stringstream ss;
        ss << "V" << setw(5) << setfill('0') << seqNumber << townCode << stationCode;
        return ss.str();
    }

    int getNextVoterSequence() {
        int maxSeq = 0;
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->voterID.length() >= 6 && v->voterID[0] == 'V') {
                    string seqStr = v->voterID.substr(1, 5);
                    bool isNumber = true;
                    for (char c : seqStr) {
                        if (!isdigit(c)) {
                            isNumber = false;
                            break;
                        }
                    }
                    if (isNumber) {
                        int seq = 0;
                        for (char c : seqStr) {
                            seq = seq * 10 + (c - '0');
                        }
                        if (seq > maxSeq) maxSeq = seq;
                    }
                }
            }
        }
        return maxSeq + 1;
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
        writeString(v->gender);
        writeString(v->contactNumber);
        writeString(v->town);
        writeString(v->pollingStation);
        writeString(v->username);
        writeString(v->password);
        out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
        out.close();
    }

    void loadFromFile() {
        ifstream in(FILENAME, ios::binary);
        if (!in) return;

        while (in.peek() != EOF) {
            Voter* v = new Voter();
            size_t len;
            char buffer[256];

            auto readString = [&in, &len, &buffer]() -> string {
                in.read((char*)&len, sizeof(len));
                if (len >= sizeof(buffer)) len = sizeof(buffer) - 1;
                in.read(buffer, len);
                buffer[len] = '\0';
                return string(buffer);
                };

            v->voterID = readString();
            v->name = readString();
            v->cnic = readString();
            v->gender = readString();
            v->contactNumber = readString();
            v->town = readString();
            v->pollingStation = readString();
            v->username = readString();
            v->password = readString();
            in.read((char*)&v->hasVoted, sizeof(v->hasVoted));

            int index = hashFunc(v->voterID);
            table[index].push_back(v);
        }
        in.close();
    }

    void saveAllToFile() {
        ofstream out(FILENAME, ios::out | ios::binary);
        if (!out) return;

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
                writeString(v->gender);
                writeString(v->contactNumber);
                writeString(v->town);
                writeString(v->pollingStation);
                writeString(v->username);
                writeString(v->password);
                out.write((char*)&v->hasVoted, sizeof(v->hasVoted));
            }
        }
        out.close();
    }

    bool existsInVector(const vector<string>& vec, const string& str) {
        for (const string& s : vec) {
            if (s == str) return true;
        }
        return false;
    }

public:
    VoterHashTable(int size) {
        tableSize = size;
        table = new vector<Voter*>[tableSize];
        initializeTownStations();
        loadFromFile();
    }

    ~VoterHashTable() {
        saveAllToFile();
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                delete v;
            }
        }
        delete[] table;
    }

    void insertVoter(string name, string cnic, string gender,
        string contact, string town,
        string username = "", string password = "") {

        if (!validateCNIC(cnic)) {
            cout << "Error: Invalid CNIC! Must be 13 digits.\n";
            return;
        }

        if (!validateContact(contact)) {
            cout << "Error: Invalid contact number! Must be 11 digits starting with 03.\n";
            return;
        }

        if (searchByCNIC(cnic) != nullptr) {
            cout << "Error: CNIC '" << cnic << "' is already registered!\n";
            return;
        }

        string station = assignPollingStation(town, cnic);
        int seqNum = getNextVoterSequence();
        string voterID = generateVoterID(seqNum, town, station);

        if (username.empty()) username = cnic;
        if (password.empty()) password = "pass" + cnic.substr(7, 4);

        int index = hashFunc(voterID);
        Voter* v = new Voter(voterID, name, cnic, gender, contact,
            town, station, username, password, false);
        table[index].push_back(v);

        saveVoterToFile(v);

        cout << "\n========================================\n";
        cout << "  VOTER REGISTERED SUCCESSFULLY!\n";
        cout << "========================================\n";
        cout << "Voter ID:      " << voterID << "\n";
        cout << "Name:          " << name << "\n";
        cout << "Polling Station: " << station << "\n";
        cout << "Town:          " << town << "\n";
        cout << "Username:      " << username << "\n";
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

    Voter* searchByUsername(string username) {
        for (int i = 0; i < tableSize; i++) {
            for (auto v : table[i]) {
                if (v->username == username)
                    return v;
            }
        }
        return nullptr;
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
                cout << "Gender:        " << v->gender << "\n";
                cout << "Contact:       " << v->contactNumber << "\n";
                cout << "Town:          " << v->town << "\n";
                cout << "Polling Station: " << v->pollingStation << "\n";
                cout << "Username:      " << v->username << "\n";
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
                    cout << "   Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
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
                    cout << "   Voted: " << (v->hasVoted ? "Yes" : "No") << "\n";
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
        if (v) {
            v->password = newPassword;
            saveAllToFile();
            return true;
        }
        return false;
    }
};

#endif