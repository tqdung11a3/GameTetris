# 🔌 Kiến Trúc Kết Nối - Tetris Online

## 📊 Tổng Quan Kiến Trúc

```
┌─────────────────┐         ┌──────────────────┐         ┌──────────────┐
│   Web Browser   │         │  WebSocket Proxy │         │  Server C    │
│  (HTML/JS/CSS)  │◄───────►│   (Node.js)      │◄───────►│  (TCP Socket)│
│  Port: 3000     │  WS     │  Port: 8080      │  TCP    │  Port: 5555   │
└─────────────────┘         └──────────────────┘         └──────────────┘
       ▲                                                          ▲
       │                                                          │
       │                                                          │
┌─────────────────┐                                    ┌──────────────┐
│  HTTP Server    │                                    │ Client C     │
│  (Node.js)      │                                    │ (Terminal)   │
│  Port: 3000     │                                    │ TCP: 5555    │
└─────────────────┘                                    └──────────────┘
```

## 🔄 Luồng Kết Nối

### 1. Kiến Trúc Tổng Thể

Game có **2 cách chơi**:

#### Cách 1: Client Terminal (C) - Kết nối trực tiếp
```
Client C (client.c) ──TCP Socket──► Server C (server.c)
```

#### Cách 2: Web UI - Kết nối qua WebSocket Proxy
```
Web Browser ──WebSocket──► WebSocket Proxy ──TCP Socket──► Server C
```

---

## 🌐 Kiến Trúc Web UI (Cách 2)

### Các Thành Phần:

1. **HTTP Server** (`http-server.js`)
   - Port: **3000** (HTTP)
   - Protocol: **HTTP** (HyperText Transfer Protocol)
   - Nhiệm vụ: Serve file HTML/CSS/JS cho trình duyệt (static files)
   - URL: `http://localhost:3000`
   - Không xử lý game logic, chỉ phục vụ file tĩnh

2. **WebSocket Proxy Server** (`websocket-proxy.js`)
   - Port: **8080** (WebSocket)
   - Protocol: **WebSocket** (WS)
   - Nhiệm vụ: Chuyển đổi WebSocket ↔ TCP Socket
   - URL: `ws://localhost:8080`
   - Kết nối với Server C qua TCP

3. **Web UI Client** (`index.html` + `game.js` + `websocket-client.js`)
   - Chạy trong trình duyệt
   - Kết nối với WebSocket Proxy qua WebSocket

4. **Server C** (`server.c`)
   - Port: 5555
   - Xử lý tất cả game logic
   - Không phân biệt client là Terminal hay Web

---

## 📡 Chi Tiết Kết Nối

### Bước 1: Khởi Động Server C

```bash
./server 5555
```

**Server C làm gì:**
- Tạo TCP socket listener trên port 5555
- Chờ kết nối từ client (Terminal hoặc WebSocket Proxy)
- Xử lý các lệnh: LOGIN, REGISTER, CREATE_ROOM, JOIN_ROOM, READY, GAME_SCORE, v.v.

### Bước 2: Khởi Động WebSocket Proxy

```bash
node websocket-proxy.js
```

**WebSocket Proxy làm gì:**

```javascript
// 1. Tạo WebSocket server trên port 8080
const wss = new WebSocket.Server({ port: 8080 });

// 2. Khi có WebSocket client kết nối
wss.on('connection', (ws) => {
    // 3. Tạo TCP connection đến Server C
    const tcpClient = new net.Socket();
    tcpClient.connect(5555, 'localhost', () => {
        // Đã kết nối đến Server C
    });
    
    // 4. Chuyển tiếp message từ WebSocket → TCP
    ws.on('message', (message) => {
        tcpClient.write(message); // Gửi đến Server C
    });
    
    // 5. Chuyển tiếp message từ TCP → WebSocket
    tcpClient.on('data', (data) => {
        ws.send(data); // Gửi đến Web Browser
    });
});
```

**Vai trò:**
- **Bridge/Proxy**: Chuyển đổi giữa WebSocket (trình duyệt) và TCP Socket (Server C)
- **Protocol Converter**: WebSocket protocol ↔ TCP Socket protocol

### Bước 3: Khởi Động HTTP Server

```bash
node http-server.js
```

**HTTP Server làm gì:**
- Serve file `index.html`, `style.css`, `game.js`, `websocket-client.js`
- Không xử lý game logic, chỉ phục vụ file tĩnh

### Bước 4: Trình Duyệt Kết Nối

**Khi mở `http://localhost:3000`:**

1. **Load HTML/CSS/JS (qua HTTP - Port 3000):**
   ```
   Browser → HTTP GET → http-server.js (port 3000) → Trả về index.html
   Browser → HTTP GET → http-server.js (port 3000) → Trả về style.css
   Browser → HTTP GET → http-server.js (port 3000) → Trả về game.js
   Browser → HTTP GET → http-server.js (port 3000) → Trả về websocket-client.js
   ```
   **Lưu ý:** Tất cả file HTML/CSS/JS được tải qua HTTP trên port 3000

