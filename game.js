// Game Logic và UI Handlers


// Tetris Game Constants
const BOARD_WIDTH = 10;
const BOARD_HEIGHT = 20;
const CELL_SIZE = 30;

// Tetromino shapes
const TETROMINOES = [
    // I
    [
        [[0,0,0,0], [1,1,1,1], [0,0,0,0], [0,0,0,0]],
        [[0,0,1,0], [0,0,1,0], [0,0,1,0], [0,0,1,0]],
        [[0,0,0,0], [0,0,0,0], [1,1,1,1], [0,0,0,0]],
        [[0,1,0,0], [0,1,0,0], [0,1,0,0], [0,1,0,0]]
    ],
    // O
    [
        [[0,0,0,0], [0,1,1,0], [0,1,1,0], [0,0,0,0]],
        [[0,0,0,0], [0,1,1,0], [0,1,1,0], [0,0,0,0]],
        [[0,0,0,0], [0,1,1,0], [0,1,1,0], [0,0,0,0]],
        [[0,0,0,0], [0,1,1,0], [0,1,1,0], [0,0,0,0]]
    ],
    // T
    [
        [[0,0,0,0], [1,1,1,0], [0,1,0,0], [0,0,0,0]],
        [[0,1,0,0], [1,1,0,0], [0,1,0,0], [0,0,0,0]],
        [[0,1,0,0], [1,1,1,0], [0,0,0,0], [0,0,0,0]],
        [[0,1,0,0], [0,1,1,0], [0,1,0,0], [0,0,0,0]]
    ],
    // L
    [
        [[0,0,0,0], [1,1,1,0], [1,0,0,0], [0,0,0,0]],
        [[1,1,0,0], [0,1,0,0], [0,1,0,0], [0,0,0,0]],
        [[0,0,1,0], [1,1,1,0], [0,0,0,0], [0,0,0,0]],
        [[0,1,0,0], [0,1,0,0], [0,1,1,0], [0,0,0,0]]
    ],
    // J
    [
        [[0,0,0,0], [1,1,1,0], [0,0,1,0], [0,0,0,0]],
        [[0,1,0,0], [0,1,0,0], [1,1,0,0], [0,0,0,0]],
        [[1,0,0,0], [1,1,1,0], [0,0,0,0], [0,0,0,0]],
        [[0,1,1,0], [0,1,0,0], [0,1,0,0], [0,0,0,0]]
    ],
    // S
    [
        [[0,0,0,0], [0,1,1,0], [1,1,0,0], [0,0,0,0]],
        [[1,0,0,0], [1,1,0,0], [0,1,0,0], [0,0,0,0]],
        [[0,0,0,0], [0,1,1,0], [1,1,0,0], [0,0,0,0]],
        [[1,0,0,0], [1,1,0,0], [0,1,0,0], [0,0,0,0]]
    ],
    // Z
    [
        [[0,0,0,0], [1,1,0,0], [0,1,1,0], [0,0,0,0]],
        [[0,1,0,0], [1,1,0,0], [1,0,0,0], [0,0,0,0]],
        [[0,0,0,0], [1,1,0,0], [0,1,1,0], [0,0,0,0]],
        [[0,1,0,0], [1,1,0,0], [1,0,0,0], [0,0,0,0]]
    ]
];

const COLORS = [
    '#00f0f0', // I - Cyan
    '#f0f000', // O - Yellow
    '#a000f0', // T - Purple
    '#f0a000', // L - Orange
    '#0000f0', // J - Blue
    '#00f000', // S - Green
    '#f00000'  // Z - Red
];

// Game State
let gameState = {
    board: Array(BOARD_HEIGHT).fill().map(() => Array(BOARD_WIDTH).fill(0)),
    currentPiece: null,
    nextPiece: null,
    score: 0,
    level: 1,                    // Level bắt đầu từ 1
    totalLinesCleared: 0,       // Tổng số hàng đã xóa
    timeLimit: 0,
    timeLeft: 0,
    gameStarted: false,
    gameOver: false,
    dropInterval: 500,
    lastDropTime: 0,
    startTime: 0  // Thời gian bắt đầu game
};

