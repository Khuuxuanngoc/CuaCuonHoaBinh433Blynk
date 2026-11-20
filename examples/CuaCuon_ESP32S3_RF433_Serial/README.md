# Hướng Dẫn Sử Dụng: Điều Khiển Cửa Cuốn RF 433MHz (Phiên Bản V1 - Serial CLI)

Dự án điều khiển cửa cuốn từ xa sử dụng vi điều khiển **ESP32-S3** (sẵn sàng chuyển đổi sang **ESP8266**), học lệnh 4 nút chức năng: **Lên (Up)**, **Xuống (Down)**, **Dừng (Stop)**, **Khóa (Lock)** từ remote RF 433MHz gốc, lưu trữ vĩnh viễn vào bộ nhớ Flash và kiểm tra phát lệnh qua cổng Serial Monitor.

---

## 1. Tính Năng Nổi Bật

* **Học lệnh RF 433MHz chính xác:** Ghi nhận đầy đủ 4 thông số: `Mã code`, `Độ dài bit` (bitLength), `Giao thức` (protocol), và `Độ rộng xung` (pulseLength). Đảm bảo phát lại chuẩn xác 100% cho mọi loại cửa cuốn mã cố định (Fixed code) hoặc mã học (Learning code EV1527, PT2262, v.v.).
* **Giao diện dòng lệnh Serial CLI chuyên nghiệp:**
  * Học lệnh từng nút với đếm ngược timeout (15s), chống dội tín hiệu (debounce).
  * Hiển thị bảng mã chi tiết dạng ASCII Table trực quan.
  * Thử phát lệnh trực tiếp (`test up`, `test down`, `test stop`, `test lock`).
  * Lưu trữ vĩnh viễn vào Flash (`save`), nạp lại (`load`), hoặc xóa trắng (`clear`).
  * Chế độ **RF Sniffer / Scan** quét sóng xung quanh để kiểm tra phần cứng và remote.
* **Kiến trúc Hướng Đối Tượng (OOP) chuẩn mực:**
  * Tách biệt hoàn toàn tầng điều khiển logic (`RollingDoorController`) khỏi tầng truyền thông/giao diện (`SerialCLI`).
  * Sẵn sàng tích hợp thêm WebServer AP, Blynk IoT Cloud, hoặc Google Apps Script mà **không cần sửa đổi logic cửa**.
* **Đa nền tảng (Cross-platform):** Tự động nhận diện cấu hình cho cả **ESP32-S3** (Preferences/NVS) và **ESP8266** (EEPROM Flash Sector).

---

## 2. Sơ Đồ Đấu Nối Phần Cứng (Wiring Diagram)

### 2.1. Danh Sách Linh Kiện
1. Vi điều khiển: **ESP32-S3** (hoặc ESP8266 NodeMCU / WeMos D1 Mini).
2. Module thu RF 433MHz: RXB6, SYN480R, hoặc MX-05V (khuyến nghị **RXB6** hoặc **SYN480R** độ nhạy cao).
3. Module phát RF 433MHz: FS1000A, SYN115, hoặc STX882.
4. Ăng-ten: Dây đồng đơn lõi dài **17.3 cm** (tương ứng 1/4 bước sóng 433.92MHz) hàn vào chân `ANT` của cả module thu và phát để tăng cự ly thu phát.

---

### 2.2. Sơ Đồ Chân ESP32-S3

| Tên Module | Chân Module | Chân Trên ESP32-S3 | Ghi Chú |
|:---|:---|:---|:---|
| **Bộ Thu RF 433** | VCC | 5V | Cấp 5V để cự ly bắt sóng tốt nhất |
| | GND | GND | Nối chung mass |
| | DATA | **GPIO 8 (Qua FET)** | FET hạ áp 5V -> 3.3V, bảo vệ chân ESP32-S3 |
| **Bộ Phát RF 433** | VCC | 5V (hoặc 9-12V) | Cấp 5V phát sóng xa và khỏe |
| | GND | GND | Nối chung mass |
| | DATA | **GPIO 9 (Qua FET)** | FET kích áp xung từ 3.3V lên 5V công suất cao |
| **LED Trạng Thái** | Chân Dương (+) | **GPIO 2** (qua trở 1k) | Nhấp nháy khi học lệnh, sáng khi phát |

