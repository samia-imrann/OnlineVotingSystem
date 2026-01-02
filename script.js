// const SERVER_URL = 'http://localhost:8080';
// let currentUser = null;
// let selectedCandidate = null;

// // Utility Functions
// function showScreen(screenId) {
//     document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
//     const screen = document.getElementById(screenId);
//     if (screen) screen.classList.add('active');
    
//     if (screenId === 'voterRegister' || screenId === 'candidateRegister') {
//         loadTowns();
//     }
    
//     if (screenId === 'results') {
//         loadResults();
//     }
// }

// function showMessage(elementId, message, type) {
//     const msgElement = document.getElementById(elementId);
//     if (!msgElement) return;
    
//     msgElement.textContent = message;
//     msgElement.className = `message show ${type}`;
//     setTimeout(() => {
//         msgElement.classList.remove('show');
//     }, 5000);
// }

// async function sendRequest(command, data = '') {
//     try {
//         const response = await fetch(SERVER_URL, {
//             method: 'POST',
//             headers: {
//                 'Content-Type': 'text/plain',
//             },
//             body: `${command} ${data}`
//         });
        
//         if (!response.ok) {
//             throw new Error(`HTTP error! status: ${response.status}`);
//         }
        
//         const text = await response.text();
//         try {
//             return JSON.parse(text);
//         } catch (e) {
//             throw new Error('Invalid JSON response');
//         }
//     } catch (error) {
//         throw new Error('Connection failed: ' + error.message);
//     }
// }

// // Load Towns
// async function loadTowns() {
//     try {
//         const response = await sendRequest('GET_TOWNS');
//         if (response.status === 'success') {
//             const voterSelect = document.getElementById('voterTown');
//             const candidateSelect = document.getElementById('candidateTown');
            
//             [voterSelect, candidateSelect].forEach(select => {
//                 if (select) {
//                     select.innerHTML = '<option value="">Select Town</option>';
//                     response.towns.forEach(town => {
//                         const option = document.createElement('option');
//                         option.value = town;
//                         option.textContent = town;
//                         select.appendChild(option);
//                     });
//                 }
//             });
//         }
//     } catch (error) {
//         console.error('Failed to load towns:', error);
//     }
// }

// // Voter Registration
// async function handleVoterRegister(event) {
//     event.preventDefault();
    
//     const name = document.getElementById('voterName')?.value;
//     const cnic = document.getElementById('voterCnic')?.value;
//     const gender = document.getElementById('voterGender')?.value;
//     const contact = document.getElementById('voterContact')?.value;
//     const town = document.getElementById('voterTown')?.value;
//     const password = document.getElementById('voterPassword')?.value;
    
//     if (!name || !cnic || !gender || !contact || !town || !password) {
//         showMessage('voterRegisterMsg', 'All fields are required', 'error');
//         return;
//     }
    
//     const data = `${name}|${cnic}|${gender}|${contact}|${town}|${password}`;
    
//     try {
//         const response = await sendRequest('REGISTER_VOTER', data);
        
//         if (response.status === 'success') {
//             showMessage('voterRegisterMsg', 
//                 `Registration successful! Your Voter ID: ${response.voterID}. Station: ${response.station}`, 
//                 'success');
//             if (event.target.reset) event.target.reset();
//         } else {
//             showMessage('voterRegisterMsg', response.message, 'error');
//         }
//     } catch (error) {
//         showMessage('voterRegisterMsg', 'Registration failed: ' + error.message, 'error');
//     }
// }

// // Voter Login
// async function handleVoterLogin(event) {
//     event.preventDefault();
    
//     const id = document.getElementById('voterLoginId')?.value;
//     const password = document.getElementById('voterLoginPass')?.value;
    
//     if (!id || !password) {
//         showMessage('voterLoginMsg', 'ID and password required', 'error');
//         return;
//     }
    
//     const data = `${id}|${password}`;
    
//     try {
//         const response = await sendRequest('LOGIN_VOTER', data);
        
//         if (response.status === 'success') {
//             currentUser = {
//                 type: 'voter',
//                 id: response.voterID,
//                 name: response.name,
//                 station: response.station,
//                 town: response.town,
//                 hasVoted: response.hasVoted
//             };
            
//             showVoterDashboard();
//         } else {
//             showMessage('voterLoginMsg', response.message, 'error');
//         }
//     } catch (error) {
//         showMessage('voterLoginMsg', 'Login failed: ' + error.message, 'error');
//     }
// }

// // Show Voter Dashboard
// async function showVoterDashboard() {
//     if (!currentUser) return;
    
//     const nameDisplay = document.getElementById('voterNameDisplay');
//     const idDisplay = document.getElementById('voterIdDisplay');
//     const stationDisplay = document.getElementById('voterStationDisplay');
//     const statusDisplay = document.getElementById('voterStatusDisplay');
//     const votingSection = document.getElementById('votingSection');
    
//     if (nameDisplay) nameDisplay.textContent = currentUser.name || 'N/A';
//     if (idDisplay) idDisplay.textContent = currentUser.id || 'N/A';
//     if (stationDisplay) stationDisplay.textContent = currentUser.station || 'N/A';
//     if (statusDisplay) statusDisplay.textContent = currentUser.hasVoted ? 'Already Voted' : 'Not Voted';
    
//     if (currentUser.hasVoted && votingSection) {
//         votingSection.innerHTML = '<p style="color: #666; text-align: center; padding: 20px;">You have already cast your vote.</p>';
//     }
    
//     await loadCandidates();
//     showScreen('voterDashboard');
// }

// // Load Candidates
// async function loadCandidates() {
//     if (!currentUser) return;
    
//     try {
//         const response = await sendRequest('GET_CANDIDATES', currentUser.station);
        