let currentUsername = '';
let currentRoomId = -1;
let isReady = false;
let leaderboardData = [];

// Canvas setup
const boardCanvas = document.getElementById('game-board');
const boardCtx = boardCanvas.getContext('2d');
const nextCanvas = document.getElementById('next-piece-canvas');
const nextCtx = nextCanvas.getContext('2d');

// Initialize canvas size
boardCanvas.width = BOARD_WIDTH * CELL_SIZE;
boardCanvas.height = BOARD_HEIGHT * CELL_SIZE;

// UI Functions
function switchTab(tab) {
    const loginForm = document.getElementById('login-form');
    const registerForm = document.getElementById('register-form');
    const tabs = document.querySelectorAll('.tab-btn');
    
    tabs.forEach(t => t.classList.remove('active'));
    
    if (tab === 'login') {
        loginForm.style.display = 'flex';
        registerForm.style.display = 'none';
        tabs[0].classList.add('active');
    } else {
        loginForm.style.display = 'none';
        registerForm.style.display = 'flex';
        tabs[1].classList.add('active');
    }
}

function handleLogin() {
    const username = document.getElementById('login-username').value.trim();
    const password = document.getElementById('login-password').value;
    
    if (!username || !password) {
        showMessage('Vui lòng nhập đầy đủ thông tin', 'error');
        return;
    }
    
    wsClient.send(`LOGIN ${username} ${password}`);
}

function handleRegister() {
    const username = document.getElementById('register-username').value.trim();
    const password = document.getElementById('register-password').value;
    
    if (!username || !password) {
        showMessage('Vui lòng nhập đầy đủ thông tin', 'error');
        return;
    }
    
    wsClient.send(`REGISTER ${username} ${password}`);
}

function listRooms() {
    wsClient.send('LIST_ROOMS');
}

function createRoom() {
    if (!currentUsername) {
        showMessage('Vui lòng đăng nhập trước', 'error');
        return;
    }
    wsClient.send('CREATE_ROOM');
}

function joinRoom(roomId) {
    if (!roomId) {
        const input = prompt('Nhập Room ID:');
        if (input) {
            wsClient.send(`JOIN_ROOM ${input}`);
        }
    } else {
        wsClient.send(`JOIN_ROOM ${roomId}`);
    }
}

function leaveRoom() {
    wsClient.send('LEAVE_ROOM');
    currentRoomId = -1;
    isReady = false;
    document.getElementById('room-info').style.display = 'none';
}

function toggleReady() {
    if (currentRoomId < 0) {
        showMessage('Bạn chưa ở trong phòng nào', 'error');
        return;
    }
    wsClient.send('READY');
}

function selectMode(mode) {
    wsClient.send(`SET_MODE ${mode}`);
    document.getElementById('mode-selector').style.display = 'none';
}

function viewHistory() {
    if (!wsClient || !wsClient.isConnected()) {
        showMessage('Chưa kết nối đến server', 'error');
        return;
    }
    wsClient.send('VIEW_HISTORY');
    showHistoryModal();
}

function viewRecords() {
    if (!wsClient || !wsClient.isConnected()) {
        showMessage('Chưa kết nối đến server', 'error');
        return;
    }
    wsClient.send('VIEW_RECORDS');
    showRecordsModal();
}

let historyData = [];
let recordsData = [];

function showHistoryModal() {
    const modal = document.getElementById('history-modal');
    if (modal) {
        modal.style.display = 'block';
    }
}

function showRecordsModal() {
    const modal = document.getElementById('records-modal');
    if (modal) {
        modal.style.display = 'block';
    }
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.style.display = 'none';
    }
}