> [!TIP]
> Người dùng có thể thay đổi các chân kết nối tùy theo thiết kế bo mạch thực tế trong file `Config.h`.

---

### 2.3. Sơ Đồ Chân Chuyển Sang ESP8266 (NodeMCU / WeMos D1)

| Tên Module | Chân Module | Chân ESP8266 (GPIO) | Ký Hiệu Board |
|:---|:---|:---|:---|
| **Bộ Thu RF 433** | DATA | **GPIO 4** | **D2** |
| **Bộ Phát RF 433** | DATA | **GPIO 5** | **D1** |
| **LED Trạng Thái** | LED Built-in | **GPIO 2** | **D4** (LED tích hợp trên board) |

---

## 3. Cài Đặt Thư Viện

Trước khi biên dịch bằng Arduino IDE, hãy cài đặt thư viện giải mã 433MHz tiêu chuẩn:

1. Mở **Arduino IDE**.
2. Vào **Sketch** -> **Include Library** -> **Manage Libraries...** (hoặc nhấn `Ctrl + Shift + I`).
3. Trong ô tìm kiếm, gõ: `rc-switch` (tác giả: **sui77**).
4. Nhấn **Install** để cài đặt.

---

## 4. Hướng Dẫn Sử Dụng Serial CLI

Sau khi nạp chương trình vào bo mạch:
1. Mở **Serial Monitor** trong Arduino IDE.
2. Cài đặt tốc độ Baud: **`115200`**.
3. Cài đặt chế độ kết thúc dòng: **`Both NL & CR`** hoặc **`Newline`**.

### Bảng Lệnh CLI

| Lệnh | Ý Nghĩa & Ví Dụ |
|:---|:---|
| `help` hoặc `?` | Hiển thị bảng trợ giúp và hướng dẫn sử dụng. |
| `show` hoặc `list` | Hiển thị bảng danh sách mã RF 4 trạng thái (Lên, Xuống, Dừng, Khóa). |
| `learn <nut>` | Chuyển sang chế độ học lệnh cho nút (`up`, `down`, `stop`, `lock` hoặc `1..4`).<br>Ví dụ: `learn up` hoặc `learn 1` |
| `cancel` | Hủy chế độ học lệnh nếu đang đợi remote mà không muốn tiếp tục. |
| `test <nut>` | Thử phát sóng RF cho nút tương ứng để kiểm tra cửa cuốn.<br>Ví dụ: `test up` hoặc `test 1` |
| `save` | Ghi vĩnh viễn các lệnh vừa học vào bộ nhớ Flash (NVS / EEPROM). |
| `load` | Tải lại các lệnh đã lưu từ Flash ra RAM. |
| `clear` | Xóa sạch bộ nhớ Flash và đặt các nút về trạng thái chưa học. |
| `export` | Xuất 4 mã đã học từ ESP32-S3 để copy sang ESP8266 (hoặc dán vào `Config.h`). |
| `import <u d s l>` | Nạp nhanh toàn bộ 4 mã vào bộ nhớ mà không cần remote (Tự lưu vào Flash).<br>Ví dụ: `import 0xA1B2C3 0xA1B2C4 0xA1B2C5 0xA1B2C6 24 1 350` |
| `set <nut> <code>` | Gán thủ công mã RF cho một nút xác định.<br>Ví dụ: `set up 0xA1B2C3 24 1 350` |
| `set pass <pwd>` | Đổi Master Password nhanh chóng qua cổng Serial. |
| `set ap <pwd>` | Đổi mật khẩu phát WiFi AP của ESP (tối thiểu 8 ký tự). |
| `set wifi <s> <p>` | Cài đặt WiFi gia đình (SSID và Mật khẩu). |
| `set gas <url>` | Cài đặt đường dẫn Google Apps Script Webhook. |
| `wifi` hoặc `ip` | Xem thông tin IP, WiFi và mật khẩu hiện tại. |
| `reset` | Khôi phục toàn bộ cài đặt gốc (Factory Reset) và khởi động lại. |
| `sniff on` | Bật chế độ quét sóng liên tục (hữu ích để dò remote lạ hoặc test bộ thu). |
| `sniff off` | Tắt chế độ quét sóng liên tục. |

