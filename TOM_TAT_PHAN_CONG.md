# 📋 TÓM TẮT PHÂN CÔNG CÔNG VIỆC

## 👥 NHÓM 1: CHỨC NĂNG CƠ BẢN VÀ GIAO DIỆN (2 người)

### 👤 Người 1: Authentication & Lobby
**Trách nhiệm:**
- ✅ Đăng ký/Đăng nhập (server.c, client.c, game.js)
- ✅ Giao diện lobby (terminal + web)
- ✅ Quản lý phòng cơ bản (tạo/tham gia)

**Files chính:**
- `server.c`: dòng 73-116, 431-456
- `client.c`: dòng 814-835, 885-940
- `game.js`: dòng 102-204
- `index.html`: dòng 35-87

---

### 👤 Người 2: UI/UX & Display
**Trách nhiệm:**
- ✅ Giao diện game (vẽ board, khối, màu sắc)
- ✅ Hiển thị scoreboard, leaderboard
- ✅ Game over screen
- ✅ History & Records display

**Files chính:**
- `client.c`: dòng 368-458
- `game.js`: dòng 401-467, 477-494, 789-915
- `index.html`: dòng 89-154
- `style.css`: toàn bộ file

---

## 👥 NHÓM 2: CHỨC NĂNG CHÍNH (2 người)

### 👤 Người 3: Game Logic & Core Mechanics
**Trách nhiệm:**
- ✅ Logic game Tetris (khối, xoay, rơi, xóa hàng)
- ✅ Game loop
- ✅ Xử lý input
- ✅ Tính điểm

**Files chính:**
- `client.c`: dòng 118-365, 477-736
- `game.js`: dòng 10-60, 283-374, 505-526, 529-554

---

### 👤 Người 4: Multiplayer & Server Logic
**Trách nhiệm:**
- ✅ Quản lý phòng (tạo/join/leave)
- ✅ Đồng bộ game multiplayer
- ✅ Ready system, game mode
- ✅ Lưu lịch sử & kỷ lục
- ✅ Client-server communication

**Files chính:**
- `server.c`: dòng 183-928 (phần lớn)
- `client.c`: dòng 760-810
- `game.js`: dòng 558-732
- `websocket-client.js`: toàn bộ file

---

## 📊 PHÂN BỔ THEO FILE

| File | Người 1 | Người 2 | Người 3 | Người 4 |
|------|---------|---------|---------|---------|
| `server.c` | Auth (73-116) | - | - | Room, Game, Records (183-928) |
| `client.c` | Menu, Auth (814-940) | Display (368-458) | Game Logic (118-736) | Comm (760-810) |
| `game.js` | Auth, Lobby (102-204) | Display (401-915) | Game Logic (10-554) | WebSocket (558-732) |
| `index.html` | Auth, Lobby (35-87) | Game UI (89-154) | - | - |
| `style.css` | - | Toàn bộ | - | - |
| `websocket-client.js` | - | - | - | Toàn bộ |

---

## ⏱️ TIMELINE ĐỀ XUẤT

**Tuần 1-2:** Setup & Basic
- Người 1: Auth system
- Người 2: UI framework
- Người 3: Basic game logic
- Người 4: Server structure

**Tuần 3-4:** Core Features
- Tất cả: Tích hợp và hoàn thiện chức năng

**Tuần 5-6:** Polish & Test
- Tất cả: Testing, bug fix, optimization

---

## ✅ CHECKLIST NHANH

### Người 1
- [ ] Login/Register hoạt động
- [ ] Lobby UI đẹp
- [ ] Room list hiển thị đúng

### Người 2
- [ ] Game display đẹp
- [ ] Màu sắc rõ ràng
- [ ] Leaderboard hiển thị đúng

### Người 3
- [ ] Game chơi được
- [ ] Khối rơi/xoay đúng
- [ ] Tính điểm chính xác

### Người 4
- [ ] Multiplayer hoạt động
- [ ] Room management đúng
- [ ] Lưu trữ dữ liệu đúng

---

**Xem chi tiết trong file `PHAN_CONG_CONG_VIEC.md`**

