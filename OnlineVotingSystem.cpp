// main.cpp
#include <iostream>
#include <string>
#include <ctime>
#include <limits>
#include <iomanip>
#include "config.h"
#include "voter.h"
#include "candidate.h"
#include "voting_manager.h"
using namespace std;

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void printMainMenu() {
    cout << "\n" << string(70, '=') << "\n";
    cout << "              ENHANCED ONLINE VOTING SYSTEM\n";
    cout << string(70, '=') << "\n";
    cout << "1. Voter Login / Registration\n";
    cout << "2. Candidate Login / Registration\n";
    cout << "3. Admin Login\n";
    cout << "4. View Public Information\n";
    cout << "0. Exit System\n";
    cout << string(70, '=') << "\n";
    cout << "Enter your choice: ";
}

void candidateLoginRegistration(CandidateBTree& candidates, VotingManager& vm) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "        CANDIDATE LOGIN / REGISTRATION\n";
    cout << string(60, '=') << "\n";
    cout << "1. Login (Existing Candidate)\n";
    cout << "2. Register (New Candidate)\n";
    cout << "0. Back to Main Menu\n";
    cout << string(60, '=') << "\n";
    cout << "Enter your choice: ";

    int choice;
    cin >> choice;
    clearInput();

    switch (choice) {
    case 1: 
        vm.candidateView(candidates);
        break;
    case 2: 
    {
        cout << "\n" << string(70, '=') << "\n";
        cout << "          CANDIDATE REGISTRATION PORTAL\n";
        cout << string(70, '=') << "\n";

        cout << "\nIMPORTANT: You must have the party secret code to register as a candidate.\n";
        cout << "If you're an independent candidate, use Party: 'IND' and Secret Code: 'INDEP1234'\n";
        cout << string(50, '-') << "\n";

        string name, cnic, partyName, secretCode, pollingStation, password, confirmPassword;

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

        cout << "Confirm password: ";
        getline(cin, confirmPassword);

        if (password != confirmPassword) {
            cout << "\nError: Passwords do not match!\n";
            return;
        }

        // Show summary
        cout << "\n" << string(50, '-') << "\n";
        cout << "Registration Summary:\n";
        cout << "Name: " << name << "\n";
        cout << "CNIC: " << cnic << "\n";
        cout << "Party: " << partyName << "\n";
        cout << "Polling Station: " << pollingStation << "\n";
        cout << string(50, '-') << "\n";

        cout << "\nConfirm registration? (yes/no): ";
        string confirm;
        getline(cin, confirm);

        if (confirm == "yes" || confirm == "y" || confirm == "Y") {
            bool success = candidates.registerCandidate(name, cnic, partyName,
                secretCode, pollingStation,
                password);
            if (!success) {
                cout << "\nRegistration failed. Please check your party details and try again.\n";
            }
            else {
                cout << "\nRegistration successful! You can now login with your Candidate ID and password.\n";
            }
        }
        else {
            cout << "\nRegistration cancelled.\n";
        }
        break;
    }
    case 0:
        return;
    default:
        cout << "\nInvalid choice!\n";
    }
}

