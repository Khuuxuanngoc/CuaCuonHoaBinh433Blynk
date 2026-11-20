# Hướng Dẫn Triển Khai: Google Apps Script - Giám Sát Cửa Cuốn

Thư mục này chứa toàn bộ mã nguồn của hệ thống **Google Apps Script** phục vụ việc:
* Tiếp nhận sự kiện đóng/mở cửa từ **ESP32-S3 / ESP8266** gửi lên qua Webhook HTTPS.
* Tự động ghi nhật ký lịch sử chi tiết vào **Google Sheets**.
* Cung cấp giao diện **Web App** chuyên nghiệp có mật khẩu bảo vệ để tra cứu lịch sử và theo dõi trạng thái cửa trên điện thoại di động mọi lúc mọi nơi.

---

## 1. Cấu Trúc Tệp

```text
GoogleAppsScript/
├── Code.gs      # Backend: Xử lý Webhook doPost, API doGet, ghi Google Sheet
├── Index.html   # Frontend: Giao diện Web App (HTML5 + Tailwind CSS + Lucide Icons)
└── README.md    # Hướng dẫn chi tiết từng bước thiết lập
```

---

## 2. Hướng Dẫn Cài Đặt Từng Bước (Chỉ mất 3 phút)

### Bước 1: Tạo Google Sheet Mới
1. Mở trình duyệt web và truy cập: [Google Sheets](https://sheets.google.com).
2. Tạo một trang tính trống mới, đặt tên file là: **`NhatKy_CuaCuon_HoaBinh`**.

https://docs.google.com/spreadsheets/d/1AwXbFpeI6E8ltdCdcy7W8HV9dIpllDA-UgtZzE1Fr8k/edit?gid=0#gid=0

---

### Bước 2: Mở Trình Soạn Thảo Google Apps Script
1. Trên thanh menu của Google Sheet, bấm vào: **Tiện ích mở rộng (Extensions)** $\rightarrow$ **Apps Script**.
2. Một tab mới sẽ mở ra trình soạn thảo mã nguồn.

---

### Bước 3: Dán Mã Nguồn Vào Dự Án
1. **Dán file `Code.gs`:**
   * Trong danh sách tệp bên trái, nhấp vào file `Code.gs`.
   * Xóa toàn bộ nội dung mặc định và **Copy toàn bộ nội dung từ file `GoogleAppsScript/Code.gs`** trong repository này dán vào.
   * Nhấn biểu tượng đĩa mềm **Lưu (Save - `Ctrl + S`)**.

2. **Tạo file `Index.html`:**
   * Ở cột danh sách tệp bên trái, bấm vào dấu **`+`** (Thêm tệp) $\rightarrow$ Chọn **HTML**.
   * Đặt tên tệp là: **`Index`** (hệ thống sẽ tự thêm đuôi `.html`).
   * Xóa toàn bộ nội dung mặc định và **Copy toàn bộ nội dung từ file `GoogleAppsScript/Index.html`** dán vào.
   * Nhấn biểu tượng đĩa mềm **Lưu (Save - `Ctrl + S`)**.

---

### Bước 4: Triển Khai Thành Ứng Dụng Web (Deploy Web App)
1. Ở góc trên cùng bên phải, bấm vào nút màu xanh: **Triển khai (Deploy)** $\rightarrow$ Chọn **Tùy chọn triển khai mới... (New deployment)**.
2. Bấm vào biểu tượng bánh răng **Chọn loại (Select type)** $\rightarrow$ Chọn **Ứng dụng web (Web app)**.
3. Điền các thông số:
   * **Mô tả (Description):** `He Thong Cua Cuon V1`
   * **Thực thi dưới dạng (Execute as):** **Tôi (Me - your_email@gmail.com)**
   * **Ai có quyền truy cập (Who has access):** **Bất kỳ ai (Anyone)** *(Bắt buộc chọn Anyone để ESP32/ESP8266 có thể gửi dữ liệu Webhook lên mà không bị chặn xác thực Google OAuth)*.
4. Bấm **Triển khai (Deploy)**.
5. Cửa sổ cấp quyền hiện ra: Bấm **Ủy quyền truy cập (Authorize access)** $\rightarrow$ Chọn tài khoản Google của bạn $\rightarrow$ Bấm **Nâng cao (Advanced)** $\rightarrow$ Chọn **Đi tới ... (không an toàn)** $\rightarrow$ Bấm **Cho phép (Allow)**.

---

### Bước 5: Lấy URL Ứng Dụng Web & Webhook
Sau khi triển khai xong, Google sẽ cung cấp cho bạn một đường link dạng:
```text
https://script.google.com/macros/s/AKfycbx.../exec
```

* **Dùng để mở Web App:** Bạn có thể mở link này trên điện thoại (thêm vào màn hình chính Home Screen) để xem giao diện điều khiển và nhật ký.
* **Mật khẩu đăng nhập mặc định:** **`123456`** *(Bạn có thể đổi mật khẩu bất kỳ lúc nào trong mục Cài đặt của Web App)*.
* **Dùng cho ESP32/ESP8266:** Copy đường link này để dán vào file cấu hình `Config.h` trên ESP để tự động gửi thông báo.

---

## 3. Các Tính Năng Web App Toàn Diện (Đồng Bộ 100% Với Web AP Local)

Giao diện Web App trên Google Apps Script cung cấp trọn vẹn 4 tab tính năng tương tự Web AP trên ESP:

1. **🎮 Cửa Cuốn (Dashboard)**:
   - Theo dõi trạng thái đóng kín / đang mở / tạm dừng / đã khóa theo thời gian thực.
   - Bảng 4 nút điều khiển từ xa: Lên, Xuống, Tạm dừng, Khóa hộp.
   - Đèn báo **Online / Offline** (tự động phát hiện khi ESP mất kết nối quá 150 giây).
2. **📻 Quản Lý & Học Lệnh Mã Remote RF 433MHz**:
   - Hiển thị danh sách 4 nút với đầy đủ thông số: Mã Hex, Mã Dec, Bits, Protocol, Độ rộng xung (Pulse us).
   - Nút **"📡 Yêu Cầu Học Lệnh"**: Kích hoạt ESP chuyển sang chế độ học lệnh từ xa qua internet.
   - Nút **"▶ Phát Lệnh"**: Gửi lệnh phát RF từ xa qua cloud.
   - Nút **"✏ Sửa Mã"**: Chỉnh sửa trực tiếp thông số mã RF trên Google Sheet.
   - Tiện ích **Export / Import chuỗi mã**: Cho phép sao chép chuỗi mã 4 nút để di chuyển giữa các thiết bị mà không cần remote.
3. **⚡ Lịch Sử Cúp Điện (`NhatKyCupDien`)**:
   - Tự động ghi lại thời điểm bắt đầu mất điện, thời điểm có điện lại, và tổng thời gian mất điện (giờ, phút).
4. **📜 Lịch Sử Đóng/Mở Cửa (`NhatKyCuaCuon`)**:
   - Nhật ký chi tiết từng lần mở cửa, thời gian cửa mở liên tục và nguồn kích hoạt (Remote RF cầm tay / Web AP Local / Google Web App).
5. **⚙ Cài Đặt Hệ Thống (Settings Modal)**:
   - Đổi Master Password.
   - Điều chỉnh thời gian hành trình cửa (giây).

---

## 4. Kiểm Thử Gửi Webhook Từ ESP32 / Curl

Gói tin JSON mà ESP32 / ESP8266 gửi lên có cấu trúc chuẩn như sau:

```bash
curl -L -X POST "https://script.google.com/macros/s/AKfycbx.../exec" \
  -H "Content-Type: application/json" \
  -d '{
    "token": "HoaBinh_RollingDoor_2026",
    "event": "MỞ CỬA",
    "doorState": "ĐÃ MỞ HOÀN TOÀN",
    "openDurationSeconds": 1500,
    "source": "Remote RF",
    "note": "Phát hiện bấm remote gốc"
  }'
```

Ngay lập tức, trên Google Sheet của bạn sẽ tự động xuất hiện 1 dòng ghi nhận có ngày giờ chính xác!