//         if (response.status === 'success') {
//             const candidatesList = document.getElementById('candidatesList');
//             if (!candidatesList) return;
            
//             candidatesList.innerHTML = '';
            
//             if (response.candidates.length === 0) {
//                 candidatesList.innerHTML = '<p style="color: #666;">No candidates in your polling station yet.</p>';
//                 return;
//             }
            
//             response.candidates.forEach(candidate => {
//                 const card = document.createElement('div');
//                 card.className = 'candidate-card';
//                 card.onclick = () => selectCandidate(candidate);
                
//                 card.innerHTML = `
//                     <div class="candidate-info-left">
//                         <h4>${candidate.name || 'Unknown'}</h4>
//                         <p>Party: ${candidate.party || 'N/A'} | ID: ${candidate.id || 'N/A'}</p>
//                     </div>
//                     <div class="candidate-votes">
//                         ${candidate.votes || 0} votes
//                     </div>
//                 `;
                
//                 candidatesList.appendChild(card);
//             });
//         }
//     } catch (error) {
//         console.error('Failed to load candidates:', error);
//         const candidatesList = document.getElementById('candidatesList');
//         if (candidatesList) {
//             candidatesList.innerHTML = '<p style="color: #666;">Error loading candidates.</p>';
//         }
//     }
// }

// // Select Candidate
// function selectCandidate(candidate) {
//     if (currentUser?.hasVoted) return;
    
//     selectedCandidate = candidate;
    
//     document.querySelectorAll('.candidate-card').forEach(card => {
//         card.classList.remove('selected');
//     });
    
//     if (event && event.target && event.target.closest('.candidate-card')) {
//         event.target.closest('.candidate-card').classList.add('selected');
//     }
// }

// // Voting functions with null checks
// function showVotingModal() {
//     if (!selectedCandidate) {
//         alert('Please select a candidate first');
//         return;
//     }
    
//     if (currentUser?.hasVoted) {
//         alert('You have already voted');
//         return;
//     }
    
//     const details = document.getElementById('voteConfirmDetails');
//     if (details) {
//         details.innerHTML = `
//             <p><strong>Candidate:</strong> ${selectedCandidate.name || 'Unknown'}</p>
//             <p><strong>Party:</strong> ${selectedCandidate.party || 'N/A'}</p>
//             <p><strong>ID:</strong> ${selectedCandidate.id || 'N/A'}</p>
//             <p style="margin-top: 15px; color: #d32f2f;">⚠️ This action cannot be undone!</p>
//         `;
//     }
    
//     const modal = document.getElementById('votingModal');
//     if (modal) modal.classList.add('active');
// }

// function closeVotingModal() {
//     const modal = document.getElementById('votingModal');
//     const msg = document.getElementById('voteMsg');
//     if (modal) modal.classList.remove('active');
//     if (msg) msg.classList.remove('show');
// }

// // Confirm Vote
// async function confirmVote() {
//     if (!currentUser || !selectedCandidate) return;
    
//     const data = `${currentUser.id}|${selectedCandidate.id}`;
    
//     try {
//         const response = await sendRequest('CAST_VOTE', data);
        
//         if (response.status === 'success') {
//             showMessage('voteMsg', 'Vote cast successfully!', 'success');
//             currentUser.hasVoted = true;
            
//             setTimeout(() => {
//                 closeVotingModal();
//                 showVoterDashboard();
//             }, 2000);
//         } else {
//             showMessage('voteMsg', response.message, 'error');
//         }
//     } catch (error) {
//         showMessage('voteMsg', 'Failed to cast vote: ' + error.message, 'error');
//     }
// }

// // Candidate Registration
// async function handleCandidateRegister(event) {
//     event.preventDefault();
    
//     const name = document.getElementById('candidateName')?.value;
//     const cnic = document.getElementById('candidateCnic')?.value;
//     const party = document.getElementById('candidateParty')?.value;
//     const secret = document.getElementById('candidateSecret')?.value;
//     const town = document.getElementById('candidateTown')?.value;
//     const password = document.getElementById('candidatePassword')?.value;
    
//     if (!name || !cnic || !party || !secret || !town || !password) {
//         showMessage('candidateRegisterMsg', 'All fields are required', 'error');
//         return;
//     }
    
//     const data = `${name}|${cnic}|${party}|${secret}|${town}|${password}`;
    
//     try {
//         const response = await sendRequest('REGISTER_CANDIDATE', data);
        
//         if (response.status === 'success') {
//             showMessage('candidateRegisterMsg', response.message, 'success');
//             if (event.target.reset) event.target.reset();
//         } else {
//             showMessage('candidateRegisterMsg', response.message, 'error');
//         }
//     } catch (error) {
//         showMessage('candidateRegisterMsg', 'Registration failed: ' + error.message, 'error');
//     }
// }

// // Candidate Login
// async function handleCandidateLogin(event) {
//     event.preventDefault();
    
//     const id = document.getElementById('candidateLoginId')?.value;
//     const password = document.getElementById('candidateLoginPass')?.value;
    
//     if (!id || !password) {
//         showMessage('candidateLoginMsg', 'ID and password required', 'error');
//         return;
//     }
    
//     const data = `${id}|${password}`;
    
//     try {
//         const response = await sendRequest('LOGIN_CANDIDATE', data);
        
//         if (response.status === 'success') {
//             currentUser = {
//                 type: 'candidate',
//                 id: response.id,
//                 name: response.name,
//                 party: response.party,
//                 station: response.station,
//                 votes: response.votes
//             };
            
//             showCandidateDashboard();
//         } else {
//             showMessage('candidateLoginMsg', response.message, 'error');
//         }
//     } catch (error) {
//         showMessage('candidateLoginMsg', 'Login failed: ' + error.message, 'error');
//     }
// }

