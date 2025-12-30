#ifndef VOTING_MANAGER_H
#define VOTING_MANAGER_H

#include <iostream>
#include <ctime>
#include <string>
#include <iomanip>
#include <limits>
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

                // Fix: Declare these variables locally
                string voterStation = v->pollingStation;

                // Get candidates in the voter's polling station
                // First, get all candidates and filter by station
                vector<string> stationCandidates;

                // We need a way to get candidates by station - let's add a helper method
                // For now, we'll use a workaround
                printHeader("CANDIDATES IN YOUR POLLING STATION");
                cout << "Candidates contesting from Station: " << voterStation << "\n";
                cout << string(80, '-') << "\n";

                // Display candidates from this station
                candidates.printCandidatesByStation(voterStation);

                // Since we can't easily get candidate IDs by station, we'll prompt directly
                cout << "\nEnter Candidate ID to vote for: ";
                string candidateID;
                getline(cin, candidateID);

                // Check if candidate exists and is in the right station
                CandidateNode* candidate = candidates.getCandidate(candidateID);
                if (!candidate) {
                    cout << "Error: Candidate not found!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                // Check if candidate is in the voter's polling station
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
                cout << "Will Implement - View Result\n";
                cout << "Press Enter to continue...";
                cin.get();
                break;
            }
            case 4: {
                printHeader("UPDATE PASSWORD");

                string oldPass, newPass, confirmPass;

                // Step 1: Verify old password
                cout << "Enter your current password: ";
                getline(cin, oldPass);

                if (oldPass != v->password) {
                    cout << "Error: Incorrect current password!\n";
                    cout << "Press Enter to continue...";
                    cin.get();
                    break;
                }

                // Step 2: Get new password with validation
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

                    // Step 3: Confirm new password
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

                // Step 4: Update password using the function
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
            // LOGIN FLOW
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

            // Show dashboard for logged in user
            showVoterDashboard(v, voters, candidates);
        }
        else if (choice == 2) {
            // REGISTRATION FLOW
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

            // Show dashboard for newly registered user
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

    void candidateRegistrationView(CandidateBTree& candidates) {
        printHeader("CANDIDATE REGISTRATION");

        cout << "\nIMPORTANT: You must have the party secret code to register as a candidate.\n";
        cout << "If you're an independent candidate, use Party: 'IND' and Secret Code: 'INDEP1234'\n";
        cout << string(50, '-') << "\n";

        string name, cnic, partyName, secretCode, pollingStation, password;

        cout << "\nEnter your details:\n";
        cout << "Full Name: ";
        getline(cin, name);

        cout << "CNIC (13 digits): ";
        getline(cin, cnic);

        cout << "Party Name (exactly as shown above): ";
        getline(cin, partyName);

        cout << "Party Secret Code: ";
        getline(cin, secretCode);

        cout << "Polling Station you want to contest from (e.g., KHI01, LHR01): ";
        getline(cin, pollingStation);

        cout << "Set your password for candidate login (min 6 characters): ";
        getline(cin, password);

        bool success = candidates.registerCandidate(name, cnic, partyName,
            secretCode, pollingStation,
            password);

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
    void adminManageVoters(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("MANAGE VOTERS");
            cout << "1. View All Voters\n";
            cout << "2. Search Voter\n";
            cout << "3. View by Polling Station\n";
            cout << "4. View by Town\n";
            cout << "5. View Voter Statistics\n";
            cout << "0. Back\n";
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
            case 5:
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
            cout << "4: View All Candidates (Brute-force)\n";
            cout << "5. Rebuild/Verify B-tree\n";
            cout << "0. Back\n";
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
                candidates.printAllCandidatesBruteForce();
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

    void adminGeographicReports(VoterHashTable& voters, CandidateBTree& candidates) {
        int choice;
        do {
            printHeader("GEOGRAPHIC REPORTS");
            cout << "1. Town-wise Voter Statistics\n";
            cout << "2. Polling Station Reports\n";
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
                    cout << "Candidates in this station:\n";
                    candidates.printCandidatesByStation(station);
                }
                break;
            }
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