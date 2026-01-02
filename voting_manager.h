#ifndef VOTING_MANAGER_H
#define VOTING_MANAGER_H

#include <iostream>
#include <ctime>
#include <string>
#include <iomanip>
#include <limits>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include "config.h"
#include "candidate.h"
#include "voter.h"
using namespace std;

class VotingManager {
private:
    time_t startTime;
    int durationSeconds;
    bool votingActive;
    string adminPassword;

    void clearInput() {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    void printHeader(string title) {
        cout << "\n" << string(60, '=') << "\n";
        cout << "   " << title << "\n";
        cout << string(60, '=') << "\n";
    }

public:
    VotingManager(int duration = DEFAULT_VOTING_DURATION, string adminPass = DEFAULT_ADMIN_PASSWORD) {
        durationSeconds = duration;
        votingActive = false;
        adminPassword = adminPass;
    }

    void startVoting() {
        startTime = time(nullptr);
        votingActive = true;
        printHeader("VOTING STARTED");
        cout << "Voting duration: " << (durationSeconds / 60) << " minutes.\n";
        cout << "Voting will end at: ";

        time_t endTime = startTime + durationSeconds;
        // GCC compatible version
        char* timeStr = ctime(&endTime);
        if (timeStr) {
            // Remove newline character
            string timeString(timeStr);
            if (!timeString.empty() && timeString.back() == '\n') {
                timeString.pop_back();
            }
            cout << timeString << "\n";
        }
    }

    bool canVote() {
        if (!votingActive) {
            cout << "Voting is not active!\n";
            return false;
        }

        time_t now = time(nullptr);
        if (difftime(now, startTime) > durationSeconds) {
            votingActive = false;
            printHeader("VOTING ENDED");
            cout << "Voting period has ended!\n";
            return false;
        }

        int remaining = durationSeconds - difftime(now, startTime);
        cout << "Time remaining: " << remaining / 60 << " minutes " << remaining % 60 << " seconds\n";
        return true;
    }

    void showVoterDashboard(Voter* v, VoterHashTable& voters, CandidateBTree& candidates) {
        bool logout = false;
        while (!logout) {
            printHeader("VOTER DASHBOARD");
            cout << "Welcome, " << v->name << "!\n";
            cout << "Voter ID: " << v->voterID << "\n";
            if (v->hasVoted) {
                cout << "Status: Already Voted\n";
            }
            else {
                cout << "Status: Not Voted Yet\n";
            }
            cout << string(50, '-') << "\n\n";

            int choice;
            cout << "1. View Profile\n";
            cout << "2. Cast Vote\n";
            cout << "3. View Result\n";
            cout << "4. Update Password\n";
            cout << "5. Logout\n";
            cout << "Enter your choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1: {
                voters.viewProfile(v);
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            case 2: {
                if (v->hasVoted) {
                    cout << "You have already voted!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                if (!canVote()) {
                    cout << "Voting is not allowed at this time.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                string voterStation = v->pollingStation;

                printHeader("CANDIDATES IN YOUR POLLING STATION");
                cout << "Candidates contesting from Station: " << voterStation << "\n";
                cout << string(80, '-') << "\n";

                candidates.printCandidatesByStation(voterStation);

                cout << "\nEnter Candidate ID to vote for: ";
                string candidateID;
                getline(cin, candidateID);

                CandidateNode* candidate = candidates.getCandidate(candidateID);
                if (!candidate) {
                    cout << "Error: Candidate not found!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                if (string(candidate->pollingStation) != voterStation) {
                    cout << "Error: Candidate " << candidateID << " is not contesting in your polling station!\n";
                    cout << "You can only vote for candidates in Station: " << voterStation << "\n";
                    cout << "Candidate's station: " << candidate->pollingStation << "\n";
                    delete candidate;
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                cout << "\nYou are voting for:\n";
                cout << "Name: " << candidate->name << "\n";
                cout << "Party: " << candidate->party << "\n";
                delete candidate;

                cout << "\nConfirm vote for candidate " << candidateID << "? (yes/no): ";
                string confirm;
                getline(cin, confirm);

                if (confirm == "yes" || confirm == "y" || confirm == "Y") {
                    candidates.voteCandidate(candidateID);
                    voters.markAsVoted(v->voterID);
                    v->hasVoted = true;

                    cout << "\n" << string(50, '*') << "\n";
                    cout << "    VOTE CAST SUCCESSFULLY!\n";
                    cout << string(50, '*') << "\n";
                    cout << "Thank you for voting, " << v->name << "!\n";
                    cout << "Your vote has been recorded.\n";
                }
                else {
                    cout << "\nVote cancelled.\n";
                }
                cout << "Press Enter to continue...";
                cin.get();
                break;
            }
            case 3: {
                printHeader("ELECTION RESULTS");

                if (votingActive) {
                    cout << "Voting is still in progress!\n";
                    cout << "Results will be available after voting ends.\n";

                    cout << "\n" << string(50, '-') << "\n";
                    cout << "CURRENT STANDINGS IN YOUR POLLING STATION\n";
                    cout << string(50, '-') << "\n";
                    candidates.printCandidatesByStation(v->pollingStation);
                }
                else {
                    cout << "Voting has ended. Final Results:\n\n";

                    cout << string(50, '=') << "\n";
                    cout << "OVERALL WINNER:\n";
                    cout << string(50, '=') << "\n";
                    candidates.printWinner();

                    cout << "\n" << string(50, '=') << "\n";
                    cout << "YOUR POLLING STATION RESULTS (" << v->pollingStation << "):\n";
                    cout << string(50, '=') << "\n";
                    candidates.printCandidatesByStation(v->pollingStation);

                    cout << "\n" << string(50, '=') << "\n";
                    cout << "VOTER STATISTICS:\n";
                    cout << string(50, '=') << "\n";
                    int totalVoters = voters.getTotalVoters();
                    int votedCount = voters.getVotedCount();
                    cout << "Total Registered Voters: " << totalVoters << "\n";
                    cout << "Total Votes Cast: " << votedCount << "\n";
                    if (totalVoters > 0) {
                        float turnout = (float)votedCount / totalVoters * 100;
                        cout << "Voter Turnout: " << fixed << setprecision(1) << turnout << "%\n";
                    }

                    string voterTown = v->town;
                    cout << "\n" << string(50, '-') << "\n";
                    cout << "STATISTICS FOR YOUR TOWN (" << voterTown << "):\n";
                    cout << string(50, '-') << "\n";
                    cout << "Note: Detailed town statistics available in admin view.\n";
                }

                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            case 4: {
                printHeader("UPDATE PASSWORD");

                string oldPass, newPass, confirmPass;

                cout << "Enter your current password: ";
                getline(cin, oldPass);

                if (oldPass != v->password) {
                    cout << "Error: Incorrect current password!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                bool validPassword = false;
                int attempts = 0;
                const int MAX_ATTEMPTS = 3;

                while (!validPassword && attempts < MAX_ATTEMPTS) {
                    cout << "Enter new password (min 6 characters): ";
                    getline(cin, newPass);

                    if (newPass.length() < 6) {
                        cout << "Error: Password must be at least 6 characters long!\n";
                        attempts++;
                        continue;
                    }

                    cout << "Confirm new password: ";
                    getline(cin, confirmPass);

                    if (newPass != confirmPass) {
                        cout << "Error: Passwords do not match! Please try again.\n";
                        attempts++;
                    }
                    else {
                        validPassword = true;
                    }
                }

                if (!validPassword) {
                    cout << "Too many failed attempts. Password update cancelled.\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                bool updated = voters.updatePassword(v->voterID, newPass);

                if (updated) {
                    cout << "\n" << string(50, '=') << "\n";
                    cout << "    PASSWORD UPDATED SUCCESSFULLY!\n";
                    cout << string(50, '=') << "\n";
                    cout << "Your password has been changed.\n";
                    cout << "Please use the new password for future logins.\n";
                }
                else {
                    cout << "Error: Failed to update password!\n";
                }

                cout << "Press Enter to continue...";
                cin.get();
                break;
            }
            case 5: {
                logout = true;
                cout << "Logging out...\n";
                break;
            }
            default: {
                cout << "Invalid choice! Please try again.\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
            }
            }
        }
    }

    void voterView(VoterHashTable& voters, CandidateBTree& candidates) {
        printHeader("VOTER LOGIN / REGISTRATION");
        int choice;
        cout << "1. Login (Already Registered)\n";
        cout << "2. Register (New Voter)\n";
        cout << "Enter choice: ";
        cin >> choice;
        clearInput();

        Voter* v = nullptr;

        if (choice == 1) {
            printHeader("VOTER LOGIN");
            string vid, password;
            cout << "Enter your ID (or CNIC): ";
            getline(cin, vid);
            cout << "Enter Password: ";
            getline(cin, password);

            v = voters.searchForLogin(vid);
            if (!v && voters.searchByCNIC(vid) != nullptr) {
                v = voters.searchByCNIC(vid);
            }

            if (!v) {
                cout << "Error: Voter not found!\n";
                return;
            }

            if (v->password != password) {
                cout << "Error: Incorrect password!\n";
                return;
            }

            showVoterDashboard(v, voters, candidates);
        }
        else if (choice == 2) {
            printHeader("NEW VOTER REGISTRATION");

            string name, cnic, gender, contact, town, password;
            cout << "Enter Full Name: ";
            getline(cin, name);
            cout << "Enter CNIC (13 digits): ";
            getline(cin, cnic);
            cout << "Enter Gender (Male/Female/Other): ";
            getline(cin, gender);
            cout << "Enter Contact Number (11 digits starting with 03): ";
            getline(cin, contact);
            cout << "Enter Town: ";
            getline(cin, town);
            cout << "Enter Password (min 6 characters): ";
            getline(cin, password);

            voters.insertVoter(name, cnic, gender, contact, town, password);

            v = voters.searchByCNIC(cnic);
            if (!v) {
                cout << "Registration failed! Please try again.\n";
                return;
            }

            cout << "\n" << string(50, '=') << "\n";
            cout << "    REGISTRATION SUCCESSFUL!\n";
            cout << string(50, '=') << "\n";
            cout << "Your Voter ID is: " << v->voterID << "\n";
            cout << "Password: " << v->password << "\n";
            cout << "Please remember these credentials for login.\n";
            cout << string(50, '=') << "\n\n";

            cout << "You are now automatically logged in!\n";
            cout << "Press Enter to continue to dashboard...";
            cin.get();

            showVoterDashboard(v, voters, candidates);
        }
        else {
            cout << "Invalid choice!\n";
            return;
        }
    }

    void candidateView(CandidateBTree& candidates) {
        printHeader("CANDIDATE LOGIN");

        string id, pass;
        cout << "Enter Candidate ID: ";
        getline(cin, id);

        cout << "Enter Password: ";
        getline(cin, pass);

        if (!candidates.verifyCandidatePassword(id, pass)) {
            cout << "Error: Invalid candidate ID or password!\n";
            return;
        }

        CandidateNode* candidate = candidates.getCandidate(id);
        if (!candidate) {
            cout << "Error: Candidate not found!\n";
            return;
        }

        printHeader("YOUR PROFILE");
        cout << "Candidate ID: " << candidate->candidateID << "\n";
        cout << "Name: " << candidate->name << "\n";
        cout << "CNIC: " << candidate->cnic << "\n";
        cout << "Party: " << candidate->party << "\n";
        cout << "Town: " << candidate->town << "\n";
        cout << "Polling Station: " << candidate->pollingStation << "\n";
        cout << "Current Votes: " << candidate->voteCount << "\n";
        delete candidate;

        cout << "\n" << string(50, '-') << "\n";
        cout << "OTHER CANDIDATES IN YOUR POLLING STATION:\n";
        cout << string(50, '-') << "\n";

        CandidateNode* current = candidates.getCandidate(id);
        if (current) {
            candidates.printCandidatesByStation(string(current->pollingStation));
            delete current;
        }

        if (votingActive)
            cout << "\nVoting is still in progress.\n";
        else
            cout << "\nVoting has ended.\n";
    }

    // MODIFIED: Updated to use town-based system
    void candidateRegistrationView(CandidateBTree& candidates) {
        printHeader("CANDIDATE REGISTRATION");

        cout << "\nIMPORTANT: You must have the party secret code to register as a candidate.\n";
        cout << "If you're an independent candidate, use Party: 'IND' and Secret Code: 'INDEP1234'\n";
        cout << "\nELECTION RULES:\n";
        cout << "1. Only one candidate per party per polling station\n";
        cout << "2. Polling station will be automatically assigned based on your town\n";
        cout << "3. If all stations in your town have candidates from your party, registration will be rejected\n";
        cout << string(50, '-') << "\n";

        string name, cnic, partyName, secretCode, town, password;

        cout << "\nEnter your details:\n";
        cout << "Full Name: ";
        getline(cin, name);

        cout << "CNIC (13 digits): ";
        getline(cin, cnic);

        cout << "Party Name (exactly as shown above): ";
        getline(cin, partyName);

        cout << "Party Secret Code: ";
        getline(cin, secretCode);

        cout << "\nAVAILABLE TOWNS:\n";
        vector<string> towns = GeographicConfig::getAllTowns();
        for (const auto& t : towns) {
            cout << " - " << t << "\n";
        }

        cout << "\nEnter your Town (from the list above): ";
        getline(cin, town);

        cout << "Set your password for candidate login (min 6 characters): ";
        getline(cin, password);

        // Check availability before attempting registration
        cout << "\n" << string(50, '-') << "\n";
        cout << "CHECKING AVAILABILITY...\n";

        if (!GeographicConfig::isValidTown(town)) {
            cout << "Error: Invalid town! Please select from the list above.\n";
            return;
        }

        // Check if any station is available for this party in the selected town
        vector<string> availableStations = candidates.getAvailableStations(partyName, town);

        if (availableStations.empty()) {
            cout << "\n" << string(60, '=') << "\n";
            cout << "  REGISTRATION NOT POSSIBLE!\n";
            cout << string(60, '=') << "\n";
            cout << "All polling stations in " << town << " already have a candidate from " << partyName << ".\n";
            cout << "Better luck next time!\n";
            cout << string(60, '=') << "\n";
            return;
        }

        cout << "Available polling stations for " << partyName << " in " << town << ":\n";
        for (const string& station : availableStations) {
            cout << " - " << station << "\n";
        }
        cout << string(50, '-') << "\n";

        bool success = candidates.registerCandidate(name, cnic, partyName,
            secretCode, town, password);

        if (success) {
            cout << "\n" << string(50, '=') << "\n";
            cout << "  REGISTRATION SUCCESSFUL!\n";
            cout << "You can now login as a candidate with your credentials.\n";
            cout << string(50, '=') << "\n";
        }
        else {
            cout << "\nRegistration failed. Please check your party details and try again.\n";
        }
    }

    void adminView(VoterHashTable& voters, CandidateBTree& candidates) {
        printHeader("ADMIN LOGIN");

        string pass;
        cout << "Enter Admin Password: ";
        getline(cin, pass);

        if (pass != adminPassword) {
            cout << "Error: Incorrect admin password!\n";
            return;
        }

        cout << "\nAdmin access granted!\n";

        int adminChoice;
        do {
            printHeader("ADMIN DASHBOARD");
            cout << "1. Manage Voters\n";
            cout << "2. Manage Candidates\n";
            cout << "3. Voting Control\n";
            cout << "4. View Statistics & Reports\n";
            cout << "5. System Tools\n";
            cout << "0. Return to Main Menu\n";
            cout << "\nEnter choice: ";
            cin >> adminChoice;
            clearInput();

            switch (adminChoice) {
            case 1: adminManageVoters(voters, candidates); break;
            case 2: adminManageCandidates(candidates); break;
            case 3: adminVotingControl(voters, candidates); break;
            case 4: adminStatisticsAndReports(voters, candidates); break;
            case 5: adminSystemTools(voters, candidates); break;
            case 0: cout << "Returning to main menu...\n"; break;
            default: cout << "Invalid choice!\n";
            }
        } while (adminChoice != 0);
    }

private:
    void adminStatisticsAndReports(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("STATISTICS & REPORTS");
            cout << "1. Voter Statistics\n";
            cout << "2. Candidate Statistics\n";
            cout << "3. Geographic Reports\n";
            cout << "4. Overall System Statistics\n";
            cout << "5. Party Allocations Report\n";
            cout << "0. Back to Admin Dashboard\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1: adminVoterStatistics(voters); break;
            case 2: adminCandidateStatistics(candidates); break;
            case 3: adminGeographicReports(voters, candidates); break;
            case 4: adminOverallStatistics(voters, candidates); break;
            case 5: adminPartyAllocations(candidates); break;
            case 0: cout << "Returning to admin dashboard...\n"; break;
            default: cout << "Invalid choice!\n";
            }
        } while (choice != 0);
    }

    void adminVoterStatistics(VoterHashTable& voters) {
        printHeader("VOTER STATISTICS");

        cout << "\nOVERALL STATISTICS:\n";
        cout << string(40, '-') << "\n";
        int totalVoters = voters.getTotalVoters();
        int votedCount = voters.getVotedCount();
        int notVoted = voters.getNotVotedCount();

        cout << "Total Registered Voters: " << totalVoters << "\n";
        cout << "Votes Cast: " << votedCount << "\n";
        cout << "Votes Remaining: " << notVoted << "\n";

        if (totalVoters > 0) {
            float turnout = (float)votedCount / totalVoters * 100;
            cout << "Voter Turnout: " << fixed << setprecision(1) << turnout << "%\n";
        }

        cout << "\nTOWN-WISE BREAKDOWN:\n";
        cout << string(40, '-') << "\n";
        voters.printTownStatistics();

        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void adminCandidateStatistics(CandidateBTree& candidates) {
        printHeader("CANDIDATE STATISTICS");

        cout << "\nCANDIDATE OVERVIEW:\n";
        cout << string(40, '-') << "\n";
        candidates.printCandidatesTable();

        cout << "\nPOLLING STATION DISTRIBUTION:\n";
        cout << string(40, '-') << "\n";
        vector<string> stations = GeographicConfig::getAllPollingStations();
        for (const string& station : stations) {
            vector<CandidateNode> stationCandidates = candidates.getCandidatesByStation(station);
            if (!stationCandidates.empty()) {
                cout << "\nStation " << station << " (" << stationCandidates.size() << " candidates):\n";
                cout << string(30, '-') << "\n";
                for (const auto& candidate : stationCandidates) {
                    // GCC compatible version
                    string party(candidate.party);
                    cout << " - " << candidate.name << " (" << party << ") - Votes: " << candidate.voteCount << "\n";
                }
            }
        }

        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void adminOverallStatistics(VoterHashTable& voters, CandidateBTree& candidates) {
        printHeader("OVERALL SYSTEM STATISTICS");

        cout << "VOTER STATISTICS:\n";
        cout << string(40, '-') << "\n";
        int totalVoters = voters.getTotalVoters();
        int votedCount = voters.getVotedCount();
        int notVoted = voters.getNotVotedCount();

        cout << "Total Registered Voters: " << totalVoters << "\n";
        cout << "Votes Cast: " << votedCount << "\n";
        cout << "Votes Remaining: " << notVoted << "\n";

        if (totalVoters > 0) {
            float turnout = (float)votedCount / totalVoters * 100;
            cout << "Voter Turnout: " << fixed << setprecision(1) << turnout << "%\n";
        }

        cout << "\nVOTING STATUS: ";
        if (votingActive) {
            cout << "ACTIVE ";
            time_t now = time(nullptr);
            int remaining = durationSeconds - difftime(now, startTime);
            cout << "(Time left: " << remaining / 60 << "m " << remaining % 60 << "s)\n";
        }
        else {
            cout << "INACTIVE\n";
        }

        cout << "\nCANDIDATE STATISTICS:\n";
        cout << string(40, '-') << "\n";

        vector<string> stations = GeographicConfig::getAllPollingStations();
        int stationsWithCandidates = 0;
        for (const string& station : stations) {
            vector<CandidateNode> stationCandidates = candidates.getCandidatesByStation(station);
            if (!stationCandidates.empty()) {
                stationsWithCandidates++;
            }
        }

        cout << "Polling Stations with Candidates: " << stationsWithCandidates << "/" << stations.size() << "\n";
        if (stations.size() > 0) {
            float coverage = (float)stationsWithCandidates / stations.size() * 100;
            cout << "Station Coverage: " << fixed << setprecision(1) << coverage << "%\n";
        }

        cout << "\nPress Enter to continue...";
        cin.get();
    }

    void adminGeographicReports(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("GEOGRAPHIC REPORTS");
            cout << "1. Town-wise Voter Statistics\n";
            cout << "2. Town-wise Candidate Distribution\n";
            cout << "3. Polling Station Analysis\n";
            cout << "4. Candidate Summary by Town (Table)\n";
            cout << "0. Back to Statistics Menu\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1: {
                printHeader("TOWN-WISE VOTER STATISTICS");
                voters.printTownStatistics();
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            case 2: {
                printHeader("TOWN-WISE CANDIDATE DISTRIBUTION");
                vector<string> towns = GeographicConfig::getAllTowns();

                cout << "\n" << string(70, '=') << "\n";
                cout << left << setw(15) << "TOWN" << setw(15) << "CANDIDATES"
                    << setw(15) << "STATIONS" << setw(15) << "PARTIES" << setw(15) << "FILLED STATIONS" << "\n";
                cout << string(70, '=') << "\n";

                for (const string& town : towns) {
                    vector<CandidateNode> townCandidates = candidates.getCandidatesByTown(town);
                    int filledStations = 0;

                    // Count unique stations with candidates
                    unordered_set<string> stationsWithCandidates;
                    for (const auto& candidate : townCandidates) {
                        stationsWithCandidates.insert(string(candidate.pollingStation));
                    }
                    filledStations = stationsWithCandidates.size();

                    // Count unique parties
                    unordered_set<string> parties;
                    for (const auto& candidate : townCandidates) {
                        parties.insert(string(candidate.party));
                    }

                    cout << left << setw(15) << town
                        << setw(15) << townCandidates.size()
                        << setw(15) << parties.size()
                        << setw(15) << filledStations << "\n";
                }
                cout << string(70, '=') << "\n";

                cout << "\n\nDETAILED PARTY ALLOCATIONS BY TOWN:\n";
                cout << string(60, '-') << "\n";
                for (const string& town : towns) {
                    cout << "\n" << town << ":\n";
                    candidates.printPartyAllocationsByTown(town);
                }

                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            case 3: {
                printHeader("POLLING STATION ANALYSIS");
                vector<string> stations = GeographicConfig::getAllPollingStations();

                cout << "\n" << string(80, '=') << "\n";
                cout << left << setw(15) << "STATION" << setw(15) << "TOWN"
                    << setw(15) << "CANDIDATES" << setw(15) << "VOTERS" << setw(20) << "STATUS" << "\n";
                cout << string(80, '=') << "\n";

                for (const string& station : stations) {
                    // Get town from station prefix
                    string prefix = station.substr(0, 3);
                    string town;

                    if (prefix == "KHI") town = "Karachi";
                    else if (prefix == "LHR") town = "Lahore";
                    else if (prefix == "ISB") town = "Islamabad";
                    else if (prefix == "RWP") town = "Rawalpindi";
                    else if (prefix == "FSD") town = "Faisalabad";
                    else if (prefix == "MUL") town = "Multan";
                    else if (prefix == "PES") town = "Peshawar";
                    else if (prefix == "QTA") town = "Quetta";
                    else town = prefix;

                    // Count candidates in this station
                    vector<CandidateNode> stationCandidates = candidates.getCandidatesByStation(station);

                    string status = stationCandidates.empty() ? "NO CANDIDATES" : "ACTIVE";

                    cout << left << setw(15) << station
                        << setw(15) << town
                        << setw(15) << stationCandidates.size()
                        << setw(20) << status << "\n";
                }
                cout << string(80, '=') << "\n";

                cout << "\n\nDETAILED STATION REPORTS (Active Stations Only):\n";
                cout << string(60, '-') << "\n";

                // Only show detailed report for stations with candidates
                int activeCount = 0;
                for (const string& station : stations) {
                    vector<CandidateNode> stationCandidates = candidates.getCandidatesByStation(station);
                    if (!stationCandidates.empty()) {
                        activeCount++;
                        cout << "\nStation: " << station << " (" << stationCandidates.size() << " candidates)\n";
                        cout << string(40, '-') << "\n";

                        cout << left << setw(10) << "ID" << setw(25) << "Name"
                            << setw(15) << "Party" << setw(10) << "Votes" << "\n";
                        cout << string(60, '-') << "\n";

                        for (const auto& candidate : stationCandidates) {
                            // GCC compatible version
                            string id(candidate.candidateID);
                            string name(candidate.name);
                            string party(candidate.party);

                            cout << left << setw(10) << id
                                << setw(25) << name
                                << setw(15) << party
                                << setw(10) << candidate.voteCount << "\n";
                        }
                        cout << string(60, '-') << "\n";
                    }
                }

                if (activeCount == 0) {
                    cout << "\nNo active polling stations found.\n";
                }

                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            case 4: {
                printHeader("CANDIDATE SUMMARY BY TOWN");

                vector<string> towns = GeographicConfig::getAllTowns();
                vector<string> parties = {
                    "PMLN", "PPP", "MQM", "ANP", "JUI",
                    "PTI", "TLP", "BAP", "GDA", "JI", "IND"
                };

                cout << "\n" << string(100, '=') << "\n";
                cout << left << setw(15) << "TOWN";
                for (const string& party : parties) {
                    cout << setw(8) << party;
                }
                cout << setw(10) << "TOTAL" << "\n";
                cout << string(100, '=') << "\n";

                for (const string& town : towns) {
                    vector<CandidateNode> townCandidates = candidates.getCandidatesByTown(town);

                    // Count candidates per party in this town
                    unordered_map<string, int> partyCounts;
                    for (const auto& candidate : townCandidates) {
                        // GCC compatible version
                        string party(candidate.party);
                        partyCounts[party]++;
                    }

                    cout << left << setw(15) << town;
                    int townTotal = 0;

                    for (const string& party : parties) {
                        int count = partyCounts[party];
                        townTotal += count;

                        if (count > 0) {
                            cout << setw(8) << count;
                        }
                        else {
                            cout << setw(8) << "-";
                        }
                    }
                    cout << setw(10) << townTotal << "\n";
                }
                cout << string(100, '=') << "\n";

                // Add a summary section
                cout << "\n\nSUMMARY:\n";
                cout << string(60, '-') << "\n";

                int totalStations = GeographicConfig::getAllPollingStations().size();
                int stationsWithCandidates = 0;

                vector<string> allStations = GeographicConfig::getAllPollingStations();
                for (const string& station : allStations) {
                    vector<CandidateNode> stationCandidates = candidates.getCandidatesByStation(station);
                    if (!stationCandidates.empty()) {
                        stationsWithCandidates++;
                    }
                }

                cout << "Total Polling Stations: " << totalStations << "\n";
                cout << "Stations with Candidates: " << stationsWithCandidates << "\n";
                cout << "Empty Stations: " << (totalStations - stationsWithCandidates) << "\n";

                if (totalStations > 0) {
                    float coverage = (float)stationsWithCandidates / totalStations * 100;
                    cout << "Station Coverage: " << fixed << setprecision(1) << coverage << "%\n";
                }

                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
            }
        } while (choice != 0);
    }

    void adminManageVoters(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("MANAGE VOTERS");
            cout << "1. View All Voters\n";
            cout << "2. Search Voter\n";
            cout << "3. View by Polling Station\n";
            cout << "4. View by Town\n";
            cout << "0. Back to Admin Dashboard\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1:
                printHeader("ALL VOTERS");
                voters.printTable();
                break;
            case 2: {
                int searchChoice;
                cout << "Search by:\n";
                cout << "1. Voter ID\n";
                cout << "2. CNIC\n";
                cout << "Choice: ";
                cin >> searchChoice;
                clearInput();

                if (searchChoice == 1) {
                    string id;
                    cout << "Enter Voter ID: "; getline(cin, id);
                    Voter* v = voters.searchVoter(id);
                    displayVoterDetails(v);
                }
                else if (searchChoice == 2) {
                    string cnic;
                    cout << "Enter CNIC: "; getline(cin, cnic);
                    Voter* v = voters.searchByCNIC(cnic);
                    displayVoterDetails(v);
                }
                else {
                    cout << "Invalid choice!\n";
                }
                break;
            }
            case 3: {
                string station;
                cout << "Enter Polling Station ID: "; getline(cin, station);
                voters.printVotersByStation(station);
                break;
            }
            case 4: {
                string town;
                cout << "Enter Town: "; getline(cin, town);
                voters.printVotersByTown(town);
                break;
            }
            }
        } while (choice != 0);
    }

    void displayVoterDetails(Voter* v) {
        if (v) {
            cout << "\n" << string(50, '-') << "\n";
            cout << "VOTER DETAILS\n";
            cout << string(50, '-') << "\n";
            cout << "Voter ID:      " << v->voterID << "\n";
            cout << "Name:          " << v->name << "\n";
            cout << "Password: " << v->password << "\n";
            cout << "CNIC:          " << v->cnic << "\n";
            cout << "Gender:        " << v->gender << "\n";
            cout << "Contact:       " << v->contactNumber << "\n";
            cout << "Town:          " << v->town << "\n";
            cout << "Polling Station: " << v->pollingStation << "\n";
            cout << "Voted:         " << (v->hasVoted ? "Yes" : "No") << "\n";
            cout << string(50, '-') << "\n";
        }
        else {
            cout << "Voter not found!\n";
        }
    }

    void adminManageCandidates(CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("MANAGE CANDIDATES");
            cout << "1. View All Candidates\n";
            cout << "2. Search Candidate\n";
            cout << "3. View Candidates by Station\n";
            cout << "4. View Candidates by Town\n";
            cout << "0. Back to Admin Dashboard\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1:
                candidates.printCandidatesTable();
                break;
            case 2: {
                string id;
                cout << "Enter Candidate ID: "; getline(cin, id);
                CandidateNode* candidate = candidates.getCandidate(id);
                if (candidate) {
                    cout << "\n" << string(50, '-') << "\n";
                    cout << "CANDIDATE DETAILS\n";
                    cout << string(50, '-') << "\n";
                    cout << "ID:           " << candidate->candidateID << "\n";
                    cout << "Name:         " << candidate->name << "\n";
                    cout << "CNIC:         " << candidate->cnic << "\n";
                    cout << "Party:        " << candidate->party << "\n";
                    cout << "Town:         " << candidate->town << "\n";
                    cout << "Station:      " << candidate->pollingStation << "\n";
                    cout << "Votes:        " << candidate->voteCount << "\n";
                    cout << string(50, '-') << "\n";
                    delete candidate;
                }
                else {
                    cout << "Candidate not found!\n";
                }
                break;
            }
            case 3: {
                string station;
                cout << "Enter Polling Station ID: "; getline(cin, station);
                candidates.printCandidatesByStation(station);
                break;
            }
            case 4: {
                string town;
                cout << "Enter Town: "; getline(cin, town);
                vector<CandidateNode> townCandidates = candidates.getCandidatesByTown(town);
                if (townCandidates.empty()) {
                    cout << "No candidates found in " << town << "\n";
                }
                else {
                    cout << "\nCandidates in " << town << ":\n";
                    cout << string(80, '-') << "\n";
                    cout << left << setw(10) << "ID" << setw(25) << "Name"
                        << setw(15) << "Party" << setw(15) << "Station" << setw(10) << "Votes" << "\n";
                    cout << string(80, '-') << "\n";

                    for (const auto& c : townCandidates) {
                        // GCC compatible version
                        string id(c.candidateID);
                        string name(c.name);
                        string party(c.party);
                        string station(c.pollingStation);

                        cout << left << setw(10) << id
                            << setw(25) << name
                            << setw(15) << party
                            << setw(15) << station
                            << setw(10) << c.voteCount << "\n";
                    }
                    cout << string(80, '-') << "\n";
                    cout << "Total: " << townCandidates.size() << " candidates\n";
                }
                break;
            }
            }
        } while (choice != 0);
    }

    void adminVotingControl(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("VOTING CONTROL");
            cout << "1. Start Voting Session\n";
            cout << "2. Stop Voting Session\n";
            cout << "3. Check Voting Status\n";
            cout << "4. Announce Results\n";
            cout << "5. View Station-wise Results\n";
            cout << "0. Back to Admin Dashboard\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1:
                if (!votingActive)
                    startVoting();
                else
                    cout << "Voting is already active!\n";
                break;
            case 2:
                if (votingActive) {
                    votingActive = false;
                    cout << "Voting stopped manually.\n";
                }
                else {
                    cout << "Voting is not active.\n";
                }
                break;
            case 3:
                if (votingActive) {
                    cout << "Voting is ACTIVE\n";
                    time_t now = time(nullptr);
                    int remaining = durationSeconds - difftime(now, startTime);
                    cout << "Time remaining: " << remaining / 60 << "m " << remaining % 60 << "s\n";
                }
                else {
                    cout << "Voting is INACTIVE\n";
                }
                break;
            case 4:
                printHeader("ELECTION RESULTS");
                candidates.printWinner();
                cout << "\nDetailed Results:\n";
                candidates.printCandidatesTable();
                break;
            case 5: {
                string station;
                cout << "Enter Polling Station ID: "; getline(cin, station);
                cout << "\nResults for Station: " << station << "\n";
                cout << string(50, '-') << "\n";
                candidates.printCandidatesByStation(station);
                break;
            }
            }
        } while (choice != 0);
    }