function handleLogout() {
    // Xác nhận đăng xuất
    if (!confirm('Bạn có chắc muốn đăng xuất?')) {
        return;
    }
    
    // Rời phòng nếu đang ở trong phòng
    if (currentRoomId >= 0) {
        wsClient.send('LEAVE_ROOM');
    }
    
    // Reset state
    currentUsername = '';
    currentRoomId = -1;
    isReady = false;
    gameState.gameStarted = false;
    gameState.gameOver = false;
    historyData = [];
    recordsData = [];
    
    // Clear form
    const loginUsername = document.getElementById('login-username');
    const loginPassword = document.getElementById('login-password');
    if (loginUsername) loginUsername.value = '';
    if (loginPassword) loginPassword.value = '';
    
    // Ẩn các panel
    document.getElementById('lobby-panel').style.display = 'none';
    document.getElementById('game-panel').style.display = 'none';
    document.getElementById('gameover-panel').style.display = 'none';
    document.getElementById('room-info').style.display = 'none';
    
    // Hiển thị auth panel
    document.getElementById('auth-panel').style.display = 'block';
    
    // Clear room list
    const roomsList = document.getElementById('rooms-list');
    if (roomsList) roomsList.innerHTML = '';
    
    // Clear username display
    const usernameDisplay = document.getElementById('current-username-display');
    if (usernameDisplay) usernameDisplay.textContent = '';
    
    showMessage('Đã đăng xuất thành công', 'success');
}

function returnToLobby() {
    document.getElementById('gameover-panel').style.display = 'none';
    document.getElementById('lobby-panel').style.display = 'block';
    gameState.gameStarted = false;
    gameState.gameOver = false;
}

// Game Functions
function initGame() {
    gameState.board = Array(BOARD_HEIGHT).fill().map(() => Array(BOARD_WIDTH).fill(0));
    gameState.score = 0;
    gameState.level = 1;
    gameState.totalLinesCleared = 0;
    gameState.dropInterval = 500;
    gameState.gameOver = false;
    gameState.lastDropTime = Date.now();
    gameState.startTime = Date.now();  // Lưu thời gian bắt đầu game
    spawnPiece();
    updateScore();
    updateLevel();
    draw();
}

function spawnPiece() {
    const shape = Math.floor(Math.random() * 7);
    gameState.currentPiece = {
        shape: shape,
        rotation: 0,
        x: Math.floor(BOARD_WIDTH / 2) - 2,
        y: 0
    };
    
    // Spawn next piece
    gameState.nextPiece = {
        shape: Math.floor(Math.random() * 7),
        rotation: 0
    };
    
    if (!pieceFits(gameState.currentPiece)) {
        gameState.gameOver = true;
        endGame();
    }
}

function pieceFits(piece, dx = 0, dy = 0, rot = null) {
    const shape = TETROMINOES[piece.shape][rot !== null ? rot : piece.rotation];
    const x = piece.x + dx;
    const y = piece.y + dy;
    
    for (let py = 0; py < 4; py++) {
        for (let px = 0; px < 4; px++) {
            if (shape[py][px]) {
                const bx = x + px;
                const by = y + py;
                if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return false;
                if (by >= 0 && gameState.board[by][bx]) return false;
            }
        }
    }
    return true;
}

function lockPiece() {
    const shape = TETROMINOES[gameState.currentPiece.shape][gameState.currentPiece.rotation];
    for (let py = 0; py < 4; py++) {
        for (let px = 0; px < 4; px++) {
            if (shape[py][px]) {
                const bx = gameState.currentPiece.x + px;
                const by = gameState.currentPiece.y + py;
                if (by >= 0 && by < BOARD_HEIGHT && bx >= 0 && bx < BOARD_WIDTH) {
                    gameState.board[by][bx] = gameState.currentPiece.shape + 1;
                }
            }
        }
    }
}

