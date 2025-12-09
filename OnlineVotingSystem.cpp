#include <iostream>
#include <string>
#include <ctime>
#include <limits>
#include <iomanip>
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
    cout << "2. Candidate Login / Registration\n";
    cout << "3. Admin Login\n";
    cout << "4. View Public Information\n";
    cout << "0. Exit System\n";
    cout << string(70, '=') << "\n";
    cout << "Enter your choice: ";
}

// Unified candidate login/registration menu
void candidateLoginRegistration(CandidateBTree& candidates) {
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
    case 1: // Candidate Login
        // This will be handled by voting_manager.cpp
        break;
    case 2: // Candidate Registration
    {
        cout << "\n" << string(70, '=') << "\n";
        cout << "          CANDIDATE REGISTRATION PORTAL\n";
        cout << string(70, '=') << "\n";

        // First, show available parties
        candidates.displayAvailableParties();

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

    // Show available parties
    cout << "\nREGISTERED POLITICAL PARTIES:\n";
    cout << string(40, '-') << "\n";
    candidates.displayAvailableParties();

    cout << "\n" << string(70, '=') << "\n";
    cout << "IMPORTANT NOTES:\n";
    cout << "1. Voters can only vote for candidates in their assigned polling station.\n";
    cout << "2. Candidates must be registered with a valid party secret code.\n";
    cout << "3. Login with your Voter ID/CNIC and password to vote.\n";
    cout << "4. Candidates can login to view their profile and station competitors.\n";
    cout << string(70, '=') << "\n";
}

// Updated helper function for demo data - only adds if file is empty
void addDemoData(VoterHashTable& voters, CandidateBTree& candidates) {
    // Check if we already have data
    int existingVoters = voters.getTotalVoters();

    if (existingVoters == 0) {
        cout << "\nNo existing voter data found. Adding demo data...\n";

        // Add some demo voters with passwords
        voters.insertVoter("Ali Khan", "3520277777701", "M", "03123456789", "Karachi", "pass111");
        voters.insertVoter("Sara Ahmed", "3520277777702", "F", "03219876543", "Lahore", "pass222");
        voters.insertVoter("Bilal Shah", "3520277777703", "M", "03331234567", "Islamabad", "pass333");
        voters.insertVoter("Fatima Malik", "3520277777704", "F", "03451234567", "Karachi", "pass444");
        voters.insertVoter("Usman Raza", "3520277777705", "M", "03561234567", "Lahore", "pass555");
        voters.insertVoter("Alex Taylor", "3520277777706", "O", "03661234567", "Islamabad", "pass666");

        cout << "Demo voters added successfully!\n";
    }
    else {
        cout << "\nFound " << existingVoters << " existing voters in database.\n";
    }

    // Add demo candidates using the new registration system
    cout << "\nChecking for existing candidates...\n";

    // Test if we have candidates by trying to display parties
    vector<string> parties = candidates.getAllParties();
    if (parties.empty()) {
        cout << "No existing candidates found. Adding demo candidates...\n";

        // Using direct insertion for demo (bypassing secret code for testing)
        // Note: Using the correct insertCandidate signature
        candidates.insertCandidate("CID00001", "Imran Khan", "4220100012345",
            "PTI", "KHI01", "pti123");
        candidates.insertCandidate("CID00002", "Bilawal Bhutto", "4220100023456",
            "PPP", "KHI01", "ppp123");
        candidates.insertCandidate("CID00003", "Nawaz Sharif", "4220100034567",
            "PMLN", "LHR01", "pmln123");
        candidates.insertCandidate("CID00004", "Asad Qaiser", "4220100045678",
            "PTI", "ISB01", "pti456");
        candidates.insertCandidate("CID00005", "Independent Candidate", "4220100056789",
            "IND", "KHI02", "indep123");

        cout << "Demo candidates added successfully!\n";
    }
    else {
        cout << "Found existing candidates in database.\n";
    }

    cout << "\nTest Credentials:\n";
    cout << "===================\n";
    cout << "VOTER:\n";
    cout << "CNIC: 4220112345678\n";
    cout << "Password: password123\n";
    cout << "\nCANDIDATES:\n";
    cout << "Imran Khan - ID: CID00001, Password: pti123\n";
    cout << "Bilawal Bhutto - ID: CID00002, Password: ppp123\n";
    cout << "Nawaz Sharif - ID: CID00003, Password: pmln123\n";
    cout << "Asad Qaiser - ID: CID00004, Password: pti456\n";
    cout << "Independent Candidate - ID: CID00005, Password: indep123\n";
    cout << "\nADMIN:\n";
    cout << "Password: admin123\n";
    cout << "===================\n";
}

int main() {
    // Initialize with larger hash table for more voters
    VoterHashTable voters(50);
    CandidateBTree candidates;
    VotingManager vm(120); // 2 minutes default voting duration

    // Welcome message
    cout << "\n" << string(80, '*') << "\n";
    cout << "          WELCOME TO ENHANCED ONLINE VOTING SYSTEM\n";
    cout << "               (Geographic Constituency Based)\n";
    cout << string(80, '*') << "\n";
    cout << "\nSystem Features:\n";
    cout << "1. Voter registration with full details (CNIC, Contact, Town)\n";
    cout << "2. Auto-generated Voter IDs (VID00001 format)\n";
    cout << "3. Candidate registration with party secret codes\n";
    cout << "4. Auto-generated Candidate IDs (CID00001 format)\n";
    cout << "5. Polling station assignment based on town\n";
    cout << "6. Candidates contest from specific polling stations\n";
    cout << "7. Voters can only vote for candidates in their station\n";
    cout << "8. Party-based candidate registration with secret codes\n";
    cout << "9. Geographic statistics and reports\n";
    cout << "10. Secure login for voters, candidates, and admin\n";
    cout << string(80, '*') << "\n\n";

    // Add demo data only if file is empty
    addDemoData(voters, candidates);

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
            candidateLoginRegistration(candidates);
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

            // Show final statistics - variables declared inside case block
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
            cout << "\nCANDIDATE STATISTICS:\n";
            cout << string(30, '-') << "\n";
            vector<string> parties = candidates.getAllParties();
            cout << "Registered Parties: " << parties.size() << "\n";
            if (!parties.empty()) {
                for (const string& party : parties) {
                    cout << "  - " << party << "\n";
                }
            }

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