2. **Kết nối WebSocket (qua WebSocket - Port 8080):**
   ```javascript
   // Trong websocket-client.js (đã được load từ port 3000)
   const wsClient = new TetrisWebSocketClient();
   wsClient.connect('ws://localhost:8080'); // Kết nối WebSocket trên port 8080
   ```
   **Lưu ý:** Sau khi load xong file, JavaScript trong trình duyệt sẽ tự động kết nối WebSocket đến port 8080

3. **Kết nối được thiết lập:**
   ```
   Browser (WebSocket trên port 8080) ←→ WebSocket Proxy ←→ Server C (TCP port 5555)
   ```

**Tóm lại:**
- **Port 3000 (HTTP)**: Để tải file HTML/CSS/JS (giao diện)
- **Port 8080 (WebSocket)**: Để gửi/nhận dữ liệu game (logic)

---

## 🔄 Luồng Dữ Liệu

### Ví dụ: Đăng Nhập

#### 1. User nhập username/password và click "Đăng Nhập"

```javascript
// game.js
function handleLogin() {
    const username = document.getElementById('login-username').value;
    const password = document.getElementById('login-password').value;
    wsClient.send(`LOGIN ${username} ${password}`);
}
```

#### 2. WebSocket Client gửi message

```javascript
// websocket-client.js
send(message) {
    this.ws.send(message + '\n'); // Gửi qua WebSocket
}
```

**Luồng:**
```
Browser → WebSocket.send("LOGIN user pass\n")
         ↓
WebSocket Proxy nhận message
         ↓
WebSocket Proxy → TCP.write("LOGIN user pass\n")
         ↓
Server C nhận qua TCP socket
```

#### 3. Server C xử lý

```c
// server.c
if (strcmp(cmd, "LOGIN") == 0) {
    // Xử lý đăng nhập
    // ...
    send_to_client(cindex, "LOGIN_OK\n");
}
```

#### 4. Server C gửi phản hồi

**Luồng ngược lại:**
```
Server C → TCP.send("LOGIN_OK\n")
         ↓
WebSocket Proxy nhận qua TCP
         ↓
WebSocket Proxy → WebSocket.send("LOGIN_OK\n")
         ↓
Browser nhận qua WebSocket
```

#### 5. Web UI xử lý phản hồi

```javascript
// game.js
wsClient.onMessage((message) => {
    if (cmd === 'LOGIN_OK') {
        // Chuyển sang lobby panel
        document.getElementById('auth-panel').style.display = 'none';
        document.getElementById('lobby-panel').style.display = 'block';
    }
});
```

---

## 📋 Protocol Messages

### Format Message

Tất cả message đều kết thúc bằng `\n` (newline):

```
COMMAND [args...]\n
```

### Ví dụ Messages:

**Từ Client → Server:**
- `LOGIN username password\n`
- `REGISTER username password\n`
- `CREATE_ROOM\n`
- `JOIN_ROOM 1\n`
- `READY\n`
- `GAME_SCORE 1234\n`
- `VIEW_HISTORY\n`
- `VIEW_RECORDS\n`

**Từ Server → Client:**
- `LOGIN_OK\n`
- `LOGIN_FAIL Wrong_credentials\n`
- `ROOM_CREATED 1\n`
- `JOIN_OK 1\n`
- `SCORE_UPDATE user1:100 user2:200\n`
- `START_GAME 0 0\n`
- `HISTORY_BEGIN\n`
- `HISTORY_LINE === Match Room 1 ===\n`
- `HISTORY_END\n`

---

## 🔧 Chi Tiết Kỹ Thuật

### WebSocket Proxy - Xử Lý Buffer

```javascript
// websocket-proxy.js
let tcpBuffer = '';

tcpClient.on('data', (data) => {
    tcpBuffer += data.toString();
    
    // Tách từng dòng (vì protocol dùng \n)
    const lines = tcpBuffer.split('\n');
    tcpBuffer = lines.pop() || ''; // Giữ phần chưa hoàn chỉnh
    
    lines.forEach(line => {
        if (line.trim().length > 0) {
            ws.send(line + '\n'); // Gửi từng dòng
        }
    });
});
```

**Tại sao cần buffer?**
- TCP có thể gửi nhiều message trong 1 lần
- Cần tách theo dòng (`\n`) để xử lý đúng protocol

### WebSocket Client - Xử Lý Message

```javascript
// websocket-client.js
this.ws.onmessage = (event) => {
    const message = event.data;
    // Gọi tất cả handlers đã đăng ký
    this.onMessageHandlers.forEach(handler => handler(message));
};
```

**Game.js đăng ký handler:**
```javascript
// game.js
wsClient.onMessage((message) => {
    const parts = message.trim().split(' ');
    const cmd = parts[0];
    
    if (cmd === 'LOGIN_OK') {
        // Xử lý đăng nhập thành công
    } else if (cmd === 'SCORE_UPDATE') {
        // Cập nhật leaderboard
    }
    // ...
});
```

---

## 🆚 So Sánh: Terminal Client vs Web UI

