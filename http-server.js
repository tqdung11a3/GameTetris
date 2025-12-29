// Simple HTTP Server để serve file HTML
// Chạy: node http-server.js

const http = require('http');
const fs = require('fs');
const path = require('path');
const os = require('os');

const PORT = 3000;
const MIME_TYPES = {
    '.html': 'text/html',
    '.css': 'text/css',
    '.js': 'application/javascript',
    '.json': 'application/json',
    '.png': 'image/png',
    '.jpg': 'image/jpeg',
    '.gif': 'image/gif',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon'
};

const server = http.createServer((req, res) => {
    // Parse URL
    let filePath = '.' + req.url;
    if (filePath === './') {
        filePath = './index.html';
    }

    // Get file extension
    const extname = String(path.extname(filePath)).toLowerCase();
    const contentType = MIME_TYPES[extname] || 'application/octet-stream';

    // Read and serve file
    fs.readFile(filePath, (error, content) => {
        if (error) {
            if (error.code === 'ENOENT') {
                res.writeHead(404, { 'Content-Type': 'text/html' });
                res.end('<h1>404 - File Not Found</h1>', 'utf-8');
            } else {
                res.writeHead(500);
                res.end(`Server Error: ${error.code}`, 'utf-8');
            }
        } else {
            res.writeHead(200, { 'Content-Type': contentType });
            res.end(content, 'utf-8');
        }
    });
});

// Lấy IP thực tế của máy (không phải localhost)
function getLocalIP() {
    const interfaces = os.networkInterfaces();
    for (const name of Object.keys(interfaces)) {
        for (const iface of interfaces[name]) {
            // Bỏ qua internal (localhost) và non-IPv4
            if (iface.family === 'IPv4' && !iface.internal) {
                return iface.address;
            }
        }
    }
    return 'localhost';
}

const HOST = '0.0.0.0'; // Bind với mọi interface để nhận connection từ mọi IP
const localIP = getLocalIP();

server.listen(PORT, HOST, () => {
    console.log('='.repeat(50));
    console.log('🚀 HTTP Server đang chạy!');
    console.log('='.repeat(50));
    console.log(`📂 Serving files từ: ${__dirname}`);
    console.log(`🌐 Mở trình duyệt và truy cập:`);
    console.log(`   Local:  http://localhost:${PORT}`);
    console.log(`   Network: http://${localIP}:${PORT}`);
    console.log('='.repeat(50));
    console.log('💡 Để các máy khác kết nối, dùng địa chỉ Network ở trên');
    console.log('='.repeat(50));
    console.log('Nhấn Ctrl+C để dừng server');
    console.log('='.repeat(50));
});

