#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <algorithm>

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
    #define closesocket close
#endif

#include "config.h"
#include "voter.h"
#include "candidate.h"
#include "voting_manager.h"

using namespace std;

mutex voterMutex;
mutex candidateMutex;

VoterHashTable* voters = nullptr;
CandidateBTree* candidates = nullptr;
VotingManager* vm = nullptr;

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

string handleRequest(const string& request) {
    stringstream ss(request);
    string command;
    ss >> command;

    if (command == "REGISTER_VOTER") {
        string name, cnic, gender, contact, town, password;
        getline(ss, name, '|');
        getline(ss, cnic, '|');
        getline(ss, gender, '|');
        getline(ss, contact, '|');
        getline(ss, town, '|');
        getline(ss, password, '|');

        name = name.substr(1);

        lock_guard<mutex> lock(voterMutex);
        
        if (voters->searchByCNIC(cnic)) {
            return "{\"status\":\"error\",\"message\":\"CNIC already registered\"}";
        }

        voters->insertVoter(name, cnic, gender, contact, town, password);
        Voter* v = voters->searchByCNIC(cnic);
        
        if (v) {
            return "{\"status\":\"success\",\"voterID\":\"" + jsonEscape(v->voterID) + 
                   "\",\"station\":\"" + jsonEscape(v->pollingStation) + "\"}";
        }
        return "{\"status\":\"error\",\"message\":\"Registration failed\"}";
    }
    
    else if (command == "LOGIN_VOTER") {
        string id, password;
        getline(ss, id, '|');
        getline(ss, password, '|');
        id = id.substr(1);

        lock_guard<mutex> lock(voterMutex);
        Voter* v = voters->searchForLogin(id);
        
        if (v && v->password == password) {
            return "{\"status\":\"success\",\"voterID\":\"" + jsonEscape(v->voterID) + 
                   "\",\"name\":\"" + jsonEscape(v->name) + 
                   "\",\"station\":\"" + jsonEscape(v->pollingStation) + 
                   "\",\"town\":\"" + jsonEscape(v->town) + 
                   "\",\"hasVoted\":" + (v->hasVoted ? "true" : "false") + "}";
        }
        return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
    }
    
    else if (command == "GET_CANDIDATES") {
        string station;
        getline(ss, station);
        station = station.substr(1);

        lock_guard<mutex> lock(candidateMutex);
        vector<CandidateNode> cands = candidates->getCandidatesByStation(station);
        
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
    
    else if (command == "CAST_VOTE") {
        string voterID, candidateID;
        getline(ss, voterID, '|');
        getline(ss, candidateID, '|');
        voterID = voterID.substr(1);

        lock_guard<mutex> lock1(voterMutex);
        lock_guard<mutex> lock2(candidateMutex);
        
        Voter* v = voters->searchVoter(voterID);
        if (!v) {
            return "{\"status\":\"error\",\"message\":\"Voter not found\"}";
        }
        if (v->hasVoted) {
            return "{\"status\":\"error\",\"message\":\"Already voted\"}";
        }

        CandidateNode* cand = candidates->getCandidate(candidateID);
        if (!cand) {
            return "{\"status\":\"error\",\"message\":\"Candidate not found\"}";
        }
        
        if (string(cand->pollingStation) != v->pollingStation) {
            delete cand;
            return "{\"status\":\"error\",\"message\":\"Candidate not in your station\"}";
        }
        
        delete cand;
        candidates->voteCandidate(candidateID);
        voters->markAsVoted(voterID);
        
        return "{\"status\":\"success\",\"message\":\"Vote cast successfully\"}";
    }
    
    else if (command == "REGISTER_CANDIDATE") {
        string name, cnic, party, secretCode, town, password;
        getline(ss, name, '|');
        getline(ss, cnic, '|');
        getline(ss, party, '|');
        getline(ss, secretCode, '|');
        getline(ss, town, '|');
        getline(ss, password, '|');
        name = name.substr(1);

        lock_guard<mutex> lock(candidateMutex);
        bool success = candidates->registerCandidate(name, cnic, party, secretCode, town, password);
        
        if (success) {
            return "{\"status\":\"success\",\"message\":\"Registration successful\"}";
        }
        return "{\"status\":\"error\",\"message\":\"Registration failed\"}";
    }
    
    else if (command == "LOGIN_CANDIDATE") {
        string id, password;
        getline(ss, id, '|');
        getline(ss, password, '|');
        id = id.substr(1);

        lock_guard<mutex> lock(candidateMutex);
        if (candidates->verifyCandidatePassword(id, password)) {
            CandidateNode* c = candidates->getCandidate(id);
            if (c) {
                string json = "{\"status\":\"success\",\"id\":\"" + jsonEscape(string(c->candidateID)) +
                             "\",\"name\":\"" + jsonEscape(string(c->name)) +
                             "\",\"party\":\"" + jsonEscape(string(c->party)) +
                             "\",\"station\":\"" + jsonEscape(string(c->pollingStation)) +
                             "\",\"votes\":" + to_string(c->voteCount) + "}";
                delete c;
                return json;
            }
        }
        return "{\"status\":\"error\",\"message\":\"Invalid credentials\"}";
    }
    
    else if (command == "GET_RESULTS") {
        lock_guard<mutex> lock(candidateMutex);
        vector<CandidateNode> allCands = candidates->getAllCandidates();
        
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
    
    else if (command == "GET_TOWNS") {
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

    return "{\"status\":\"error\",\"message\":\"Unknown command\"}";
}

void handleClient(SOCKET clientSocket) {
    char buffer[4096];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        string request(buffer);
        string response = handleRequest(request);
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    closesocket(clientSocket);
}

int main() {
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }
    #endif

    cout << "Initializing Voting System...\n";
    voters = new VoterHashTable(DEFAULT_HASH_TABLE_SIZE);
    candidates = new CandidateBTree();
    vm = new VotingManager(DEFAULT_VOTING_DURATION);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed\n";
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed\n";
        closesocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
        cerr << "Listen failed\n";
        closesocket(serverSocket);
        return 1;
    }

    cout << "Server running on port 8080...\n";

    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);
        
        if (clientSocket != INVALID_SOCKET) {
            thread clientThread(handleClient, clientSocket);
            clientThread.detach();
        }
    }

    delete voters;
    delete candidates;
    delete vm;
    closesocket(serverSocket);
    
    #ifdef _WIN32
    WSACleanup();
    #endif
    
    return 0;
}