// // Show Candidate Dashboard
// function showCandidateDashboard() {
//     if (!currentUser) return;
    
//     const nameDisplay = document.getElementById('candidateNameDisplay');
//     const idDisplay = document.getElementById('candidateIdDisplay');
//     const partyDisplay = document.getElementById('candidatePartyDisplay');
//     const stationDisplay = document.getElementById('candidateStationDisplay');
//     const votesDisplay = document.getElementById('candidateVotesDisplay');
    
//     if (nameDisplay) nameDisplay.textContent = currentUser.name || 'N/A';
//     if (idDisplay) idDisplay.textContent = currentUser.id || 'N/A';
//     if (partyDisplay) partyDisplay.textContent = currentUser.party || 'N/A';
//     if (stationDisplay) stationDisplay.textContent = currentUser.station || 'N/A';
//     if (votesDisplay) votesDisplay.textContent = currentUser.votes || 0;
    
//     showScreen('candidateDashboard');
// }

// // Load Results
// async function loadResults() {
//     try {
//         const response = await sendRequest('GET_RESULTS');
        
//         if (response.status === 'success') {
//             const resultsList = document.getElementById('resultsList');
//             if (!resultsList) return;
            
//             resultsList.innerHTML = '';
            
//             if (!response.results || response.results.length === 0) {
//                 resultsList.innerHTML = '<p style="color: #666;">No results available yet.</p>';
//                 return;
//             }
            
//             // Sort by votes
//             const sorted = response.results.sort((a, b) => (b.votes || 0) - (a.votes || 0));
            
//             sorted.forEach((result, index) => {
//                 const item = document.createElement('div');
//                 item.className = index === 0 ? 'result-item winner' : 'result-item';
                
//                 item.innerHTML = `
//                     <div class="result-info">
//                         <h4>${index === 0 ? '🏆 ' : ''}${result.name || 'Unknown'}</h4>
//                         <p>Party: ${result.party || 'N/A'} | Station: ${result.station || 'N/A'}</p>
//                     </div>
//                     <div class="result-votes">${result.votes || 0}</div>
//                 `;
                
//                 resultsList.appendChild(item);
//             });
//         }
//     } catch (error) {
//         console.error('Failed to load results:', error);
//         const resultsList = document.getElementById('resultsList');
//         if (resultsList) {
//             resultsList.innerHTML = '<p style="color: #666;">Error loading results.</p>';
//         }
//     }
// }

// // Logout
// function logout() {
//     currentUser = null;
//     selectedCandidate = null;
//     showScreen('mainMenu');
// }

// // Initialize - fixed null check
// if (typeof document !== 'undefined') {
//     document.addEventListener('DOMContentLoaded', () => {
//         loadTowns();
//         console.log('Online Voting System initialized');
//     });
// } else {
//     console.log('Running in non-browser environment');
// }

const SERVER_URL = 'http://localhost:8080';
let currentUser = null;
let selectedCandidate = null;
let adminStats = {
    totalVoters: 0,
    totalCandidates: 0,
    votesCast: 0,
    turnoutRate: '0%'
};

// Utility Functions
function showScreen(screenId) {
    document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));
    const screen = document.getElementById(screenId);
    if (screen) screen.classList.add('active');
    
    // Load specific data for each screen
    switch(screenId) {
        case 'voterRegister':
        case 'candidateRegister':
            loadTowns();
            break;
        case 'results':
            loadResults();
            break;
        case 'adminDashboard':
            loadAdminStats();
            updateActivityLog();
            break;
        case 'adminVoters':
            loadVotersTable();
            break;
        case 'adminCandidates':
            loadCandidatesTable();
            break;
    }
}

function showMessage(elementId, message, type = 'info') {
    const msgElement = document.getElementById(elementId);
    if (!msgElement) return;
    
    msgElement.textContent = message;
    msgElement.className = `message show ${type}`;
    setTimeout(() => {
        msgElement.classList.remove('show');
    }, 5000);
}

function showLoading(text = 'Processing...') {
    const loadingModal = document.getElementById('loadingModal');
    const loadingText = document.getElementById('loadingText');
    if (loadingText) loadingText.textContent = text;
    if (loadingModal) loadingModal.classList.add('active');
}

function hideLoading() {
    const loadingModal = document.getElementById('loadingModal');
    if (loadingModal) loadingModal.classList.remove('active');
}

function showConfirmation(title, message, confirmCallback) {
    const modal = document.getElementById('confirmationModal');
    const modalTitle = document.getElementById('modalTitle');
    const modalMessage = document.getElementById('modalMessage');
    const confirmBtn = document.getElementById('modalConfirmBtn');
    
    if (modalTitle) modalTitle.textContent = title;
    if (modalMessage) modalMessage.textContent = message;
    
    confirmBtn.onclick = function() {
        closeModal();
        if (confirmCallback) confirmCallback();
    };
    
    if (modal) modal.classList.add('active');
}

function closeModal() {
    const modal = document.getElementById('confirmationModal');
    const loadingModal = document.getElementById('loadingModal');
    if (modal) modal.classList.remove('active');
    if (loadingModal) loadingModal.classList.remove('active');
}

async function sendRequest(command, data = '') {
    showLoading();
    try {
        const response = await fetch(SERVER_URL, {
            method: 'POST',
            headers: {
                'Content-Type': 'text/plain',
            },
            body: `${command} ${data}`
        });
        
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        
        const text = await response.text();
        hideLoading();
        try {
            return JSON.parse(text);
        } catch (e) {
            console.error('Invalid JSON:', text);
            throw new Error('Invalid server response');
        }
    } catch (error) {
        hideLoading();
        console.error('Request failed:', error);
        throw error;
    }
}

