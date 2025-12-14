# 🎮 Hướng Dẫn Sử Dụng Giao Diện Web UI

## 📋 Yêu Cầu

1. **Node.js** (phiên bản 12 trở lên)
   - Tải về tại: https://nodejs.org/
   - Hoặc cài đặt qua package manager:
     ```bash
     # macOS
     brew install node
     
     # Ubuntu/Debian
     sudo apt install nodejs npm
     ```

2. **Server C đang chạy**
   - Server C phải được khởi động trước (xem hướng dẫn trong TINH_NANG_MOI.md)

## 🚀 Cài Đặt và Chạy

### Bước 1: Cài đặt dependencies

```bash
npm install
```

Hoặc cài đặt thủ công:
```bash
npm install ws
```

### Bước 2: Khởi động Server C

Mở terminal và chạy:
```bash
./server 5555
```

### Bước 3: Khởi động WebSocket Proxy Server

Mở terminal mới và chạy:
```bash
node websocket-proxy.js
```

Bạn sẽ thấy:
```
WebSocket Proxy Server đang chạy trên port 8080
Kết nối đến TCP server tại localhost:5555
Đang chờ kết nối từ WebSocket clients...
```

### Bước 4: Mở giao diện web

Có 2 cách:

**Cách 1: Mở trực tiếp file HTML**
- Mở file `index.html` trong trình duyệt
- Lưu ý: Một số trình duyệt có thể chặn WebSocket từ file://
- Nếu gặp lỗi, sử dụng Cách 2

**Cách 2: Sử dụng HTTP server có sẵn (KHUYẾN NGHỊ)**

Chạy HTTP server đơn giản đã được tạo sẵn:
```bash
node http-server.js
```

Hoặc sử dụng npm script:
```bash
npm run http
```

Sau đó mở trình duyệt và truy cập:
```
http://localhost:3000
```

**Cách 3: Sử dụng Python (nếu đã cài)**

```bash
# Python 3
python3 -m http.server 3000

# Python 2
python -m SimpleHTTPServer 3000
```

Sau đó mở trình duyệt:
```
http://localhost:3000
```

**Cách 4: Cài đặt http-server global (tùy chọn)**

Nếu muốn sử dụng http-server:
```bash
npm install -g http-server
http-server -p 3000
```

## 🎮 Sử Dụng

1. **Đăng ký/Đăng nhập**
   - Nhập username và password
   - Click "Đăng Nhập" hoặc "Đăng Ký"

2. **Tạo hoặc tham gia phòng**
   - Click "Tạo Phòng" để tạo phòng mới
   - Hoặc click "Danh Sách Phòng" để xem và tham gia phòng có sẵn

3. **Sẵn sàng chơi**
   - Click "✅ Ready" để báo sẵn sàng
   - Nếu bạn là người ready đầu tiên, chọn chế độ chơi
   - Khi tất cả ready, game sẽ tự động bắt đầu

4. **Chơi game**
   - Sử dụng phím mũi tên hoặc WASD để điều khiển:
     - ← (A): Di chuyển trái
     - → (D): Di chuyển phải
     - ↓ (S): Rơi nhanh
     - ↑ (W): Xoay khối
     - Q: Thoát game

5. **Xem kết quả**
   - Sau khi game kết thúc, xem bảng xếp hạng
   - Click "Quay Lại Lobby" để chơi tiếp

## 🔧 Cấu Hình

### Thay đổi cổng WebSocket

Sửa trong `websocket-proxy.js`:
```javascript
const WS_PORT = 8080; // Đổi số cổng ở đây
```

Và trong `websocket-client.js`:
```javascript
const wsUrl = localStorage.getItem('wsServerUrl') || 'ws://localhost:8080';
```

### Thay đổi địa chỉ TCP server

Sửa trong `websocket-proxy.js`:
```javascript
const TCP_SERVER_HOST = 'localhost';
const TCP_SERVER_PORT = 5555;
```

## 🐛 Xử Lý Lỗi

### Lỗi "Cannot connect to WebSocket"
- Đảm bảo WebSocket proxy server đang chạy
- Kiểm tra cổng 8080 có bị chiếm dụng không
- Thử đổi cổng trong cấu hình

### Lỗi "TCP connection failed"
- Đảm bảo server C đang chạy trên cổng 5555
- Kiểm tra địa chỉ TCP_SERVER_HOST trong cấu hình

### Giao diện không hiển thị đúng
- Xóa cache trình duyệt (Ctrl+Shift+Delete)
- Thử trình duyệt khác (Chrome, Firefox, Edge)
- Kiểm tra console trình duyệt (F12) để xem lỗi

### Game không phản hồi
- Kiểm tra kết nối WebSocket trong tab Network của DevTools
- Đảm bảo server C đang chạy và nhận kết nối
- Xem log của WebSocket proxy server

## 📝 Lưu Ý

1. **Bảo mật**: Giao diện web này chỉ dùng cho development. Để deploy production, cần:
   - Thêm HTTPS/WSS
   - Xác thực và authorization
   - Rate limiting
   - CORS configuration

2. **Hiệu năng**: 
   - Game loop sử dụng requestAnimationFrame cho mượt mà
   - Canvas được tối ưu để render nhanh

3. **Tương thích**:
   - Hỗ trợ các trình duyệt hiện đại (Chrome, Firefox, Safari, Edge)
   - Cần hỗ trợ WebSocket và Canvas API

## 🎉 Chúc Bạn Chơi Vui Vẻ!

Nếu có vấn đề, hãy kiểm tra:
1. Server C có đang chạy không
2. WebSocket proxy server có đang chạy không
3. Console trình duyệt có lỗi gì không
4. Log của WebSocket proxy server

