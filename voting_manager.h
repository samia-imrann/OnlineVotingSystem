#ifndef VOTING_MANAGER_H
#define VOTING_MANAGER_H

#include <iostream>
#include <ctime>
#include <string>
#include <iomanip>
#include <limits>
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
    VotingManager(int duration = 300, string adminPass = "admin123") {
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
        char buffer[80];
        ctime_s(buffer, sizeof(buffer), &endTime);
        cout << buffer;
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

    // -------------------------------------------------
    //                   VOTER VIEW (UPDATED)
    // -------------------------------------------------
    void voterView(VoterHashTable& voters, CandidateBTree& candidates) {
        printHeader("VOTER LOGIN");

        string username, password;
        cout << "Enter Username (or CNIC): ";
        getline(cin, username);

        cout << "Enter Password: ";
        getline(cin, password);

        // Try searching by username first
        Voter* v = voters.searchByUsername(username);

        // If not found by username, try searching by CNIC
        if (!v && voters.searchByCNIC(username) != nullptr) {
            v = voters.searchByCNIC(username);
        }

        if (!v) {
            cout << "Error: Voter not found!\n";
            return;
        }

        if (v->password != password) {
            cout << "Error: Incorrect password!\n";
            return;
        }

        cout << "\n" << string(50, '-') << "\n";
        cout << "Welcome, " << v->name << "!\n";
        cout << "Voter ID: " << v->voterID << "\n";
        cout << "Polling Station: " << v->pollingStation << "\n";
        cout << "Town: " << v->town << "\n";
        cout << string(50, '-') << "\n";

        if (!canVote()) {
            return;
        }

        if (v->hasVoted) {
            cout << "You have already voted!\n";
            return;
        }

        // Get candidates available in voter's polling station
        string voterStation = v->pollingStation;
        vector<string> stationCandidates = candidates.getCandidatesInStation(voterStation);

        if (stationCandidates.empty()) {
            cout << "No candidates available in your polling station (" << voterStation << ").\n";
            return;
        }

        printHeader("CANDIDATES IN YOUR POLLING STATION");
        candidates.printFilteredCandidates(stationCandidates);

        cout << "\nEnter Candidate ID to vote for: ";
        string candidateID;
        getline(cin, candidateID);

        // Check if candidate is in voter's polling station
        bool validCandidate = false;
        for (const string& id : stationCandidates) {
            if (id == candidateID) {
                validCandidate = true;
                break;
            }
        }

        if (!validCandidate) {
            cout << "Error: Candidate " << candidateID << " is not contesting in your polling station!\n";
            cout << "You can only vote for candidates in Station: " << voterStation << "\n";
            return;
        }

        int found = candidates.searchCandidate(candidateID);
        if (found == -1) {
            cout << "Error: Candidate not found!\n";
            return;
        }

        // Show candidate details
        CandidateNode* candidate = candidates.getCandidate(candidateID);
        if (candidate) {
            cout << "\nYou are voting for:\n";
            cout << "Name: " << candidate->name << "\n";
            cout << "Party: " << candidate->party << "\n";
            cout << "Symbol: " << candidate->symbol << "\n";
            cout << "Constituency: " << candidate->constituency << "\n";
            delete candidate;
        }

        cout << "\nConfirm vote for candidate " << candidateID << "? (yes/no): ";
        string confirm;
        getline(cin, confirm);

        if (confirm == "yes" || confirm == "y" || confirm == "Y") {
            candidates.voteCandidate(candidateID);
            voters.markAsVoted(v->voterID);

            cout << "\n" << string(50, '*') << "\n";
            cout << "    VOTE CAST SUCCESSFULLY!\n";
            cout << string(50, '*') << "\n";
            cout << "Thank you for voting, " << v->name << "!\n";
            cout << "Your vote has been recorded.\n";
        }
        else {
            cout << "\nVote cancelled.\n";
        }
    }

    // -------------------------------------------------
    //                CANDIDATE VIEW (UPDATED)
    // -------------------------------------------------
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
        cout << "Symbol: " << candidate->symbol << "\n";
        cout << "Polling Station: " << candidate->pollingStation << "\n";
        cout << "Constituency: " << candidate->constituency << "\n";
        cout << "Current Votes: " << candidate->voteCount << "\n";
        delete candidate;

        cout << "\n" << string(50, '-') << "\n";
        cout << "OTHER CANDIDATES IN YOUR POLLING STATION:\n";
        cout << string(50, '-') << "\n";

        // Get current candidate to know their station
        CandidateNode* current = candidates.getCandidate(id);
        if (current) {
            candidates.printCandidatesByStation(current->pollingStation);
            delete current;
        }

        if (votingActive)
            cout << "\nVoting is still in progress.\n";
        else
            cout << "\nVoting has ended.\n";
    }

    // -------------------------------------------------
    //                ADMIN VIEW
    // -------------------------------------------------
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
            cout << "4. View Statistics\n";
            cout << "5. Geographic Reports\n";
            cout << "6. System Reset\n";
            cout << "0. Return to Main Menu\n";
            cout << "\nEnter choice: ";
            cin >> adminChoice;
            clearInput();

            switch (adminChoice) {
            case 1: adminManageVoters(voters, candidates); break;
            case 2: adminManageCandidates(candidates); break;
            case 3: adminVotingControl(voters, candidates); break;
            case 4: adminViewStatistics(voters, candidates); break;
            case 5: adminGeographicReports(voters, candidates); break;
            case 6: adminSystemReset(voters, candidates); break;
            case 0: cout << "Returning to main menu...\n"; break;
            default: cout << "Invalid choice!\n";
            }
        } while (adminChoice != 0);
    }