// Load Towns
async function loadTowns() {
    try {
        const response = await sendRequest('GET_TOWNS');
        if (response.status === 'success') {
            const voterSelect = document.getElementById('voterTown');
            const candidateSelect = document.getElementById('candidateTown');
            
            [voterSelect, candidateSelect].forEach(select => {
                if (select) {
                    select.innerHTML = '<option value="">Select Town</option>';
                    response.towns.forEach(town => {
                        const option = document.createElement('option');
                        option.value = town;
                        option.textContent = town;
                        select.appendChild(option);
                    });
                }
            });
        }
    } catch (error) {
        console.error('Failed to load towns:', error);
    }
}

// Voter Registration
async function handleVoterRegister(event) {
    event.preventDefault();
    
    const name = document.getElementById('voterName')?.value;
    const cnic = document.getElementById('voterCnic')?.value;
    const gender = document.getElementById('voterGender')?.value;
    const contact = document.getElementById('voterContact')?.value;
    const town = document.getElementById('voterTown')?.value;
    const password = document.getElementById('voterPassword')?.value;
    
    if (!name || !cnic || !gender || !contact || !town || !password) {
        showMessage('voterRegisterMsg', 'All fields are required', 'error');
        return;
    }
    
    if (cnic.length !== 13) {
        showMessage('voterRegisterMsg', 'CNIC must be 13 digits', 'error');
        return;
    }
    
    if (password.length < 6) {
        showMessage('voterRegisterMsg', 'Password must be at least 6 characters', 'error');
        return;
    }
    
    const data = `${name}|${cnic}|${gender}|${contact}|${town}|${password}`;
    
    try {
        const response = await sendRequest('REGISTER_VOTER', data);
        
        if (response.status === 'success') {
            showMessage('voterRegisterMsg', 
                `Registration successful! Your Voter ID: ${response.voterID}. Station: ${response.station}`, 
                'success');
            // Auto login after registration
            currentUser = {
                type: 'voter',
                id: response.voterID,
                name: name,
                station: response.station,
                town: response.town,
                hasVoted: false
            };
            showVoterDashboard();
        } else {
            showMessage('voterRegisterMsg', response.message, 'error');
        }
    } catch (error) {
        showMessage('voterRegisterMsg', 'Registration failed: ' + error.message, 'error');
    }
}

// Voter Login
async function handleVoterLogin(event) {
    event.preventDefault();
    
    const id = document.getElementById('voterLoginId')?.value;
    const password = document.getElementById('voterLoginPass')?.value;
    
    if (!id || !password) {
        showMessage('voterLoginMsg', 'ID and password required', 'error');
        return;
    }
    
    const data = `${id}|${password}`;
    
    try {
        const response = await sendRequest('LOGIN_VOTER', data);
        
        if (response.status === 'success') {
            currentUser = {
                type: 'voter',
                id: response.voterID,
                name: response.name,
                station: response.station,
                town: response.town,
                hasVoted: response.hasVoted
            };
            
            showVoterDashboard();
        } else {
            showMessage('voterLoginMsg', response.message, 'error');
        }
    } catch (error) {
        showMessage('voterLoginMsg', 'Login failed: ' + error.message, 'error');
    }
}

// Show Voter Dashboard
async function showVoterDashboard() {
    if (!currentUser) return;
    
    // Update voter info display
    const nameDisplay = document.getElementById('voterNameDisplay');
    const idDisplay = document.getElementById('voterIdDisplay');
    const stationDisplay = document.getElementById('voterStationDisplay');
    const statusDisplay = document.getElementById('voterStatusDisplay');
    
    if (nameDisplay) nameDisplay.textContent = currentUser.name || 'N/A';
    if (idDisplay) idDisplay.textContent = currentUser.id || 'N/A';
    if (stationDisplay) stationDisplay.textContent = currentUser.station || 'N/A';
    
    if (statusDisplay) {
        statusDisplay.textContent = currentUser.hasVoted ? 'Already Voted' : 'Not Voted';
        statusDisplay.className = currentUser.hasVoted ? 'badge badge-warning' : 'badge badge-success';
    }
    
    // Load candidates for voting
    await loadCandidates();
    showScreen('voterDashboard');
}

// Load Candidates for Voting
async function loadCandidates() {
    if (!currentUser) return;
    
    try {
        const response = await sendRequest('GET_CANDIDATES', currentUser.station);
        
        if (response.status === 'success') {
            const candidatesList = document.getElementById('candidatesList');
            if (!candidatesList) return;
            
            candidatesList.innerHTML = '';
            
            if (response.candidates.length === 0) {
                candidatesList.innerHTML = '<p style="color: #666; text-align: center; padding: 20px;">No candidates in your polling station yet.</p>';
                return;
            }
            
            response.candidates.forEach(candidate => {
                const card = document.createElement('div');
                card.className = 'candidate-card';
                card.onclick = () => selectCandidate(candidate);
                
                card.innerHTML = `
                    <div class="candidate-info-left">
                        <h4>${candidate.name || 'Unknown'}</h4>
                        <p>Party: ${candidate.party || 'N/A'} | ID: ${candidate.id || 'N/A'}</p>
                    </div>
                    <div class="candidate-votes">
                        ${candidate.votes || 0} votes
                    </div>
                `;
                
                candidatesList.appendChild(card);
            });
            
            // Update voting section visibility
            const votingSection = document.getElementById('votingSection');
            if (votingSection) {
                if (currentUser.hasVoted) {
                    votingSection.innerHTML = `
                        <div style="text-align: center; padding: 30px; background: #f8f9fa; border-radius: 10px;">
                            <i class="fas fa-check-circle" style="font-size: 48px; color: #28a745;"></i>
                            <h3 style="margin: 20px 0 10px 0;">You have already voted!</h3>
                            <p style="color: #666;">Thank you for participating in the election.</p>
                        </div>
                    `;
                }
            }
        }
    } catch (error) {
        console.error('Failed to load candidates:', error);
        const candidatesList = document.getElementById('candidatesList');
        if (candidatesList) {
            candidatesList.innerHTML = '<p style="color: #666; text-align: center; padding: 20px;">Error loading candidates. Please try again.</p>';
        }
    }
}