function clearLines() {
    let linesCleared = 0;
    for (let y = BOARD_HEIGHT - 1; y >= 0; y--) {
        if (gameState.board[y].every(cell => cell !== 0)) {
            gameState.board.splice(y, 1);
            gameState.board.unshift(Array(BOARD_WIDTH).fill(0));
            linesCleared++;
            y++; // Check same line again
        }
    }
    
    if (linesCleared > 0) {
                    // Cập nhật tổng số hàng đã xóa và level (tăng level mỗi 5 hàng để rõ ràng hơn)
                    gameState.totalLinesCleared += linesCleared;
                    const newLevel = Math.floor(gameState.totalLinesCleared / 5) + 1;
        if (newLevel > gameState.level) {
            gameState.level = newLevel;
            // Tính toán tốc độ rơi mới: giảm 20ms mỗi level, tối thiểu 50ms
            gameState.dropInterval = 500 - (gameState.level - 1) * 20;
            if (gameState.dropInterval < 50) gameState.dropInterval = 50;
            updateLevel();
            showMessage(`🚀 LEVEL UP! LEVEL ${gameState.level} - Tốc độ rơi: ${gameState.dropInterval}ms/ô`, 'success');
        }
        
        let points = 0;
        switch(linesCleared) {
            case 1: points = 100; break;
            case 2: points = 300; break;
            case 3: points = 500; break;
            case 4: points = 800; break;
            default: points = linesCleared * 100;
        }
        gameState.score += points;
        updateScore();
        wsClient.send(`GAME_SCORE ${gameState.score}`);
        showMessage(`${linesCleared} hàng! +${points} điểm`, 'success');
    }
}

function dropPiece() {
    if (pieceFits(gameState.currentPiece, 0, 1)) {
        gameState.currentPiece.y++;
    } else {
        lockPiece();
        clearLines();
        // Sử dụng next piece đã có làm current piece
        if (gameState.nextPiece) {
            gameState.currentPiece = {
                shape: gameState.nextPiece.shape,
                rotation: 0,
                x: Math.floor(BOARD_WIDTH / 2) - 2,
                y: 0
            };
        } else {
            // Fallback: tạo khối mới nếu không có next piece
            gameState.currentPiece = {
                shape: Math.floor(Math.random() * 7),
                rotation: 0,
                x: Math.floor(BOARD_WIDTH / 2) - 2,
                y: 0
            };
        }
        // Tạo next piece mới
        gameState.nextPiece = {
            shape: Math.floor(Math.random() * 7),
            rotation: 0
        };
        // Kiểm tra game over
        if (!pieceFits(gameState.currentPiece)) {
            gameState.gameOver = true;
            endGame();
        }
    }
}

function movePiece(dx) {
    if (pieceFits(gameState.currentPiece, dx, 0)) {
        gameState.currentPiece.x += dx;
        draw();
    }
}

function rotatePiece() {
    const newRot = (gameState.currentPiece.rotation + 1) % 4;
    if (pieceFits(gameState.currentPiece, 0, 0, newRot)) {
        gameState.currentPiece.rotation = newRot;
        draw();
    }
}

function draw() {
    // Clear board
    boardCtx.fillStyle = '#000';
    boardCtx.fillRect(0, 0, boardCanvas.width, boardCanvas.height);
    
    // Draw locked pieces
    for (let y = 0; y < BOARD_HEIGHT; y++) {
        for (let x = 0; x < BOARD_WIDTH; x++) {
            if (gameState.board[y][x]) {
                const color = COLORS[gameState.board[y][x] - 1];
                drawCell(boardCtx, x, y, color);
            }
        }
    }
    
    // Draw current piece
    if (gameState.currentPiece) {
        const shape = TETROMINOES[gameState.currentPiece.shape][gameState.currentPiece.rotation];
        const color = COLORS[gameState.currentPiece.shape];
        for (let py = 0; py < 4; py++) {
            for (let px = 0; px < 4; px++) {
                if (shape[py][px]) {
                    const x = gameState.currentPiece.x + px;
                    const y = gameState.currentPiece.y + py;
                    if (y >= 0) {
                        drawCell(boardCtx, x, y, color, true);
                    }
                }
            }
        }
    }
    
    // Draw next piece
    if (gameState.nextPiece) {
        nextCtx.fillStyle = '#000';
        nextCtx.fillRect(0, 0, nextCanvas.width, nextCanvas.height);
        const shape = TETROMINOES[gameState.nextPiece.shape][0];
        const color = COLORS[gameState.nextPiece.shape];
        const offsetX = (nextCanvas.width / CELL_SIZE - 4) / 2;
        const offsetY = (nextCanvas.height / CELL_SIZE - 4) / 2;
        for (let py = 0; py < 4; py++) {
            for (let px = 0; px < 4; px++) {
                if (shape[py][px]) {
                    drawCell(nextCtx, px + offsetX, py + offsetY, color);
                }
            }
        }
    }
}

