# TODO

## Giai đoạn 1: Serial CLI (Hoàn thành)
- [x] Tạo kiến trúc Hướng Đối Tượng (OOP): `IRollingDoor`, `RollingDoorController`, `RFTransceiver`, `StorageManager`, `SerialCLI`.
- [x] Triển khai học lệnh và giải mã 4 nút: UP, DOWN, STOP, LOCK.
- [x] Lưu trữ lệnh vĩnh viễn vào Flash (Preferences trên ESP32-S3, EEPROM trên ESP8266).
- [x] Giao diện dòng lệnh Serial CLI tương tác trực tiếp (`learn`, `show`, `test`, `save`, `load`, `clear`, `sniff`).
- [x] Lưu phiên bản đầu tiên tại `examples/CuaCuon_ESP32S3_RF433_Serial`.

## Giai đoạn 2: WebServer Access Point (Kế hoạch tiếp theo)
- [ ] Xây dựng `WebServerManager` (chế độ SoftAP phát WiFi cấu hình).
- [ ] Giao diện Web HTML5/CSS responsive cho điện thoại di động (4 nút bấm lớn, nút học lệnh).
- [ ] API REST endpoint điều khiển: `/api/control`, `/api/learn`, `/api/status`.

## Giai đoạn 3: Blynk IoT Cloud & Google Apps Script
- [ ] Tích hợp Blynk IoT (Virtual Pins V1..V4 điều khiển cửa cuốn từ xa qua Internet).
- [ ] Kết nối Google Apps Script (Webhook HTTP ghi nhận lịch sử mở/đóng cửa vào Google Sheets).

## Giai đoạn 4: Tối ưu chi phí & Port sang ESP8266
- [ ] Kiểm thử thực tế trên bo mạch ESP8266 (NodeMCU / WeMos D1).
- [ ] Đóng gói thư viện dùng chung vào `libraries/`.
