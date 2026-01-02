// #include <iostream>
// #include <string>
// #include <sstream>
// #include <cstring>
// #include <vector>
// #include <map>
// #include <algorithm>

// #ifdef _WIN32
//     #include <winsock2.h>
//     #include <ws2tcpip.h>
//     #pragma comment(lib, "ws2_32.lib")
//     typedef int socklen_t;
//     // REMOVED: #define close closesocket
// #else
//     #include <sys/socket.h>
//     #include <netinet/in.h>
//     #include <arpa/inet.h>
//     #include <unistd.h>
//     #define SOCKET int
//     #define INVALID_SOCKET -1
//     #define SOCKET_ERROR -1
// #endif

// #include "config.h"
// #include "voter.h"
// #include "candidate.h"
// #include "voting_manager.h"

// using namespace std;

// // Global Data
// VoterHashTable* votersDB = nullptr;
// CandidateBTree* candidatesDB = nullptr;
// VotingManager* votingManager = nullptr;

// string jsonEscape(const string& s) {
//     string result;
//     for (char c : s) {
//         switch (c) {
//             case '"': result += "\\\""; break;
//             case '\\': result += "\\\\"; break;
//             case '\n': result += "\\n"; break;
//             case '\r': result += "\\r"; break;
//             case '\t': result += "\\t"; break;
//             default: result += c;
//         }
//     }
//     return result;
// }

// vector<string> split(const string& str, char delimiter) {
//     vector<string> tokens;
//     stringstream ss(str);
//     string token;
//     while (getline(ss, token, delimiter)) {
//         tokens.push_back(token);
//     }
//     return tokens;
// }

// string trim(const string& str) {
//     size_t first = str.find_first_not_of(" \t\n\r");
//     if (first == string::npos) return "";
//     size_t last = str.find_last_not_of(" \t\n\r");
//     return str.substr(first, last - first + 1);
// }

// string handleRegisterVoter(const vector<string>& params) {
//     if (params.size() < 6) {
//         return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
//     }

//     string name = trim(params[0]);
//     string cnic = trim(params[1]);
//     string gender = trim(params[2]);
//     string contact = trim(params[3]);
//     string town = trim(params[4]);
//     string password = trim(params[5]);

//     if (votersDB->searchByCNIC(cnic) != nullptr) {
//         return "{\"status\":\"error\",\"message\":\"CNIC already registered\"}";
//     }

//     votersDB->insertVoter(name, cnic, gender, contact, town, password);
//     Voter* v = votersDB->searchByCNIC(cnic);
    
//     if (v) {
//         stringstream json;
//         json << "{\"status\":\"success\","
//              << "\"voterID\":\"" << jsonEscape(v->voterID) << "\","
//              << "\"station\":\"" << jsonEscape(v->pollingStation) << "\","
//              << "\"town\":\"" << jsonEscape(v->town) << "\"}";
//         return json.str();
//     }

//     return "{\"status\":\"error\",\"message\":\"Registration failed\"}";
// }

// string handleLoginVoter(const vector<string>& params) {
//     if (params.size() < 2) {
//         return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
//     }

//     string id = trim(params[0]);
//     string password = trim(params[1]);

//     Voter* v = votersDB->searchForLogin(id);
//     if (v && v->password == password) {
//         stringstream json;
//         json << "{\"status\":\"success\","
//              << "\"voterID\":\"" << jsonEscape(v->voterID) << "\","
//              << "\"name\":\"" << jsonEscape(v->name) << "\","
//              << "\"cnic\":\"" << jsonEscape(v->cnic) << "\","
//              << "\"station\":\"" << jsonEscape(v->pollingStation) << "\","
//              << "\"town\":\"" << jsonEscape(v->town) << "\","
//              << "\"hasVoted\":" << (v->hasVoted ? "true" : "false") << "}";
//         return json.str();
//     }

//     return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
// }

// string handleGetCandidates(const vector<string>& params) {
//     if (params.size() < 1) {
//         return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
//     }

//     string station = trim(params[0]);
//     vector<CandidateNode> cands = candidatesDB->getCandidatesByStation(station);

//     stringstream json;
//     json << "{\"status\":\"success\",\"candidates\":[";
    
//     for (size_t i = 0; i < cands.size(); i++) {
//         if (i > 0) json << ",";
//         json << "{\"id\":\"" << jsonEscape(string(cands[i].candidateID)) << "\","
//              << "\"name\":\"" << jsonEscape(string(cands[i].name)) << "\","
//              << "\"party\":\"" << jsonEscape(string(cands[i].party)) << "\","
//              << "\"votes\":" << cands[i].voteCount << "}";
//     }
    
