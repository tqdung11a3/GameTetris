// WebSocket Client để kết nối với WebSocket Proxy Server
// Proxy server sẽ chuyển đổi WebSocket <-> TCP socket với server C

class TetrisWebSocketClient {
    constructor() {
        this.ws = null;
        this.reconnectAttempts = 0;
        this.maxReconnectAttempts = 5;
        this.reconnectDelay = 3000;
        this.onMessageHandlers = [];
        this.onConnectHandlers = [];
        this.onDisconnectHandlers = [];
    }

    connect(serverUrl = 'ws://localhost:8080') {
        return new Promise((resolve, reject) => {
            try {
                this.ws = new WebSocket(serverUrl);
                
                this.ws.onopen = () => {
                    console.log('Connected to server');
                    this.reconnectAttempts = 0;
                    this.onConnectHandlers.forEach(handler => handler());
                    updateConnectionStatus(true);
                    resolve();
                };

                this.ws.onmessage = (event) => {
                    const message = event.data;
                    console.log('Received:', message);
                    this.onMessageHandlers.forEach(handler => handler(message));
                };

                this.ws.onerror = (error) => {
                    console.error('WebSocket error:', error);
                    showMessage('Lỗi kết nối', 'error');
                };

                this.ws.onclose = () => {
                    console.log('Disconnected from server');
                    updateConnectionStatus(false);
                    this.onDisconnectHandlers.forEach(handler => handler());
                    
                    // Tự động kết nối lại
                    if (this.reconnectAttempts < this.maxReconnectAttempts) {
                        this.reconnectAttempts++;
                        setTimeout(() => {
                            console.log(`Reconnecting... (${this.reconnectAttempts}/${this.maxReconnectAttempts})`);
                            this.connect(serverUrl);
                        }, this.reconnectDelay);
                    } else {
                        showMessage('Không thể kết nối đến server', 'error');
                    }
                };
            } catch (error) {
                reject(error);
            }
        });
    }

    send(message) {
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(message + '\n');
            console.log('Sent:', message);
            return true;
        } else {
            console.error('WebSocket not connected');
            showMessage('Chưa kết nối đến server', 'error');
            return false;
        }
    }

    onMessage(handler) {
        this.onMessageHandlers.push(handler);
    }

    onConnect(handler) {
        this.onConnectHandlers.push(handler);
    }

    onDisconnect(handler) {
        this.onDisconnectHandlers.push(handler);
    }

    disconnect() {
        if (this.ws) {
            this.ws.close();
            this.ws = null;
        }
    }

    isConnected() {
        return this.ws && this.ws.readyState === WebSocket.OPEN;
    }
}

// Global WebSocket client instance
const wsClient = new TetrisWebSocketClient();

// Helper functions - Định nghĩa trước khi sử dụng
function updateConnectionStatus(connected) {
    const statusEl = document.getElementById('connection-status');
    if (statusEl) {
        statusEl.textContent = connected ? 'Connected' : 'Disconnected';
        statusEl.className = 'value ' + (connected ? 'connected' : 'disconnected');
    }
}

function showMessage(text, type = 'info') {
    const container = document.getElementById('message-container');
    if (!container) return;

    const message = document.createElement('div');
    message.className = `message-toast ${type}`;
    message.textContent = text;
    
    container.appendChild(message);
    
    setTimeout(() => {
        message.style.animation = 'slideIn 0.3s ease-out reverse';
        setTimeout(() => message.remove(), 300);
    }, 3000);
}

// Expose showMessage globally để game.js có thể sử dụng
window.showMessage = showMessage;
window.updateConnectionStatus = updateConnectionStatus;

// Initialize connection when page loads
window.addEventListener('load', () => {
    // Đảm bảo DOM đã load xong
    if (typeof wsClient === 'undefined') {
        console.error('wsClient chưa được khởi tạo!');
        showMessage('Lỗi: WebSocket client chưa được khởi tạo', 'error');
        return;
    }
    
    // Tự động phát hiện WebSocket URL từ URL hiện tại
    // Nếu truy cập từ http://192.168.1.70:3000 → ws://192.168.1.70:8080
    // Nếu truy cập từ http://localhost:3000 → ws://localhost:8080
    function getWebSocketUrl() {
        // Kiểm tra localStorage trước (nếu user đã set thủ công)
        const savedUrl = localStorage.getItem('wsServerUrl');
        if (savedUrl) {
            return savedUrl;
        }
        
        // Tự động phát hiện từ URL hiện tại
        const hostname = window.location.hostname;
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        const wsPort = 8080;
        
        // Nếu là localhost hoặc 127.0.0.1, dùng localhost
        if (hostname === 'localhost' || hostname === '127.0.0.1') {
            return `ws://localhost:${wsPort}`;
        }
        
        // Nếu là IP thực tế, dùng IP đó
        return `${protocol}//${hostname}:${wsPort}`;
    }
    
    const wsUrl = getWebSocketUrl();
    console.log('Connecting to WebSocket:', wsUrl);
    
    wsClient.connect(wsUrl).catch(err => {
        console.error('Failed to connect:', err);
        showMessage('Không thể kết nối đến server. Đảm bảo WebSocket proxy server đang chạy.', 'error');
    });
});

