# TODO

## Giai đoạn 1: Serial CLI (Hoàn thành)
- [x] Tạo kiến trúc Hướng Đối Tượng (OOP): `IRollingDoor`, `RollingDoorController`, `RFTransceiver`, `StorageManager`, `SerialCLI`.
- [x] Triển khai học lệnh và giải mã 4 nút: UP, DOWN, STOP, LOCK.
- [x] Lưu trữ lệnh vĩnh viễn vào Flash (Preferences trên ESP32-S3, EEPROM trên ESP8266).
- [x] Giao diện dòng lệnh Serial CLI tương tác trực tiếp (`learn`, `show`, `test`, `save`, `load`, `clear`, `export`, `import`, `set`, `sniff`).
- [x] Lưu phiên bản đầu tiên tại `examples/CuaCuon_ESP32S3_RF433_Serial`.

## Giai đoạn 2: Giám Sát Chuyển Động Ảo & WebServer AP + Web OTA (Hoàn thành)
- [x] Xây dựng `DoorMotionTracker`: Lắng nghe remote 24/7, tính toán thời gian hành trình, phần trăm độ mở (0-100%) và thời gian cửa đang mở mà không cần cảm biến.
- [x] Xây dựng `WebServerManager`: Tự phát WiFi Access Point `CuaCuon_HoaBinh` (IP `192.168.4.1`).
- [x] Giao diện Web App nhúng trên ESP (Dark mode, responsive, điều khiển 4 nút, đồng hồ đếm thời gian mở).
- [x] Màn hình khóa đăng nhập bảo mật (Master Password) trước khi vào Dashboard.
- [x] **Setup Học Lệnh RF trên Web AP**: Modal đếm ngược 15s với animation radar, tự động bắt gói sóng remote và lưu vĩnh viễn vào Flash.
- [x] Quản lý mã nút RF trên Web: Thử phát, sửa mã Hex, xóa mã, và công cụ Export / Import chuỗi mã trực tiếp.
- [x] Trang cấu hình WiFi gia đình (Quét WiFi xung quanh) & Google Apps Script Webhook URL trực tiếp trên điện thoại.
- [x] Mô hình bảo mật 2 mật khẩu: WiFi AP Pass và Master Password.
- [x] Nạp Firmware Web OTA trực tiếp qua trình duyệt tại `/update` (bảo vệ bằng mật khẩu).
- [x] Khôi phục cài đặt gốc phần cứng (Hardware Factory Reset) giữ nút BOOT 5 giây.
- [x] Bổ sung lệnh Serial CLI: `set pass`, `set ap`, `set wifi`, `set gas`, `wifi`, `reset`.

## Giai đoạn 3: Google Apps Script & Giám Sát Cúp Điện (Hoàn thành)
- [x] Xây dựng trọn bộ mã nguồn trong thư mục `GoogleAppsScript/` (`Code.gs`, `Index.html`, `README.md`).
- [x] Ghi nhật ký đóng/mở cửa vào Google Sheet kèm thời gian mở chi tiết.
- [x] Cơ chế Heartbeat Ping định kỳ phát hiện Online / Offline.
- [x] Tự động ghi nhận **Lịch sử cúp điện** (thời điểm mất điện, thời điểm có điện lại, tổng thời gian mất điện).
- [x] **Đồng bộ tính năng 100% với Web AP Local**:
  - Tab Quản lý & Học Lệnh Mã Remote RF 433MHz 4 nút.
  - Gửi lệnh Yêu cầu học lệnh và Phát lệnh từ xa qua cloud.
  - Chỉnh sửa thông số mã RF và Tiện ích Export / Import chuỗi mã.
  - Bảng điều khiển 4 nút, Tab Lịch sử cúp điện và Tab Lịch sử đóng mở cửa.

## Giai đoạn 4: Tối ưu chi phí & Port sang ESP8266 (Hoàn thành)
- [x] Biên dịch thành công 100% trên `esp32:esp32:esp32s3` qua `arduino-cli`.
- [x] Biên dịch thành công 100% trên `esp8266:esp8266:nodemcuv2` qua `arduino-cli`.
- [ ] Thử nghiệm thực tế trên mạch phần cứng.
- [ ] Đóng gói thư viện dùng chung vào `libraries/`.