// Select Candidate
function selectCandidate(candidate) {
    if (currentUser?.hasVoted) {
        alert('You have already voted!');
        return;
    }
    
    selectedCandidate = candidate;
    
    // Update UI
    document.querySelectorAll('.candidate-card').forEach(card => {
        card.classList.remove('selected');
    });
    
    event.currentTarget.classList.add('selected');
    
    // Enable vote button
    const voteBtn = document.getElementById('voteButton');
    if (voteBtn) voteBtn.disabled = false;
}

// Cast Vote
async function castVote() {
    if (!currentUser || !selectedCandidate) {
        alert('Please select a candidate first');
        return;
    }
    
    if (currentUser.hasVoted) {
        alert('You have already voted');
        return;
    }
    
    showConfirmation(
        'Confirm Your Vote',
        `Are you sure you want to vote for ${selectedCandidate.name} (${selectedCandidate.party})?`,
        async () => {
            const data = `${currentUser.id}|${selectedCandidate.id}`;
            
            try {
                const response = await sendRequest('CAST_VOTE', data);
                
                if (response.status === 'success') {
                    showMessage('voteMsg', 'Vote cast successfully!', 'success');
                    currentUser.hasVoted = true;
                    
                    // Update UI
                    setTimeout(() => {
                        showVoterDashboard();
                    }, 2000);
                } else {
                    showMessage('voteMsg', response.message, 'error');
                }
            } catch (error) {
                showMessage('voteMsg', 'Failed to cast vote: ' + error.message, 'error');
            }
        }
    );
}

// Candidate Registration
async function handleCandidateRegister(event) {
    event.preventDefault();
    
    const name = document.getElementById('candidateName')?.value;
    const cnic = document.getElementById('candidateCnic')?.value;
    const party = document.getElementById('candidateParty')?.value;
    const secret = document.getElementById('candidateSecret')?.value;
    const town = document.getElementById('candidateTown')?.value;
    const password = document.getElementById('candidatePassword')?.value;
    
    if (!name || !cnic || !party || !secret || !town || !password) {
        showMessage('candidateRegisterMsg', 'All fields are required', 'error');
        return;
    }
    
    if (password.length < 6) {
        showMessage('candidateRegisterMsg', 'Password must be at least 6 characters', 'error');
        return;
    }
    
    const data = `${name}|${cnic}|${party}|${secret}|${town}|${password}`;
    
    try {
        const response = await sendRequest('REGISTER_CANDIDATE', data);
        
        if (response.status === 'success') {
            showMessage('candidateRegisterMsg', 'Registration successful! You can now login.', 'success');
            if (event.target.reset) event.target.reset();
        } else {
            showMessage('candidateRegisterMsg', response.message, 'error');
        }
    } catch (error) {
        showMessage('candidateRegisterMsg', 'Registration failed: ' + error.message, 'error');
    }
}

// Candidate Login
async function handleCandidateLogin(event) {
    event.preventDefault();
    
    const id = document.getElementById('candidateLoginId')?.value;
    const password = document.getElementById('candidateLoginPass')?.value;
    
    if (!id || !password) {
        showMessage('candidateLoginMsg', 'ID and password required', 'error');
        return;
    }
    
    const data = `${id}|${password}`;
    
    try {
        const response = await sendRequest('LOGIN_CANDIDATE', data);
        
        if (response.status === 'success') {
            currentUser = {
                type: 'candidate',
                id: response.id,
                name: response.name,
                party: response.party,
                station: response.station,
                votes: response.votes
            };
            
            showCandidateDashboard();
        } else {
            showMessage('candidateLoginMsg', response.message, 'error');
        }
    } catch (error) {
        showMessage('candidateLoginMsg', 'Login failed: ' + error.message, 'error');
    }
}

// Show Candidate Dashboard
function showCandidateDashboard() {
    if (!currentUser) return;
    
    // Update candidate info display
    const nameDisplay = document.getElementById('candidateNameDisplay');
    const idDisplay = document.getElementById('candidateIdDisplay');
    const partyDisplay = document.getElementById('candidatePartyDisplay');
    const stationDisplay = document.getElementById('candidateStationDisplay');
    const votesDisplay = document.getElementById('candidateVotesDisplay');
    
    if (nameDisplay) nameDisplay.textContent = currentUser.name || 'N/A';
    if (idDisplay) idDisplay.textContent = currentUser.id || 'N/A';
    if (partyDisplay) partyDisplay.textContent = currentUser.party || 'N/A';
    if (stationDisplay) stationDisplay.textContent = currentUser.station || 'N/A';
    if (votesDisplay) votesDisplay.textContent = currentUser.votes || 0;
    
    showScreen('candidateDashboard');
}

// Admin Functions
function showAdminLogin() {
    showScreen('adminLogin');
}

async function handleAdminLogin(event) {
    event.preventDefault();
    
    const username = document.getElementById('adminUsername')?.value;
    const password = document.getElementById('adminPassword')?.value;
    
    if (!username || !password) {
        showMessage('adminLoginMsg', 'Username and password required', 'error');
        return;
    }
    
    // Simple admin authentication
    if (username === 'admin' && password === 'admin123') {
        currentUser = { type: 'admin', username: 'admin' };
        showScreen('adminDashboard');
        loadAdminStats();
        updateActivityLog();
        logActivity('Admin login', 'admin', 'System accessed');
    } else {
        showMessage('adminLoginMsg', 'Invalid admin credentials!', 'error');
    }
}

