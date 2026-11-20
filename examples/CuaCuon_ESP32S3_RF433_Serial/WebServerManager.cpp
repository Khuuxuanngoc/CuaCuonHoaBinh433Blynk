#include "WebServerManager.h"
#include "MqttManager.h"

// ============================================================================
// HTML WEB APP NỘI BỘ TRÊN ESP (LƯU TRONG PROGMEM FLASH)
// ============================================================================
static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Cửa Cuốn Hòa Bình - Quản Trị Hệ Thống</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background-color: #0b0f19; color: #f1f5f9; display: flex; justify-content: center; padding: 12px; min-height: 100vh; }
    .app { width: 100%; max-width: 440px; display: flex; flex-direction: column; gap: 14px; }
    
    /* Header */
    .header { display: flex; justify-content: space-between; align-items: center; padding-bottom: 10px; border-bottom: 1px solid #1e293b; }
    .title { font-size: 16px; font-weight: 700; color: #fff; }
    .header-right { display: flex; align-items: center; gap: 8px; }
    .badge-ip { font-size: 11px; color: #38bdf8; background: rgba(56,189,248,0.1); padding: 3px 7px; border-radius: 6px; border: 1px solid rgba(56,189,248,0.2); }
    .badge-board { font-size: 10px; font-weight: 800; color: #c084fc; background: rgba(168,85,247,0.15); border: 1px solid rgba(168,85,247,0.3); padding: 2px 7px; border-radius: 6px; text-transform: uppercase; }
    .btn-logout { background: transparent; border: 1px solid #475569; color: #94a3b8; font-size: 11px; padding: 3px 8px; border-radius: 6px; cursor: pointer; }
    
    /* Navigation Tabs */
    .tabs { display: flex; background: #131b2e; border-radius: 12px; padding: 4px; border: 1px solid #1e293b; gap: 4px; }
    .tab-btn { flex: 1; padding: 8px 2px; font-size: 11px; font-weight: 700; color: #94a3b8; background: transparent; border: none; border-radius: 8px; cursor: pointer; transition: all 0.2s; text-align: center; }
    .tab-btn.active { background: #3b82f6; color: #fff; }
    
    /* Cards */
    .card { background: #131b2e; border: 1px solid #1e293b; border-radius: 18px; padding: 16px; text-align: center; box-shadow: 0 8px 20px -4px rgba(0,0,0,0.4); }
    .status-badge { display: inline-block; font-size: 11px; font-weight: 800; padding: 4px 12px; border-radius: 999px; margin-bottom: 10px; text-transform: uppercase; }
    .status-open { background: rgba(16,185,129,0.15); color: #34d399; border: 1px solid rgba(16,185,129,0.3); }
    .status-close { background: rgba(59,130,246,0.15); color: #60a5fa; border: 1px solid rgba(59,130,246,0.3); }
    .status-stop { background: rgba(245,158,11,0.15); color: #fbbf24; border: 1px solid rgba(245,158,11,0.3); }
    .status-lock { background: rgba(239,68,68,0.15); color: #f87171; border: 1px solid rgba(239,68,68,0.3); }
    .status-text { font-size: 24px; font-weight: 900; margin-bottom: 4px; }
    .status-sub { font-size: 11px; color: #94a3b8; }
    .progress-bar-bg { width: 100%; height: 8px; background: #0f172a; border-radius: 99px; margin: 14px 0; overflow: hidden; }
    .progress-bar-fill { height: 100%; width: 0%; background: #3b82f6; transition: width 0.3s; }
    
    /* Control Buttons */
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; }
    .btn-ctrl { display: flex; flex-direction: column; align-items: center; justify-content: center; padding: 16px 10px; border-radius: 14px; border: none; font-size: 13px; font-weight: 800; cursor: pointer; color: #fff; transition: all 0.15s; }
    .btn-ctrl:active { transform: scale(0.96); }
    .btn-up { background: #10b981; }
    .btn-down { background: #3b82f6; }
    .btn-stop { background: #f59e0b; }
    .btn-lock { background: #475569; }
    
    /* Forms & Inputs */
    .form-group { text-align: left; margin-bottom: 12px; }
    .form-label { display: block; font-size: 11px; font-weight: 700; color: #94a3b8; margin-bottom: 4px; text-transform: uppercase; }
    .form-input { width: 100%; background: #0f172a; border: 1px solid #334155; border-radius: 10px; padding: 10px 12px; color: #f8fafc; font-size: 13px; outline: none; }
    .form-input:focus { border-color: #3b82f6; }
    .btn-action { width: 100%; background: #3b82f6; color: #fff; border: none; border-radius: 10px; padding: 11px; font-size: 13px; font-weight: 700; cursor: pointer; margin-top: 6px; }
    .btn-action:active { transform: scale(0.98); }
    .btn-secondary { background: #1e293b; color: #38bdf8; border: 1px solid #334155; }
    .btn-sm { padding: 6px 10px; font-size: 11px; border-radius: 8px; border: none; cursor: pointer; font-weight: 700; }
    
    /* RF Code Card */
    .rf-card { background: #0f172a; border: 1px solid #1e293b; border-radius: 14px; padding: 12px; text-align: left; margin-bottom: 10px; }
    .rf-card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 6px; }
    .rf-name { font-size: 13px; font-weight: 800; color: #f1f5f9; display: flex; align-items: center; gap: 6px; }
    .rf-badge { font-size: 10px; font-weight: 700; padding: 2px 8px; border-radius: 6px; }
    .rf-badge-learned { background: rgba(16,185,129,0.15); color: #34d399; border: 1px solid rgba(16,185,129,0.3); }
    .rf-badge-empty { background: rgba(239,68,68,0.15); color: #f87171; border: 1px solid rgba(239,68,68,0.3); }
    .rf-details { font-size: 11px; color: #94a3b8; font-family: monospace; margin-bottom: 10px; line-height: 1.5; }
    .rf-actions { display: flex; gap: 6px; flex-wrap: wrap; }
    .btn-learn { background: #10b981; color: #fff; }
    .btn-test { background: #3b82f6; color: #fff; }
    .btn-edit { background: #334155; color: #f1f5f9; }
    .btn-del { background: #dc2626; color: #fff; }
    
    /* WiFi List Scan */
    .wifi-list { max-height: 150px; overflow-y: auto; background: #0f172a; border-radius: 10px; border: 1px solid #1e293b; margin: 8px 0; text-align: left; }
    .wifi-item { padding: 8px 12px; font-size: 12px; border-bottom: 1px solid #1e293b; display: flex; justify-content: space-between; cursor: pointer; }
    .wifi-item:hover { background: #1e293b; }
    .wifi-item:last-child { border-bottom: none; }
    
    /* Modal Learning Overlay */
    .modal-backdrop { position: fixed; inset: 0; background: rgba(0,0,0,0.8); z-index: 999; display: none; align-items: center; justify-content: center; padding: 16px; }
    .modal-box { background: #131b2e; border: 1px solid #334155; border-radius: 20px; padding: 24px; width: 100%; max-width: 360px; text-align: center; }
    .radar-icon { font-size: 48px; margin: 10px 0; display: inline-block; animation: pulseRadar 1.5s infinite; }
    @keyframes pulseRadar { 0% { transform: scale(0.9); opacity: 0.7; } 50% { transform: scale(1.15); opacity: 1; } 100% { transform: scale(0.9); opacity: 0.7; } }
    
    /* Login Screen */
    .login-overlay { display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 70vh; }
    .msg-box { font-size: 12px; padding: 8px 12px; border-radius: 8px; margin-top: 10px; display: none; }
    .msg-success { background: rgba(16,185,129,0.15); color: #34d399; border: 1px solid rgba(16,185,129,0.3); }
    .msg-error { background: rgba(239,68,68,0.15); color: #f87171; border: 1px solid rgba(239,68,68,0.3); }
    
    /* Links */
    .footer-links { text-align: center; margin-top: 10px; font-size: 11px; display: flex; justify-content: center; gap: 14px; }
    .footer-links a { color: #64748b; text-decoration: none; cursor: pointer; }
    .footer-links a:hover { color: #94a3b8; }
  </style>
</head>
<body>
  <div class="app">
    <!-- MÀN HÌNH ĐĂNG NHẬP -->
    <div id="loginView" class="login-overlay">
      <div class="card" style="width:100%">
        <div style="font-size:36px;margin-bottom:8px">🔒</div>
        <div class="title" style="font-size:18px;margin-bottom:4px">Cửa Cuốn Hòa Bình</div>
        <div style="font-size:12px;color:#94a3b8;margin-bottom:18px">Nhập Master Password để mở khóa</div>
        
        <div class="form-group">
          <input type="password" id="loginPass" class="form-input" placeholder="Mật khẩu Quản trị..." onkeydown="if(event.key==='Enter')doLogin()">
        </div>
        <button id="btnLogin" onclick="doLogin()" class="btn-action">🔓 ĐĂNG NHẬP</button>
        <div id="loginMsg" class="msg-box msg-error"></div>
        <div style="font-size:11px;color:#94a3b8;margin-top:14px;line-height:1.4;text-align:left;background:#0f172a;padding:8px 10px;border-radius:8px;border:1px solid #1e293b">
          💡 <b>Mẹo kết nối AP:</b> Nếu truy cập bằng điện thoại, hãy <b>tắt 4G/Dữ liệu di động</b> để điện thoại không tự ý chuyển mạng khi WiFi AP không có Internet.
        </div>
      </div>
    </div>

    <!-- MÀN HÌNH CHÍNH (KHI ĐÃ ĐĂNG NHẬP) -->
    <div id="mainView" style="display:none;flex-direction:column;gap:14px">
      <div class="header">
        <div>
          <div class="title">Cửa Cuốn Hòa Bình</div>
          <div style="font-size:11px;color:#64748b;display:flex;align-items:center;gap:6px">
            <span>ESP WebServer</span>
            <span id="boardBadge" class="badge-board">ESP...</span>
          </div>
        </div>
        <div class="header-right">
          <div id="ipBadge" class="badge-ip">192.168.4.1</div>
          <button onclick="doLogout()" class="btn-logout">Đăng xuất</button>
        </div>
      </div>

      <!-- Navigation Tabs -->
      <div class="tabs">
        <button class="tab-btn active" onclick="switchTab(0)">🎮 ĐIỀU KHIỂN</button>
        <button class="tab-btn" onclick="switchTab(1)">📻 HỌC LỆNH RF</button>
        <button class="tab-btn" onclick="switchTab(2)">📶 WIFI & CLOUD</button>
        <button class="tab-btn" onclick="switchTab(3)">🔒 ĐỔI PASS</button>
      </div>

      <!-- TAB 0: ĐIỀU KHIỂN -->
      <div id="tabCtrl" style="display:flex;flex-direction:column;gap:12px">
        <div class="card">
          <div id="badge" class="status-badge status-close">Đang tải...</div>
          <div id="stateText" class="status-text">ĐANG KẾT NỐI...</div>
          <div id="subText" class="status-sub">Vui lòng chờ...</div>
          <div class="progress-bar-bg">
            <div id="pBar" class="progress-bar-fill"></div>
          </div>
          <div id="timerText" style="font-size:12px;color:#fbbf24;font-weight:700"></div>
          <div id="wifiStatus" style="font-size:11px;color:#64748b;margin-top:8px"></div>
        </div>

        <div class="grid">
          <button onclick="sendAction('up')" class="btn-ctrl btn-up">▲ LÊN (MỞ)</button>
          <button onclick="sendAction('down')" class="btn-ctrl btn-down">▼ XUỐNG (ĐÓNG)</button>
          <button onclick="sendAction('stop')" class="btn-ctrl btn-stop">■ TẠM DỪNG</button>
          <button onclick="sendAction('lock')" class="btn-ctrl btn-lock">🔒 KHÓA HỘP</button>
        </div>
      </div>

      <!-- TAB 1: HỌC LỆNH & CÀI ĐẶT MÃ NÚT RF -->
      <div id="tabRf" style="display:none" class="card">
        <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:12px">
          <div style="font-size:15px;font-weight:700;text-align:left">📻 Cài Đặt Mã Remote RF 433MHz</div>
          <button onclick="loadRfList()" class="btn-sm btn-secondary">🔄 Tải lại</button>
        </div>

        <!-- Danh sách 4 nút -->
        <div id="rfCardsContainer">
          <div style="color:#94a3b8;font-size:12px;padding:16px">Đang nạp danh sách mã RF...</div>
        </div>

        <!-- Tiện ích Xuất / Nhập Mã -->
        <div style="margin-top:14px;padding-top:12px;border-top:1px solid #1e293b;text-align:left">
          <div style="font-size:12px;font-weight:700;color:#38bdf8;margin-bottom:6px">📋 Xuất / Nhập Mã Chuyển Tiếp (Export / Import)</div>
          <p style="font-size:11px;color:#94a3b8;margin-bottom:8px">Sao chép nhanh chuỗi mã giữa ESP32-S3 và ESP8266 mà không cần remote.</p>
          
          <div style="display:flex;gap:6px;margin-bottom:8px">
            <input type="text" id="importStr" class="form-input" style="font-size:11px;padding:8px" placeholder="Dán chuỗi 4 mã vào đây...">
            <button onclick="doImportCodes()" class="btn-sm btn-action" style="width:auto;margin:0;white-space:nowrap">📥 Nhập Mã</button>
          </div>
          <button onclick="doExportCodes()" class="btn-sm btn-secondary" style="width:100%">📋 Xem & Sao Chép Chuỗi Mã Hiện Tại</button>
        </div>
      </div>

      <!-- TAB 2: CẤU HÌNH WIFI & CLOUD -->
      <div id="tabWifi" style="display:none" class="card">
        <div style="font-size:15px;font-weight:700;margin-bottom:12px;text-align:left">📶 Cấu Hình WiFi & Dịch Vụ Cloud</div>
        
        <button onclick="scanWifi()" class="btn-action btn-secondary" style="margin-bottom:10px">🔍 Quét mạng WiFi xung quanh</button>
        <div id="wifiList" class="wifi-list" style="display:none"></div>

        <div class="form-group">
          <label class="form-label">Tên WiFi nhà (SSID)</label>
          <input type="text" id="staSSID" class="form-input" placeholder="Tên mạng WiFi gia đình">
        </div>
        <div class="form-group">
          <label class="form-label">Mật khẩu WiFi nhà</label>
          <input type="password" id="staPass" class="form-input" placeholder="Mật khẩu WiFi gia đình">
        </div>
        <div class="form-group">
          <label class="form-label">Google Apps Script Webhook URL</label>
          <input type="text" id="gasUrl" class="form-input" placeholder="https://script.google.com/macros/s/.../exec">
        </div>
        <div class="form-group">
          <label class="form-label">Thời gian hành trình cửa (giây)</label>
          <input type="number" id="travelSec" class="form-input" placeholder="25">
        </div>

        <!-- Khối Cấu hình MQTT Cloud -->
        <div style="background:#0f172a;border:1px solid #1e293b;border-radius:12px;padding:12px;margin:12px 0;text-align:left">
          <div style="display:flex;justify-content:space-between;align-items:center;margin-bottom:8px">
            <span style="font-size:13px;font-weight:700;color:#38bdf8">⚡ MQTT Cloud (Điều khiển tức thì &lt; 0.2s)</span>
            <label style="font-size:12px;color:#94a3b8;cursor:pointer">
              <input type="checkbox" id="mqttEnabled" style="vertical-align:middle;margin-right:4px"> Kích hoạt
            </label>
          </div>
          <p style="font-size:11px;color:#94a3b8;margin-bottom:10px">Hỗ trợ HiveMQ Cloud, EMQX, Adafruit IO... Giúp bấm nút qua 4G nhận ngay lập tức.</p>
          <div class="form-group">
            <label class="form-label">Broker Host (Server)</label>
            <input type="text" id="mqttServer" class="form-input" placeholder="vd: broker.emqx.io hoặc xxx.hivemq.cloud">
          </div>
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px">
            <div class="form-group">
              <label class="form-label">Cổng (1883 hoặc 8883 TLS)</label>
              <input type="number" id="mqttPort" class="form-input" placeholder="1883">
            </div>
            <div class="form-group">
              <label class="form-label">Tiền tố Topic</label>
              <input type="text" id="mqttTopic" class="form-input" placeholder="hoabinh/cuacuon">
            </div>
          </div>
          <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px">
            <div class="form-group">
              <label class="form-label">Username</label>
              <input type="text" id="mqttUser" class="form-input" placeholder="Để trống nếu không có">
            </div>
            <div class="form-group">
              <label class="form-label">Password</label>
              <input type="password" id="mqttPass" class="form-input" placeholder="Để trống nếu không có">
            </div>
          </div>
          <div id="mqttStatusText" style="font-size:11px;color:#64748b;margin-top:4px">Trạng thái: Chưa kích hoạt</div>
        </div>

        <button onclick="saveWifiConfig()" class="btn-action">💾 LƯU CẤU HÌNH & KẾT NỐI</button>
        <div id="wifiMsg" class="msg-box"></div>

        <!-- Khối Thông Tin Phần Cứng & Firmware -->
        <div style="background:#0f172a;border:1px solid #1e293b;border-radius:12px;padding:12px;margin-top:14px;text-align:left">
          <div style="font-size:12px;font-weight:700;color:#c084fc;margin-bottom:8px">📦 THÔNG TIN PHẦN CỨNG & OTA</div>
          <div style="font-size:11px;color:#94a3b8;display:grid;grid-template-columns:1fr 1fr;gap:6px">
            <div>Bo mạch: <b id="hwBoard" style="color:#38bdf8">---</b></div>
            <div>CPU: <b id="hwCpu" style="color:#f1f5f9">---</b></div>
            <div>Bộ nhớ Flash: <b id="hwFlash" style="color:#f1f5f9">---</b></div>
            <div>RAM trống: <b id="hwHeap" style="color:#f1f5f9">---</b></div>
          </div>
          <div style="margin-top:10px">
            <a href="/update" class="btn-sm btn-action" style="display:block;text-align:center;text-decoration:none;background:#7c3aed">⚡ Nạp Firmware Web OTA</a>
          </div>
        </div>
      </div>

      <!-- TAB 3: ĐỔI MẬT KHẨU -->
      <div id="tabSecurity" style="display:none" class="card">
        <div style="font-size:15px;font-weight:700;margin-bottom:8px;text-align:left">🔒 Quản Lý Mật Khẩu (2 Lớp)</div>
        <p style="font-size:11px;color:#94a3b8;margin-bottom:14px;text-align:left">Phân tách giữa mật khẩu phát sóng WiFi AP và mật khẩu đăng nhập Master.</p>

        <div class="form-group">
          <label class="form-label">1. Mật khẩu WiFi AP của ESP (≥ 8 ký tự)</label>
          <input type="text" id="newApPass" class="form-input" placeholder="Để trống nếu không đổi">
        </div>
        <div class="form-group">
          <label class="form-label">2. Mật khẩu Quản trị Master (Web & Cloud)</label>
          <input type="password" id="newMasterPass" class="form-input" placeholder="Để trống nếu không đổi">
        </div>

        <button onclick="saveSecurityConfig()" class="btn-action">💾 CẬP NHẬT MẬT KHẨU</button>
        <div id="secMsg" class="msg-box"></div>
      </div>

      <!-- Footer Links -->
      <div class="footer-links">
        <a href="/update">⚙ Cập nhật Firmware OTA</a>
        <span>•</span>
        <a onclick="rebootDevice()">🔄 Khởi động lại ESP</a>
      </div>
    </div>
  </div>

  <!-- ===================================================================== -->
  <!-- MODAL ĐANG HỌC LỆNH REMOTE RF (COUNTDOWN & ANIMATION) -->
  <!-- ===================================================================== -->
  <div id="learnModal" class="modal-backdrop">
    <div class="modal-box">
      <div id="learnTitle" style="font-size:16px;font-weight:800;color:#fff">HỌC LỆNH NÚT LÊN</div>
      <div class="radar-icon">📡</div>
      <div id="learnTimer" style="font-size:36px;font-weight:900;color:#38bdf8;margin:6px 0">15s</div>
      <p id="learnInstruction" style="font-size:12px;color:#94a3b8;margin-bottom:18px">Hướng Remote vào ăng-ten ESP và <b>BẤM GIỮ NÚT</b> trong 1-2 giây...</p>
      
      <div id="learnResultNotice" style="display:none;font-size:13px;font-weight:700;margin-bottom:14px"></div>
      <button onclick="cancelLearning()" id="btnCancelLearn" class="btn-action btn-secondary" style="margin:0">✖ HỦY BỎ</button>
    </div>
  </div>

  <script>
    let authToken = localStorage.getItem("master_pwd") || "";
    let pollInterval = null;
    let learnPollInterval = null;
    let currentLearningAction = "";

    function fetchWithTimeout(url, options = {}, timeoutMs = 4000) {
      if (!window.AbortController) return fetch(url, options);
      const controller = new AbortController();
      const id = setTimeout(() => controller.abort(), timeoutMs);
      options.signal = controller.signal;
      return fetch(url, options).finally(() => clearTimeout(id));
    }

    function init() {
      if (authToken) {
        verifyAuth(authToken, true);
      } else {
        showLogin();
      }
    }

    function showLogin() {
      document.getElementById('loginView').style.display = 'flex';
      document.getElementById('mainView').style.display = 'none';
      if (pollInterval) clearInterval(pollInterval);
    }

    function showMain() {
      document.getElementById('loginView').style.display = 'none';
      document.getElementById('mainView').style.display = 'flex';
      updateStatus();
      if (!pollInterval) pollInterval = setInterval(updateStatus, 1500);
    }

    function doLogin() {
      let pwd = document.getElementById('loginPass').value.trim();
      if (!pwd) return;
      verifyAuth(pwd, false);
    }

    function verifyAuth(pwd, isAuto = false) {
      let btn = document.getElementById('btnLogin');
      if (btn) { btn.disabled = true; btn.innerText = isAuto ? "Đang xác thực..." : "Đang kết nối..."; }

      let formData = new URLSearchParams();
      formData.append('pass', pwd);

      fetchWithTimeout('/api/login', { method: 'POST', body: formData }, 4000)
        .then(r => r.json())
        .then(res => {
          if (btn) { btn.disabled = false; btn.innerText = "🔓 ĐĂNG NHẬP"; }
          if (res.success) {
            authToken = pwd;
            localStorage.setItem("master_pwd", pwd);
            showMain();
          } else {
            showLogin();
            showMsg('loginMsg', res.message || 'Sai mật khẩu!', false);
          }
        })
        .catch(() => {
          if (btn) { btn.disabled = false; btn.innerText = "🔓 ĐĂNG NHẬP"; }
          showLogin();
          showMsg('loginMsg', 'Không nhận được phản hồi từ ESP. Vui lòng kiểm tra đã kết nối WiFi AP!', false);
        });
    }

    function doLogout() {
      localStorage.removeItem("master_pwd");
      authToken = "";
      location.reload();
    }

    function switchTab(idx) {
      const tabs = [
        document.getElementById('tabCtrl'), 
        document.getElementById('tabRf'), 
        document.getElementById('tabWifi'), 
        document.getElementById('tabSecurity')
      ];
      const btns = document.querySelectorAll('.tab-btn');
      tabs.forEach((t, i) => t.style.display = (i === idx) ? (i === 0 ? 'flex' : 'block') : 'none');
      btns.forEach((b, i) => b.className = (i === idx) ? 'tab-btn active' : 'tab-btn');

      if (idx === 1) {
        loadRfList();
      }
    }

    function updateStatus() {
      fetchWithTimeout('/api/status?pass=' + encodeURIComponent(authToken), {}, 3500)
        .then(r => {
          if (r.status === 401) { doLogout(); throw new Error("Unauthorized"); }
          return r.json();
        })
        .then(data => {
          document.getElementById('stateText').innerText = data.state;
          document.getElementById('subText').innerText = "Nguồn: " + data.source + " | Hành trình: " + data.travelTime + "s";
          document.getElementById('pBar').style.width = data.percentage + "%";
          
          let badge = document.getElementById('badge');
          if (data.stateCode === 1 || data.stateCode === 2) {
            badge.className = "status-badge status-open";
            badge.innerText = "ĐANG MỞ";
          } else if (data.stateCode === 0) {
            badge.className = "status-badge status-close";
            badge.innerText = "ĐÃ ĐÓNG KÍN";
          } else if (data.stateCode === 5) {
            badge.className = "status-badge status-lock";
            badge.innerText = "ĐÃ KHÓA";
          } else {
            badge.className = "status-badge status-stop";
            badge.innerText = data.state;
          }

          let timer = document.getElementById('timerText');
          if (data.openDuration > 0) {
            let m = Math.floor(data.openDuration / 60);
            let s = data.openDuration % 60;
            timer.innerText = "⏱ Đang mở: " + m + " phút " + s + " giây";
          } else {
            timer.innerText = "";
          }

          let wf = document.getElementById('wifiStatus');
          if (data.staConnected) {
            wf.innerHTML = "📶 WiFi nhà: <b style='color:#34d399'>" + data.staSSID + "</b> (" + data.staIP + ")";
            document.getElementById('ipBadge').innerText = data.staIP;
          } else {
            wf.innerHTML = "📶 WiFi nhà: <span style='color:#f87171'>Chưa kết nối</span> (AP: " + data.apIP + ")";
            document.getElementById('ipBadge').innerText = data.apIP;
          }

          if (!window.formLoaded) {
            if (data.staSSID) document.getElementById('staSSID').value = data.staSSID;
            if (data.gasUrl) document.getElementById('gasUrl').value = data.gasUrl;
            if (data.travelTime) document.getElementById('travelSec').value = data.travelTime;

            if (data.mqttServer) document.getElementById('mqttServer').value = data.mqttServer;
            if (data.mqttPort) document.getElementById('mqttPort').value = data.mqttPort;
            if (data.mqttTopic) document.getElementById('mqttTopic').value = data.mqttTopic;
            if (data.mqttUser) document.getElementById('mqttUser').value = data.mqttUser;
            document.getElementById('mqttEnabled').checked = !!data.mqttEnabled;
            window.formLoaded = true;
          }

          let mqText = document.getElementById('mqttStatusText');
          if (mqText) {
            if (!data.mqttEnabled) {
              mqText.innerHTML = "Trạng thái: <span style='color:#64748b'>Chưa kích hoạt</span>";
            } else if (data.mqttConnected) {
              mqText.innerHTML = "Trạng thái: <b style='color:#34d399'>🟢 Đã kết nối Broker (" + data.mqttServer + ")</b>";
            } else {
              mqText.innerHTML = "Trạng thái: <span style='color:#f87171'>🔴 Đang kết nối lại...</span>";
            }
          }

          if (data.board) {
            let bb = document.getElementById('boardBadge');
            if (bb) bb.innerText = data.board;
            let hb = document.getElementById('hwBoard');
            if (hb) hb.innerText = data.board;
            let hc = document.getElementById('hwCpu');
            if (hc) hc.innerText = data.cpu || '---';
            let hf = document.getElementById('hwFlash');
            if (hf) hf.innerText = (data.flashMB || 0) + ' MB';
            let hh = document.getElementById('hwHeap');
            if (hh) hh.innerText = Math.round((data.freeHeap || 0) / 1024) + ' KB';
          }
        })
        .catch(() => {});
    }

    function sendAction(act) {
      fetchWithTimeout(`/api/control?action=${act}&pass=${encodeURIComponent(authToken)}`, {}, 3500)
        .then(r => r.json())
        .then(res => {
          if (!res.success) {
            alert(res.message);
          } else {
            updateStatus();
          }
        })
        .catch(() => alert("Khong nhan duoc phan hoi tu ESP!"));
    }

    // =========================================================================
    // HỌC LỆNH & QUẢN LÝ MÃ RF
    // =========================================================================
    function loadRfList() {
      fetch('/api/rf/list?pass=' + encodeURIComponent(authToken))
        .then(r => r.json())
        .then(list => {
          let container = document.getElementById('rfCardsContainer');
          let html = "";
          list.forEach(item => {
            let isLearned = item.isValid;
            let badgeClass = isLearned ? "rf-badge rf-badge-learned" : "rf-badge rf-badge-empty";
            let badgeText = isLearned ? "ĐÃ HỌC" : "CHƯA HỌC";
            let hex = isLearned ? item.hex : "---";
            let dec = isLearned ? item.code : "---";

            html += `
              <div class="rf-card">
                <div class="rf-card-header">
                  <div class="rf-name">${item.name}</div>
                  <div class="${badgeClass}">${badgeText}</div>
                </div>
                <div class="rf-details">
                  Hex: <b style="color:#38bdf8">${hex}</b> | Dec: ${dec}<br>
                  Bit: ${item.bits} | Proto: ${item.proto} | Xung: ${item.pulse} us
                </div>
                <div class="rf-actions">
                  <button onclick="startLearning('${item.action}', '${item.name}')" class="btn-sm btn-learn">📡 Học Lệnh</button>
                  <button onclick="sendAction('${item.action}')" class="btn-sm btn-test" ${!isLearned ? 'disabled style="opacity:0.5"' : ''}>▶ Thử Phát</button>
                  <button onclick="editRfCode('${item.action}', '${item.hex}', ${item.bits}, ${item.proto}, ${item.pulse})" class="btn-sm btn-edit">✏ Sửa</button>
                  <button onclick="clearRfCode('${item.action}')" class="btn-sm btn-del" ${!isLearned ? 'disabled style="opacity:0.5"' : ''}>🗑 Xóa</button>
                </div>
              </div>
            `;
          });
          container.innerHTML = html;
        })
        .catch(() => {
          document.getElementById('rfCardsContainer').innerHTML = "<div style='color:#f87171;padding:12px'>Lỗi nạp danh sách mã RF</div>";
        });
    }

    function startLearning(action, name) {
      currentLearningAction = action;
      let modal = document.getElementById('learnModal');
      let title = document.getElementById('learnTitle');
      let timer = document.getElementById('learnTimer');
      let notice = document.getElementById('learnResultNotice');
      let btnCancel = document.getElementById('btnCancelLearn');

      title.innerText = "HỌC LỆNH: " + name.toUpperCase();
      timer.innerText = "15s";
      notice.style.display = 'none';
      btnCancel.style.display = 'inline-block';
      modal.style.display = 'flex';

      let params = new URLSearchParams();
      params.append('action', action);
      params.append('pass', authToken);

      fetch('/api/rf/learn', { method: 'POST', body: params })
        .then(r => r.json())
        .then(res => {
          if (!res.success) {
            alert(res.message);
            modal.style.display = 'none';
            return;
          }
          // Bắt đầu chu kỳ thăm dò kết quả học lệnh mỗi 400ms
          if (learnPollInterval) clearInterval(learnPollInterval);
          learnPollInterval = setInterval(pollLearnStatus, 400);
        })
        .catch(() => {
          modal.style.display = 'none';
          alert("Lỗi kết nối tới ESP khi bắt đầu học lệnh");
        });
    }

    function pollLearnStatus() {
      fetch('/api/rf/learn_status?pass=' + encodeURIComponent(authToken))
        .then(r => r.json())
        .then(st => {
          let timer = document.getElementById('learnTimer');
          let notice = document.getElementById('learnResultNotice');
          let btnCancel = document.getElementById('btnCancelLearn');

          if (st.isLearning) {
            let s = Math.ceil(st.remainingMs / 1000);
            timer.innerText = s + "s";
          } else {
            // Đã kết thúc phiên học lệnh
            clearInterval(learnPollInterval);
            learnPollInterval = null;
            btnCancel.style.display = 'none';

            if (st.success) {
              timer.innerText = "✓";
              timer.style.color = "#10b981";
              notice.style.display = 'block';
              notice.style.color = "#34d399";
              notice.innerHTML = `🎉 HỌC THÀNH CÔNG!<br>Mã Hex: <b>${st.codeHex}</b> (${st.bits} bit, xung ${st.pulse} us)`;
              setTimeout(() => {
                document.getElementById('learnModal').style.display = 'none';
                loadRfList();
              }, 1800);
            } else {
              timer.innerText = "✗";
              timer.style.color = "#f87171";
              notice.style.display = 'block';
              notice.style.color = "#f87171";
              notice.innerText = "⚠️ Hết thời gian chờ hoặc đã hủy bỏ!";
              setTimeout(() => {
                document.getElementById('learnModal').style.display = 'none';
              }, 2000);
            }
          }
        })
        .catch(() => {});
    }

    function cancelLearning() {
      let params = new URLSearchParams();
      params.append('pass', authToken);
      fetch('/api/rf/cancel', { method: 'POST', body: params });
      if (learnPollInterval) clearInterval(learnPollInterval);
      document.getElementById('learnModal').style.display = 'none';
    }

    function editRfCode(action, curHex, curBits, curProto, curPulse) {
      let hex = prompt(`Nhập mã Hex cho nút ${action.toUpperCase()} (VD: 0xA1B2C3):`, curHex !== '---' ? curHex : '0x');
      if (!hex || hex === '0x') return;

      let bits = prompt("Số Bit (thường là 24):", curBits || 24);
      if (!bits) return;

      let proto = prompt("Giao thức Protocol (thường là 1):", curProto || 1);
      if (!proto) return;

      let pulse = prompt("Độ rộng xung us (thường là 350):", curPulse || 350);
      if (!pulse) return;

      let params = new URLSearchParams();
      params.append('action', action);
      params.append('code', hex);
      params.append('bits', bits);
      params.append('proto', proto);
      params.append('pulse', pulse);
      params.append('pass', authToken);

      fetch('/api/rf/set', { method: 'POST', body: params })
        .then(r => r.json())
        .then(res => {
          alert(res.message);
          loadRfList();
        });
    }

    function clearRfCode(action) {
      if (!confirm(`Bạn có chắc muốn xóa mã RF của nút ${action.toUpperCase()}?`)) return;
      let params = new URLSearchParams();
      params.append('action', action);
      params.append('pass', authToken);

      fetch('/api/rf/clear', { method: 'POST', body: params })
        .then(r => r.json())
        .then(res => {
          alert(res.message);
          loadRfList();
        });
    }

    function doExportCodes() {
      fetch('/api/rf/export?pass=' + encodeURIComponent(authToken))
        .then(r => r.json())
        .then(res => {
          if (res.success) {
            document.getElementById('importStr').value = res.exportString;
            navigator.clipboard.writeText(res.exportString).then(() => {
              alert("Đã sao chép chuỗi mã vào Clipboard!\n\nChuỗi mã:\n" + res.exportString);
            }).catch(() => {
              alert("Chuỗi mã:\n" + res.exportString);
            });
          }
        });
    }

    function doImportCodes() {
      let str = document.getElementById('importStr').value.trim();
      if (!str) {
        alert("Vui lòng dán chuỗi mã cần nạp!");
        return;
      }
      if (!confirm("Bạn có chắc muốn nạp chuỗi 4 mã RF này vào ESP?")) return;

      let params = new URLSearchParams();
      params.append('string', str);
      params.append('pass', authToken);

      fetch('/api/rf/import', { method: 'POST', body: params })
        .then(r => r.json())
        .then(res => {
          alert(res.message);
          if (res.success) loadRfList();
        });
    }

    // =========================================================================
    // CẤU HÌNH WIFI & BẢO MẬT
    // =========================================================================
    function scanWifi() {
      let list = document.getElementById('wifiList');
      list.style.display = 'block';
      list.innerHTML = "<div style='padding:10px;text-align:center;color:#94a3b8'>Đang quét mạng WiFi...</div>";

      fetch('/api/wifi/scan?pass=' + encodeURIComponent(authToken))
        .then(r => r.json())
        .then(items => {
          if (!items || items.length === 0) {
            list.innerHTML = "<div style='padding:10px;text-align:center;color:#f87171'>Không tìm thấy WiFi nào</div>";
            return;
          }
          let html = "";
          items.forEach(it => {
            html += `<div class="wifi-item" onclick="selectWifi('${it.ssid}')"><span>${it.ssid}</span><span style="color:#64748b">${it.rssi} dBm</span></div>`;
          });
          list.innerHTML = html;
        })
        .catch(() => list.innerHTML = "<div style='padding:10px;text-align:center;color:#f87171'>Lỗi khi quét WiFi</div>");
    }

    function selectWifi(ssid) {
      document.getElementById('staSSID').value = ssid;
      document.getElementById('wifiList').style.display = 'none';
      document.getElementById('staPass').focus();
    }

    function saveWifiConfig() {
      let params = new URLSearchParams();
      params.append('pass', authToken);
      params.append('ssid', document.getElementById('staSSID').value.trim());
      params.append('wifi_pass', document.getElementById('staPass').value.trim());
      params.append('gas_url', document.getElementById('gasUrl').value.trim());
      params.append('travel_time', document.getElementById('travelSec').value.trim());

      params.append('mqtt_en', document.getElementById('mqttEnabled').checked ? '1' : '0');
      params.append('mqtt_server', document.getElementById('mqttServer').value.trim());
      params.append('mqtt_port', document.getElementById('mqttPort').value.trim());
      params.append('mqtt_user', document.getElementById('mqttUser').value.trim());
      params.append('mqtt_pass', document.getElementById('mqttPass').value.trim());
      params.append('mqtt_topic', document.getElementById('mqttTopic').value.trim());

      fetch('/api/config/wifi', { method: 'POST', body: params })
        .then(r => r.json())
        .then(res => {
          showMsg('wifiMsg', res.message, res.success);
          if (res.success) {
            window.formLoaded = false;
            updateStatus();
          }
        })
        .catch(() => showMsg('wifiMsg', 'Lỗi khi lưu cấu hình', false));
    }

    function saveSecurityConfig() {
      let ap = document.getElementById('newApPass').value.trim();
      let master = document.getElementById('newMasterPass').value.trim();

      if (!ap && !master) {
        showMsg('secMsg', 'Vui lòng nhập ít nhất một mật khẩu mới', false);
        return;
      }

      let params = new URLSearchParams();
      params.append('pass', authToken);
      params.append('new_ap_pass', ap);
      params.append('new_master_pass', master);

      fetch('/api/config/security', { method: 'POST', body: params })
        .then(r => r.json())
        .then(res => {
          if (res.success) {
            showMsg('secMsg', res.message, true);
            if (master) {
              authToken = master;
              localStorage.setItem("master_pwd", master);
            }
          } else {
            showMsg('secMsg', res.message, false);
          }
        })
        .catch(() => showMsg('secMsg', 'Lỗi khi đổi mật khẩu', false));
    }

    function rebootDevice() {
      if (!confirm("Bạn có chắc chắn muốn khởi động lại ESP không?")) return;
      let params = new URLSearchParams();
      params.append('pass', authToken);
      fetch('/api/reboot', { method: 'POST', body: params })
        .then(() => alert("ESP đang khởi động lại. Vui lòng kết nối lại sau 15 giây!"))
        .catch(() => {});
    }

    function showMsg(id, text, isSuccess) {
      let el = document.getElementById(id);
      el.innerText = text;
      el.className = 'msg-box ' + (isSuccess ? 'msg-success' : 'msg-error');
      el.style.display = 'block';
      setTimeout(() => { el.style.display = 'none'; }, 5000);
    }

    init();
  </script>
</body>
</html>
)rawliteral";

// Giao diện trang cập nhật Firmware OTA (Có bảo vệ bằng Master Password)
static const char OTA_HTML[] PROGMEM = R"rawliteral(
// Giao diện trang cập nhật Firmware OTA (Có hiển thị loại Board & Cảnh báo an toàn)
static const char OTA_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Cập Nhật Firmware Web OTA - Cửa Cuốn Hòa Bình</title>
  <style>
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; }
    body { background:#0b0f19; color:#f1f5f9; display:flex; justify-content:center; align-items:center; min-height:100vh; padding:16px; margin:0; }
    .box { background:#131b2e; border:1px solid #1e293b; border-radius:20px; padding:24px; width:100%; max-width:400px; text-align:center; box-shadow:0 10px 25px -5px rgba(0,0,0,0.5); }
    h2 { font-size:18px; margin-bottom:6px; color:#fff; }
    .sub { font-size:12px; color:#94a3b8; margin-bottom:16px; }
    .hw-card { background:#0f172a; border:1px solid #334155; border-radius:12px; padding:14px; text-align:left; margin-bottom:16px; }
    .hw-row { display:flex; justify-content:space-between; font-size:12px; margin-bottom:6px; }
    .hw-row:last-child { margin-bottom:0; }
    .hw-label { color:#94a3b8; }
    .hw-val { font-weight:700; color:#38bdf8; }
    .chip-badge { display:inline-block; font-size:13px; font-weight:800; background:rgba(168,85,247,0.15); color:#c084fc; border:1px solid rgba(168,85,247,0.3); padding:4px 10px; border-radius:8px; margin-bottom:8px; }
    .warn-box { background:rgba(239,68,68,0.12); border:1px solid rgba(239,68,68,0.3); border-radius:12px; padding:12px; text-align:left; font-size:11px; line-height:1.5; color:#fca5a5; margin-bottom:16px; }
    .warn-box b { color:#f87171; }
    .form-group { text-align:left; margin-bottom:12px; }
    .form-label { display:block; font-size:11px; font-weight:700; color:#94a3b8; margin-bottom:4px; text-transform:uppercase; }
    input[type="password"] { width:100%; box-sizing:border-box; padding:10px 12px; background:#0f172a; border:1px solid #334155; border-radius:10px; color:#fff; font-size:13px; }
    input[type="file"] { width:100%; box-sizing:border-box; padding:8px; background:#0f172a; border:1px dashed #475569; border-radius:10px; color:#cbd5e1; font-size:12px; cursor:pointer; }
    button { background:#3b82f6; color:#fff; border:none; padding:12px 20px; border-radius:10px; font-weight:700; width:100%; cursor:pointer; font-size:13px; margin-top:8px; transition:0.2s; }
    button:hover { background:#2563eb; }
    a { display:inline-block; margin-top:16px; font-size:12px; color:#64748b; text-decoration:none; }
    a:hover { color:#94a3b8; }
  </style>
</head>
<body>
  <div class="box">
    <div style="font-size:32px;margin-bottom:6px">⚡</div>
    <h2>Cập Nhật Firmware Web OTA</h2>
    <div class="sub">Nâng cấp chương trình điều khiển không dây qua WiFi</div>
    
    <div class="hw-card">
      <div style="text-align:center">
        <span class="chip-badge">🏷️ %BOARD_NAME%</span>
      </div>
      <div class="hw-row"><span class="hw-label">Kiến trúc CPU:</span><span class="hw-val">%CPU_ARCH%</span></div>
      <div class="hw-row"><span class="hw-label">Tần số hoạt động:</span><span class="hw-val">%CPU_MHZ% MHz</span></div>
      <div class="hw-row"><span class="hw-label">Bộ nhớ Flash:</span><span class="hw-val">%FLASH_MB% MB</span></div>
      <div class="hw-row"><span class="hw-label">Bộ nhớ trống OTA:</span><span class="hw-val">%FREE_OTA% KB</span></div>
    </div>
    
    <div class="warn-box">
      ⚠️ <b>CẢNH BÁO AN TOÀN QUAN TRỌNG:</b><br>
      Bo mạch đang chạy là <b>%BOARD_NAME%</b>.<br>
      Chỉ được chọn file <code>.bin</code> biên dịch cho đúng dòng chip <b>%BOARD_NAME%</b>.<br>
      <span style="color:#ef4444;font-weight:700">Tuyệt đối KHÔNG nạp file .bin của %OPPOSITE_BOARD%!</span>
    </div>
    
    <form method="POST" action="/update" enctype="multipart/form-data">
      <div class="form-group">
        <label class="form-label">Master Password</label>
        <input type="password" name="pass" placeholder="Nhập mật khẩu quản trị..." required>
      </div>
      <div class="form-group">
        <label class="form-label">File Firmware (.bin)</label>
        <input type="file" name="update" accept=".bin" required>
      </div>
      <button type="submit" onclick="return confirm('Bạn có chắc chắn file .bin này được biên dịch cho %BOARD_NAME% không?')">🚀 BẮT ĐẦU NẠP FIRMWARE</button>
    </form>
    <a href="/">← Quay lại Bảng Điều Khiển</a>
  </div>
</body>
</html>
)rawliteral";

WebServerManager::WebServerManager(RollingDoorController& controller, DoorMotionTracker& tracker, GoogleSheetClient& sheetClient)
    : _controller(controller), _tracker(tracker), _sheetClient(sheetClient), _mqttManager(nullptr), _server(80),
      _staState(STA_IDLE), _staConnectStartTime(0), _lastStaRetryTime(0),
      _otaSuccess(false), _otaAuthenticated(false) {
}

WebServerManager::~WebServerManager() {
}

bool WebServerManager::begin() {
    // 1. Tải cấu hình đã lưu từ Flash/EEPROM
    _controller.getStorage().loadSystemConfig(_config);

    // Cập nhật thời gian hành trình cho Tracker
    if (_config.travelTimeSec > 0) {
        _tracker.setTravelTimeSec(_config.travelTimeSec);
    }

    // 2. Cấu hình WiFi Access Point (AP Mode)
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_AP_SSID, _config.apPassword);

    // 3. Kết nối WiFi gia đình nếu có cấu hình
    if (strlen(_config.wifiSSID) > 0) {
        _staState = STA_CONNECTING;
        _staConnectStartTime = millis();
        _lastStaRetryTime = millis();
        WiFi.begin(_config.wifiSSID, _config.wifiPass);
        Serial.print(F("[WIFI] Dang ket noi vao WiFi nha: "));
        Serial.println(_config.wifiSSID);
    } else {
        _staState = STA_IDLE;
    }

    // 4. Thiết lập các tuyến đường HTTP (Routes)
    setupRoutes();

    // 5. Bắt đầu lắng nghe
    _server.begin();
    return true;
}

void WebServerManager::update() {
    _server.handleClient();
    handleStaConnection();
}

IPAddress WebServerManager::getAPIP() const {
    return WiFi.softAPIP();
}

IPAddress WebServerManager::getStaIP() const {
    return WiFi.localIP();
}

bool WebServerManager::isStaConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

void WebServerManager::setConfig(const SystemConfig& cfg) {
    _config = cfg;
}

void WebServerManager::handleStaConnection() {
    if (strlen(_config.wifiSSID) == 0) return;
    uint32_t now = millis();

    switch (_staState) {
        case STA_CONNECTING:
            if (WiFi.status() == WL_CONNECTED) {
                _staState = STA_CONNECTED;
                Serial.print(F("[WIFI] Da ket noi thanh cong WiFi nha: "));
                Serial.print(_config.wifiSSID);
                Serial.print(F(" (IP: "));
                Serial.print(WiFi.localIP());
                Serial.println(F(")"));
            } else if (now - _staConnectStartTime >= 10000) {
                // Neu qua 10s khong ket noi duoc den WiFi nha (vi du router cup dien hoac ngoai phu song),
                // ngat ket noi STA ngay lap tuc de giu kenh softAP co dinh, tranh packet loss gay treo Web AP.
                _staState = STA_DISCONNECTED_PAUSED;
                _lastStaRetryTime = now;
                WiFi.disconnect(false);
                Serial.println(F("[WIFI] Khong tim thay WiFi nha sau 10s. Tam dung de uu tien SoftAP chay on dinh."));
            }
            break;

        case STA_CONNECTED:
            if (WiFi.status() != WL_CONNECTED) {
                _staState = STA_DISCONNECTED_PAUSED;
                _lastStaRetryTime = now;
                WiFi.disconnect(false);
                Serial.println(F("[WIFI] Mat ket noi WiFi nha! Tam dung ket noi de bao ve SoftAP."));
            }
            break;

        case STA_DISCONNECTED_PAUSED:
            // Dinh ky 60 giay thu ket noi lai 1 lan trong 10 giay
            if (now - _lastStaRetryTime >= 60000) {
                _staState = STA_CONNECTING;
                _staConnectStartTime = now;
                _lastStaRetryTime = now;
                WiFi.begin(_config.wifiSSID, _config.wifiPass);
                Serial.println(F("[WIFI] Dang thu ket noi lai vao WiFi nha..."));
            }
            break;

        default:
            break;
    }
}

bool WebServerManager::checkAuth() {
    String pass = _server.arg("pass");
    if (pass.length() == 0) return false;
    return (pass == _config.webPassword);
}

void WebServerManager::setupRoutes() {
    _server.on("/", HTTP_GET, std::bind(&WebServerManager::handleRoot, this));
    _server.on("/api/login", HTTP_POST, std::bind(&WebServerManager::handleApiLogin, this));
    _server.on("/api/status", HTTP_GET, std::bind(&WebServerManager::handleApiStatus, this));
    _server.on("/api/control", HTTP_GET, std::bind(&WebServerManager::handleApiControl, this));

    // Endpoints quản lý mã RF & Học lệnh không dây
    _server.on("/api/rf/list", HTTP_GET, std::bind(&WebServerManager::handleApiRfList, this));
    _server.on("/api/rf/learn", HTTP_POST, std::bind(&WebServerManager::handleApiRfLearn, this));
    _server.on("/api/rf/learn_status", HTTP_GET, std::bind(&WebServerManager::handleApiRfLearnStatus, this));
    _server.on("/api/rf/cancel", HTTP_POST, std::bind(&WebServerManager::handleApiRfCancel, this));
    _server.on("/api/rf/set", HTTP_POST, std::bind(&WebServerManager::handleApiRfSet, this));
    _server.on("/api/rf/clear", HTTP_POST, std::bind(&WebServerManager::handleApiRfClear, this));
    _server.on("/api/rf/export", HTTP_GET, std::bind(&WebServerManager::handleApiRfExport, this));
    _server.on("/api/rf/import", HTTP_POST, std::bind(&WebServerManager::handleApiRfImport, this));

    // Endpoints cấu hình WiFi & Hệ thống
    _server.on("/api/wifi/scan", HTTP_GET, std::bind(&WebServerManager::handleApiWifiScan, this));
    _server.on("/api/config/wifi", HTTP_POST, std::bind(&WebServerManager::handleApiConfigWifi, this));
    _server.on("/api/config/security", HTTP_POST, std::bind(&WebServerManager::handleApiConfigSecurity, this));
    _server.on("/api/reboot", HTTP_POST, std::bind(&WebServerManager::handleApiReboot, this));

    // Endpoint cập nhật Firmware OTA
    _server.on("/update", HTTP_GET, std::bind(&WebServerManager::handleUpdateGet, this));
    _server.on("/update", HTTP_POST, 
               std::bind(&WebServerManager::handleUpdatePost, this),
               std::bind(&WebServerManager::handleUpdateUpload, this));
}

void WebServerManager::handleRoot() {
    _server.send_P(200, "text/html", INDEX_HTML);
}

void WebServerManager::handleApiLogin() {
    if (checkAuth()) {
        _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đăng nhập thành công\"}");
    } else {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Sai mật khẩu Quản trị\"}");
    }
}

void WebServerManager::handleApiStatus() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    uint32_t flashMB = 0;
#if defined(ESP8266)
    flashMB = ESP.getFlashChipRealSize() / (1024 * 1024);
#elif defined(ESP32)
    flashMB = ESP.getFlashChipSize() / (1024 * 1024);
#endif

    char json[800];
    snprintf(json, sizeof(json),
             "{\"state\":\"%s\",\"stateCode\":%u,\"percentage\":%u,\"openDuration\":%lu,\"travelTime\":%u,\"source\":\"%s\","
             "\"apIP\":\"%s\",\"staIP\":\"%s\",\"staConnected\":%s,\"staSSID\":\"%s\",\"gasUrl\":\"%s\","
             "\"mqttEnabled\":%s,\"mqttConnected\":%s,\"mqttServer\":\"%s\",\"mqttPort\":%u,\"mqttTopic\":\"%s\",\"mqttUser\":\"%s\","
             "\"board\":\"%s\",\"cpu\":\"%s (%uMHz)\",\"flashMB\":%u,\"freeHeap\":%u}",
             _tracker.getStateNameVi(),
             (unsigned int)_tracker.getState(),
             (unsigned int)_tracker.getOpenPercentage(),
             (unsigned long)_tracker.getCurrentOpenDurationSec(),
             (unsigned int)_tracker.getTravelTimeSec(),
             _tracker.getLastSource(),
             WiFi.softAPIP().toString().c_str(),
             WiFi.localIP().toString().c_str(),
             isStaConnected() ? "true" : "false",
             _config.wifiSSID,
             _config.gasWebhookUrl,
             _config.mqttEnabled ? "true" : "false",
             (_mqttManager && _mqttManager->isConnected()) ? "true" : "false",
             _config.mqttServer,
             (unsigned int)_config.mqttPort,
             _config.mqttTopic,
             _config.mqttUser,
             BOARD_TARGET_NAME,
             BOARD_CPU_ARCH,
             (unsigned int)ESP.getCpuFreqMHz(),
             (unsigned int)flashMB,
             (unsigned int)ESP.getFreeHeap());

    _server.send(200, "application/json", json);
}

void WebServerManager::handleApiControl() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String actionStr = _server.arg("action");
    DoorAction action;
    if (parseAction(actionStr.c_str(), action)) {
        bool ok = _controller.sendCommand(action);
        _tracker.triggerAction(action, "Web Local AP");

        if (ok) {
            _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã phát lệnh thành công\"}");
        } else {
            _server.send(400, "application/json", "{\"success\":false,\"message\":\"Nút chưa được học lệnh RF\"}");
        }
    } else {
        _server.send(400, "application/json", "{\"success\":false,\"message\":\"Lệnh không hợp lệ\"}");
    }
}

void WebServerManager::handleApiRfList() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String json = "[";
    for (uint8_t i = 0; i < ACTION_COUNT; i++) {
        DoorAction act = (DoorAction)i;
        const RFCommand& cmd = _controller.getCommand(act);
        if (i > 0) json += ",";

        char hexBuf[14];
        if (cmd.isValid) {
            snprintf(hexBuf, sizeof(hexBuf), "0x%06lX", (unsigned long)cmd.code);
        } else {
            strncpy(hexBuf, "---", sizeof(hexBuf));
        }

        json += "{\"action\":\"";
        json += getActionName(act);
        json += "\",\"name\":\"";
        json += getActionDescVi(act);
        json += "\",\"code\":";
        json += String(cmd.code);
        json += ",\"hex\":\"";
        json += hexBuf;
        json += "\",\"bits\":";
        json += String(cmd.bitLength);
        json += ",\"proto\":";
        json += String(cmd.protocol);
        json += ",\"pulse\":";
        json += String(cmd.pulseLength);
        json += ",\"isValid\":";
        json += (cmd.isValid ? "true" : "false");
        json += "}";
    }
    json += "]";

    _server.send(200, "application/json", json);
}

void WebServerManager::handleApiRfLearn() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String actionStr = _server.arg("action");
    DoorAction act;
    if (!parseAction(actionStr.c_str(), act)) {
        _server.send(400, "application/json", "{\"success\":false,\"message\":\"Nút không hợp lệ\"}");
        return;
    }

    bool started = _controller.startLearning(act, 15000);
    if (started) {
        _server.send(200, "application/json", "{\"success\":true,\"message\":\"Bắt đầu chế độ học lệnh\"}");
    } else {
        _server.send(500, "application/json", "{\"success\":false,\"message\":\"Không thể bắt đầu học lệnh\"}");
    }
}

void WebServerManager::handleApiRfLearnStatus() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    bool isLearning = _controller.isLearning();
    uint32_t rem = _controller.getLearningRemainingMs();
    bool finished = _controller.isLastLearnFinished();
    bool success = _controller.isLastLearnSuccess();
    const RFCommand& cmd = _controller.getLastLearnedCommand();

    char hexBuf[14];
    snprintf(hexBuf, sizeof(hexBuf), "0x%06lX", (unsigned long)cmd.code);

    char json[256];
    snprintf(json, sizeof(json),
             "{\"isLearning\":%s,\"remainingMs\":%lu,\"finished\":%s,\"success\":%s,\"codeHex\":\"%s\",\"codeDec\":%lu,\"bits\":%u,\"proto\":%u,\"pulse\":%u}",
             isLearning ? "true" : "false",
             (unsigned long)rem,
             finished ? "true" : "false",
             success ? "true" : "false",
             hexBuf,
             (unsigned long)cmd.code,
             cmd.bitLength,
             cmd.protocol,
             cmd.pulseLength);

    _server.send(200, "application/json", json);
}

void WebServerManager::handleApiRfCancel() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    _controller.cancelLearning();
    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã hủy học lệnh\"}");
}

void WebServerManager::handleApiRfSet() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String actionStr = _server.arg("action");
    DoorAction act;
    if (!parseAction(actionStr.c_str(), act)) {
        _server.send(400, "application/json", "{\"success\":false,\"message\":\"Nút không hợp lệ\"}");
        return;
    }

    String codeStr = _server.arg("code");
    uint32_t code = strtoul(codeStr.c_str(), nullptr, 0);

    uint8_t bits = _server.hasArg("bits") ? (uint8_t)_server.arg("bits").toInt() : 24;
    uint8_t proto = _server.hasArg("proto") ? (uint8_t)_server.arg("proto").toInt() : 1;
    uint16_t pulse = _server.hasArg("pulse") ? (uint16_t)_server.arg("pulse").toInt() : 350;

    _controller.setCommand(act, RFCommand(code, bits, proto, pulse));
    _controller.save();

    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã gán và lưu mã RF thành công\"}");
}

void WebServerManager::handleApiRfClear() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String actionStr = _server.arg("action");
    DoorAction act;
    if (parseAction(actionStr.c_str(), act)) {
        RFCommand emptyCmd;
        _controller.setCommand(act, emptyCmd);
        _controller.save();
        _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã xóa mã của nút này\"}");
    } else {
        _controller.clear();
        _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã xóa toàn bộ mã RF\"}");
    }
}

void WebServerManager::handleApiRfExport() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    const RFCommand& cUp   = _controller.getCommand(ACTION_UP);
    const RFCommand& cDown = _controller.getCommand(ACTION_DOWN);
    const RFCommand& cStop = _controller.getCommand(ACTION_STOP);
    const RFCommand& cLock = _controller.getCommand(ACTION_LOCK);

    char expBuf[128];
    snprintf(expBuf, sizeof(expBuf), "0x%lX 0x%lX 0x%lX 0x%lX %u %u %u",
             (unsigned long)cUp.code,
             (unsigned long)cDown.code,
             (unsigned long)cStop.code,
             (unsigned long)cLock.code,
             cUp.bitLength > 0 ? cUp.bitLength : 24,
             cUp.protocol > 0 ? cUp.protocol : 1,
             cUp.pulseLength > 0 ? cUp.pulseLength : 350);

    char json[200];
    snprintf(json, sizeof(json), "{\"success\":true,\"exportString\":\"%s\"}", expBuf);
    _server.send(200, "application/json", json);
}

void WebServerManager::handleApiRfImport() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String str = _server.arg("string");
    char buf[128];
    strncpy(buf, str.c_str(), sizeof(buf) - 1);

    char* token = strtok(buf, " ");
    if (!token) { _server.send(400, "application/json", "{\"success\":false,\"message\":\"Sai định dạng\"}"); return; }
    uint32_t cUp = strtoul(token, nullptr, 0);

    token = strtok(nullptr, " ");
    if (!token) { _server.send(400, "application/json", "{\"success\":false,\"message\":\"Thiếu mã Down\"}"); return; }
    uint32_t cDown = strtoul(token, nullptr, 0);

    token = strtok(nullptr, " ");
    if (!token) { _server.send(400, "application/json", "{\"success\":false,\"message\":\"Thiếu mã Stop\"}"); return; }
    uint32_t cStop = strtoul(token, nullptr, 0);

    token = strtok(nullptr, " ");
    if (!token) { _server.send(400, "application/json", "{\"success\":false,\"message\":\"Thiếu mã Lock\"}"); return; }
    uint32_t cLock = strtoul(token, nullptr, 0);

    uint8_t bits = 24, proto = 1;
    uint16_t pulse = 350;

    token = strtok(nullptr, " ");
    if (token) bits = (uint8_t)atoi(token);
    token = strtok(nullptr, " ");
    if (token) proto = (uint8_t)atoi(token);
    token = strtok(nullptr, " ");
    if (token) pulse = (uint16_t)atoi(token);

    _controller.setCommand(ACTION_UP, RFCommand(cUp, bits, proto, pulse));
    _controller.setCommand(ACTION_DOWN, RFCommand(cDown, bits, proto, pulse));
    _controller.setCommand(ACTION_STOP, RFCommand(cStop, bits, proto, pulse));
    _controller.setCommand(ACTION_LOCK, RFCommand(cLock, bits, proto, pulse));
    _controller.save();

    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã nạp và lưu 4 mã RF thành công!\"}");
}

void WebServerManager::handleApiWifiScan() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    int n = WiFi.scanNetworks();
    String json = "[";
    for (int i = 0; i < n; i++) {
        if (i > 0) json += ",";
        json += "{\"ssid\":\"";
        json += WiFi.SSID(i);
        json += "\",\"rssi\":";
        json += WiFi.RSSI(i);
        json += "}";
    }
    json += "]";
    WiFi.scanDelete();

    _server.send(200, "application/json", json);
}

void WebServerManager::handleApiConfigWifi() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String ssid = _server.arg("ssid");
    String pass = _server.arg("wifi_pass");
    String gasUrl = _server.arg("gas_url");
    String travelStr = _server.arg("travel_time");

    String mqttEnStr = _server.arg("mqtt_en");
    String mqttSrv = _server.arg("mqtt_server");
    String mqttPortStr = _server.arg("mqtt_port");
    String mqttUser = _server.arg("mqtt_user");
    String mqttPass = _server.arg("mqtt_pass");
    String mqttTopic = _server.arg("mqtt_topic");

    strncpy(_config.wifiSSID, ssid.c_str(), sizeof(_config.wifiSSID) - 1);
    if (pass.length() > 0) {
        strncpy(_config.wifiPass, pass.c_str(), sizeof(_config.wifiPass) - 1);
    }
    strncpy(_config.gasWebhookUrl, gasUrl.c_str(), sizeof(_config.gasWebhookUrl) - 1);

    if (travelStr.length() > 0) {
        uint16_t t = travelStr.toInt();
        if (t >= 5 && t <= 120) {
            _config.travelTimeSec = t;
            _tracker.setTravelTimeSec(t);
        }
    }

    _config.mqttEnabled = (mqttEnStr == "1" || mqttEnStr == "true");
    strncpy(_config.mqttServer, mqttSrv.c_str(), sizeof(_config.mqttServer) - 1);
    if (mqttPortStr.length() > 0) {
        uint16_t p = (uint16_t)mqttPortStr.toInt();
        if (p > 0) _config.mqttPort = p;
    }
    strncpy(_config.mqttUser, mqttUser.c_str(), sizeof(_config.mqttUser) - 1);
    if (mqttPass.length() > 0) {
        strncpy(_config.mqttPass, mqttPass.c_str(), sizeof(_config.mqttPass) - 1);
    }
    if (mqttTopic.length() > 0) {
        strncpy(_config.mqttTopic, mqttTopic.c_str(), sizeof(_config.mqttTopic) - 1);
    }

    _controller.getStorage().saveSystemConfig(_config);
    _sheetClient.begin(_config.gasWebhookUrl, GAS_WEBHOOK_TOKEN);

    Serial.print(F("[CONFIG] Da luu cau hinh MQTT: "));
    Serial.print(_config.mqttEnabled ? F("BAT") : F("TAT"));
    Serial.print(F(" | Host: "));
    Serial.print(_config.mqttServer);
    Serial.print(F(":"));
    Serial.print(_config.mqttPort);
    Serial.print(F(" | Topic: "));
    Serial.println(_config.mqttTopic);

    if (_mqttManager) {
        _mqttManager->setConfig(_config);
    }

    if (strlen(_config.wifiSSID) > 0) {
        _staState = STA_CONNECTING;
        _staConnectStartTime = millis();
        _lastStaRetryTime = millis();
        WiFi.begin(_config.wifiSSID, _config.wifiPass);
        Serial.print(F("[WIFI] Bat dau ket noi WiFi nha: "));
        Serial.println(_config.wifiSSID);
    } else {
        _staState = STA_IDLE;
        WiFi.disconnect(false);
    }

    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đã lưu cấu hình WiFi & Cloud thành công!\"}");
}

void WebServerManager::handleApiConfigSecurity() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    String newApPass = _server.arg("new_ap_pass");
    String newMasterPass = _server.arg("new_master_pass");

    if (newApPass.length() > 0 && newApPass.length() < 8) {
        _server.send(400, "application/json", "{\"success\":false,\"message\":\"Mật khẩu WiFi AP phải có ít nhất 8 ký tự!\"}");
        return;
    }

    if (newApPass.length() >= 8) {
        strncpy(_config.apPassword, newApPass.c_str(), sizeof(_config.apPassword) - 1);
        WiFi.softAP(WIFI_AP_SSID, _config.apPassword);
    }

    if (newMasterPass.length() > 0) {
        strncpy(_config.webPassword, newMasterPass.c_str(), sizeof(_config.webPassword) - 1);
    }

    _controller.getStorage().saveSystemConfig(_config);

    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Đổi mật khẩu thành công!\"}");
}

void WebServerManager::handleApiReboot() {
    if (!checkAuth()) {
        _server.send(401, "application/json", "{\"success\":false,\"message\":\"Chưa đăng nhập\"}");
        return;
    }

    _server.send(200, "application/json", "{\"success\":true,\"message\":\"Thiết bị đang khởi động lại...\"}");
    delay(500);
    ESP.restart();
}

void WebServerManager::handleUpdateGet() {
    String html = OTA_HTML;
    html.replace("%BOARD_NAME%", BOARD_TARGET_NAME);
    html.replace("%CPU_ARCH%", BOARD_CPU_ARCH);
    html.replace("%CPU_MHZ%", String(ESP.getCpuFreqMHz()));
#if defined(ESP8266)
    html.replace("%FLASH_MB%", String(ESP.getFlashChipRealSize() / (1024 * 1024)));
    html.replace("%FREE_OTA%", String(ESP.getFreeSketchSpace() / 1024));
#elif defined(ESP32)
    html.replace("%FLASH_MB%", String(ESP.getFlashChipSize() / (1024 * 1024)));
    html.replace("%FREE_OTA%", String(ESP.getFreeSketchSpace() / 1024));
#else
    html.replace("%FLASH_MB%", "0");
    html.replace("%FREE_OTA%", "0");
#endif
    html.replace("%OPPOSITE_BOARD%", BOARD_OPPOSITE_NAME);
    _server.send(200, "text/html", html);
}

void WebServerManager::handleUpdatePost() {
    if (_otaAuthenticated && _otaSuccess) {
        _server.send(200, "text/html", "<script>alert('Nạp Firmware thành công! Thiết bị đang khởi động lại...');window.location.href='/';</script>");
        delay(1000);
        ESP.restart();
    } else {
        String msg = "Cập nhật Firmware thất bại!";
        if (_otaErrorMsg.length() > 0) {
            msg = _otaErrorMsg;
        } else if (!_otaAuthenticated) {
            msg = "Sai Master Password!";
        }
        String resp = "<script>alert('" + msg + "');window.location.href='/update';</script>";
        _server.send(500, "text/html", resp);
    }
}

void WebServerManager::handleUpdateUpload() {
    HTTPUpload& upload = _server.upload();

    if (upload.status == UPLOAD_FILE_START) {
        _otaSuccess = false;
        _otaAuthenticated = (_server.arg("pass") == _config.webPassword);
        _otaErrorMsg = "";

        if (!_otaAuthenticated) {
            _otaErrorMsg = "Sai Master Password!";
            Serial.println(F("[OTA LOI] Tu choi nap firmware do sai Master Password!"));
            return;
        }

#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) {
            Update.printError(Serial);
            _otaErrorMsg = "Khong du bo nho Flash!";
        }
#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            Update.printError(Serial);
            _otaErrorMsg = "Khong the khoi dong Update!";
        }
#endif
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (_otaAuthenticated) {
            // Kiểm tra an toàn loại bo mạch ngay ở gói tin đầu tiên
            if (upload.totalSize == 0 && upload.currentSize >= 16) {
                uint8_t magic = upload.buf[0];
                if (magic != 0xE9) {
                    _otaAuthenticated = false;
                    _otaErrorMsg = "File nap khong phai dinh dang Firmware ESP (.bin hop le)!";
                    Serial.println(F("[OTA LOI] Magic byte khong hop le!"));
                    Update.end(false);
                    return;
                }

#if defined(ARDUINO_ARCH_ESP8266) || defined(ESP8266)
                // ĐANG CHẠY TRÊN BO MẠCH ESP8266:
                // Nếu người dùng chọn nhầm file .bin của ESP32 / ESP32-S3 (chip_id tại offset 12..13 là 0, 2, 5, 9, 12, 13)
                // và entry point address (offset 4..7) không nằm trong không gian nhớ ESP8266 (0x4010xxxx / 0x402xxxxx)
                uint16_t chipId = (uint16_t)upload.buf[12] | ((uint16_t)upload.buf[13] << 8);
                uint32_t entryAddr = (uint32_t)upload.buf[4] | ((uint32_t)upload.buf[5] << 8) | ((uint32_t)upload.buf[6] << 16) | ((uint32_t)upload.buf[7] << 24);
                
                bool isEsp32Family = (chipId == 0 || chipId == 2 || chipId == 5 || chipId == 9 || chipId == 12 || chipId == 13);
                bool isNotEsp8266Addr = ((entryAddr & 0xFFF00000) != 0x40100000 && (entryAddr & 0xFFF00000) != 0x40200000);

                if (isEsp32Family && isNotEsp8266Addr) {
                    _otaAuthenticated = false;
                    _otaErrorMsg = "CANH BAO AN TOAN: Ban dang nap file .bin cua ESP32 vao bo mach ESP8266! Da chan nap de tranh hong thiet bi.";
                    Serial.println(F("[OTA AN TOAN] DA CHAN: Phat hien file .bin ESP32 tren bo mach ESP8266!"));
                    Update.end(false);
                    return;
                }
#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
                // ĐANG CHẠY TRÊN BO MẠCH ESP32 / ESP32-S3:
                uint16_t chipId = (uint16_t)upload.buf[12] | ((uint16_t)upload.buf[13] << 8);
                #if defined(CONFIG_IDF_TARGET_ESP32S3)
                if (chipId != 0x0009) {
                    _otaAuthenticated = false;
                    _otaErrorMsg = "CANH BAO AN TOAN: File .bin khong phai danh cho ESP32-S3 (chip_id khac 0x0009)!";
                    Serial.println(F("[OTA AN TOAN] DA CHAN: File .bin khong phai cua ESP32-S3!"));
                    Update.end(false);
                    return;
                }
                #endif
#endif
            }

            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
                _otaErrorMsg = "Loi khi ghi du lieu Flash!";
            }
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        if (_otaAuthenticated) {
            if (Update.end(true)) {
                _otaSuccess = true;
            } else {
                Update.printError(Serial);
                _otaErrorMsg = "Loi hoan tat nap Flash!";
            }
        }
    }
}
