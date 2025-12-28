# 🚀 TÍNH NĂNG NÂNG CAO: HỆ THỐNG LEVEL VÀ TĂNG TỐC ĐỘ

## 📋 Tổng Quan

Đã thêm **hệ thống level** vào game Tetris Online, giúp tăng độ khó dần dần bằng cách tăng tốc độ rơi của khối theo số hàng đã xóa.

---

## ✨ Tính Năng

### 1. Hệ Thống Level
- **Level bắt đầu**: Level 1
- **Tăng level**: Mỗi khi xóa được **5 hàng**, level tăng lên 1 (để tăng dần rõ ràng hơn)
- **Hiển thị**: Level được hiển thị trong header của game

### 2. Tốc Độ Rơi Tự Động
- **Tốc độ ban đầu**: 500ms/ô (Level 1)
- **Giảm tốc độ**: Mỗi level tăng, tốc độ giảm **20ms**
- **Công thức**: `dropInterval = 500 - (level - 1) * 20`
- **Tốc độ tối thiểu**: 50ms/ô (không giảm thêm sau Level 23)

### 3. Bảng Tốc Độ Theo Level

| Level | Tốc Độ Rơi (ms/ô) | Hàng Đã Xóa |
|-------|-------------------|-------------|
| 1     | 500ms             | 0-9         |
| 2     | 480ms             | 10-19       |
| 3     | 460ms             | 20-29       |
| 4     | 440ms             | 30-39       |
| 5     | 420ms             | 40-49       |
| 10    | 320ms             | 90-99       |
| 15    | 220ms             | 140-149     |
| 20    | 120ms             | 190-199     |
| 23+   | 50ms (tối thiểu)  | 220+        |

---

## 🎮 Cách Hoạt Động

### Trong Game

1. **Bắt đầu game**: Level 1, tốc độ 500ms/ô
2. **Xóa hàng**: Mỗi khi xóa hàng, hệ thống đếm tổng số hàng đã xóa
3. **Tăng level**: Khi đạt 10, 20, 30... hàng → Level tăng
4. **Tăng tốc độ**: Tốc độ rơi tự động giảm (khối rơi nhanh hơn)
5. **Thông báo**: Khi level up, hiển thị thông báo đặc biệt

### Thông Báo Level Up

**Terminal Client:**
```
╔════════════════════════════════════════════════════════════╗
║              🚀 LEVEL UP! LEVEL 2 🚀              ║
║         Tốc độ rơi tăng! (480ms/ô)         ║
╚════════════════════════════════════════════════════════════╝
```

**Web Client:**
- Hiển thị message: `🚀 LEVEL UP! LEVEL 2 - Tốc độ rơi: 480ms/ô`
- Level được cập nhật ngay trong header

---

## 💻 Thay Đổi Code

### Client Terminal (`client.c`)

**Thêm biến:**
```c
int level = 1;              // Level bắt đầu từ 1
int total_lines_cleared = 0; // Tổng số hàng đã xóa
int drop_interval = 500;     // Tốc độ rơi ban đầu (ms)
```

**Cập nhật khi xóa hàng:**
```c
total_lines_cleared += lines;
int new_level = (total_lines_cleared / 10) + 1;
if (new_level > level) {
    level = new_level;
    drop_interval = 500 - (level - 1) * 20;
    if (drop_interval < 50) drop_interval = 50;
    // Hiển thị thông báo level up
}
```

**Sử dụng drop_interval:**
```c
if (now_ms - last_drop >= drop_interval) {
    // Khối rơi xuống
}
```

### Web Client (`game.js`)

**Thêm vào gameState:**
```javascript
level: 1,                    // Level bắt đầu từ 1
totalLinesCleared: 0,       // Tổng số hàng đã xóa
```

**Cập nhật khi xóa hàng:**
```javascript
gameState.totalLinesCleared += linesCleared;
const newLevel = Math.floor(gameState.totalLinesCleared / 10) + 1;
if (newLevel > gameState.level) {
    gameState.level = newLevel;
    gameState.dropInterval = 500 - (gameState.level - 1) * 20;
    if (gameState.dropInterval < 50) gameState.dropInterval = 50;
    updateLevel();
    showMessage(`🚀 LEVEL UP! LEVEL ${gameState.level}...`);
}
```

**HTML (`index.html`):**
```html
<div class="status-item">
    <span class="label">Level:</span>
    <span id="level" class="value">1</span>
</div>
```

---

## 🎯 Lợi Ích

1. **Tăng độ khó dần dần**: Game không quá khó ngay từ đầu
2. **Thử thách người chơi**: Càng chơi lâu càng khó
3. **Cảm giác tiến bộ**: Level up tạo động lực chơi tiếp
4. **Cân bằng game**: Tốc độ tăng hợp lý, không quá đột ngột

---

## 🔧 Tùy Chỉnh

Nếu muốn thay đổi tốc độ tăng level, có thể chỉnh:

**Thay đổi số hàng để tăng level:**
```c
// Thay vì 10 hàng, có thể đổi thành 5, 15, 20...
int new_level = (total_lines_cleared / 5) + 1;  // Tăng level mỗi 5 hàng
```

**Thay đổi tốc độ giảm:**
```c
// Thay vì giảm 20ms, có thể giảm 10ms, 30ms...
drop_interval = 500 - (level - 1) * 30;  // Giảm nhanh hơn
```

**Thay đổi tốc độ ban đầu:**
```c
int drop_interval = 600;  // Chậm hơn ban đầu
// hoặc
int drop_interval = 400;  // Nhanh hơn ban đầu
```

---

## ✅ Kiểm Tra

Sau khi thêm tính năng, kiểm tra:

- [ ] Level hiển thị đúng trong header
- [ ] Level tăng khi xóa đủ 10 hàng
- [ ] Tốc độ rơi tăng đúng theo level
- [ ] Thông báo level up hiển thị
- [ ] Hoạt động đúng trong cả terminal và web client

---

## 📝 Ghi Chú

- Level được tính riêng cho mỗi người chơi (không ảnh hưởng đến người khác)
- Tốc độ tối thiểu là 50ms để đảm bảo game vẫn chơi được
- Level reset về 1 mỗi khi bắt đầu game mới
- Tổng số hàng đã xóa được đếm từ đầu game, không reset giữa các khối

---

**Tính năng này làm cho game thú vị và thử thách hơn! 🎮✨**