function adminLogout() {
    currentUser = null;
    showScreen('mainMenu');
}

function showAdminSection(section) {
    // Hide all sections
    document.querySelectorAll('.admin-section').forEach(s => {
        s.classList.remove('active');
    });
    
    // Update nav links
    document.querySelectorAll('.admin-nav a').forEach(a => {
        a.classList.remove('active');
    });
    
    // Show selected section
    const sectionElement = document.getElementById('admin' + capitalize(section));
    if (sectionElement) sectionElement.classList.add('active');
    
    // Activate nav link
    if (event.target.tagName === 'A') {
        event.target.classList.add('active');
    } else if (event.target.parentElement.tagName === 'A') {
        event.target.parentElement.classList.add('active');
    }
    
    // Load specific data for section
    switch(section) {
        case 'voters':
            loadVotersTable();
            break;
        case 'candidates':
            loadCandidatesTable();
            break;
    }
}

async function loadAdminStats() {
    try {
        // Get voter statistics
        const votersResponse = await sendRequest('GET_RESULTS');
        const results = votersResponse.results || [];
        
        // Count votes
        let totalVotes = 0;
        results.forEach(candidate => {
            totalVotes += candidate.votes || 0;
        });
        
        // Estimate counts (in a real system, these would come from separate API calls)
        adminStats = {
            totalVoters: 1500 + Math.floor(Math.random() * 500), // Simulated
            totalCandidates: results.length,
            votesCast: totalVotes,
            turnoutRate: '65%' // Simulated
        };
        
        // Update UI
        document.getElementById('totalVoters').textContent = adminStats.totalVoters;
        document.getElementById('totalCandidates').textContent = adminStats.totalCandidates;
        document.getElementById('votesCast').textContent = adminStats.votesCast;
        document.getElementById('turnoutRate').textContent = adminStats.turnoutRate;
        
    } catch (error) {
        console.error('Failed to load admin stats:', error);
    }
}