void printPublicInfo(CandidateBTree& candidates, VoterHashTable& voters, VotingManager& vm) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "              PUBLIC INFORMATION PORTAL\n";
    cout << string(70, '=') << "\n";
    candidates.printCandidatesTable();

    cout << "\nVOTER STATISTICS:\n";
    cout << string(40, '-') << "\n";
    int totalVoters = voters.getTotalVoters();
    int voted = voters.getVotedCount();
    cout << "Total Registered Voters: " << totalVoters << "\n";
    cout << "Votes Cast: " << voted << "\n";
    if (totalVoters > 0) {
        float percentage = (float)voted / totalVoters * 100;
        cout << "Voter Turnout: " << fixed << setprecision(1) << percentage << "%\n";
    }

    // Get all towns and stations
    vector<string> towns = voters.getAllTowns();
    vector<string> stations = voters.getAllPollingStations();

    cout << "\nGEOGRAPHIC DISTRIBUTION:\n";
    cout << string(40, '-') << "\n";
    cout << "Towns Registered: " << towns.size() << "\n";
    cout << "Polling Stations Active: " << stations.size() << "\n";

    cout << "\nAVAILABLE POLLING STATIONS (first 10):\n";
    cout << string(40, '-') << "\n";
    for (size_t i = 0; i < stations.size() && i < 10; i++) {
        cout << i + 1 << ". " << stations[i] << "\n";
    }
    if (stations.size() > 10) {
        cout << "... and " << (stations.size() - 10) << " more\n";
    }

    cout << "\nREGISTERED POLITICAL PARTIES:\n";
    cout << string(40, '-') << "\n";

    cout << "\n" << string(70, '=') << "\n";
    cout << "IMPORTANT NOTES:\n";
    cout << "1. Voters can only vote for candidates in their assigned polling station.\n";
    cout << "2. Candidates must be registered with a valid party secret code.\n";
    cout << "3. Login with your Voter ID/CNIC and password to vote.\n";
    cout << "4. Candidates can login to view their profile and station competitors.\n";
    cout << string(70, '=') << "\n";
}

int main() {
    VoterHashTable voters(DEFAULT_HASH_TABLE_SIZE);
    CandidateBTree candidates;
    VotingManager vm(DEFAULT_VOTING_DURATION); 

    cout << "Initializing B-tree...\n";

    // Test if tree is empty first
    if (candidates.isEmpty()) {
        cout << "B-tree is empty, ready for use.\n";
    }
    else {
        cout << "B-tree has data, debugging structure...\n";
        candidates.debugPrintTree();
    }

    cout << "Testing B-tree...\n";
    candidates.debugPrintTree();

    cout << " WELCOME TO ONLINE VOTING SYSTEM\n";

    int choice;

    do {
        printMainMenu();
        cin >> choice;
        clearInput();

        switch (choice) {
        case 1: // Voter Login / Registration
            vm.voterView(voters, candidates);
            break;

        case 2: // Candidate Login / Registration
            candidateLoginRegistration(candidates, vm);
            break;

        case 3: // Admin Login
            vm.adminView(voters, candidates);
            break;

        case 4: // Public Information
            printPublicInfo(candidates, voters, vm);
            break;

        case 0: // Exit
        {
            cout << "\n" << string(60, '=') << "\n";
            cout << "  THANK YOU FOR USING THE ONLINE VOTING SYSTEM!\n";
            cout << "           System shutting down...\n";
            cout << string(60, '=') << "\n";

            // Show final statistics
            cout << "\nFINAL SYSTEM STATISTICS:\n";
            cout << string(40, '-') << "\n";
            int totalVoters = voters.getTotalVoters();
            int votedCount = voters.getVotedCount();
            cout << "Total Voters Registered: " << totalVoters << "\n";
            cout << "Total Votes Cast: " << votedCount << "\n";
            if (totalVoters > 0) {
                float turnout = (float)votedCount / totalVoters * 100;
                cout << "Final Turnout: " << fixed << setprecision(1) << turnout << "%\n";
            }

            // Show candidate count
         /*   cout << "\nCANDIDATE STATISTICS:\n";
            cout << string(30, '-') << "\n";
            vector<string> parties = candidates.printFilteredCandidates();
            cout << "Registered Parties: " << parties.size() << "\n";
            if (!parties.empty()) {
                for (const string& party : parties) {
                    cout << "  - " << party << "\n";
                }
            }*/

            cout << string(40, '-') << "\n";
            cout << "\nData saved successfully. Goodbye!\n";
        }
        break;

        default:
            cout << "\nInvalid choice! Please try again.\n";
        }

        if (choice != 0) {
            cout << "\nPress Enter to continue...";
            cin.get();
        }

    } while (choice != 0);

    return 0;
}