# CHANGELOG

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