---

## 5. Mô Hình Bảo Mật 2 Mật Khẩu (Security Model)

Hệ thống tối ưu hóa trải nghiệm người dùng với mô hình phân cấp **2 Mật Khẩu**:

1. **Mật khẩu WiFi AP của ESP (Mặc định: `12345678`)**:
   - Dùng để kết nối điện thoại vào mạng WiFi nội bộ phát ra từ ESP (`CuaCuon_HoaBinh`).
   - Chuẩn bảo mật WPA2 yêu cầu tối thiểu 8 ký tự.
2. **Mật khẩu Quản trị Master (Mặc định: `123456`)**:
   - Dùng để mở khóa màn hình đăng nhập bảng điều khiển Web App Local (`192.168.4.1` hoặc IP mạng nhà).
   - Dùng để đăng nhập Web App trên Google Apps Script (truy cập từ xa 4G).
   - Dùng để xác thực nạp Firmware Web OTA (`/update`).

---

## 6. Giao Diện Web App Local & Cấu Hình Trực Tiếp

Khi kết nối vào WiFi `CuaCuon_HoaBinh`, mở trình duyệt truy cập: **`http://192.168.4.1`** (hoặc IP mạng nhà):

- **Màn hình đăng nhập**: Nhập Master Password để mở khóa toàn bộ bảng điều khiển.
- **Tab 1 - Điều khiển**: 4 nút bấm to (Lên, Xuống, Dừng, Khóa), trạng thái cửa, thanh tiến trình % hành trình, đồng hồ đếm thời gian mở.
- **Tab 2 - Học lệnh & Cài đặt mã RF**:
  - Xem thông số 4 nút: Hex, Dec, Bits, Protocol, Độ rộng xung us.
  - **Học lệnh không dây**: Bấm nút **📡 Học Lệnh**, modal đếm ngược 15s với animation radar hiện ra, bấm giữ remote gần anten -> ESP tự bắt mã và lưu vĩnh viễn vào Flash!
  - **Phát thử & Sửa mã**: Thử phát sóng trực tiếp hoặc sửa thủ công thông số mã RF.
  - **Xuất / Nhập mã (Export / Import)**: Copy hoặc dán chuỗi 4 mã RF để nạp nhanh sang ESP khác mà không cần remote.
- **Tab 3 - Cài đặt WiFi & Cloud**: Quét mạng WiFi xung quanh, chọn mạng, nhập mật khẩu WiFi nhà, nhập Google Script URL và thời gian hành trình cửa. Bấm **Lưu & Kết nối** để ESP tự động kết nối mạng gia đình.
- **Tab 4 - Đổi mật khẩu**: Cho phép đổi mật khẩu WiFi AP và Master Password trực tiếp trên điện thoại.
- **Web OTA (`/update`)**: Nâng cấp firmware file `.bin` trực tiếp qua WiFi có bảo vệ bằng Master Password.

---

## 7. Khôi Phục Cài Đặt Gốc Phần Cứng (Hardware Factory Reset)

Nếu lỡ quên mật khẩu hoặc cần bàn giao thiết bị:
- **Cách 1 (Nút BOOT vật lý)**: Nhấn và giữ nút **BOOT** (GPIO 0 trên ESP32) hoặc **FLASH** (D3 trên ESP8266) trong **5 giây**. Đèn LED trạng thái sẽ nhấp nháy nhanh liên tục và ESP tự động xóa Flash, đưa toàn bộ mật khẩu và WiFi về mặc định ban đầu (`123456` / `12345678`).
- **Cách 2 (Serial CLI)**: Cắm cổng USB mở Serial Monitor gõ `reset` hoặc `set pass 123456`.

---

### Quy Trình Học Lệnh Mẫu

