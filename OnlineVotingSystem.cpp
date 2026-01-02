// OnlineVotingSystem.cpp
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

// UPDATED: Simplified candidateLoginRegistration function
void candidateLoginRegistration(CandidateBTree& candidates, VotingManager& vm) {
    int choice;

    do {
        cout << "\n" << string(60, '=') << "\n";
        cout << "        CANDIDATE LOGIN / REGISTRATION\n";
        cout << string(60, '=') << "\n";
        cout << "1. Login (Existing Candidate)\n";
        cout << "2. Register (New Candidate)\n";
        cout << "0. Back to Main Menu\n";
        cout << string(60, '=') << "\n";
        cout << "Enter your choice: ";

        cin >> choice;
        clearInput();

        switch (choice) {
        case 1:
            vm.candidateView(candidates);
            break;
        case 2:
            // Use the updated registration view from VotingManager
            vm.candidateRegistrationView(candidates);
            break;
        case 0:
            return;
        default:
            cout << "\nInvalid choice! Please try again.\n";
        }

        if (choice != 0) {
            cout << "\nPress Enter to continue...";
            cin.get();
        }
    } while (choice != 0);
}

void printPublicInfo(CandidateBTree& candidates, VoterHashTable& voters, VotingManager& vm) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "              PUBLIC INFORMATION PORTAL\n";
    cout << string(70, '=') << "\n";

    // Show candidate information
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
    vector<string> towns = GeographicConfig::getAllTowns();
    vector<string> stations = voters.getAllPollingStations();

    cout << "\nGEOGRAPHIC DISTRIBUTION:\n";
    cout << string(40, '-') << "\n";
    cout << "Available Towns: " << towns.size() << "\n";
    cout << "Registered Towns: " << voters.getAllTowns().size() << "\n";
    cout << "Active Polling Stations: " << stations.size() << "\n";

    cout << "\nAVAILABLE TOWNS FOR REGISTRATION:\n";
    cout << string(40, '-') << "\n";
    for (size_t i = 0; i < towns.size() && i < 15; i++) {
        cout << i + 1 << ". " << towns[i] << "\n";
    }
    if (towns.size() > 15) {
        cout << "... and " << (towns.size() - 15) << " more\n";
    }

    cout << "\nREGISTERED POLITICAL PARTIES:\n";
    cout << string(40, '-') << "\n";
    for (const auto& party : PARTY_SECRET_CODES) {
        cout << " - " << party.first << "\n";
    }

    cout << "\nIMPORTANT ELECTION RULES:\n";
    cout << string(40, '-') << "\n";
    cout << "1. Only one candidate per party per polling station\n";
    cout << "2. Polling stations are assigned based on town\n";
    cout << "3. Voters can only vote for candidates in their polling station\n";
    cout << "4. Candidates must have valid party secret code\n";

    cout << "\n" << string(70, '=') << "\n";
    cout << "NOTES:\n";
    cout << "- Voters login with Voter ID/CNIC and password\n";
    cout << "- Candidates login with Candidate ID and password\n";
    cout << "- Admin login requires admin password\n";
    cout << string(70, '=') << "\n";
}

int main() {

        cout << "\n" << string(70, '=') << "\n";
    cout << "         ENHANCED ONLINE VOTING SYSTEM\n";
    cout << "              TOWN-BASED VOTING SYSTEM\n";
    cout << string(70, '=') << "\n\n";

    cout << "Initializing system components...\n";

    VoterHashTable voters(DEFAULT_HASH_TABLE_SIZE);
    CandidateBTree candidates;
    VotingManager vm(DEFAULT_VOTING_DURATION);


    // Test if tree is empty first
    if (candidates.isEmpty()) {
        cout << "Candidate database is empty, ready for registrations.\n";
    }
    else {
        int candidateCount = candidates.getCandidateCount();
        cout << "Loaded " << candidateCount << " candidates from database.\n";
    }

    // Show available towns
    vector<string> towns = GeographicConfig::getAllTowns();
    cout << "\nAvailable Towns in the System: " << towns.size() << "\n";
    for (const auto& town : towns) {
        cout << "  - " << town << "\n";
    }

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

            int candidateCount = candidates.getCandidateCount();
            cout << "Total Candidates Registered: " << candidateCount << "\n";

            vector<string> registeredTowns = voters.getAllTowns();
            cout << "Towns with Registered Voters: " << registeredTowns.size() << "\n";

            vector<string> activeStations = voters.getAllPollingStations();
            cout << "Active Polling Stations: " << activeStations.size() << "\n";

            cout << string(40, '-') << "\n";
            cout << "\nAll data saved successfully. Goodbye!\n";
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