private:
    // -------------------------------------------------
    //            ADMIN — MANAGE VOTERS (UPDATED)
    // -------------------------------------------------
    void adminManageVoters(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("MANAGE VOTERS");
            cout << "1. Add New Voter\n";
            cout << "2. View All Voters\n";
            cout << "3. Search Voter\n";
            cout << "4. View by Polling Station\n";
            cout << "5. View by Town\n";
            cout << "6. View Voter Statistics\n";
            cout << "0. Back\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1: {
                string name, cnic, gender, contact, town;
                cout << "Enter Voter Name: "; getline(cin, name);
                cout << "Enter CNIC (13 digits): "; getline(cin, cnic);
                cout << "Enter Gender (Male/Female/Other): "; getline(cin, gender);
                cout << "Enter Contact Number (11 digits starting with 03): "; getline(cin, contact);
                cout << "Enter Town: "; getline(cin, town);

                voters.insertVoter(name, cnic, gender, contact, town);
                break;
            }
            case 2:
                printHeader("ALL VOTERS");
                voters.printTable();
                break;
            case 3: {
                int searchChoice;
                cout << "Search by:\n";
                cout << "1. Voter ID\n";
                cout << "2. CNIC\n";
                cout << "3. Username\n";
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
                else if (searchChoice == 3) {
                    string username;
                    cout << "Enter Username: "; getline(cin, username);
                    Voter* v = voters.searchByUsername(username);
                    displayVoterDetails(v);
                }
                else {
                    cout << "Invalid choice!\n";
                }
                break;
            }
            case 4: {
                string station;
                cout << "Enter Polling Station ID: "; getline(cin, station);
                voters.printVotersByStation(station);
                break;
            }
            case 5: {
                string town;
                cout << "Enter Town: "; getline(cin, town);
                voters.printVotersByTown(town);
                break;
            }
            case 6:
                voters.printTownStatistics();
                break;
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
            cout << "CNIC:          " << v->cnic << "\n";
            cout << "Gender:        " << v->gender << "\n";
            cout << "Contact:       " << v->contactNumber << "\n";
            cout << "Town:          " << v->town << "\n";
            cout << "Polling Station: " << v->pollingStation << "\n";
            cout << "Username:      " << v->username << "\n";
            cout << "Voted:         " << (v->hasVoted ? "Yes" : "No") << "\n";
            cout << string(50, '-') << "\n";
        }
        else {
            cout << "Voter not found!\n";
        }
    }

    // -------------------------------------------------
    //          ADMIN — MANAGE CANDIDATES (UPDATED)
    // -------------------------------------------------
    void adminManageCandidates(CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("MANAGE CANDIDATES");
            cout << "1. Add New Candidate\n";
            cout << "2. View All Candidates\n";
            cout << "3. Search Candidate\n";
            cout << "4. View Candidates by Station\n";
            cout << "0. Back\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1: {
                string name, cnic, party, symbol, station, constituency, password;
                cout << "Enter Candidate Name: "; getline(cin, name);
                cout << "Enter CNIC (13 digits): "; getline(cin, cnic);
                cout << "Enter Party: "; getline(cin, party);
                cout << "Enter Symbol: "; getline(cin, symbol);
                cout << "Enter Polling Station (e.g., KHI01): "; getline(cin, station);
                cout << "Enter Constituency: "; getline(cin, constituency);
                cout << "Enter Password (optional): "; getline(cin, password);

                // Generate candidate ID automatically
                // Format: C + station + sequence (e.g., CKHI0101)
                string candidateID = "C" + station + "01"; // Simple ID for now

                candidates.insertCandidate(candidateID, name, cnic, party, symbol,
                    station, constituency, password);
                break;
            }
            case 2:
                candidates.printCandidatesTable();
                break;
            case 3: {
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
                    cout << "Symbol:       " << candidate->symbol << "\n";
                    cout << "Station:      " << candidate->pollingStation << "\n";
                    cout << "Constituency: " << candidate->constituency << "\n";
                    cout << "Votes:        " << candidate->voteCount << "\n";
                    cout << string(50, '-') << "\n";
                    delete candidate;
                }
                else {
                    cout << "Candidate not found!\n";
                }
                break;
            }
            case 4: {
                string station;
                cout << "Enter Polling Station ID: "; getline(cin, station);
                candidates.printCandidatesByStation(station);
                break;
            }
            }
        } while (choice != 0);
    }

    // -------------------------------------------------
    //               ADMIN — VOTING CONTROL
    // -------------------------------------------------
    void adminVotingControl(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("VOTING CONTROL");
            cout << "1. Start Voting Session\n";
            cout << "2. Stop Voting Session\n";
            cout << "3. Check Voting Status\n";
            cout << "4. Announce Results\n";
            cout << "5. View Station-wise Results\n";
            cout << "0. Back\n";
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

    // -------------------------------------------------
    //            ADMIN — SYSTEM STATISTICS (UPDATED)
    // -------------------------------------------------
    void adminViewStatistics(VoterHashTable& voters, CandidateBTree& candidates) {
        printHeader("SYSTEM STATISTICS");

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

        cout << "\nCANDIDATE OVERVIEW:\n";
        cout << string(40, '-') << "\n";
        candidates.printCandidatesTable();
    }

    // -------------------------------------------------
    //            ADMIN — GEOGRAPHIC REPORTS (NEW)
    // -------------------------------------------------
    void adminGeographicReports(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("GEOGRAPHIC REPORTS");
            cout << "1. Town-wise Voter Statistics\n";
            cout << "2. Polling Station Reports\n";
            cout << "3. Constituency-wise Candidates\n";
            cout << "0. Back\n";
            cout << "\nEnter choice: ";
            cin >> choice;
            clearInput();

            switch (choice) {
            case 1:
                voters.printTownStatistics();
                break;
            case 2: {
                vector<string> stations = voters.getAllPollingStations();
                cout << "\n" << string(60, '=') << "\n";
                cout << "           POLLING STATION REPORTS\n";
                cout << string(60, '=') << "\n";

                for (const string& station : stations) {
                    cout << "\nStation: " << station << "\n";
                    cout << string(40, '-') << "\n";

                    // Count voters in this station
                    int stationVoters = 0;
                    int stationVoted = 0;
                    for (int i = 0; i < voters.getTotalVoters(); i++) {
                        // This is simplified - need to implement proper station counting
                    }

                    cout << "Candidates in this station:\n";
                    candidates.printCandidatesByStation(station);
                }
                break;
            }
            case 3: {
                // Get unique constituencies from candidates
                cout << "\nConstituency Report\n";
                cout << "Note: Implement constituency tracking in CandidateBTree\n";
                break;
            }
            }
        } while (choice != 0);
    }

    // -------------------------------------------------
    //             ADMIN — SYSTEM RESET
    // -------------------------------------------------
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
    // -------------------------------------------------
    //              FINAL RESULTS
    // -------------------------------------------------
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

#endif // VOTING_MANAGER_H