function drawCell(ctx, x, y, color, isCurrent = false) {
    const px = x * CELL_SIZE;
    const py = y * CELL_SIZE;
    
    // Draw cell
    ctx.fillStyle = color;
    ctx.fillRect(px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2);
    
    // Draw border
    ctx.strokeStyle = isCurrent ? '#fff' : '#333';
    ctx.lineWidth = 2;
    ctx.strokeRect(px + 1, py + 1, CELL_SIZE - 2, CELL_SIZE - 2);
}

function updateScore() {
    document.getElementById('score').textContent = gameState.score;
}

function updateLevel() {
    document.getElementById('level').textContent = gameState.level;
}

function updateTime() {
    if (gameState.timeLimit > 0) {
        document.getElementById('time-container').style.display = 'flex';
        document.getElementById('time-left').textContent = gameState.timeLeft;
        document.getElementById('time-limit').textContent = gameState.timeLimit;
    }
}

function updateLeaderboard(data) {
    const container = document.getElementById('leaderboard-content');
    if (!data || data.length === 0) {
        container.innerHTML = '<p class="waiting">Đang chờ điểm số...</p>';
        return;
    }
    
    container.innerHTML = '';
    data.forEach((item, index) => {
        const div = document.createElement('div');
        div.className = `leaderboard-item rank-${index + 1}`;
        div.innerHTML = `
            <span>#${index + 1} ${item.name}</span>
            <span>${item.score} pts</span>
        `;
        container.appendChild(div);
    });
}

function endGame() {
    gameState.gameStarted = false;
    wsClient.send('GAME_END');
    document.getElementById('game-panel').style.display = 'none';
    document.getElementById('gameover-panel').style.display = 'block';
    document.getElementById('final-score').textContent = gameState.score;
}

// Game Loop
function gameLoop() {
    if (!gameState.gameStarted || gameState.gameOver) return;
    
    const now = Date.now();
    if (now - gameState.lastDropTime >= gameState.dropInterval) {
        dropPiece();
        gameState.lastDropTime = now;
        draw();
    }
    
    if (gameState.timeLimit > 0) {
        // Tính thời gian đã trôi qua từ khi bắt đầu game
        const elapsed = Math.floor((Date.now() - gameState.startTime) / 1000);
        gameState.timeLeft = Math.max(0, gameState.timeLimit - elapsed);
        updateTime();
        if (gameState.timeLeft <= 0) {
            endGame();
        }
    }
    
    requestAnimationFrame(gameLoop);
}

// Keyboard Controls
document.addEventListener('keydown', (e) => {
    if (!gameState.gameStarted || gameState.gameOver) return;
    
    switch(e.key.toLowerCase()) {
        case 'a':
        case 'arrowleft':
            movePiece(-1);
            break;
        case 'd':
        case 'arrowright':
            movePiece(1);
            break;
        case 's':
        case 'arrowdown':
            dropPiece();
            draw();
            break;
        case 'w':
        case 'arrowup':
            rotatePiece();
            break;
        case 'q':
            endGame();
            break;
    }
});