| Tính Năng | Terminal Client | Web UI |
|-----------|----------------|--------|
| **Kết nối** | TCP Socket trực tiếp | HTTP (3000) + WebSocket (8080) → Proxy → TCP |
| **Giao diện** | Terminal với ANSI colors | HTML5 Canvas + CSS |
| **Input** | Keyboard (a, d, s, w, q) | Keyboard (A/D/S/W/Q hoặc Arrow keys) |
| **Render** | printf với escape codes | Canvas API |
| **Protocol** | TCP Socket | HTTP (file) + WebSocket (data) |
| **Port** | 5555 (trực tiếp) | 3000 (HTTP) + 8080 (WebSocket) → 5555 (TCP) |

## 🔌 Giải Thích Port 3000 vs 8080

### Port 3000 (HTTP Server)
- **Giao thức**: HTTP (HyperText Transfer Protocol)
- **Mục đích**: Phục vụ file tĩnh (HTML, CSS, JavaScript)
- **Khi nào dùng**: Khi trình duyệt cần tải file
- **Ví dụ**: 
  - Mở `http://localhost:3000` → Tải `index.html`
  - Tải `style.css`, `game.js`, `websocket-client.js`
- **Tương tự**: Giống như một website thông thường

### Port 8080 (WebSocket Proxy)
- **Giao thức**: WebSocket (WS)
- **Mục đích**: Gửi/nhận dữ liệu game real-time
- **Khi nào dùng**: Sau khi file đã tải xong, JavaScript tự động kết nối
- **Ví dụ**:
  - Gửi lệnh `LOGIN user pass`
  - Nhận `LOGIN_OK`
  - Gửi `GAME_SCORE 1234`
- **Tương tự**: Giống như một kênh chat real-time

### Tại sao cần 2 port?

**Port 3000 (HTTP):**
- ✅ Trình duyệt cần HTTP để tải file
- ✅ An toàn, được hỗ trợ rộng rãi
- ❌ Không phù hợp cho real-time (phải request mỗi lần)

**Port 8080 (WebSocket):**
- ✅ Kết nối persistent (giữ kết nối)
- ✅ Real-time, 2 chiều (gửi/nhận ngay lập tức)
- ✅ Phù hợp cho game
- ❌ Không thể serve file HTML/CSS/JS

**Giải pháp:** Dùng cả 2!
- Port 3000: Tải giao diện (một lần)
- Port 8080: Gửi/nhận dữ liệu game (liên tục)

---

## 🎯 Tại Sao Cần WebSocket Proxy?

### Vấn đề:
- **Trình duyệt không thể kết nối TCP Socket trực tiếp** (vì lý do bảo mật)
- **Server C chỉ hiểu TCP Socket**, không hiểu WebSocket

### Giải pháp:
- **WebSocket Proxy** đóng vai trò "dịch thuật":
  - Nhận WebSocket từ trình duyệt
  - Chuyển đổi thành TCP Socket
  - Gửi đến Server C
  - Và ngược lại

### Lợi ích:
- ✅ Server C không cần thay đổi
- ✅ Web UI có thể kết nối từ trình duyệt
- ✅ Hỗ trợ cả Terminal và Web UI cùng lúc

---

## 📝 Tóm Tắt

1. **Server C** (`server.c`): Xử lý game logic, chỉ hiểu TCP Socket
2. **WebSocket Proxy** (`websocket-proxy.js`): Bridge giữa WebSocket và TCP
3. **HTTP Server** (`http-server.js`): Serve file HTML/CSS/JS
4. **Web UI** (`index.html` + `game.js`): Giao diện game trong trình duyệt
5. **WebSocket Client** (`websocket-client.js`): Kết nối WebSocket đến Proxy

**Luồng hoàn chỉnh:**
```
User Action (Click/Keyboard)
    ↓
game.js (xử lý UI)
    ↓
websocket-client.js (gửi WebSocket)
    ↓
websocket-proxy.js (chuyển đổi)
    ↓
server.c (xử lý logic)
    ↓
websocket-proxy.js (chuyển đổi)
    ↓
websocket-client.js (nhận WebSocket)
    ↓
game.js (cập nhật UI)
    ↓
User thấy kết quả
```

---

## 🔍 Debug Kết Nối

### Kiểm tra các cổng:

```bash
# Kiểm tra Server C
lsof -i :5555

# Kiểm tra WebSocket Proxy
lsof -i :8080

# Kiểm tra HTTP Server
lsof -i :3000
```

### Log để debug:

**WebSocket Proxy:**
```javascript
console.log('TCP -> WS:', message.trim());
console.log('WS -> TCP:', text.trim());
```

**Web UI (Console trình duyệt - F12):**
```javascript
console.log('Sent:', message);
console.log('Received:', message);
console.log('Handling command:', cmd);
```

---

## 🎉 Kết Luận

Kiến trúc này cho phép:
- ✅ Server C không cần thay đổi để hỗ trợ Web UI
- ✅ Web UI và Terminal Client có thể chơi cùng lúc
- ✅ Dễ dàng mở rộng thêm tính năng
- ✅ Tách biệt rõ ràng giữa frontend và backend

**Happy Coding! 🚀**

