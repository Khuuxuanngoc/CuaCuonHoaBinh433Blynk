# CHANGELOG

## [2.0.0] - 2026-11-19
### Added
- **Màn hình đăng nhập & Quản lý bảo mật 2 lớp**:
  - Màn hình khóa bảo vệ bằng Master Password trước khi vào giao diện điều khiển Web.
  - Phân tách rõ ràng giữa Mật khẩu WiFi AP (`12345678`) và Mật khẩu Quản trị Master (`123456`).
- **Trang Cấu hình WiFi & Cloud trực tiếp trên Web**:
  - Hỗ trợ quét tìm các mạng WiFi xung quanh (Scan WiFi SSIDs & RSSI).
  - Cho phép cấu hình tên WiFi, mật khẩu WiFi nhà, Google Apps Script Webhook URL và thời gian hành trình mà không cần mở Arduino IDE.
- **Khôi phục cài đặt gốc phần cứng (Hardware Factory Reset)**:
  - Giữ nút BOOT (GPIO 0) trong 5 giây để xóa Flash và khôi phục toàn bộ cài đặt gốc.
- **Bộ lệnh Serial CLI mở rộng**: `set pass`, `set ap`, `set wifi`, `set gas`, `wifi`, `reset`.
- **Google Apps Script & Giám sát cúp điện**:
  - Webhook ghi nhật ký đóng/mở cửa, tự động tính thời gian mở chu kỳ.
  - Heartbeat ping và ghi nhật ký cúp điện (`NhatKyCupDien`).
- **Xác minh đa nền tảng**: Biên dịch thành công 100% trên cả `ESP32-S3` và `ESP8266`.

## [1.0.0] - 2026-11-18
### Added
- Khởi tạo phiên bản đầu tiên của bộ điều khiển cửa cuốn RF 433MHz tại `examples/CuaCuon_ESP32S3_RF433_Serial`.
- Thiết kế kiến trúc Hướng Đối Tượng (OOP):
  - `IRollingDoor`: Interface trừu tượng chuẩn hóa các hàm điều khiển cửa cuốn.
  - `RollingDoorController`: Trái tim điều khiển, quản lý State Machine, thời gian chờ, chống dội, phát lệnh và sự kiện.
  - `RFTransceiver`: Lớp trừu tượng hóa phần cứng thu/phát RF 433MHz sử dụng thư viện RCSwitch.
  - `StorageManager`: Bộ lưu trữ đa nền tảng (Preferences/NVS trên ESP32/ESP32-S3 và EEPROM trên ESP8266).
  - `SerialCLI`: Giao diện dòng lệnh qua cổng Serial Monitor non-blocking, zero dynamic allocation.
- Hỗ trợ đầy đủ 4 trạng thái cửa: Lên (UP), Xuống (DOWN), Dừng (STOP), Khóa (LOCK).
- Các lệnh Serial: `learn`, `cancel`, `show`, `test`, `save`, `load`, `clear`, `sniff`, `help`.
- Tài liệu hướng dẫn chi tiết sơ đồ nối dây và roadmap mở rộng tại `README.md`.