// WebSocket Message Handler
// Đợi DOM và wsClient sẵn sàng
function setupWebSocketHandlers() {
    if (typeof wsClient === 'undefined') {
        console.error('wsClient chưa được khởi tạo!');
        setTimeout(setupWebSocketHandlers, 100);
        return;
    }
    
    wsClient.onMessage((message) => {
    // Xử lý message có thể có nhiều dòng
    const lines = message.split('\n').filter(line => line.trim().length > 0);
    
    lines.forEach(line => {
        const trimmed = line.trim();
        const parts = trimmed.split(/\s+/);
        const cmd = parts[0];
        
        console.log('Handling command:', cmd, 'Full message:', trimmed);
        
        if (cmd === 'LOGIN_OK') {
            // Lưu username trước khi ẩn form
            const usernameInput = document.getElementById('login-username');
            if (usernameInput) {
                currentUsername = usernameInput.value.trim();
            }
            
            console.log('Login successful, username:', currentUsername);
            showMessage('Đăng nhập thành công!', 'success');
            
            // Cập nhật hiển thị username
            const usernameDisplay = document.getElementById('current-username-display');
            if (usernameDisplay) {
                usernameDisplay.textContent = currentUsername;
            }
            
            // Chuyển panel
            const authPanel = document.getElementById('auth-panel');
            const lobbyPanel = document.getElementById('lobby-panel');
            
            if (authPanel) {
                authPanel.style.display = 'none';
            }
            if (lobbyPanel) {
                lobbyPanel.style.display = 'block';
            } else {
                console.error('Lobby panel not found!');
            }
        } else if (cmd === 'LOGIN_FAIL') {
        showMessage('Đăng nhập thất bại: ' + parts.slice(1).join(' '), 'error');
    } else if (cmd === 'REGISTER_OK') {
        showMessage('Đăng ký thành công!', 'success');
        switchTab('login');
        } else if (cmd === 'REGISTER_FAIL') {
            showMessage('Đăng ký thất bại: ' + parts.slice(1).join(' '), 'error');
            } else if (cmd === 'ROOM_CREATED') {
            currentRoomId = parseInt(parts[1]);
            showMessage(`Đã tạo phòng ${currentRoomId}`, 'success');
            updateRoomInfo();
        } else if (cmd === 'JOIN_OK') {
            currentRoomId = parseInt(parts[1]);
            showMessage(`Đã tham gia phòng ${currentRoomId}`, 'success');
            updateRoomInfo();
        } else if (cmd === 'ROOM_LIST_BEGIN') {
            // Clear room list
            const roomsList = document.getElementById('rooms-list');
            if (roomsList) {
                roomsList.innerHTML = '';
            }
        } else if (cmd === 'ROOM') {
            // Room list item: ROOM <id> <num_players>
            const roomId = parts[1];
            const numPlayers = parts[2];
            addRoomToList(roomId, numPlayers);
        } else if (cmd === 'ROOM_LIST_END') {
            // End of room list
        } else if (cmd === 'READY_OK') {
            isReady = true;
            const readyBtn = document.getElementById('ready-btn');
            if (readyBtn) {
                readyBtn.textContent = '✅ Ready';
                readyBtn.classList.add('ready');
            }
            showMessage('Bạn đã sẵn sàng!', 'success');
        } else if (cmd === 'CHOOSE_MODE') {
            const modeSelector = document.getElementById('mode-selector');
            if (modeSelector) {
                modeSelector.style.display = 'block';
            }
        } else if (cmd === 'GAME_MODE') {
            const modeName = parts[1];
            const timeLimit = parseInt(parts[2]) || 0;
            showMessage(`Chế độ: ${modeName}${timeLimit > 0 ? ` (${timeLimit}s)` : ''}`, 'info');
        } else if (cmd === 'COUNTDOWN') {
            const count = parts[1];
            showMessage(`Game bắt đầu trong ${count}...`, 'info');
        } else if (cmd === 'START_GAME') {
            gameState.timeLimit = parseInt(parts[2]) || 0;
            gameState.timeLeft = gameState.timeLimit;
            gameState.startTime = Date.now();  // Đặt thời gian bắt đầu
            gameState.gameStarted = true;
            
            const lobbyPanel = document.getElementById('lobby-panel');
            const gamePanel = document.getElementById('game-panel');
            
            if (lobbyPanel) {
                lobbyPanel.style.display = 'none';
            }
            if (gamePanel) {
                gamePanel.style.display = 'block';
            }
            
            initGame();
            gameLoop();
            showMessage('Game bắt đầu!', 'success');
        } else if (cmd === 'SCORE_UPDATE') {
            // SCORE_UPDATE user1:score1 user2:score2 ...
            leaderboardData = [];
            for (let i = 1; i < parts.length; i++) {
                const [name, score] = parts[i].split(':');
                if (name && score) {
                    leaderboardData.push({ name, score: parseInt(score) || 0 });
                }
            }
            leaderboardData.sort((a, b) => b.score - a.score);
            updateLeaderboard(leaderboardData);
        } else if (cmd === 'FINAL_RESULTS') {
            // FINAL_RESULTS user1:score1 user2:score2 ...
            const results = [];
            for (let i = 1; i < parts.length; i++) {
                const [name, score] = parts[i].split(':');
                if (name && score) {
                    results.push({ name, score: parseInt(score) || 0 });
                }
            }
            results.sort((a, b) => b.score - a.score);
            displayFinalResults(results);
        } else if (cmd === 'GAME_END_OK') {
            // Game ended
        } else if (cmd === 'READY_STATUS') {
            // READY_STATUS user1:1 user2:0 ...
            updateReadyStatus(parts.slice(1));
        } else if (cmd === 'HISTORY_BEGIN') {
            historyData = [];
        } else if (cmd === 'HISTORY_LINE') {
            // HISTORY_LINE <text>
            const text = parts.slice(1).join(' ').replace(/\|/g, '\n');
            historyData.push(text);
        } else if (cmd === 'HISTORY_END') {
            displayHistory();
        } else if (cmd === 'RECORDS_BEGIN') {
            recordsData = [];
        } else if (cmd === 'RECORD') {
            // RECORD <username> <score>
            if (parts.length >= 3) {
                recordsData.push({
                    username: parts[1],
                    score: parseInt(parts[2]) || 0
                });
            }
        } else if (cmd === 'RECORDS_END') {
            displayRecords();
        } else if (cmd === 'PLAYER_FINISHED' || cmd === 'PLAYER_DISCONNECTED') {
            // Thông báo người chơi kết thúc hoặc disconnect
            const username = parts[1];
            const score = parts[2] || '0';
            showMessage(`${username} ${cmd === 'PLAYER_FINISHED' ? 'đã kết thúc' : 'đã rời game'} (${score} điểm)`, 'info');
        } else if (cmd.startsWith('ERROR') || cmd === 'WELCOME') {
            // Ignore welcome message and errors are handled
            if (cmd.startsWith('ERROR')) {
                showMessage('Lỗi: ' + parts.slice(1).join(' '), 'error');
            }
        } else {
            console.log('Unknown command:', cmd, 'Full message:', trimmed);
        }
    });
    });
}