//     json << "]}";
//     return json.str();
// }

// string handleCastVote(const vector<string>& params) {
//     if (params.size() < 2) {
//         return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
//     }

//     string voterID = trim(params[0]);
//     string candidateID = trim(params[1]);

//     Voter* v = votersDB->searchVoter(voterID);
//     if (!v) {
//         return "{\"status\":\"error\",\"message\":\"Voter not found\"}";
//     }

//     if (v->hasVoted) {
//         return "{\"status\":\"error\",\"message\":\"Already voted\"}";
//     }

//     CandidateNode* cand = candidatesDB->getCandidate(candidateID);
//     if (!cand) {
//         return "{\"status\":\"error\",\"message\":\"Candidate not found\"}";
//     }

//     if (string(cand->pollingStation) != v->pollingStation) {
//         delete cand;
//         return "{\"status\":\"error\",\"message\":\"Candidate not in your station\"}";
//     }

//     delete cand;
//     candidatesDB->voteCandidate(candidateID);
//     votersDB->markAsVoted(voterID);

//     return "{\"status\":\"success\",\"message\":\"Vote cast successfully\"}";
// }

// string handleRegisterCandidate(const vector<string>& params) {
//     if (params.size() < 6) {
//         return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
//     }

//     string name = trim(params[0]);
//     string cnic = trim(params[1]);
//     string party = trim(params[2]);
//     string secretCode = trim(params[3]);
//     string town = trim(params[4]);
//     string password = trim(params[5]);

//     bool success = candidatesDB->registerCandidate(name, cnic, party, secretCode, town, password);

//     if (success) {
//         return "{\"status\":\"success\",\"message\":\"Registration successful\"}";
//     }

//     return "{\"status\":\"error\",\"message\":\"Registration failed\"}";
// }

// string handleLoginCandidate(const vector<string>& params) {
//     if (params.size() < 2) {
//         return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
//     }

//     string id = trim(params[0]);
//     string password = trim(params[1]);

//     if (candidatesDB->verifyCandidatePassword(id, password)) {
//         CandidateNode* c = candidatesDB->getCandidate(id);
//         if (c) {
//             stringstream json;
//             json << "{\"status\":\"success\","
//                  << "\"id\":\"" << jsonEscape(string(c->candidateID)) << "\","
//                  << "\"name\":\"" << jsonEscape(string(c->name)) << "\","
//                  << "\"party\":\"" << jsonEscape(string(c->party)) << "\","
//                  << "\"station\":\"" << jsonEscape(string(c->pollingStation)) << "\","
//                  << "\"votes\":" << c->voteCount << "}";
//             delete c;
//             return json.str();
//         }
//     }

//     return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
// }

// string handleGetResults(const vector<string>& params) {
//     vector<CandidateNode> allCands = candidatesDB->getAllCandidates();

//     stringstream json;
//     json << "{\"status\":\"success\",\"results\":[";

//     for (size_t i = 0; i < allCands.size(); i++) {
//         if (i > 0) json << ",";
//         json << "{\"id\":\"" << jsonEscape(string(allCands[i].candidateID)) << "\","
//              << "\"name\":\"" << jsonEscape(string(allCands[i].name)) << "\","
//              << "\"party\":\"" << jsonEscape(string(allCands[i].party)) << "\","
//              << "\"station\":\"" << jsonEscape(string(allCands[i].pollingStation)) << "\","
//              << "\"votes\":" << allCands[i].voteCount << "}";
//     }

//     json << "]}";
//     return json.str();
// }

// string handleGetTowns(const vector<string>& params) {
//     vector<string> towns = GeographicConfig::getAllTowns();

//     stringstream json;
//     json << "{\"status\":\"success\",\"towns\":[";

//     for (size_t i = 0; i < towns.size(); i++) {
//         if (i > 0) json << ",";
//         json << "\"" << jsonEscape(towns[i]) << "\"";
//     }

//     json << "]}";
//     return json.str();
// }

// string handleRequest(const string& request) {
//     stringstream ss(request);
//     string command;
//     ss >> command;

//     string rest;
//     getline(ss, rest);
//     rest = trim(rest);

//     vector<string> params = split(rest, '|');

//     cout << "[REQUEST] " << command << endl;