```text
1. Gõ: learn up
   >>> [CHẾ ĐỘ HỌC LỆNH] Đang chờ tín hiệu cho nút: UP - Lên (Mở cửa)
   >>> Hướng remote vào anten thu 433MHz và BẤM GIỮ NÚT trong 1-2 giây...

2. Bấm giữ nút LÊN trên remote:
   >>> [HỌC THÀNH CÔNG] Nút: UP (Lên (Mở cửa))
       - Mã Hex     : 0x00A1B2C3
       - Mã Dec     : 10597059
       - Số Bit     : 24
       - Protocol   : 1
       - Pulse delay: 350 us

3. Lặp lại với các nút còn lại:
   learn down  -> Bấm nút XUỐNG trên remote
   learn stop  -> Bấm nút DỪNG trên remote
   learn lock  -> Bấm nút KHÓA trên remote

4. Kiểm tra danh sách:
   show

5. Thử điều khiển cửa:
   test up     -> Cửa cuốn bắt đầu mở lên!

6. Lưu vào bộ nhớ Flash:
   save
   >>> [THÀNH CÔNG] Đã lưu 4 lệnh vào Flash! Không bị mất khi khởi động lại.
```

---

## 5. Kiến Trúc Hướng Đối Tượng (OOP Design)

Dự án được thiết kế theo mô hình tách lớp độc lập:

```mermaid
classDiagram
    class IRollingDoor {
        <<interface>>
        +begin()
        +update()
        +startLearning(action, timeout)
        +cancelLearning()
        +sendCommand(action)
        +save()
        +load()
        +clear()
        +getCommand(action)
    }

    class RollingDoorController {
        -RFTransceiver _rf
        -StorageManager _storage
        -RFCommand _commands[4]
        -ControllerState _state
        +begin()
        +update()
        +startLearning()
        +sendCommand()
        +setOnLearnCallback()
        +setOnActionCallback()
    }

    class RFTransceiver {
        -RCSwitch _rf
        +begin()
        +readReceivedCommand()
        +sendCommand()
    }

    class StorageManager {
        +begin()
        +saveAll()
        +loadAll()
        +clear()
    }

    class SerialCLI {
        -RollingDoorController& _controller
        +begin()
        +update()
        +printTable()
    }

    IRollingDoor <|.. RollingDoorController
    RollingDoorController *-- RFTransceiver
    RollingDoorController *-- StorageManager
    SerialCLI o-- RollingDoorController
```

* **`RollingDoorController`**: Hoàn toàn không phụ thuộc vào `Serial`. Nó quản lý State Machine, thời gian chờ, chống dội, kiểm tra tính hợp lệ và gọi các Callbacks khi có sự kiện.
* **`SerialCLI`**: Chỉ là một tầng giao diện (Adapter). Khi có ký tự từ cổng nối tiếp, nó gọi phương thức tương ứng của `RollingDoorController`.

---

## 6. Lộ Trình Nâng Cấp (Roadmap)

Nhờ kiến trúc OOP này, các tính năng tiếp theo có thể được thêm vào cực kỳ đơn giản:

1. **Nâng cấp WebServer Access Point (AP Mode):**
   * Tạo module `WebServerManager.h/.cpp`.
   * Tạo trang web giao diện responsive trên điện thoại (4 nút: Lên, Xuống, Dừng, Khóa và nút Học Lệnh).
   * Khi người dùng bấm nút trên Web, WebServer chỉ cần gọi:
     ```cpp
     doorController.sendCommand(ACTION_UP);
     ```
2. **Nâng cấp Blynk IoT Cloud:**
   * Thêm `BlynkSimpleEsp32.h`.
   * Gán nút bấm ảo Blynk Virtual Pin:
     ```cpp
     BLYNK_WRITE(V1) {
         if (param.asInt()) doorController.sendCommand(ACTION_UP);
     }
     ```
3. **Nâng cấp Google Apps Script:**
   * Gửi HTTP Webhook tới Google Sheet / AppScript để ghi log thời gian đóng/mở cửa hoặc nhận lệnh điều khiển từ xa.
4. **Chuyển đổi sang ESP8266 (Cost Optimization):**
   * Chỉ cần đổi cấu hình Board trong Arduino IDE sang **NodeMCU 1.0 (ESP-12E)** hoặc **WeMos D1 R2 & mini**.
   * Toàn bộ mã nguồn tự động biên dịch sang nền tảng EEPROM của ESP8266 mà không cần sửa 1 dòng code nào.
