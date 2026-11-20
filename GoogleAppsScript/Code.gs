/**
 * ============================================================================
 * HỆ THỐNG ĐIỀU KHIỂN & GIÁM SÁT CỬA CUỐN RF 433MHz - GOOGLE APPS SCRIPT
 * Tác giả : Hệ thống Robotics & IoT Hòa Bình
 * Chức năng: 
 *   - Tiếp nhận Webhook ghi nhật ký đóng mở cửa
 *   - Giám sát trạng thái Online/Offline (Heartbeat Ping)
 *   - TỰ ĐỘNG GHI NHẬN LỊCH SỬ CÚP ĐIỆN (Thời điểm & Tổng thời gian)
 *   - Quản lý mã remote RF 433MHz 4 nút (Hex, Dec, Bits, Pulse)
 *   - Yêu cầu học lệnh và điều khiển từ xa đồng bộ với ESP
 *   - Web App Responsive quản trị toàn diện, đa tab, bảo mật mật khẩu
 * ============================================================================
 */

const CONFIG = {
  DEFAULT_PASSWORD   : "123456",                   // Mật khẩu Web App mặc định
  WEBHOOK_TOKEN      : "HoaBinh_RollingDoor_2026", // Khóa bí mật ESP gửi lên
  SHEET_LOGS_NAME    : "NhatKyCuaCuon",            // Nhật ký đóng mở cửa
  SHEET_OUTAGE_NAME  : "NhatKyCupDien",            // Nhật ký cúp điện / mất kết nối
  SHEET_CONFIG_NAME  : "CaiDat",                   // Bảng thông số & trạng thái tức thời
  TIMEZONE           : "Asia/Ho_Chi_Minh",         // Múi giờ Việt Nam GMT+7
  OFFLINE_TIMEOUT_SEC: 150                         // Quá 2.5 phút không có ping -> Báo cúp điện/offline
};

function doGet(e) {
  if (e && e.parameter && e.parameter.action) {
    return handleApiGet(e.parameter);
  }

  setupSpreadsheet();

  const template = HtmlService.createTemplateFromFile('Index');
  return template.evaluate()
    .setTitle('Điều Khiển & Giám Sát Cửa Cuốn - Hòa Bình')
    .addMetaTag('viewport', 'width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no')
    .setXFrameOptionsMode(HtmlService.XFrameOptionsMode.ALLOWALL);
}

function doPost(e) {
  try {
    if (!e || !e.postData || !e.postData.contents) {
      return jsonResponse({ status: "error", message: "No data payload" }, 400);
    }

    const data = JSON.parse(e.postData.contents);

    // Xác thực Token
    if (data.token !== CONFIG.WEBHOOK_TOKEN) {
      return jsonResponse({ status: "error", message: "Unauthorized Token" }, 401);
    }

    const type = data.type || "event"; // "ping", "boot", "event"

    // Lưu thông tin bo mạch phần cứng gửi từ ESP
    if (data.board) {
      let cfgSheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName(CONFIG.SHEET_CONFIG_NAME);
      if (cfgSheet) {
        setCfgValue(cfgSheet, "Loại Thiết Bị (Board)", data.board);
      }
    }

    // Kiểm tra và lấy lệnh chờ thực thi (Pending Command) gửi về cho ESP
    const pendingCmd = getAndClearPendingCommand();

    // Đồng bộ mã RF từ ESP gửi lên nếu có
    if (data.rfCodes && Array.isArray(data.rfCodes)) {
      syncRfCodesFromEsp(data.rfCodes);
    }

    if (type === "ping") {
      handleHeartbeat(data);
      return jsonResponse({ 
        status: "success", 
        message: "Pong", 
        pendingCommand: pendingCmd 
      }, 200);
    } 
    else if (type === "boot") {
      handleBootEvent(data);
      return jsonResponse({ 
        status: "success", 
        message: "Boot logged", 
        pendingCommand: pendingCmd 
      }, 200);
    } 
    else {
      const result = recordDoorEvent(data);
      return jsonResponse({ 
        status: "success", 
        data: result, 
        pendingCommand: pendingCmd 
      }, 200);
    }

  } catch (err) {
    return jsonResponse({ status: "error", message: err.toString() }, 500);
  }
}

/**
 * Xử lý khi ESP vừa khởi động lại (Phát hiện có điện trở lại)
 */