//     try {
//         if (command == "REGISTER_VOTER") return handleRegisterVoter(params);
//         else if (command == "LOGIN_VOTER") return handleLoginVoter(params);
//         else if (command == "GET_CANDIDATES") return handleGetCandidates(params);
//         else if (command == "CAST_VOTE") return handleCastVote(params);
//         else if (command == "REGISTER_CANDIDATE") return handleRegisterCandidate(params);
//         else if (command == "LOGIN_CANDIDATE") return handleLoginCandidate(params);
//         else if (command == "GET_RESULTS") return handleGetResults(params);
//         else if (command == "GET_TOWNS") return handleGetTowns(params);
//         else return "{\"status\":\"error\",\"message\":\"Unknown command\"}";
//     }
//     catch (const exception& e) {
//         return "{\"status\":\"error\",\"message\":\"Exception occurred\"}";
//     }
// }

// int main(int argc, char* argv[]) {
//     int port = 8080;

//     #ifdef _WIN32
//     WSADATA wsaData;
//     WSAStartup(MAKEWORD(2, 2), &wsaData);
//     #endif

//     cout << "Initializing system..." << endl;
//     votersDB = new VoterHashTable(DEFAULT_HASH_TABLE_SIZE);
//     candidatesDB = new CandidateBTree();
//     votingManager = new VotingManager(DEFAULT_VOTING_DURATION);

//     SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    
//     int opt = 1;
//     setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

//     sockaddr_in serverAddr;
//     serverAddr.sin_family = AF_INET;
//     serverAddr.sin_port = htons(port);
//     serverAddr.sin_addr.s_addr = INADDR_ANY;

//     if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
//         cerr << "Bind failed!" << endl;
//         return 1;
//     }

//     listen(serverSocket, 5);

//     cout << "========================================" << endl;
//     cout << "  SERVER RUNNING ON PORT " << port << endl;
//     cout << "========================================" << endl;

//     while (true) {
//         sockaddr_in clientAddr;
//         socklen_t clientSize = sizeof(clientAddr);
//         SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

//         if (clientSocket != INVALID_SOCKET) {
//             cout << "[CLIENT CONNECTED]" << endl;

//             char buffer[8192];
//             memset(buffer, 0, sizeof(buffer));
//             int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

//             if (bytesReceived > 0) {
//                 buffer[bytesReceived] = '\0';
//                 string response = handleRequest(string(buffer));
//                 send(clientSocket, response.c_str(), response.length(), 0);
//                 cout << "[RESPONSE SENT]" << endl;
//             }

//             // Platform-specific socket closing
//             #ifdef _WIN32
//                 closesocket(clientSocket);
//             #else
//                 close(clientSocket);
//             #endif
//         }
//     }

//     delete votersDB;
//     delete candidatesDB;
//     delete votingManager;
    
//     // Platform-specific socket closing for server socket
//     #ifdef _WIN32
//         closesocket(serverSocket);
//     #else
//         close(serverSocket);
//     #endif

//     #ifdef _WIN32
//     WSACleanup();
//     #endif

//     return 0;
// }

// server_main.cpp - Complete working server
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include <algorithm>
#include <ctime>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#include "config.h"
#include "voter.h"
#include "candidate.h"
#include "voting_manager.h"

using namespace std;

// Global Data
VoterHashTable* votersDB = nullptr;
CandidateBTree* candidatesDB = nullptr;
VotingManager* votingManager = nullptr;

