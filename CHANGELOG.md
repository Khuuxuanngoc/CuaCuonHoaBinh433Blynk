# CHANGELOG

## [2.1.0] - 2026-11-20
### Added
- **Tích hợp MQTT Cloud (Điều khiển tức thì < 0.2s qua 4G/Internet)**:
  - Thêm module `MqttManager` hỗ trợ cả kết nối TCP tiêu chuẩn (Port 1883) và bảo mật SSL/TLS (Port 8883 - HiveMQ Cloud).
  - Tối ưu bộ đệm BearSSL trên ESP8266 (`1024 bytes`) tránh tràn RAM.
  - Cơ chế Non-blocking tự động kết nối lại khi có WiFi mà không bao giờ gây treo sóng Web AP hay nút bấm.
  - Tự động báo trạng thái sống LWT (`<topic>/status`), nhận lệnh tức thì (`<topic>/command`), phát trạng thái JSON (`<topic>/state`).
  - Tích hợp giao diện bật/tắt và điền thông số Broker ngay trong Tab **WIFI & CLOUD** của Web AP Local.
- **Khắc phục triệt để lỗi treo Web AP khi router nhà mất điện**:
  - `handleStaConnection()` State Machine: Hạn chế thử kết nối WiFi nhà tối đa 10 giây, sau đó gọi `WiFi.disconnect(false)` ngắt channel-hopping để giữ sóng SoftAP `192.168.4.1` luôn ổn định < 5ms.
  - Frontend `fetchWithTimeout()` 4 giây chống treo trình duyệt.
- **Bảo vệ an toàn chống nạp nhầm Firmware Web OTA**:
  - Tự động nhận diện bo mạch và kiến trúc chip trên giao diện Web OTA `/update`.
  - Kiểm tra Magic Byte và Chip ID phần cứng ngay từ gói tin đầu tiên, chủ động từ chối nạp file `.bin` ESP32 vào ESP8266 hoặc ngược lại để chống brick bo mạch.
- **Chuẩn hóa Terminal Serial**: Chuyển toàn bộ `Serial.print` sang tiếng Việt không dấu (`tieng viet khong dau`) chống lỗi font màn hình.

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
