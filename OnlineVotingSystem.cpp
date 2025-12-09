#include <iostream>
#include <string>
#include <ctime>
#include <limits>
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
    cout << "           (With Geographic Constituency System)\n";
    cout << string(70, '=') << "\n";
    cout << "1. Voter Login / Registration\n";
    cout << "2. Candidate Login\n";
    cout << "3. Admin Login\n";
    cout << "4. View Public Information\n";
    cout << "5. Voter Self-Registration\n";
    cout << "0. Exit System\n";
    cout << string(70, '=') << "\n";
    cout << "Enter your choice: ";
}

// New function for voter self-registration
void voterSelfRegistration(VoterHashTable& voters) {
    cout << "\n" << string(60, '=') << "\n";
    cout << "        VOTER SELF-REGISTRATION PORTAL\n";
    cout << string(60, '=') << "\n";

    string name, cnic, gender, contact, town;

    cout << "\nPlease provide your details:\n";
    cout << "Full Name: ";
    getline(cin, name);

    cout << "CNIC (13 digits, no dashes): ";
    getline(cin, cnic);

    cout << "Gender (Male/Female/Other): ";
    getline(cin, gender);

    cout << "Contact Number (11 digits, starting with 03): ";
    getline(cin, contact);

    cout << "Town/City: ";
    getline(cin, town);

    cout << "\n" << string(50, '-') << "\n";
    cout << "Please review your information:\n";
    cout << "Name: " << name << "\n";
    cout << "CNIC: " << cnic << "\n";
    cout << "Gender: " << gender << "\n";
    cout << "Contact: " << contact << "\n";
    cout << "Town: " << town << "\n";
    cout << string(50, '-') << "\n";

    cout << "\nConfirm registration? (yes/no): ";
    string confirm;
    getline(cin, confirm);

    if (confirm == "yes" || confirm == "y" || confirm == "Y") {
        voters.insertVoter(name, cnic, gender, contact, town);
    }
    else {
        cout << "\nRegistration cancelled.\n";
    }
}

void printPublicInfo(CandidateBTree& candidates, VoterHashTable& voters, VotingManager& vm) {
    cout << "\n" << string(70, '=') << "\n";
    cout << "              PUBLIC INFORMATION PORTAL\n";
    cout << string(70, '=') << "\n";

    // Show voting status
    cout << "\nVOTING STATUS:\n";
    cout << string(40, '-') << "\n";
    if (true) { // We'll check voting status properly later
        cout << "Status: ";
        // This is a placeholder - we need to access vm's votingActive
        cout << "Check with admin for current status\n";
    }

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

    cout << "\nAVAILABLE POLLING STATIONS:\n";
    cout << string(40, '-') << "\n";
    for (size_t i = 0; i < stations.size() && i < 10; i++) { // Show first 10
        cout << stations[i];
        if (i < stations.size() - 1 && i < 9) cout << ", ";
    }
    if (stations.size() > 10) {
        cout << "... and " << (stations.size() - 10) << " more";
    }
    cout << "\n";

    cout << "\nCANDIDATES INFORMATION:\n";
    cout << string(40, '-') << "\n";
    cout << "Note: For detailed candidate list, please login as candidate or admin.\n";
    cout << "Total candidates registered: (Login to view)\n";

    cout << "\n" << string(70, '=') << "\n";
    cout << "Note: Voters can only vote for candidates in their assigned polling station.\n";
    cout << string(70, '=') << "\n";
}

// Helper function for demo data
void addDemoData(VoterHashTable& voters, CandidateBTree& candidates) {
    cout << "\nAdding demo data for testing...\n";

    // Add some demo voters
    voters.insertVoter("Ali Khan", "4220112345678", "Male", "03123456789", "Karachi");
    voters.insertVoter("Sara Ahmed", "4220176543210", "Female", "03219876543", "Lahore");
    voters.insertVoter("Bilal Shah", "4220155512345", "Male", "03331234567", "Islamabad");
    voters.insertVoter("Fatima Malik", "4220166623456", "Female", "03451234567", "Karachi");
    voters.insertVoter("Usman Raza", "4220199934567", "Male", "03561234567", "Lahore");

    // Add some demo candidates
    candidates.insertCandidate("CKHI0101", "Imran Khan", "4220100012345", "PTI", "Bat", "KHI01", "NA-247 Karachi", "pti123");
    candidates.insertCandidate("CKHI0102", "Bilawal Bhutto", "4220100023456", "PPP", "Arrow", "KHI01", "NA-247 Karachi", "ppp123");
    candidates.insertCandidate("CLHR0101", "Nawaz Sharif", "4220100034567", "PML-N", "Lion", "LHR01", "NA-130 Lahore", "pmln123");
    candidates.insertCandidate("CISB0101", "Asad Qaiser", "4220100045678", "PTI", "Bat", "ISB01", "NA-54 Islamabad", "pti456");

    cout << "Demo data added successfully!\n";
}

int main() {
    // Initialize with larger hash table for more voters
    VoterHashTable voters(50);
    CandidateBTree candidates;
    VotingManager vm(300); // 5 minutes default voting duration

    // Welcome message
    cout << "\n" << string(80, '*') << "\n";
    cout << "          WELCOME TO ENHANCED ONLINE VOTING SYSTEM\n";
    cout << "               (Geographic Constituency Based)\n";
    cout << string(80, '*') << "\n";
    cout << "\nSystem Features:\n";
    cout << "1. Voter registration with full details (CNIC, Contact, Town)\n";
    cout << "2. Auto-generated Voter IDs (V00123KA01 format)\n";
    cout << "3. Polling station assignment based on town\n";
    cout << "4. Candidates contest from specific polling stations\n";
    cout << "5. Voters can only vote for candidates in their station\n";
    cout << "6. Geographic statistics and reports\n";
    cout << string(80, '*') << "\n\n";

    // Ask if user wants to add demo data
    cout << "Would you like to add demo data for testing? (yes/no): ";
    string demoChoice;
    getline(cin, demoChoice);

    if (demoChoice == "yes" || demoChoice == "y" || demoChoice == "Y") {
        addDemoData(voters, candidates);
        cout << "\nPress Enter to continue to main menu...";
        cin.get();
    }

    int choice;

    do {
        printMainMenu();
        cin >> choice;
        clearInput();

        switch (choice) {
        case 1: // Voter Login
            vm.voterView(voters, candidates);
            break;

        case 2: // Candidate Login
            vm.candidateView(candidates);
            break;

        case 3: // Admin Login
            vm.adminView(voters, candidates);
            break;

        case 4: // Public Information
            printPublicInfo(candidates, voters, vm);
            break;

        case 5: // Voter Self-Registration
            voterSelfRegistration(voters);
            break;

        case 0: // Exit
            cout << "\n" << string(60, '=') << "\n";
            cout << "  THANK YOU FOR USING THE ONLINE VOTING SYSTEM!\n";
            cout << "           System shutting down...\n";
            cout << string(60, '=') << "\n";

            // Show final statistics
            cout << "\nFINAL SYSTEM STATISTICS:\n";
            cout << string(40, '-') << "\n";
            cout << "Total Voters Registered: " << voters.getTotalVoters() << "\n";
            cout << "Total Votes Cast: " << voters.getVotedCount() << "\n";
            if (voters.getTotalVoters() > 0) {
                float turnout = (float)voters.getVotedCount() / voters.getTotalVoters() * 100;
                cout << "Final Turnout: " << fixed << setprecision(1) << turnout << "%\n";
            }
            cout << string(40, '-') << "\n";
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