string jsonEscape(const string& s) {
    string result;
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

vector<string> split(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

string trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

string handleRegisterVoter(const vector<string>& params) {
    if (params.size() < 6) {
        return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    }

    string name = trim(params[0]);
    string cnic = trim(params[1]);
    string gender = trim(params[2]);
    string contact = trim(params[3]);
    string town = trim(params[4]);
    string password = trim(params[5]);

    if (votersDB->searchByCNIC(cnic) != nullptr) {
        return "{\"status\":\"error\",\"message\":\"CNIC already registered\"}";
    }

    bool success = votersDB->insertVoter(name, cnic, gender, contact, town, password);
    
    if (success) {
        Voter* v = votersDB->searchByCNIC(cnic);
        if (v) {
            stringstream json;
            json << "{\"status\":\"success\","
                 << "\"voterID\":\"" << jsonEscape(v->voterID) << "\","
                 << "\"name\":\"" << jsonEscape(v->name) << "\","
                 << "\"station\":\"" << jsonEscape(v->pollingStation) << "\","
                 << "\"town\":\"" << jsonEscape(v->town) << "\"}";
            return json.str();
        }
    }

    return "{\"status\":\"error\",\"message\":\"Registration failed\"}";
}

string handleLoginVoter(const vector<string>& params) {
    if (params.size() < 2) {
        return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    }

    string id = trim(params[0]);
    string password = trim(params[1]);

    Voter* v = votersDB->searchForLogin(id);
    if (v && v->password == password) {
        stringstream json;
        json << "{\"status\":\"success\","
             << "\"voterID\":\"" << jsonEscape(v->voterID) << "\","
             << "\"name\":\"" << jsonEscape(v->name) << "\","
             << "\"cnic\":\"" << jsonEscape(v->cnic) << "\","
             << "\"station\":\"" << jsonEscape(v->pollingStation) << "\","
             << "\"town\":\"" << jsonEscape(v->town) << "\","
             << "\"hasVoted\":" << (v->hasVoted ? "true" : "false") << "}";
        return json.str();
    }

    return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
}

string handleGetCandidates(const vector<string>& params) {
    if (params.size() < 1) {
        return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    }

    string station = trim(params[0]);
    vector<CandidateNode> cands = candidatesDB->getCandidatesByStation(station);

    stringstream json;
    json << "{\"status\":\"success\",\"candidates\":[";
    
    for (size_t i = 0; i < cands.size(); i++) {
        if (i > 0) json << ",";
        json << "{\"id\":\"" << jsonEscape(string(cands[i].candidateID)) << "\","
             << "\"name\":\"" << jsonEscape(string(cands[i].name)) << "\","
             << "\"party\":\"" << jsonEscape(string(cands[i].party)) << "\","
             << "\"votes\":" << cands[i].voteCount << "}";
    }
    
    json << "]}";
    return json.str();
}

string handleCastVote(const vector<string>& params) {
    if (params.size() < 2) {
        return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    }

    string voterID = trim(params[0]);
    string candidateID = trim(params[1]);

    Voter* v = votersDB->searchVoter(voterID);
    if (!v) {
        return "{\"status\":\"error\",\"message\":\"Voter not found\"}";
    }

    if (v->hasVoted) {
        return "{\"status\":\"error\",\"message\":\"Already voted\"}";
    }

    CandidateNode* cand = candidatesDB->getCandidate(candidateID);
    if (!cand) {
        return "{\"status\":\"error\",\"message\":\"Candidate not found\"}";
    }

    if (string(cand->pollingStation) != v->pollingStation) {
        delete cand;
        return "{\"status\":\"error\",\"message\":\"Candidate not in your station\"}";
    }

    delete cand;
    candidatesDB->voteCandidate(candidateID);
    votersDB->markAsVoted(voterID);

    return "{\"status\":\"success\",\"message\":\"Vote cast successfully\"}";
}

string handleRegisterCandidate(const vector<string>& params) {
    if (params.size() < 6) {
        return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    }

    string name = trim(params[0]);
    string cnic = trim(params[1]);
    string party = trim(params[2]);
    string secretCode = trim(params[3]);
    string town = trim(params[4]);
    string password = trim(params[5]);

    bool success = candidatesDB->registerCandidate(name, cnic, party, secretCode, town, password);

    if (success) {
        return "{\"status\":\"success\",\"message\":\"Registration successful\"}";
    }

    return "{\"status\":\"error\",\"message\":\"Registration failed\"}";
}

string handleLoginCandidate(const vector<string>& params) {
    if (params.size() < 2) {
        return "{\"status\":\"error\",\"message\":\"Invalid parameters\"}";
    }

    string id = trim(params[0]);
    string password = trim(params[1]);

    if (candidatesDB->verifyCandidatePassword(id, password)) {
        CandidateNode* c = candidatesDB->getCandidate(id);
        if (c) {
            stringstream json;
            json << "{\"status\":\"success\","
                 << "\"id\":\"" << jsonEscape(string(c->candidateID)) << "\","
                 << "\"name\":\"" << jsonEscape(string(c->name)) << "\","
                 << "\"party\":\"" << jsonEscape(string(c->party)) << "\","
                 << "\"station\":\"" << jsonEscape(string(c->pollingStation)) << "\","
                 << "\"votes\":" << c->voteCount << "}";
            delete c;
            return json.str();
        }
    }

    return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
}

string handleGetResults(const vector<string>& params) {
    vector<CandidateNode> allCands = candidatesDB->getAllCandidates();

    stringstream json;
    json << "{\"status\":\"success\",\"results\":[";

    for (size_t i = 0; i < allCands.size(); i++) {
        if (i > 0) json << ",";
        json << "{\"id\":\"" << jsonEscape(string(allCands[i].candidateID)) << "\","
             << "\"name\":\"" << jsonEscape(string(allCands[i].name)) << "\","
             << "\"party\":\"" << jsonEscape(string(allCands[i].party)) << "\","
             << "\"station\":\"" << jsonEscape(string(allCands[i].pollingStation)) << "\","
             << "\"votes\":" << allCands[i].voteCount << "}";
    }

    json << "]}";
    return json.str();
}

string handleGetTowns(const vector<string>& params) {
    vector<string> towns = GeographicConfig::getAllTowns();

    stringstream json;
    json << "{\"status\":\"success\",\"towns\":[";

    for (size_t i = 0; i < towns.size(); i++) {
        if (i > 0) json << ",";
        json << "\"" << jsonEscape(towns[i]) << "\"";
    }

    json << "]}";
    return json.str();
}

string handleRequest(const string& request) {
    stringstream ss(request);
    string command;
    ss >> command;

    string rest;
    getline(ss, rest);
    rest = trim(rest);

    vector<string> params = split(rest, '|');

    cout << "[SERVER] Received command: " << command << endl;

    try {
        if (command == "REGISTER_VOTER") return handleRegisterVoter(params);
        else if (command == "LOGIN_VOTER") return handleLoginVoter(params);
        else if (command == "GET_CANDIDATES") return handleGetCandidates(params);
        else if (command == "CAST_VOTE") return handleCastVote(params);
        else if (command == "REGISTER_CANDIDATE") return handleRegisterCandidate(params);
        else if (command == "LOGIN_CANDIDATE") return handleLoginCandidate(params);
        else if (command == "GET_RESULTS") return handleGetResults(params);
        else if (command == "GET_TOWNS") return handleGetTowns(params);
        else if (command == "PING") return "{\"status\":\"success\",\"message\":\"Server is running\"}";
        else return "{\"status\":\"error\",\"message\":\"Unknown command\"}";
    }
    catch (const exception& e) {
        return "{\"status\":\"error\",\"message\":\"Exception occurred: " + string(e.what()) + "\"}";
    }
}

int main(int argc, char* argv[]) {
    int port = 8080;

    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return 1;
    }
    #endif

    cout << "========================================" << endl;
    cout << "  ONLINE VOTING SYSTEM SERVER" << endl;
    cout << "  Starting on port " << port << endl;
    cout << "========================================" << endl;

    // Initialize system components
    cout << "Initializing system..." << endl;
    
    try {
        votersDB = new VoterHashTable(20000);
        candidatesDB = new CandidateBTree();
        votingManager = new VotingManager(300); // 5 minutes voting duration
        
        cout << "✓ Voter database loaded" << endl;
        cout << "✓ Candidate database loaded" << endl;
        cout << "✓ Voting manager initialized" << endl;
    } catch (const exception& e) {
        cerr << "✗ Initialization failed: " << e.what() << endl;
        return 1;
    }

    // Create socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "✗ Socket creation failed!" << endl;
        return 1;
    }
    
    // Set socket options
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    // Bind socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "✗ Bind failed on port " << port << "!" << endl;
        #ifdef _WIN32
        closesocket(serverSocket);
        #else
        close(serverSocket);
        #endif
        return 1;
    }

    // Listen for connections
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cerr << "✗ Listen failed!" << endl;
        #ifdef _WIN32
        closesocket(serverSocket);
        #else
        close(serverSocket);
        #endif
        return 1;
    }

    cout << "\n✓ Server running on http://localhost:" << port << endl;
    cout << "✓ Ready to accept connections..." << endl;
    cout << "========================================\n" << endl;

    // Main server loop
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket != INVALID_SOCKET) {
            char clientIP[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
            
            cout << "[CLIENT] Connection from " << clientIP << endl;
            
            char buffer[8192];
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

            if (bytesReceived > 0) {
                buffer[bytesReceived] = '\0';
                cout << "[REQUEST] " << string(buffer).substr(0, 100) << "..." << endl;
                
                string response = handleRequest(string(buffer));
                send(clientSocket, response.c_str(), response.length(), 0);
                
                cout << "[RESPONSE] Sent " << response.length() << " bytes" << endl;
            }

            // Close client socket
            #ifdef _WIN32
                closesocket(clientSocket);
            #else
                close(clientSocket);
            #endif
            
            cout << "[CLIENT] Connection closed" << endl;
        }
    }

    // Cleanup (never reached in this loop, but kept for completeness)
    delete votersDB;
    delete candidatesDB;
    delete votingManager;
    
    #ifdef _WIN32
        closesocket(serverSocket);
        WSACleanup();
    #else
        close(serverSocket);
    #endif

    return 0;
}