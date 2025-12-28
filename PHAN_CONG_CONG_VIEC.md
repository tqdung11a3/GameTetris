# 📋 PHÂN CÔNG CÔNG VIỆC - GAME TETRIS ONLINE

## 🎯 Tổng Quan
Dự án được chia thành 2 nhóm:
- **Nhóm 1 (2 người)**: Chức năng cơ bản và Giao diện
- **Nhóm 2 (2 người)**: Chức năng chính

---

## 👥 NHÓM 1: CHỨC NĂNG CƠ BẢN VÀ GIAO DIỆN

### 👤 Người 1: Authentication & Lobby UI

#### **Công việc:**
1. **Authentication System (Server-side)**
   - File: `server.c`
   - Chức năng:
     - Đăng ký tài khoản (`REGISTER`)
     - Đăng nhập (`LOGIN`)
     - Quản lý file `accounts.txt`
     - Validation username/password
   - Dòng code: 73-116, 431-456

2. **Lobby Interface (Client Terminal)**
   - File: `client.c`
   - Chức năng:
     - Menu lobby (dòng 814-835)
     - Xử lý đăng nhập/đăng ký (dòng 885-912)
     - Hiển thị danh sách phòng (dòng 914-921)
     - Tạo/Tham gia phòng (dòng 923-940)

3. **Lobby Interface (Web Client)**
   - File: `index.html`, `game.js`
   - Chức năng:
     - Form đăng nhập/đăng ký (dòng 35-54 trong index.html)
     - UI lobby panel (dòng 57-87 trong index.html)
     - Xử lý authentication (dòng 121-143 trong game.js)
     - Xử lý room list (dòng 145-166 trong game.js)

#### **Deliverables:**
- ✅ Hoàn thiện hệ thống đăng nhập/đăng ký
- ✅ Giao diện lobby đẹp, dễ sử dụng
- ✅ Xử lý lỗi authentication rõ ràng
- ✅ Responsive design cho web client

---

### 👤 Người 2: UI/UX & Display Systems

#### **Công việc:**
1. **Game Display (Terminal)**
   - File: `client.c`
   - Chức năng:
     - Vẽ bảng game với màu sắc (dòng 368-458)
     - Hiển thị scoreboard (dòng 430-456)
     - Hiển thị thời gian (nếu có time limit)
     - Format output đẹp với box drawing

2. **Game Display (Web)**
   - File: `game.js`, `style.css`
   - Chức năng:
     - Vẽ game board trên canvas (dòng 401-449 trong game.js)
     - Hiển thị next piece (dòng 434-448)
     - Leaderboard UI (dòng 477-494)
     - Game over screen (dòng 496-502, 903-915)
     - CSS styling cho tất cả components

3. **History & Records Display**
   - File: `game.js`, `index.html`
   - Chức năng:
     - Modal hiển thị lịch sử (dòng 130-141 trong index.html)
     - Modal hiển thị kỷ lục (dòng 143-154 trong index.html)
     - Parse và format dữ liệu (dòng 789-880, 882-900 trong game.js)

#### **Deliverables:**
- ✅ Giao diện game đẹp, dễ nhìn
- ✅ Màu sắc rõ ràng, phân biệt các khối
- ✅ Responsive design
- ✅ Hiển thị thông tin rõ ràng (score, time, leaderboard)

---

## 👥 NHÓM 2: CHỨC NĂNG CHÍNH

### 👤 Người 3: Game Logic & Core Mechanics

#### **Công việc:**
1. **Tetris Game Engine (Terminal)**
   - File: `client.c`
   - Chức năng:
     - Định nghĩa tetrominoes (dòng 132-315)
     - Logic kiểm tra khối có fit không (dòng 323-335)
     - Logic khóa khối vào board (dòng 337-348)
     - Xóa hàng đầy (dòng 350-365)
     - Game loop chính (dòng 477-736)
     - Xử lý input người chơi (dòng 568-582)

2. **Tetris Game Engine (Web)**
   - File: `game.js`
   - Chức năng:
     - Định nghĩa tetrominoes (dòng 10-60)
     - Logic kiểm tra khối (dòng 316-332)
     - Logic khóa khối (dòng 334-347)
     - Xóa hàng đầy (dòng 349-374)
     - Game loop (dòng 505-526)
     - Xử lý keyboard (dòng 529-554)
     - Spawn piece (dòng 295-314)

3. **Scoring System**
   - File: `client.c` (dòng 594-633), `game.js` (dòng 360-373)
   - Chức năng:
     - Tính điểm theo số hàng xóa
     - Gửi điểm lên server
     - Hiển thị thông báo điểm

#### **Deliverables:**
- ✅ Game logic hoạt động chính xác
- ✅ Khối rơi mượt mà
- ✅ Xoay khối đúng
- ✅ Xóa hàng chính xác
- ✅ Tính điểm đúng

---

### 👤 Người 4: Multiplayer & Server Logic

#### **Công việc:**
1. **Room Management (Server)**
   - File: `server.c`
   - Chức năng:
     - Tạo phòng (dòng 360-382)
     - Tham gia phòng (dòng 390-408)
     - Rời phòng (dòng 283-342)
     - Danh sách phòng (dòng 410-420)
     - Quản lý trạng thái phòng (dòng 44-57)

2. **Game Session Management (Server)**
   - File: `server.c`
   - Chức năng:
     - Ready system (dòng 493-597)
     - Chọn game mode (dòng 664-754)
     - Bắt đầu game (dòng 565-597, 722-754)
     - Quản lý điểm số (dòng 599-663)
     - Kết thúc game (dòng 756-854)
     - Lưu lịch sử (dòng 185-225)