// Khởi tạo handlers khi DOM sẵn sàng
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', setupWebSocketHandlers);
} else {
    setupWebSocketHandlers();
}

function addRoomToList(roomId, numPlayers) {
    const container = document.getElementById('rooms-list');
    const div = document.createElement('div');
    div.className = 'room-item';
    div.innerHTML = `
        <div>
            <strong>Phòng ${roomId}</strong>
            <span style="margin-left: 10px; color: #666;">${numPlayers}/4 người chơi</span>
        </div>
        <button onclick="joinRoom(${roomId})" style="padding: 8px 16px; background: #667eea; color: white; border: none; border-radius: 6px; cursor: pointer;">Tham Gia</button>
    `;
    container.appendChild(div);
}

function updateRoomInfo() {
    const roomIdEl = document.getElementById('room-id');
    const roomInfoEl = document.getElementById('room-info');
    
    if (roomIdEl) {
        roomIdEl.textContent = currentRoomId;
    }
    if (roomInfoEl) {
        roomInfoEl.style.display = 'block';
    }
    // TODO: Update players list from READY_STATUS
}

function updateReadyStatus(statusList) {
    const playersList = document.getElementById('players-list');
    if (!playersList) return;
    
    playersList.innerHTML = '';
    
    statusList.forEach(status => {
        const [username, ready] = status.split(':');
        if (username) {
            const div = document.createElement('div');
            div.className = `player-item ${ready === '1' ? 'ready' : ''}`;
            div.innerHTML = `
                <span>${username}</span>
                <span>${ready === '1' ? '✅ Ready' : '⏳ Waiting'}</span>
            `;
            playersList.appendChild(div);
        }
    });
}