    void adminPartyAllocations(CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("PARTY ALLOCATIONS");
            cout << "1. View Allocations by Town\n";
            cout << "2. Check Party Availability\n";
            cout << "0. Back to Statistics Menu\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1: {
                string town;
                cout << "Enter Town: "; getline(cin, town);
                candidates.printPartyAllocationsByTown(town);
                break;
            }
            case 2: {
                string party, town;
                cout << "Enter Party Name: "; getline(cin, party);
                cout << "Enter Town: "; getline(cin, town);

                vector<string> availableStations = candidates.getAvailableStations(party, town);
                if (availableStations.empty()) {
                    cout << "\nNo available stations for " << party << " in " << town << "\n";
                    cout << "All stations in " << town << " already have a candidate from " << party << "\n";
                }
                else {
                    cout << "\nAvailable stations for " << party << " in " << town << ":\n";
                    cout << string(40, '-') << "\n";
                    for (const string& station : availableStations) {
                        cout << " - " << station << "\n";
                    }
                    cout << "Total available: " << availableStations.size() << " stations\n";
                }
                break;
            }
            }
        } while (choice != 0);
    }

    void adminSystemTools(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("SYSTEM TOOLS");
            cout << "1. System Reset\n";
            cout << "2. View System Configuration\n";
            cout << "0. Back to Admin Dashboard\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1:
                adminSystemReset(voters, candidates);
                break;
            case 2:
                printHeader("SYSTEM CONFIGURATION");
                cout << "Voting Duration: " << (durationSeconds / 60) << " minutes\n";
                cout << "Voting Status: " << (votingActive ? "ACTIVE" : "INACTIVE") << "\n";
                cout << "Total Towns: " << GeographicConfig::getAllTowns().size() << "\n";
                cout << "Total Polling Stations: " << GeographicConfig::getAllPollingStations().size() << "\n";
                cout << "Admin Password: ******\n";
                cout << "\nPress Enter to continue...";
                cin.get();
                break;
            }
        } while (choice != 0);
    }

    void adminSystemReset(VoterHashTable& voters, CandidateBTree& candidates) {
        printHeader("SYSTEM RESET - WARNING!");
        cout << "This will reset all votes and voting status!\n";
        cout << "All voter 'hasVoted' flags will be set to false.\n";
        cout << "All candidate vote counts will be set to zero.\n";
        cout << "Voting session will be stopped.\n\n";

        cout << "Type 'CONFIRM' to proceed: ";
        string confirm;
        getline(cin, confirm);

        if (confirm == "CONFIRM") {
            voters.resetAllVotes();
            candidates.resetAllVotes();
            votingActive = false;
            cout << "\nSystem reset successfully!\n";
        }
        else {
            cout << "\nReset cancelled.\n";
        }
    }

public:
    void showResults(CandidateBTree& candidates) {
        if (votingActive) {
            cout << "\nVoting is still in progress!\n";
            return;
        }

        printHeader("FINAL ELECTION RESULTS");
        candidates.printWinner();
        cout << "\nDetailed Vote Count:\n";
        candidates.printCandidatesTable();
    }
};

#endif