function handleBootEvent(data) {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return;

  const now = new Date();
  const nowStr = Utilities.formatDate(now, CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss");

  const lastSeenRaw = getCfgValue(cfgSheet, "LastSeen");
  let lastSeen = null;
  if (lastSeenRaw) {
    lastSeen = new Date(lastSeenRaw);
  }

  // Nếu thời gian mất điện kéo dài hơn ngưỡng OFFLINE_TIMEOUT_SEC -> Ghi log cúp điện
  if (lastSeen && (now.getTime() - lastSeen.getTime()) > (CONFIG.OFFLINE_TIMEOUT_SEC * 1000)) {
    const outageDurationSec = Math.floor((now.getTime() - lastSeen.getTime()) / 1000);
    recordOutageLog(lastSeen, now, outageDurationSec, "Cúp điện / Mất nguồn (Có điện trở lại)");
  }

  setCfgValue(cfgSheet, "Trạng Thái Thiết Bị", "ONLINE (Vừa khởi động)");
  setCfgValue(cfgSheet, "LastSeen", nowStr);
  setCfgValue(cfgSheet, "IsOutageActive", "NO");
}

/**
 * Xử lý gói tin Heartbeat gửi định kỳ
 */
function handleHeartbeat(data) {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return;

  const now = new Date();
  const nowStr = Utilities.formatDate(now, CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss");

  const isOutageActive = getCfgValue(cfgSheet, "IsOutageActive") === "YES";
  const outageStartRaw = getCfgValue(cfgSheet, "OutageStartTime");

  if (isOutageActive && outageStartRaw) {
    const outageStart = new Date(outageStartRaw);
    const durationSec = Math.floor((now.getTime() - outageStart.getTime()) / 1000);
    recordOutageLog(outageStart, now, durationSec, "Phục hồi kết nối thành công");
    setCfgValue(cfgSheet, "IsOutageActive", "NO");
    setCfgValue(cfgSheet, "OutageStartTime", "");
  }

  setCfgValue(cfgSheet, "Trạng Thái Thiết Bị", "ONLINE");
  setCfgValue(cfgSheet, "LastSeen", nowStr);
}

/**
 * Ghi 1 bản ghi vào Bảng Nhật Ký Cúp Điện
 */
function recordOutageLog(startTime, endTime, durationSec, reason) {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let outageSheet = ss.getSheetByName(CONFIG.SHEET_OUTAGE_NAME);
  if (!outageSheet) {
    setupSpreadsheet();
    outageSheet = ss.getSheetByName(CONFIG.SHEET_OUTAGE_NAME);
  }

  const startStr = Utilities.formatDate(startTime, CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss");
  const endStr   = Utilities.formatDate(endTime, CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss");
  const durationText = formatSeconds(durationSec);

  outageSheet.insertRowBefore(2);
  outageSheet.getRange(2, 1, 1, 4).setValues([[
    startStr,
    endStr,
    durationText,
    reason || "Mất nguồn"
  ]]);
}

/**
 * Ghi nhận sự kiện đóng/mở cửa vào Sheet NhatKyCuaCuon
 */
function recordDoorEvent(data) {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let sheet = ss.getSheetByName(CONFIG.SHEET_LOGS_NAME);
  if (!sheet) {
    setupSpreadsheet();
    sheet = ss.getSheetByName(CONFIG.SHEET_LOGS_NAME);
  }

  const now = new Date();
  const timeStr = Utilities.formatDate(now, CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss");

  const eventType    = data.event || "UNKNOWN";
  const doorState    = data.doorState || "UNKNOWN";
  const openDuration = data.openDurationSeconds ? formatSeconds(data.openDurationSeconds) : "---";
  const source       = data.source || "Remote RF";
  const note         = data.note || "";

  sheet.insertRowBefore(2);
  sheet.getRange(2, 1, 1, 6).setValues([[
    timeStr,
    eventType,
    doorState,
    openDuration,
    source,
    note
  ]]);

  let cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (cfgSheet) {
    setCfgValue(cfgSheet, "Trạng Thái Thiết Bị", "ONLINE");
    setCfgValue(cfgSheet, "LastSeen", timeStr);
    setCfgValue(cfgSheet, "Trạng Thái Cửa", doorState);
  }

  return { success: true, timestamp: timeStr };
}

/**
 * Tự động tạo cấu trúc các Sheet nếu chưa có
 */
function setupSpreadsheet() {
  const ss = SpreadsheetApp.getActiveSpreadsheet();

  // 1. Tạo Sheet Nhật ký đóng mở cửa
  let logSheet = ss.getSheetByName(CONFIG.SHEET_LOGS_NAME);
  if (!logSheet) {
    logSheet = ss.insertSheet(CONFIG.SHEET_LOGS_NAME);
    const headers = [["Thời Gian", "Sự Kiện", "Trạng Thái Cửa", "Thời Gian Mở", "Nguồn Kích Hoạt", "Ghi Chú"]];
    logSheet.getRange(1, 1, 1, 6).setValues(headers)
      .setBackground("#1E293B").setFontColor("#FFFFFF").setFontWeight("bold").setHorizontalAlignment("center");
    logSheet.setFrozenRows(1);
    logSheet.setColumnWidth(1, 160);
    logSheet.setColumnWidth(2, 110);
    logSheet.setColumnWidth(3, 140);
    logSheet.setColumnWidth(4, 150);
    logSheet.setColumnWidth(5, 130);
    logSheet.setColumnWidth(6, 200);
  }

  // 2. Tạo Sheet Nhật ký cúp điện
  let outageSheet = ss.getSheetByName(CONFIG.SHEET_OUTAGE_NAME);
  if (!outageSheet) {
    outageSheet = ss.insertSheet(CONFIG.SHEET_OUTAGE_NAME);
    const headers = [["Thời Điểm Bắt Đầu Mất Điện", "Thời Điểm Có Điện Lại", "Tổng Thời Gian Mất Điện", "Nguyên Nhân / Ghi Chú"]];
    outageSheet.getRange(1, 1, 1, 4).setValues(headers)
      .setBackground("#7F1D1D").setFontColor("#FFFFFF").setFontWeight("bold").setHorizontalAlignment("center");
    outageSheet.setFrozenRows(1);
    outageSheet.setColumnWidth(1, 190);
    outageSheet.setColumnWidth(2, 190);
    outageSheet.setColumnWidth(3, 180);
    outageSheet.setColumnWidth(4, 250);
  }

  // 3. Tạo Sheet Cài đặt, Mã RF & Trạng thái tức thời
  let cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) {
    cfgSheet = ss.insertSheet(CONFIG.SHEET_CONFIG_NAME);
    const cfgData = [
      ["THÔNG SỐ", "GIÁ TRỊ", "MÔ TẢ"],
      ["Password", CONFIG.DEFAULT_PASSWORD, "Mật khẩu Quản trị Master"],
      ["WebhookToken", CONFIG.WEBHOOK_TOKEN, "Token bảo mật ESP gửi lên"],
      ["Loại Thiết Bị (Board)", "ESP8266 (NodeMCU)", "Dòng vi điều khiển phần cứng đang chạy"],
      ["Trạng Thái Thiết Bị", "ONLINE", "ONLINE / OFFLINE (Cúp điện)"],
      ["LastSeen", Utilities.formatDate(new Date(), CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss"), "Thời điểm cuối nhận tin"],
      ["Trạng Thái Cửa", "ĐÓNG KÍN", "Trạng thái cửa cuốn"],
      ["IsOutageActive", "NO", "Đang trong tình trạng cúp điện (YES/NO)"],
      ["OutageStartTime", "", "Thời điểm bắt đầu ghi nhận mất điện"],
      ["TravelTimeSec", "25", "Thời gian hành trình cửa (giây)"],
      ["PendingCommand", "", "Lệnh chờ ESP thực thi từ xa (up, down, stop, lock, learn_...)"],
      ["Code_UP", "0x000000|0|24|1|350", "Mã RF Nút LÊN (Hex|Dec|Bits|Proto|Pulse)"],
      ["Code_DOWN", "0x000000|0|24|1|350", "Mã RF Nút XUỐNG (Hex|Dec|Bits|Proto|Pulse)"],
      ["Code_STOP", "0x000000|0|24|1|350", "Mã RF Nút DỪNG (Hex|Dec|Bits|Proto|Pulse)"],
      ["Code_LOCK", "0x000000|0|24|1|350", "Mã RF Nút KHÓA (Hex|Dec|Bits|Proto|Pulse)"],
      ["ExportString", "", "Chuỗi 4 mã RF sao chép (Export/Import)"]
    ];
    cfgSheet.getRange(1, 1, cfgData.length, 3).setValues(cfgData);
    cfgSheet.getRange("A1:C1").setBackground("#334155").setFontColor("#FFFFFF").setFontWeight("bold");
    cfgSheet.setColumnWidth(1, 180);
    cfgSheet.setColumnWidth(2, 260);
    cfgSheet.setColumnWidth(3, 280);
  }
}

/**
 * Helper đọc giá trị từ Sheet CaiDat theo tên thông số
 */
function getCfgValue(sheet, keyName) {
  if (!sheet) return "";
  const data = sheet.getRange("A2:B20").getValues();
  for (let i = 0; i < data.length; i++) {
    if (data[i][0] === keyName) {
      return data[i][1];
    }
  }
  return "";
}

/**
 * Helper ghi giá trị vào Sheet CaiDat theo tên thông số
 */
function setCfgValue(sheet, keyName, value) {
  if (!sheet) return;
  const data = sheet.getRange("A2:B20").getValues();
  for (let i = 0; i < data.length; i++) {
    if (data[i][0] === keyName) {
      sheet.getRange(i + 2, 2).setValue(value);
      return;
    }
  }
  // Nếu chưa có thì thêm dòng mới
  const nextRow = sheet.getLastRow() + 1;
  sheet.getRange(nextRow, 1).setValue(keyName);
  sheet.getRange(nextRow, 2).setValue(value);
}

/**
 * Lấy lệnh chờ và xóa sau khi đã gửi cho ESP
 */
function getAndClearPendingCommand() {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return "";

  const cmd = getCfgValue(cfgSheet, "PendingCommand");
  if (cmd) {
    setCfgValue(cfgSheet, "PendingCommand", "");
  }
  return cmd || "";
}

/**
 * Đồng bộ danh sách mã RF từ ESP gửi lên
 */
function syncRfCodesFromEsp(codes) {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  let cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return;

  const pendingCmd = (getCfgValue(cfgSheet, "PendingCommand") || "").toLowerCase();

  codes.forEach(c => {
    // CHỈ cập nhật vào Sheet nếu mã từ ESP là mã HỢP LỆ (code > 0, hex hợp lệ)
    // TUYỆT ĐỐI không ghi đè mã trống/chưa học của ESP lên mã đã lưu trên Google Sheet!
    const isCodeValid = (c.code && Number(c.code) > 0 && c.hex && c.hex !== "---" && c.hex !== "0x0" && c.hex !== "0x000000");

    if (isCodeValid) {
      let key = "Code_" + c.action.toUpperCase();
      // Không ghi đè nếu lệnh set_ của nút này đang chờ nạp xuống ESP
      if (!pendingCmd.includes("set_" + c.action.toLowerCase())) {
        let val = `${c.hex}|${c.code}|${c.bits}|${c.proto}|${c.pulse}`;
        setCfgValue(cfgSheet, key, val);
      }
    }
  });
}

/**
 * Trả dữ liệu tổng hợp về Web App Google Script
 */
function clientGetDashboardData(inputPass) {
  if (!clientVerifyPassword(inputPass)) {
    return { success: false, message: "Sai mật khẩu!" };
  }

  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const logSheet = ss.getSheetByName(CONFIG.SHEET_LOGS_NAME);
  const outageSheet = ss.getSheetByName(CONFIG.SHEET_OUTAGE_NAME);
  const cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);

  const now = new Date();
  let isOnline = false;
  let lastSeenStr = "---";
  let doorState = "ĐÓNG KÍN";
  let travelTime = 25;
  let pendingCmd = "";
  let boardType = "ESP8266 (NodeMCU)";

  if (cfgSheet) {
    boardType = getCfgValue(cfgSheet, "Loại Thiết Bị (Board)") || "ESP8266 (NodeMCU)";
    doorState = getCfgValue(cfgSheet, "Trạng Thái Cửa") || "ĐÓNG KÍN";
    travelTime = Number(getCfgValue(cfgSheet, "TravelTimeSec")) || 25;
    pendingCmd = getCfgValue(cfgSheet, "PendingCommand") || "";

    const lastSeenVal = getCfgValue(cfgSheet, "LastSeen");
    if (lastSeenVal) {
      lastSeenStr = Utilities.formatDate(new Date(lastSeenVal), CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss");
      const diffSec = (now.getTime() - new Date(lastSeenVal).getTime()) / 1000;
      
      if (diffSec <= CONFIG.OFFLINE_TIMEOUT_SEC) {
        isOnline = true;
      } else {
        isOnline = false;
        if (getCfgValue(cfgSheet, "IsOutageActive") !== "YES") {
          setCfgValue(cfgSheet, "IsOutageActive", "YES");
          setCfgValue(cfgSheet, "OutageStartTime", lastSeenVal);
        }
      }
    }
  }

  // Đọc danh sách 4 mã RF từ Sheet
  const rfCodes = [
    parseRfCodeString("up", "▲ LÊN (MỞ)", getCfgValue(cfgSheet, "Code_UP")),
    parseRfCodeString("down", "▼ XUỐNG (ĐÓNG)", getCfgValue(cfgSheet, "Code_DOWN")),
    parseRfCodeString("stop", "■ TẠM DỪNG", getCfgValue(cfgSheet, "Code_STOP")),
    parseRfCodeString("lock", "🔒 KHÓA HỘP", getCfgValue(cfgSheet, "Code_LOCK"))
  ];

  // Tạo chuỗi export
  const exportStr = `${rfCodes[0].hex} ${rfCodes[1].hex} ${rfCodes[2].hex} ${rfCodes[3].hex} ${rfCodes[0].bits} ${rfCodes[0].proto} ${rfCodes[0].pulse}`;

  // Lấy nhật ký cửa (tối đa 50 dòng)
  let doorLogs = [];
  if (logSheet && logSheet.getLastRow() > 1) {
    const numRows = Math.min(logSheet.getLastRow() - 1, 50);
    const raw = logSheet.getRange(2, 1, numRows, 6).getValues();
    doorLogs = raw.map(r => ({
      time: r[0] instanceof Date ? Utilities.formatDate(r[0], CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss") : r[0],
      event: r[1],
      state: r[2],
      duration: r[3],
      source: r[4],
      note: r[5]
    }));
  }

  // Lấy nhật ký cúp điện (tối đa 30 dòng)
  let outageLogs = [];
  if (outageSheet && outageSheet.getLastRow() > 1) {
    const numRows = Math.min(outageSheet.getLastRow() - 1, 30);
    const raw = outageSheet.getRange(2, 1, numRows, 4).getValues();
    outageLogs = raw.map(r => ({
      start: r[0] instanceof Date ? Utilities.formatDate(r[0], CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss") : r[0],
      end: r[1] instanceof Date ? Utilities.formatDate(r[1], CONFIG.TIMEZONE, "yyyy-MM-dd HH:mm:ss") : r[1],
      duration: r[2],
      reason: r[3]
    }));
  }

  return {
    success: true,
    device: {
      isOnline: isOnline,
      lastSeen: lastSeenStr,
      doorState: doorState,
      travelTime: travelTime,
      pendingCommand: pendingCmd,
      board: boardType
    },
    rfCodes: rfCodes,
    exportString: exportStr,
    doorLogs: doorLogs,
    outageLogs: outageLogs
  };
}

function parseRfCodeString(action, name, str) {
  if (!str || typeof str !== "string") {
    return { action: action, name: name, hex: "---", code: 0, bits: 24, proto: 1, pulse: 350, isValid: false };
  }
  const parts = str.split("|");
  const hex = parts[0] || "---";
  const code = Number(parts[1]) || 0;
  const bits = Number(parts[2]) || 24;
  const proto = Number(parts[3]) || 1;
  const pulse = Number(parts[4]) || 350;
  const isValid = (hex !== "---" && hex !== "0x000000" && code > 0);

  return { action, name, hex, code, bits, proto, pulse, isValid };
}

/**
 * Gửi lệnh điều khiển hoặc yêu cầu học lệnh xuống ESP từ xa
 */
function clientSendAction(pass, action) {
  if (!clientVerifyPassword(pass)) {
    return { success: false, message: "Sai mật khẩu!" };
  }

  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return { success: false, message: "Lỗi sheet" };

  setCfgValue(cfgSheet, "PendingCommand", action);
  return { 
    success: true, 
    message: `Đã xếp hàng lệnh '${action}'. ESP sẽ thực thi ngay trong lần kết nối gần nhất!` 
  };
}

/**
 * Lưu mã RF của một nút từ Web App
 */
function clientSaveRfCode(pass, action, hex, bits, proto, pulse) {
  if (!clientVerifyPassword(pass)) {
    return { success: false, message: "Sai mật khẩu!" };
  }

  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return { success: false, message: "Lỗi sheet" };

  let codeDec = parseInt(hex, 16) || 0;
  let val = `${hex}|${codeDec}|${bits}|${proto}|${pulse}`;
  setCfgValue(cfgSheet, "Code_" + action.toUpperCase(), val);

  // Đặt lệnh pending để ESP đồng bộ mã mới
  setCfgValue(cfgSheet, "PendingCommand", `set_${action}_${hex}_${bits}_${proto}_${pulse}`);

  return { success: true, message: `Đã lưu mã nút ${action.toUpperCase()} thành công!` };
}

/**
 * Nhập chuỗi 4 mã RF từ giao diện Web App
 */
function clientImportCodes(pass, importStr) {
  if (!clientVerifyPassword(pass)) {
    return { success: false, message: "Sai mật khẩu!" };
  }

  const parts = importStr.trim().split(/\s+/);
  if (parts.length < 4) {
    return { success: false, message: "Chuỗi mã không hợp lệ! Cần ít nhất 4 mã Hex." };
  }

  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return { success: false, message: "Lỗi sheet" };

  const bits = parts[4] || 24;
  const proto = parts[5] || 1;
  const pulse = parts[6] || 350;

  const actions = ["UP", "DOWN", "STOP", "LOCK"];
  for (let i = 0; i < 4; i++) {
    let hex = parts[i];
    let dec = parseInt(hex, 16) || 0;
    setCfgValue(cfgSheet, "Code_" + actions[i], `${hex}|${dec}|${bits}|${proto}|${pulse}`);
  }

  setCfgValue(cfgSheet, "PendingCommand", `import_${importStr.trim()}`);
  return { success: true, message: "Đã nạp thành công 4 mã RF vào hệ thống!" };
}

/**
 * Lưu cài đặt hệ thống (Travel Time & Password)
 */
function clientSaveSettings(pass, newPass, travelTimeSec) {
  if (!clientVerifyPassword(pass)) {
    return { success: false, message: "Sai mật khẩu hiện tại!" };
  }

  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (!cfgSheet) return { success: false, message: "Lỗi sheet" };

  if (newPass && newPass.length >= 4) {
    setCfgValue(cfgSheet, "Password", newPass);
  }
  if (travelTimeSec && Number(travelTimeSec) >= 5) {
    setCfgValue(cfgSheet, "TravelTimeSec", Number(travelTimeSec));
    setCfgValue(cfgSheet, "PendingCommand", `travel_${travelTimeSec}`);
  }

  return { success: true, message: "Đã cập nhật cài đặt thành công!" };
}

function clientVerifyPassword(inputPass) {
  const currentPass = getSavedPassword();
  return (inputPass === currentPass);
}

function getSavedPassword() {
  const ss = SpreadsheetApp.getActiveSpreadsheet();
  const cfgSheet = ss.getSheetByName(CONFIG.SHEET_CONFIG_NAME);
  if (cfgSheet) {
    const val = getCfgValue(cfgSheet, "Password");
    if (val) return val.toString().trim();
  }
  return CONFIG.DEFAULT_PASSWORD;
}

function formatSeconds(sec) {
  sec = Number(sec);
  if (isNaN(sec) || sec <= 0) return "0 giây";
  const h = Math.floor(sec / 3600);
  const m = Math.floor((sec % 3600) / 60);
  const s = Math.floor(sec % 60);

  if (h > 0) return `${h} giờ ${m} phút ${s} giây`;
  if (m > 0) return `${m} phút ${s} giây`;
  return `${s} giây`;
}

function jsonResponse(obj, code) {
  return ContentService.createTextOutput(JSON.stringify(obj))
    .setMimeType(ContentService.MimeType.JSON);
}

function handleApiGet(param) {
  if (param.action === "ping") {
    return jsonResponse({ status: "online", timestamp: new Date() }, 200);
  }
  return jsonResponse({ error: "Invalid action" }, 400);
}
