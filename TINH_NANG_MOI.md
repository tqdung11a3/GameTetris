# 🎮 TETRIS ONLINE - HƯỚNG DẪN ĐẦY ĐỦ

## 📋 Mục Lục
1. [Giới Thiệu](#giới-thiệu)
2. [Cài Đặt và Biên Dịch](#cài-đặt-và-biên-dịch)
3. [Chạy Game](#chạy-game)
4. [Giao Diện Web UI](#giao-diện-web-ui) ⭐ MỚI
5. [Hướng Dẫn Sử Dụng](#hướng-dẫn-sử-dụng)
6. [Các Tính Năng](#các-tính-năng)
7. [Điều Khiển](#điều-khiển)
8. [Xử Lý Lỗi](#xử-lý-lỗi)

---

## 🎯 Giới Thiệu

**Tetris Online** là một game Tetris đa người chơi qua mạng, cho phép nhiều người chơi cùng lúc trong một phòng và thi đua điểm số. Game hỗ trợ cả giao diện terminal và giao diện web hiện đại.

### Tính Năng Chính:
- ✅ **Đăng ký và đăng nhập** tài khoản
- ✅ **Đăng xuất** để đổi tài khoản
- ✅ **Tạo và tham gia phòng** chơi (tối đa 4 người/phòng)
- ✅ **Nhiều chế độ chơi**: Survival, Time Attack (60s, 180s, 300s)
- ✅ **Bảng xếp hạng real-time** trong phòng
- ✅ **Lưu lịch sử trận đấu** với giao diện đẹp
- ✅ **Kỷ lục người chơi** (top 50)
- ✅ **Giao diện terminal** đẹp với màu sắc và ký tự Unicode
- ✅ **Giao diện web UI** hiện đại với HTML5 Canvas ⭐ MỚI

---

## 🔧 Cài Đặt và Biên Dịch

### Yêu Cầu Hệ Thống:

**Cho Server và Client Terminal:**
- Hệ điều hành: Linux, macOS, hoặc WSL (Windows Subsystem for Linux)
- Trình biên dịch: GCC
- Terminal hỗ trợ ANSI color codes (hầu hết terminal hiện đại)

**Cho Giao Diện Web:**
- Node.js (phiên bản 12 trở lên) - Tải tại: https://nodejs.org/
- Trình duyệt hiện đại (Chrome, Firefox, Safari, Edge)

### Biên Dịch Server:

```bash
gcc server.c -o server
```

### Biên Dịch Client Terminal:

```bash
gcc client.c -o client
```

### Cài Đặt Dependencies cho Web UI:

```bash
npm install
```

**Lưu ý:** Nếu gặp lỗi khi biên dịch, đảm bảo bạn đang sử dụng GCC phiên bản mới và có các thư viện cần thiết.

---

## 🚀 Chạy Game

### Cách 1: Chạy với Giao Diện Terminal

#### Bước 1: Khởi động Server

Mở terminal và chạy:

```bash
./server 5555
```

Trong đó `5555` là số cổng (port) mà server sẽ lắng nghe. Bạn có thể thay đổi số cổng tùy ý.

**Kết quả mong đợi:**
```
Server listening on port 5555
```

#### Bước 2: Khởi động Client

Mở một terminal mới (hoặc nhiều terminal nếu muốn test nhiều người chơi) và chạy:

```bash
./client 127.0.0.1 5555
```

Trong đó:
- `127.0.0.1` là địa chỉ IP của server (localhost)
- `5555` là cổng của server

**Kết quả mong đợi:**
```
Server: WELCOME TETRIS SERVER
```

Sau đó bạn sẽ thấy menu chính của game.

### Cách 2: Chạy với Giao Diện Web UI ⭐ KHUYẾN NGHỊ

#### Bước 1: Khởi động Server C

```bash
./server 5555
```

#### Bước 2: Khởi động WebSocket Proxy Server

Mở terminal mới và chạy:

```bash
node websocket-proxy.js
```

Hoặc:

```bash
npm run proxy
```

Bạn sẽ thấy:
```
WebSocket Proxy Server đang chạy trên port 8080
Kết nối đến TCP server tại localhost:5555
Đang chờ kết nối từ WebSocket clients...
```

#### Bước 3: Khởi động HTTP Server

Mở terminal mới và chạy:

```bash
node http-server.js
```

Hoặc:

```bash
npm run http
```

Bạn sẽ thấy:
```
🚀 HTTP Server đang chạy!
📂 Serving files từ: /path/to/GameTetris
🌐 Mở trình duyệt và truy cập:
   http://localhost:3000
```

#### Bước 4: Mở Trình Duyệt

Truy cập: **http://localhost:3000**

---

## 🌐 Giao Diện Web UI ⭐ MỚI

### Ưu Điểm của Web UI:

- 🎨 **Giao diện hiện đại**: Thiết kế đẹp mắt với gradient, animation và responsive
- 🎮 **Game Canvas**: Render game bằng HTML5 Canvas với màu sắc đẹp
- 📊 **Bảng xếp hạng real-time**: Cập nhật tự động khi có điểm mới
- 🎯 **Điều khiển bằng bàn phím**: WASD hoặc phím mũi tên
- 📱 **Responsive**: Tự động điều chỉnh trên mobile và tablet
- 🔔 **Thông báo toast**: Hiển thị thông báo khi có sự kiện quan trọng
- 📜 **Lịch sử đẹp**: Hiển thị lịch sử trận đấu dạng card với thông tin chi tiết
- 🏆 **Kỷ lục**: Top 50 người chơi với highlight top 3
- 🚪 **Đăng xuất**: Dễ dàng đổi tài khoản

### Cấu Trúc Giao Diện:

1. **Header**: Hiển thị score, time (nếu có), và trạng thái kết nối
2. **Auth Panel**: Đăng nhập/Đăng ký
3. **Lobby Panel**: 
   - Danh sách phòng
   - Tạo/Tham gia phòng
   - Xem lịch sử và kỷ lục
   - Thông tin người dùng và nút đăng xuất
4. **Game Panel**: 
   - Canvas game bên trái
   - Leaderboard và next piece bên phải
5. **Game Over Panel**: Kết quả trận đấu

### Điều Khiển trong Web UI:

| Phím | Chức Năng |
|------|-----------|
| `A` hoặc `←` | Di chuyển khối sang trái |
| `D` hoặc `→` | Di chuyển khối sang phải |
| `S` hoặc `↓` | Rơi nhanh xuống |
| `W` hoặc `↑` | Xoay khối |
| `Q` | Thoát game |

### Cấu Hình:

**Thay đổi cổng WebSocket:**
Sửa trong `websocket-proxy.js`:
```javascript
const WS_PORT = 8080; // Đổi số cổng ở đây
```

Và trong `websocket-client.js`:
```javascript
const wsUrl = localStorage.getItem('wsServerUrl') || 'ws://localhost:8080';
```

**Thay đổi cổng HTTP:**
Sửa trong `http-server.js`:
```javascript
const PORT = 3000; // Đổi số cổng ở đây
```

Xem thêm chi tiết trong file `README_UI.md`.

---

## 🎮 Hướng Dẫn Sử Dụng

### Bước 1: Đăng Ký/Đăng Nhập

**Đăng ký tài khoản mới:**
1. Chọn tab "Đăng Ký" (hoặc chọn **1. Register** trong terminal)
2. Nhập username và password
3. Click "Đăng Ký"
4. Lưu ý: Username không được trùng với người khác

**Đăng nhập:**
1. Chọn tab "Đăng Nhập" (hoặc chọn **2. Login** trong terminal)
2. Nhập username và password đã đăng ký
3. Click "Đăng Nhập"
4. Sau khi đăng nhập thành công, bạn sẽ vào Lobby

**Đăng xuất:**
1. Click nút "🚪 Đăng Xuất" ở góc phải trên Lobby
2. Xác nhận đăng xuất
3. Hệ thống sẽ quay về màn hình đăng nhập

### Bước 2: Tạo hoặc Tham Gia Phòng

**Tạo phòng mới:**
1. Click "➕ Tạo Phòng" (hoặc chọn **4. Create room** trong terminal)
2. Server sẽ tạo phòng và gán bạn làm chủ phòng
3. Ghi nhớ Room ID được hiển thị

**Tham gia phòng:**
1. Click "📋 Danh Sách Phòng" để xem danh sách (hoặc chọn **3. List rooms** trong terminal)
2. Click "Tham Gia" ở phòng muốn vào (hoặc chọn **5. Join room** và nhập Room ID)
3. Bạn sẽ thấy thông tin phòng và danh sách người chơi

**Lưu ý:** Mỗi phòng tối đa 4 người chơi.

### Bước 3: Sẵn Sàng Chơi

1. Click "✅ Ready" để báo sẵn sàng (hoặc chọn **6. Ready** trong terminal)
2. Nếu bạn là người ready đầu tiên (và phòng có >1 người), bạn sẽ được chọn chế độ chơi:
   - **SURVIVAL**: Chơi đến khi game over
   - **TIME ATTACK 60s**: Chơi trong 60 giây
   - **TIME ATTACK 180s**: Chơi trong 180 giây (3 phút)
   - **TIME ATTACK 300s**: Chơi trong 300 giây (5 phút)
3. Khi tất cả người chơi đều ready, game sẽ bắt đầu sau countdown 3-2-1

### Bước 4: Chơi Game

- Khối Tetris sẽ tự động rơi xuống
- Sử dụng các phím điều khiển để di chuyển và xoay khối
- Xóa hàng để ghi điểm:
  - 1 hàng: +100 điểm
  - 2 hàng: +300 điểm
  - 3 hàng: +500 điểm
  - 4 hàng: +800 điểm (TETRIS!)
- Xem bảng xếp hạng real-time ở bên phải
- Xem khối tiếp theo ở góc trên

### Bước 5: Kết Thúc Game

Game kết thúc khi:
- Bạn nhấn `Q` để quit
- Không còn chỗ để spawn khối mới (game over)
- Hết thời gian (nếu chơi Time Attack mode)

Sau khi kết thúc, bạn sẽ thấy:
- Điểm của bạn
- Bảng xếp hạng cuối cùng
- Người thắng được highlight

### Bước 6: Xem Lịch Sử và Kỷ Lục

**Xem Lịch Sử Trận Đấu:**
1. Click "📜 Lịch Sử" trong Lobby (hoặc chọn **7. View Match History** trong terminal)
2. Modal sẽ hiển thị tất cả các trận đấu với:
   - Room ID và thời gian
   - Danh sách người chơi với điểm số
   - Người thắng được highlight

**Xem Kỷ Lục Người Chơi:**
1. Click "🏆 Kỷ Lục" trong Lobby (hoặc chọn **8. View Player Records** trong terminal)
2. Modal sẽ hiển thị top 50 người chơi có điểm cao nhất
3. Top 3 được highlight với màu sắc đặc biệt

---

## ⚙️ Các Tính Năng

### 1. Hệ Thống Điểm Số

Điểm được tính theo số hàng xóa cùng lúc:
- **1 hàng (SINGLE)**: +100 điểm
- **2 hàng (DOUBLE)**: +300 điểm
- **3 hàng (TRIPLE)**: +500 điểm
- **4 hàng (TETRIS)**: +800 điểm

Khi xóa hàng, bạn sẽ thấy thông báo với số điểm nhận được.

### 2. Chế Độ Chơi

**SURVIVAL (Mode 0):**
- Chơi đến khi game over
- Không giới hạn thời gian
- Người có điểm cao nhất thắng

**TIME ATTACK 60s (Mode 1):**
- Chơi trong 60 giây
- Người có điểm cao nhất trong thời gian này thắng
- Phù hợp cho trận đấu nhanh

**TIME ATTACK 180s (Mode 2):**
- Chơi trong 180 giây (3 phút)
- Phù hợp cho trận đấu vừa

**TIME ATTACK 300s (Mode 3):**
- Chơi trong 300 giây (5 phút)
- Trận đấu dài nhất

### 3. Bảng Xếp Hạng Real-Time

Trong khi chơi, bạn có thể xem bảng xếp hạng của tất cả người chơi trong phòng:
- Cập nhật tự động khi có điểm mới
- Hiển thị thông báo khi có người kết thúc game
- Hiển thị thông báo khi có người disconnect

### 4. Lịch Sử và Kỷ Lục

**Lịch Sử Trận Đấu:**
- Lưu tất cả các trận đấu đã chơi
- Hiển thị dạng card đẹp với:
  - Room ID và timestamp
  - Danh sách người chơi với rank và điểm
  - Người thắng được highlight
- Có thể xem trong Web UI hoặc terminal

**Kỷ Lục Người Chơi:**
- Lưu điểm cao nhất của mỗi người chơi
- Hiển thị top 50 người chơi
- Top 3 được highlight với icon đặc biệt (🏆🥈🥉)
- Tự động cập nhật khi bạn đạt điểm cao mới

### 5. Đăng Xuất

- Dễ dàng đổi tài khoản
- Tự động rời phòng nếu đang trong phòng
- Reset tất cả state về ban đầu
- Clear form đăng nhập

---

## 🎯 Điều Khiển

### Trong Game (Terminal):

| Phím | Chức Năng |
|------|-----------|
| `a` | Di chuyển khối sang trái (←) |
| `d` | Di chuyển khối sang phải (→) |
| `s` | Rơi nhanh xuống (↓) |
| `w` | Xoay khối (↻) |
| `q` | Thoát game (quit) |

### Trong Game (Web UI):

| Phím | Chức Năng |
|------|-----------|
| `A` hoặc `←` | Di chuyển khối sang trái |
| `D` hoặc `→` | Di chuyển khối sang phải |
| `S` hoặc `↓` | Rơi nhanh xuống |
| `W` hoặc `↑` | Xoay khối |
| `Q` | Thoát game |

### Trong Menu (Terminal):

- Nhập số tương ứng với lựa chọn (0-8)
- Nhấn Enter để xác nhận

---

## 💡 Mẹo Chơi

1. **Lập kế hoạch:** Quan sát khối tiếp theo và sắp xếp để tạo cơ hội xóa nhiều hàng cùng lúc

2. **Ưu tiên TETRIS:** Xóa 4 hàng cùng lúc cho điểm cao nhất (800 điểm)

3. **Giữ board sạch:** Tránh để các lỗ hổng ở giữa, sẽ khó lấp đầy sau này

4. **Time Attack:** Trong chế độ Time Attack, tập trung vào tốc độ và hiệu quả

5. **Theo dõi đối thủ:** Xem bảng xếp hạng để biết vị trí của mình và điều chỉnh chiến lược

6. **Sử dụng khối tiếp theo:** Quan sát khối tiếp theo để lập kế hoạch di chuyển

---

## 🐛 Xử Lý Lỗi

### Server không khởi động được:
- Kiểm tra xem cổng đã được sử dụng chưa: `lsof -i :5555`
- Thử đổi sang cổng khác
- Đảm bảo có quyền truy cập cổng

### Client không kết nối được:
- Kiểm tra địa chỉ IP và cổng có đúng không
- Đảm bảo server đã được khởi động trước
- Kiểm tra firewall có chặn kết nối không

### WebSocket không kết nối được:
- Đảm bảo WebSocket proxy server đang chạy
- Kiểm tra cổng 8080 có bị chiếm dụng không
- Xem log của WebSocket proxy server

### HTTP Server không chạy được:
- Kiểm tra cổng 3000 có bị chiếm dụng không
- Đảm bảo Node.js đã được cài đặt
- Thử đổi cổng trong `http-server.js`

### Giao diện hiển thị lỗi:
- **Terminal**: Đảm bảo terminal hỗ trợ ANSI color codes
- **Web**: Xóa cache trình duyệt (Ctrl+Shift+Delete hoặc Cmd+Shift+Delete)
- Thử trình duyệt khác (Chrome, Firefox, Safari, Edge)
- Kiểm tra Console (F12) để xem lỗi JavaScript

### Game lag hoặc chậm:
- Kiểm tra kết nối mạng
- Đảm bảo server có đủ tài nguyên
- Đóng các ứng dụng không cần thiết

### Đăng nhập thất bại:
- Kiểm tra username và password có đúng không
- Đảm bảo đã đăng ký tài khoản trước
- Kiểm tra file `accounts.txt` có tồn tại không

### Không vào được phòng:
- Kiểm tra phòng có đầy chưa (tối đa 4 người)
- Kiểm tra Room ID có đúng không
- Thử tạo phòng mới

---

## 📝 Lưu Ý

1. **Tài khoản:** Tài khoản được lưu trong file `accounts.txt`
2. **Lịch sử:** Lịch sử trận đấu được lưu trong `match_history.txt`
3. **Kỷ lục:** Kỷ lục người chơi được lưu trong `player_records.txt`
4. **Disconnect:** Nếu disconnect trong khi chơi, điểm hiện tại vẫn được lưu vào kỷ lục
5. **Multiplayer:** Để test nhiều người chơi:
   - Terminal: Mở nhiều terminal và chạy client riêng biệt
   - Web: Mở nhiều tab trình duyệt hoặc nhiều trình duyệt khác nhau
6. **Biên dịch lại:** Sau khi sửa code C, cần biên dịch lại:
   ```bash
   gcc server.c -o server
   gcc client.c -o client
   ```
7. **Refresh Web:** Sau khi sửa HTML/CSS/JS, refresh trình duyệt (Ctrl+R hoặc Cmd+R)

---

## 📂 Cấu Trúc File

```
GameTetris/
├── server.c              # Server backend (C)
├── client.c              # Client terminal (C)
├── index.html            # Giao diện web chính
├── style.css             # Styling cho web UI
├── game.js               # Game logic và UI handlers
├── websocket-client.js   # WebSocket client
├── websocket-proxy.js    # WebSocket proxy server
├── http-server.js        # HTTP server đơn giản
├── package.json          # Node.js dependencies
├── accounts.txt          # Tài khoản người dùng
├── match_history.txt     # Lịch sử trận đấu
├── player_records.txt    # Kỷ lục người chơi
├── TINH_NANG_MOI.md      # File hướng dẫn này
└── README_UI.md          # Hướng dẫn chi tiết Web UI
```

---

## 🎉 Chúc Bạn Chơi Vui Vẻ!

Nếu có vấn đề hoặc câu hỏi, hãy:
1. Kiểm tra lại các bước trong hướng dẫn
2. Xem phần Xử Lý Lỗi
3. Kiểm tra Console/Log để xem lỗi chi tiết
4. Xem lại code để hiểu rõ hơn về cách game hoạt động

**Happy Gaming! 🎮✨**

---

## 📞 Hỗ Trợ

Nếu gặp vấn đề, hãy kiểm tra:
- Console trình duyệt (F12) cho Web UI
- Log của server và WebSocket proxy
- File log nếu có

**Version:** 1.0  
**Last Updated:** 2025