function displayHistory() {
    const container = document.getElementById('history-content');
    if (!container) return;
    
    if (historyData.length === 0) {
        container.innerHTML = '<div class="empty-state"><p>📭 Chưa có lịch sử trận đấu nào</p><p class="hint">Chơi game để tạo lịch sử!</p></div>';
        return;
    }
    
    // Parse lịch sử thành các trận đấu
    const matches = [];
    let currentMatch = null;
    
    historyData.forEach(line => {
        line = line.trim();
        if (line.startsWith('=== Match Room')) {
            // Bắt đầu trận đấu mới
            if (currentMatch) {
                matches.push(currentMatch);
            }
            const matchInfo = line.match(/=== Match Room (\d+) - (.+) ===/);
            currentMatch = {
                roomId: matchInfo ? matchInfo[1] : '?',
                timestamp: matchInfo ? matchInfo[2] : '',
                players: []
            };
        } else if (line.startsWith('#') && currentMatch) {
            // Parse thông tin người chơi: #1: username - score points [WINNER]
            const playerMatch = line.match(/#(\d+):\s*(\S+)\s*-\s*(\d+)\s*points(?:\s*\[WINNER\])?/);
            if (playerMatch) {
                currentMatch.players.push({
                    rank: parseInt(playerMatch[1]),
                    username: playerMatch[2],
                    score: parseInt(playerMatch[3]),
                    isWinner: line.includes('[WINNER]')
                });
            }
        }
    });
    
    if (currentMatch) {
        matches.push(currentMatch);
    }
    
    // Hiển thị
    if (matches.length === 0) {
        container.innerHTML = '<div class="empty-state"><p>📭 Không tìm thấy dữ liệu trận đấu</p></div>';
        return;
    }
    
    container.innerHTML = '';
    matches.forEach((match, index) => {
        const matchCard = document.createElement('div');
        matchCard.className = 'match-card';
        
        let playersHtml = '';
        match.players.forEach(player => {
            playersHtml += `
                <div class="match-player ${player.isWinner ? 'winner' : ''}">
                    <div class="player-rank">
                        ${player.rank === 1 ? '🏆' : player.rank === 2 ? '🥈' : player.rank === 3 ? '🥉' : `#${player.rank}`}
                    </div>
                    <div class="player-info">
                        <span class="player-name">${player.username}</span>
                        <span class="player-score">${player.score.toLocaleString()} điểm</span>
                    </div>
                    ${player.isWinner ? '<span class="winner-badge">WINNER</span>' : ''}
                </div>
            `;
        });
        
        matchCard.innerHTML = `
            <div class="match-header">
                <div class="match-title">
                    <span class="match-icon">🎮</span>
                    <div>
                        <h3>Phòng ${match.roomId}</h3>
                        <span class="match-time">${match.timestamp}</span>
                    </div>
                </div>
                <div class="match-stats">
                    <span class="stat-item">👥 ${match.players.length} người chơi</span>
                </div>
            </div>
            <div class="match-players">
                ${playersHtml}
            </div>
        `;
        
        container.appendChild(matchCard);
    });
}

function displayRecords() {
    const container = document.getElementById('records-content');
    if (!container) return;
    
    if (recordsData.length === 0) {
        container.innerHTML = '<p style="text-align: center; color: #999; padding: 20px;">Chưa có kỷ lục nào</p>';
        return;
    }
    
    container.innerHTML = '';
    recordsData.forEach((record, index) => {
        const div = document.createElement('div');
        div.className = `leaderboard-item ${index < 3 ? 'rank-' + (index + 1) : ''}`;
        div.innerHTML = `
            <span>#${index + 1} ${record.username}</span>
            <span>${record.score} điểm</span>
        `;
        container.appendChild(div);
    });
}

function displayFinalResults(results) {
    const container = document.getElementById('final-results');
    container.innerHTML = '';
    results.forEach((result, index) => {
        const div = document.createElement('div');
        div.className = `final-results-item ${index === 0 ? 'winner' : ''}`;
        div.innerHTML = `
            <span>#${index + 1} ${result.name}</span>
            <span>${result.score} điểm</span>
        `;
        container.appendChild(div);
    });
}

// Start game loop when page loads
window.addEventListener('load', () => {
    draw();
});

