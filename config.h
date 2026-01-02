#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cstring>

using namespace std;

// Voter Structure
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
        string phone = "", string city = "", string station = "",
        string pass = "", bool voted = false) {
        voterID = id;
        name = n;
        cnic = c;
        gender = g;
        contactNumber = phone;
        town = city;
        pollingStation = station;
        password = pass;
        hasVoted = voted;
    }
};

// =============================================
// TOWN-STATION STRUCTURE
// =============================================
struct TownStation {
    string town;
    vector<string> stations;
};

class GeographicConfig {
private:
    static const TownStation TOWN_STATIONS[];
    static const int TOWN_STATIONS_COUNT;
public:
    // Get all polling stations for a specific town
    static vector<string> getStationsForTown(const string& town) {
        for (int i = 0; i < TOWN_STATIONS_COUNT; i++) {
            if (TOWN_STATIONS[i].town == town) {
                return TOWN_STATIONS[i].stations;
            }
        }
        // Fallback to Default if town not found
        return getDefaultStations();
    }

    // Check if a town is valid (exists in configuration)
    static bool isValidTown(const string& town) {
        for (int i = 0; i < TOWN_STATIONS_COUNT; i++) {
            if (TOWN_STATIONS[i].town == town) {
                return true;
            }
        }
        return false;
    }

    // Get list of all available towns
    static vector<string> getAllTowns() {
        vector<string> towns;
        for (int i = 0; i < TOWN_STATIONS_COUNT; i++) {
            towns.push_back(TOWN_STATIONS[i].town);
        }
        return towns;
    }

    // Get default polling stations (for unknown towns)
    static vector<string> getDefaultStations() {
        for (int i = 0; i < TOWN_STATIONS_COUNT; i++) {
            if (TOWN_STATIONS[i].town == "Default") {
                return TOWN_STATIONS[i].stations;
            }
        }
        return { "DEF01" }; // Ultimate fallback
    }

    // Get all unique polling stations across all towns
    static vector<string> getAllPollingStations() {
        vector<string> allStations;
        for (int i = 0; i < TOWN_STATIONS_COUNT; i++) {
            for (const auto& station : TOWN_STATIONS[i].stations) {
                bool found = false;
                for (const auto& existing : allStations) {
                    if (existing == station) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    allStations.push_back(station);
                }
            }
        }
        return allStations;
    }
};

// =============================================
// SYSTEM CONFIGURATION CONSTANTS
// =============================================

// Voter Configuration
const int DEFAULT_HASH_TABLE_SIZE = 20000;
const int VOTER_ID_LENGTH = 10;  // VID00001 format
const int MIN_PASSWORD_LENGTH = 6;
const int CNIC_LENGTH = 13;
const int CONTACT_LENGTH = 11;

// Candidate Configuration
const int MAX_BLOCKS = 1024;
const char CANDIDATE_DB_FILE[] = "candidates.bin";

// Candidate Field Sizes
const int ID_SIZE = 10;           // CID00001 format
const int NAME_SIZE = 50;
const int CNIC_SIZE = 14;         // 13 digits + null
const int PARTY_SIZE = 15;
const int STATION_SIZE = 10;      // KHI01 format
const int TOWN_SIZE = 20;         // Added: For town name storage
const int PASS_SIZE = 20;

// B-tree Configuration
const int MIN_DEGREE = 3;
const int MAX_KEYS = 2 * MIN_DEGREE - 1;
const int MIN_KEYS = MIN_DEGREE - 1;

// Voting System Configuration
const int DEFAULT_VOTING_DURATION = 300;  // 5 minutes in seconds
const string DEFAULT_ADMIN_PASSWORD = "admin123";

// Election Rules Configuration
const int MAX_CANDIDATES_PER_STATION = 10; // Maximum candidates per station
const bool ONE_PARTY_PER_STATION = true;   // Rule: Only one candidate per party per station

// =============================================
// DATA STRUCTURES
// =============================================

// Candidate Node Structure (for B-tree) - MODIFIED
struct CandidateNode {
    char candidateID[ID_SIZE];
    char name[NAME_SIZE];
    char cnic[CNIC_SIZE];
    char party[PARTY_SIZE];
    char town[TOWN_SIZE];         // ADDED: Town field
    char pollingStation[STATION_SIZE];
    char password[PASS_SIZE];
    int voteCount;

    CandidateNode() : voteCount(0) {
        memset(candidateID, 0, ID_SIZE);
        memset(name, 0, NAME_SIZE);
        memset(cnic, 0, CNIC_SIZE);
        memset(party, 0, PARTY_SIZE);
        memset(town, 0, TOWN_SIZE);        // Initialize town
        memset(pollingStation, 0, STATION_SIZE);
        memset(password, 0, PASS_SIZE);
    }

    // MODIFIED Constructor to include town - GCC COMPATIBLE VERSION
    CandidateNode(string id, string n, string c,
        string p, string townName, string station,  // Added townName parameter
        string pass)
        : voteCount(0) {

        // GCC-compatible version
        strncpy(candidateID, id.c_str(), ID_SIZE - 1);
        candidateID[ID_SIZE - 1] = '\0';

        strncpy(name, n.c_str(), NAME_SIZE - 1);
        name[NAME_SIZE - 1] = '\0';

        strncpy(cnic, c.c_str(), CNIC_SIZE - 1);
        cnic[CNIC_SIZE - 1] = '\0';

        strncpy(party, p.c_str(), PARTY_SIZE - 1);
        party[PARTY_SIZE - 1] = '\0';

        // ADDED: Store town
        strncpy(town, townName.c_str(), TOWN_SIZE - 1);
        town[TOWN_SIZE - 1] = '\0';

        strncpy(pollingStation, station.c_str(), STATION_SIZE - 1);
        pollingStation[STATION_SIZE - 1] = '\0';

        strncpy(password, pass.c_str(), PASS_SIZE - 1);
        password[PASS_SIZE - 1] = '\0';
    }
};

// B-tree Node Structure
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

// =============================================
// PARTY SECRET CODES
// =============================================

const map<string, string> PARTY_SECRET_CODES = {
    {"PMLN", "pmln123"},
    {"PPP", "ppp123"},
    {"MQM", "mqm123"},
    {"ANP", "anp123"},
    {"JUI", "jui123"},
    {"PTI", "pti123"},
    {"TLP", "tlp123"},
    {"BAP", "bap123"},
    {"GDA", "gda123"},
    {"JI", "ji123"}
};

// Define the static members at the end of the file
const TownStation GeographicConfig::TOWN_STATIONS[] = {
    {"Karachi", {"KHI01","KHI02","KHI03"}},
    {"Lahore", {"LHR01","LHR02","LHR03"}},
    {"Islamabad", {"ISB01","ISB02"}},
    {"Rawalpindi", {"RWP01"}},
    {"Faisalabad", {"FSD01"}},
    {"Multan", {"MUL01"}},
    {"Peshawar", {"PES01"}},
    {"Quetta", {"QTA01"}},
};

const int GeographicConfig::TOWN_STATIONS_COUNT = 8;

#endif // CONFIG_H