3. **Client-Server Communication**
   - File: `client.c` (dòng 760-810), `game.js` (dòng 558-732)
   - Chức năng:
     - Xử lý WebSocket messages
     - Đồng bộ trạng thái game
     - Cập nhật leaderboard real-time
     - Xử lý disconnect/reconnect

4. **Records & History System**
   - File: `server.c`
   - Chức năng:
     - Lưu kỷ lục người chơi (dòng 119-181)
     - Lưu lịch sử trận đấu (dòng 185-225)
     - Xem lịch sử (dòng 856-880)
     - Xem kỷ lục (dòng 882-923)

#### **Deliverables:**
- ✅ Hệ thống phòng hoạt động ổn định
- ✅ Đồng bộ game giữa nhiều người chơi
- ✅ Xử lý disconnect gracefully
- ✅ Lưu trữ dữ liệu chính xác
- ✅ Leaderboard real-time

---

## 📊 CHI TIẾT FILE VÀ DÒNG CODE

### Server (server.c)
- **Dòng 23-32**: Cấu trúc Account
- **Dòng 34-57**: Cấu trúc Room và Client
- **Dòng 73-115**: Authentication (Người 1)
- **Dòng 117-181**: Player Records (Người 4)
- **Dòng 183-225**: Match History (Người 4)
- **Dòng 283-342**: Room Management (Người 4)
- **Dòng 360-420**: Room Operations (Người 4)
- **Dòng 424-928**: Command Handler (Người 1, 4)
- **Dòng 954-1058**: Main Server Loop (Người 4)

### Client Terminal (client.c)
- **Dòng 118-315**: Tetrominoes Definition (Người 3)
- **Dòng 317-365**: Game Logic (Người 3)
- **Dòng 368-458**: Display System (Người 2)
- **Dòng 460-474**: Input Handler (Người 3)
- **Dòng 477-736**: Game Loop (Người 3)
- **Dòng 814-835**: Menu UI (Người 1)
- **Dòng 875-1098**: Main Menu Handler (Người 1)

### Web Client (game.js)
- **Dòng 10-60**: Tetrominoes (Người 3)
- **Dòng 72-85**: Game State (Người 3)
- **Dòng 102-119**: UI Functions (Người 1)
- **Dòng 121-204**: Lobby Functions (Người 1)
- **Dòng 283-374**: Game Logic (Người 3)
- **Dòng 401-467**: Drawing Functions (Người 2)
- **Dòng 505-526**: Game Loop (Người 3)
- **Dòng 558-732**: WebSocket Handler (Người 4)
- **Dòng 789-915**: Display Functions (Người 2)

### Web Client (index.html)
- **Dòng 35-54**: Auth Panel (Người 1)
- **Dòng 57-87**: Lobby Panel (Người 1)
- **Dòng 89-111**: Game Panel (Người 2)
- **Dòng 113-123**: Game Over Panel (Người 2)
- **Dòng 130-154**: Modals (Người 2)

---

## 🔄 QUY TRÌNH LÀM VIỆC

### Tuần 1-2: Setup & Basic Features
- **Người 1**: Hoàn thiện authentication, lobby UI
- **Người 2**: Setup UI framework, basic styling
- **Người 3**: Implement game logic cơ bản
- **Người 4**: Setup server structure, room management

### Tuần 3-4: Core Features
- **Người 1**: Tích hợp authentication với UI
- **Người 2**: Hoàn thiện game display, animations
- **Người 3**: Hoàn thiện game mechanics, scoring
- **Người 4**: Multiplayer sync, game session management

### Tuần 5-6: Advanced Features & Polish
- **Người 1**: Error handling, UX improvements
- **Người 2**: Final UI polish, responsive design
- **Người 3**: Game balance, performance optimization
- **Người 4**: History/Records system, stability testing

---

## ✅ CHECKLIST HOÀN THÀNH

### Người 1
- [ ] Authentication hoạt động đúng
- [ ] Lobby UI đẹp và responsive
- [ ] Error messages rõ ràng
- [ ] Form validation đầy đủ

### Người 2
- [ ] Game display đẹp, dễ nhìn
- [ ] Màu sắc phân biệt rõ
- [ ] Leaderboard hiển thị đúng
- [ ] Responsive trên nhiều màn hình

### Người 3
- [ ] Game logic chính xác
- [ ] Khối rơi/xoay/di chuyển mượt
- [ ] Xóa hàng đúng
- [ ] Tính điểm chính xác

### Người 4
- [ ] Multiplayer hoạt động ổn định
- [ ] Room management đúng
- [ ] Đồng bộ game tốt
- [ ] Lưu trữ dữ liệu chính xác

---

## 📝 GHI CHÚ

1. **Communication**: Sử dụng Git để quản lý code, commit thường xuyên
2. **Testing**: Mỗi người test phần của mình trước khi tích hợp
3. **Code Review**: Review code của nhau trước khi merge
4. **Documentation**: Comment code rõ ràng, đặc biệt các hàm phức tạp

---

## 🆘 HỖ TRỢ

Nếu gặp vấn đề:
1. Kiểm tra lại phần code của mình
2. Xem lại documentation trong code
3. Hỏi nhóm trưởng hoặc các thành viên khác
4. Tham khảo các file README có sẵn

---

**Chúc các bạn làm việc hiệu quả! 🎮✨**

