// WebSocket Proxy Server
// Kết nối WebSocket clients (trình duyệt) với TCP server (server.c)
// Chạy: node websocket-proxy.js

const WebSocket = require('ws');
const net = require('net');

const WS_PORT = 8080;
const TCP_SERVER_HOST = 'localhost';
const TCP_SERVER_PORT = 5555;

// Tạo WebSocket server - bind với 0.0.0.0 để nhận connection từ mọi IP
const wss = new WebSocket.Server({ 
    port: WS_PORT,
    host: '0.0.0.0'  // ⭐ Cho phép kết nối từ mọi IP
});

console.log(`WebSocket Proxy Server đang chạy trên port ${WS_PORT}`);
console.log(`Kết nối đến TCP server tại ${TCP_SERVER_HOST}:${TCP_SERVER_PORT}`);

wss.on('connection', (ws) => {
    console.log('WebSocket client đã kết nối');
    
    // Tạo kết nối TCP đến server C
    const tcpClient = new net.Socket();
    let tcpConnected = false;
    
    tcpClient.connect(TCP_SERVER_PORT, TCP_SERVER_HOST, () => {
        console.log('Đã kết nối đến TCP server');
        tcpConnected = true;
    });
    
    // Buffer để xử lý message nhiều dòng
    let tcpBuffer = '';
    
    // Nhận dữ liệu từ TCP server và gửi đến WebSocket client
    tcpClient.on('data', (data) => {
        tcpBuffer += data.toString();
        
        // Xử lý từng dòng
        const lines = tcpBuffer.split('\n');
        tcpBuffer = lines.pop() || ''; // Giữ lại phần chưa hoàn chỉnh
        
        lines.forEach(line => {
            if (line.trim().length > 0) {
                console.log('TCP -> WS:', line.trim());
                if (ws.readyState === WebSocket.OPEN) {
                    ws.send(line + '\n');
                }
            }
        });
    });
    
    // Nhận dữ liệu từ WebSocket client và gửi đến TCP server
    ws.on('message', (message) => {
        const text = message.toString();
        console.log('WS -> TCP:', text.trim());
        if (tcpConnected) {
            tcpClient.write(text);
        }
    });
    
    // Xử lý đóng kết nối
    ws.on('close', () => {
        console.log('WebSocket client đã ngắt kết nối');
        if (tcpConnected) {
            tcpClient.end();
        }
    });
    
    tcpClient.on('close', () => {
        console.log('TCP connection đã đóng');
        tcpConnected = false;
        if (ws.readyState === WebSocket.OPEN) {
            ws.close();
        }
    });
    
    tcpClient.on('error', (err) => {
        console.error('TCP error:', err);
        if (ws.readyState === WebSocket.OPEN) {
            ws.send('ERROR TCP connection failed\n');
        }
    });
    
    ws.on('error', (err) => {
        console.error('WebSocket error:', err);
    });
});

console.log('Đang chờ kết nối từ WebSocket clients...');
console.log('Mở trình duyệt và truy cập: file:///path/to/index.html');