async function loadVotersTable() {
    try {
        // In a real system, this would come from an API
        // For now, show simulated data
        const tbody = document.getElementById('votersTable');
        if (!tbody) return;
        
        tbody.innerHTML = '';
        
        const simulatedVoters = [
            { id: 'VID0000001', name: 'Ahmed Khan', cnic: '4210112345678', town: 'Karachi', station: 'KHI01', status: 'Voted' },
            { id: 'VID0000002', name: 'Fatima Ali', cnic: '4220154321098', town: 'Lahore', station: 'LHR01', status: 'Not Voted' },
            { id: 'VID0000003', name: 'Muhammad Hassan', cnic: '4230167890123', town: 'Islamabad', station: 'ISB01', status: 'Voted' },
            { id: 'VID0000004', name: 'Ayesha Malik', cnic: '4240176543210', town: 'Rawalpindi', station: 'RWP01', status: 'Not Voted' },
            { id: 'VID0000005', name: 'Bilal Ahmed', cnic: '4250189012345', town: 'Karachi', station: 'KHI02', status: 'Voted' }
        ];
        
        simulatedVoters.forEach(voter => {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>${voter.id}</td>
                <td>${voter.name}</td>
                <td>${voter.cnic}</td>
                <td>${voter.town}</td>
                <td>${voter.station}</td>
                <td><span class="badge ${voter.status === 'Voted' ? 'badge-success' : 'badge-warning'}">${voter.status}</span></td>
                <td>
                    <button class="btn btn-sm btn-primary" onclick="viewVoter('${voter.id}')">View</button>
                    <button class="btn btn-sm btn-danger" onclick="deleteVoter('${voter.id}')">Delete</button>
                </td>
            `;
            tbody.appendChild(row);
        });
        
    } catch (error) {
        console.error('Failed to load voters:', error);
    }
}

async function loadCandidatesTable() {
    try {
        const response = await sendRequest('GET_RESULTS');
        
        if (response.status === 'success') {
            const tbody = document.getElementById('candidatesTable');
            if (!tbody) return;
            
            tbody.innerHTML = '';
            
            response.results.forEach(candidate => {
                const row = document.createElement('tr');
                row.innerHTML = `
                    <td>${candidate.id}</td>
                    <td>${candidate.name}</td>
                    <td>${candidate.party}</td>
                    <td>${getTownFromStation(candidate.station)}</td>
                    <td>${candidate.station}</td>
                    <td>${candidate.votes}</td>
                    <td>
                        <button class="btn btn-sm btn-primary" onclick="viewCandidate('${candidate.id}')">View</button>
                        <button class="btn btn-sm btn-danger" onclick="deleteCandidate('${candidate.id}')">Delete</button>
                    </td>
                `;
                tbody.appendChild(row);
            });
        }
    } catch (error) {
        console.error('Failed to load candidates:', error);
    }
}

function getTownFromStation(station) {
    const prefix = station.substring(0, 3);
    const townMap = {
        'KHI': 'Karachi',
        'LHR': 'Lahore',
        'ISB': 'Islamabad',
        'RWP': 'Rawalpindi',
        'FSD': 'Faisalabad',
        'MUL': 'Multan',
        'PES': 'Peshawar',
        'QTA': 'Quetta'
    };
    return townMap[prefix] || 'Unknown';
}

function viewVoter(voterId) {
    alert(`View voter details for: ${voterId}\n\nIn a real system, this would show detailed voter information.`);
}

function deleteVoter(voterId) {
    showConfirmation(
        'Delete Voter',
        `Are you sure you want to delete voter ${voterId}? This action cannot be undone.`,
        () => {
            showMessage('adminVotersMsg', `Voter ${voterId} deleted successfully`, 'success');
            // In a real system, call API to delete voter
            setTimeout(() => loadVotersTable(), 1000);
        }
    );
}

function viewCandidate(candidateId) {
    alert(`View candidate details for: ${candidateId}\n\nIn a real system, this would show detailed candidate information.`);
}

function deleteCandidate(candidateId) {
    showConfirmation(
        'Delete Candidate',
        `Are you sure you want to delete candidate ${candidateId}? This action cannot be undone.`,
        () => {
            showMessage('adminCandidatesMsg', `Candidate ${candidateId} deleted successfully`, 'success');
            // In a real system, call API to delete candidate
            setTimeout(() => loadCandidatesTable(), 1000);
        }
    );
}

function exportVoters() {
    alert('Exporting voter data...\n\nIn a real system, this would download a CSV/Excel file.');
}

function startVotingSession() {
    showConfirmation(
        'Start Voting Session',
        'Are you sure you want to start the voting session? This will allow all registered voters to cast their votes.',
        () => {
            document.getElementById('votingStatus').textContent = 'Active';
            document.getElementById('votingStatus').style.color = '#10b981';
            showMessage('adminVotingMsg', 'Voting session started successfully', 'success');
            logActivity('Start voting', 'admin', 'Voting session initiated');
        }
    );
}

function stopVotingSession() {
    showConfirmation(
        'Stop Voting Session',
        'Are you sure you want to stop the voting session? This will prevent further voting.',
        () => {
            document.getElementById('votingStatus').textContent = 'Inactive';
            document.getElementById('votingStatus').style.color = '#ef4444';
            showMessage('adminVotingMsg', 'Voting session stopped successfully', 'success');
            logActivity('Stop voting', 'admin', 'Voting session terminated');
        }
    );
}

function resetVotingSystem() {
    showConfirmation(
        'Reset Voting System',
        'WARNING: This will reset all votes and voting status! All vote counts will be set to zero. Are you sure?',
        async () => {
            try {
                // In a real system, call API to reset votes
                showMessage('adminVotingMsg', 'Voting system reset successfully', 'success');
                logActivity('System reset', 'admin', 'All votes reset to zero');
                loadAdminStats();
                loadCandidatesTable();
            } catch (error) {
                showMessage('adminVotingMsg', 'Reset failed: ' + error.message, 'error');
            }
        }
    );
}

function generateVoterReport() {
    alert('Generating voter report...\n\nIn a real system, this would generate an Excel/PDF report.');
}

function generateCandidateReport() {
    alert('Generating candidate report...\n\nIn a real system, this would generate a CSV report.');
}

function generateResultsReport() {
    alert('Generating results report...\n\nIn a real system, this would generate a PDF report.');
}

function saveSystemConfig() {
    const duration = document.getElementById('votingDurationInput')?.value;
    const newPassword = document.getElementById('newAdminPassword')?.value;
    
    if (duration && duration > 0) {
        showMessage('adminSystemMsg', `Voting duration set to ${duration} minutes`, 'success');
    }
    
    if (newPassword && newPassword.length >= 6) {
        showMessage('adminSystemMsg', 'Admin password updated successfully', 'success');
        document.getElementById('newAdminPassword').value = '';
    }
    
    logActivity('System config', 'admin', 'Configuration updated');
}

function clearAllData() {
    showConfirmation(
        'Clear All Data',
        'WARNING: This will delete ALL data - voters, candidates, and votes! This action cannot be undone. Are you absolutely sure?',
        () => {
            showMessage('adminSystemMsg', 'All data cleared successfully. System reset complete.', 'success');
            logActivity('Clear all data', 'admin', 'Complete system reset performed');
            setTimeout(() => {
                loadAdminStats();
                loadVotersTable();
                loadCandidatesTable();
            }, 1000);
        }
    );
}

// Load Results
async function loadResults() {
    try {
        const response = await sendRequest('GET_RESULTS');
        
        if (response.status === 'success') {
            const resultsList = document.getElementById('resultsList');
            if (!resultsList) return;
            
            resultsList.innerHTML = '';
            
            if (!response.results || response.results.length === 0) {
                resultsList.innerHTML = '<p style="color: #666; text-align: center; padding: 30px;">No results available yet.</p>';
                return;
            }
            
            // Sort by votes descending
            const sorted = response.results.sort((a, b) => (b.votes || 0) - (a.votes || 0));
            
            sorted.forEach((result, index) => {
                const item = document.createElement('div');
                item.className = index === 0 ? 'result-item winner' : 'result-item';
                
                item.innerHTML = `
                    <div class="result-rank">${index + 1}</div>
                    <div class="result-info">
                        <h4>${index === 0 ? '🏆 ' : ''}${result.name || 'Unknown'}</h4>
                        <p>Party: ${result.party || 'N/A'} | Station: ${result.station || 'N/A'}</p>
                    </div>
                    <div class="result-votes">
                        <span class="vote-count">${result.votes || 0}</span>
                        <span class="vote-label">votes</span>
                    </div>
                `;
                
                resultsList.appendChild(item);
            });
            
            // Update result statistics
            const totalVotes = sorted.reduce((sum, candidate) => sum + (candidate.votes || 0), 0);
            const leadingCandidate = sorted[0];
            
            const statsDiv = document.getElementById('resultsStats');
            if (statsDiv) {
                statsDiv.innerHTML = `
                    <div class="stats-grid">
                        <div class="stat-card">
                            <i class="fas fa-user-tie"></i>
                            <div class="stat-value">${sorted.length}</div>
                            <div class="stat-label">Total Candidates</div>
                        </div>
                        <div class="stat-card">
                            <i class="fas fa-vote-yea"></i>
                            <div class="stat-value">${totalVotes}</div>
                            <div class="stat-label">Total Votes</div>
                        </div>
                        <div class="stat-card">
                            <i class="fas fa-trophy"></i>
                            <div class="stat-value">${leadingCandidate ? leadingCandidate.name : 'None'}</div>
                            <div class="stat-label">Leading</div>
                        </div>
                    </div>
                `;
            }
        }
    } catch (error) {
        console.error('Failed to load results:', error);
        const resultsList = document.getElementById('resultsList');
        if (resultsList) {
            resultsList.innerHTML = '<p style="color: #666; text-align: center; padding: 30px;">Error loading results. Please try again.</p>';
        }
    }
}

// Activity Log
function logActivity(action, user, details) {
    const logs = JSON.parse(localStorage.getItem('voting_logs') || '[]');
    logs.unshift({
        timestamp: new Date().toLocaleTimeString(),
        action,
        user,
        details
    });
    
    // Keep only last 50 logs
    if (logs.length > 50) logs.length = 50;
    
    localStorage.setItem('voting_logs', JSON.stringify(logs));
    updateActivityLog();
}

function updateActivityLog() {
    const logs = JSON.parse(localStorage.getItem('voting_logs') || '[]');
    const tbody = document.getElementById('activityLog');
    if (!tbody) return;
    
    tbody.innerHTML = '';
    
    if (logs.length === 0) {
        tbody.innerHTML = `
            <tr>
                <td colspan="4" style="text-align: center; color: #666; padding: 20px;">
                    No activity recorded yet.
                </td>
            </tr>
        `;
        return;
    }
    
    logs.forEach(log => {
        const tr = document.createElement('tr');
        tr.innerHTML = `
            <td>${log.timestamp}</td>
            <td>${log.action}</td>
            <td>${log.user}</td>
            <td>${log.details}</td>
        `;
        tbody.appendChild(tr);
    });
}

// Utility Functions
function capitalize(str) {
    return str.charAt(0).toUpperCase() + str.slice(1);
}

function logout() {
    currentUser = null;
    selectedCandidate = null;
    showScreen('mainMenu');
}

// Initialize System
function initSystem() {
    console.log('Online Voting System initialized');
    
    // Check if server is reachable
    checkServerConnection();
    
    // Initialize activity log if not exists
    if (!localStorage.getItem('voting_logs')) {
        localStorage.setItem('voting_logs', JSON.stringify([]));
    }
    
    // Add some sample activity logs
    if (JSON.parse(localStorage.getItem('voting_logs') || '[]').length === 0) {
        logActivity('System start', 'System', 'Online Voting System initialized');
        logActivity('Server started', 'System', 'Backend server running on port 8080');
    }
}

async function checkServerConnection() {
    try {
        const response = await fetch(SERVER_URL, { method: 'HEAD' });
        console.log('Server connection: OK');
        document.body.classList.add('server-connected');
    } catch (error) {
        console.warn('Server connection failed:', error);
        document.body.classList.add('server-disconnected');
        
        // Show warning
        const warning = document.createElement('div');
        warning.style.cssText = `
            position: fixed;
            top: 10px;
            right: 10px;
            background: #f59e0b;
            color: white;
            padding: 10px 15px;
            border-radius: 5px;
            z-index: 9999;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        `;
        warning.innerHTML = `
            <i class="fas fa-exclamation-triangle"></i>
            Server not connected. Please start the backend server.
        `;
        document.body.appendChild(warning);
        
        setTimeout(() => warning.remove(), 5000);
    }
}

// Initialize on load
document.addEventListener('DOMContentLoaded', () => {
    initSystem();
    
    // Back to top button
    window.addEventListener('scroll', function() {
        const btn = document.getElementById('backToTop');
        if (window.scrollY > 300) {
            btn.classList.add('show');
        } else {
            btn.classList.remove('show');
        }
    });
    
    document.getElementById('backToTop')?.addEventListener('click', function() {
        window.scrollTo({ top: 0, behavior: 'smooth' });
    });
    
    // Add CSS for result items
    const style = document.createElement('style');
    style.textContent = `
        .result-item {
            background: white;
            border-radius: 10px;
            padding: 20px;
            margin-bottom: 15px;
            display: flex;
            align-items: center;
            box-shadow: 0 2px 10px rgba(0,0,0,0.05);
            transition: transform 0.2s;
        }
        .result-item:hover {
            transform: translateY(-2px);
            box-shadow: 0 4px 15px rgba(0,0,0,0.1);
        }
        .result-item.winner {
            border-left: 5px solid #f59e0b;
            background: #fff9e6;
        }
        .result-rank {
            font-size: 24px;
            font-weight: bold;
            color: #6366f1;
            margin-right: 20px;
            min-width: 40px;
        }
        .result-info {
            flex-grow: 1;
        }
        .result-info h4 {
            margin: 0 0 5px 0;
            color: #1f2937;
        }
        .result-info p {
            margin: 0;
            color: #6b7280;
            font-size: 0.9rem;
        }
        .result-votes {
            text-align: right;
        }
        .vote-count {
            font-size: 28px;
            font-weight: bold;
            color: #6366f1;
            display: block;
        }
        .vote-label {
            font-size: 0.8rem;
            color: #6b7280;
        }
        .server-connected .header h1::after {
            content: " • Online";
            color: #10b981;
            font-size: 1rem;
        }
        .server-disconnected .header h1::after {
            content: " • Offline";
            color: #ef4444;
            font-size: 1rem;
        }
    `;
    document.head.appendChild(style);
});