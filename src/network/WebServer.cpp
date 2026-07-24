/**
 * WebServer - Implementation
 */
#include "WebServer.hpp"
#include "ConfigManager.hpp"
#include "LogManager.hpp"

namespace filament_dryer {

WebServer::WebServer(uint16_t port) : port_(port) {}

WebServer::~WebServer() {
    if (server_) {
        server_->end();
        delete server_;
    }
}

bool WebServer::begin(ConfigManager* config_mgr, LogManager* log_mgr) {
    config_mgr_ = config_mgr;
    log_mgr_ = log_mgr;
    
    server_ = new AsyncWebServer(port_);
    
    setupRoutes();
    
    server_->begin();
    
    Serial.printf("[WebServer] Started on port %d\n", port_);
    return true;
}

void WebServer::setupRoutes() {
    // Captive portal detection (Android, iOS, Windows)
    server_->on("/generate_204", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/fwlink", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/hotspot-detect.html", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/ncsi.txt", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    server_->on("/connecttest.txt", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleCaptivePortal(request);
    });
    
    // Main routes
    server_->on("/", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleRoot(request);
    });
    
    server_->on("/info", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleInfo(request);
    });
    
    server_->on("/api/wifi/config", HTTP_POST, [this](AsyncWebServerRequest* request) {
        handleWifiConfigPost(request);
    });
    
    // Log download endpoints
    server_->on("/log/drying", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLogDownload(request, true);
    });
    
    server_->on("/log/system", HTTP_GET, [this](AsyncWebServerRequest* request) {
        handleLogDownload(request, false);
    });
    
    // 404 handler
    server_->onNotFound([this](AsyncWebServerRequest* request) {
        handleNotFound(request);
    });
}

void WebServer::handleCaptivePortal(AsyncWebServerRequest* request) {
    request->redirect("/");
}

void WebServer::handleRoot(AsyncWebServerRequest* request) {
    request->send(200, "text/html", getCaptivePortalHTML());
}

void WebServer::handleInfo(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        request->send(500, "application/json", "{\"error\":\"Config manager not available\"}");
        return;
    }
    
    StaticJsonDocument<1024> doc;
    doc["firmware_version"] = FIRMWARE_VERSION;
    doc["chip_model"] = ESP.getChipModel();
    doc["chip_revision"] = ESP.getChipRevision();
    doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
    doc["mac_address"] = WiFi.macAddress();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["heap_size"] = ESP.getHeapSize();
    doc["system_status"] = "ready";
    doc["active_feature"] = "001-filament-dryer-esp32";
    
    String json;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
}

void WebServer::handleWifiConfigPost(AsyncWebServerRequest* request) {
    if (!config_mgr_) {
        request->send(500, "application/json", "{\"error\":\"Config manager not available\"}");
        return;
    }
    
    String ssid, password;
    
    if (request->hasParam("ssid", true)) {
        ssid = request->getParam("ssid", true)->value();
    }
    if (request->hasParam("password", true)) {
        password = request->getParam("password", true)->value();
    }
    
    if (ssid.isEmpty()) {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"SSID vazio\"}");
        return;
    }
    
    ConfigManager::WifiConfig config;
    config.ssid = ssid;
    config.password = password;
    config.valid = true;
    
    if (config_mgr_->setWifiConfig(config)) {
        String json = "{\"status\":\"success\",\"message\":\"Credenciais salvas. Reiniciando conexao...\"}";
        request->send(200, "application/json", json);
        
        // Restart WiFi with new config after short delay
        // Note: In practice, this would be handled by the WifiManager
    } else {
        request->send(500, "application/json", "{\"status\":\"error\",\"message\":\"Falha ao salvar\"}");
    }
}

void WebServer::handleLogDownload(AsyncWebServerRequest* request, bool drying_log) {
    if (!log_mgr_) {
        request->send(500, "application/json", "{\"error\":\"Log manager not available\"}");
        return;
    }
    
    String log_content;
    bool ok = drying_log ? log_mgr_->getDryingLog(log_content) : log_mgr_->getSystemLog(log_content);
    
    if (!ok) {
        request->send(404, "application/json", "{\"error\":\"Log nao encontrado\"}");
        return;
    }
    
    String filename = drying_log ? "drying.log" : "system.log";
    
    AsyncWebServerResponse* response = request->beginResponse(
        "text/plain", 
        log_content,
        [filename](AsyncWebServerRequest* req, const String& data) {
            // Headers are set in the response
        }
    );
    
    response->addHeader("Content-Disposition", "attachment; filename=\"" + filename + "\"");
    response->addHeader("Cache-Control", "no-cache");
    
    request->send(response);
}

void WebServer::handleNotFound(AsyncWebServerRequest* request) {
    // If captive portal check, redirect to root
    if (request->url().indexOf("generate_204") >= 0 ||
        request->url().indexOf("fwlink") >= 0 ||
        request->url().indexOf("hotspot") >= 0 ||
        request->url().indexOf("ncsi") >= 0 ||
        request->url().indexOf("connecttest") >= 0) {
        request->redirect("/");
        return;
    }
    
    request->send(404, "text/plain", "Not found");
}

const char* WebServer::getCaptivePortalHTML() {
    return R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Filament Dryer - Configuração WiFi</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%);
            min-height: 100vh; display: flex; align-items: center; justify-content: center;
            padding: 20px;
        }
        .container { 
            background: white; border-radius: 16px; padding: 40px; 
            max-width: 400px; width: 100%; box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        .logo { text-align: center; margin-bottom: 30px; }
        .logo svg { width: 80px; height: 80px; }
        h1 { color: #1e3c72; text-align: center; margin-bottom: 10px; font-size: 24px; }
        .subtitle { color: #666; text-align: center; margin-bottom: 30px; font-size: 14px; }
        .form-group { margin-bottom: 20px; }
        label { display: block; margin-bottom: 8px; color: #333; font-weight: 500; font-size: 14px; }
        input { 
            width: 100%; padding: 14px 16px; border: 2px solid #e0e0e0; 
            border-radius: 8px; font-size: 16px; transition: border-color 0.2s;
        }
        input:focus { outline: none; border-color: #1e3c72; }
        .btn { 
            width: 100%; padding: 16px; background: #1e3c72; color: white; 
            border: none; border-radius: 8px; font-size: 16px; font-weight: 600;
            cursor: pointer; transition: background 0.2s;
        }
        .btn:hover { background: #2a5298; }
        .btn:disabled { background: #999; cursor: not-allowed; }
        .info { 
            background: #f0f4f8; border-radius: 8px; padding: 16px; 
            margin-top: 20px; font-size: 13px; color: #555;
        }
        .info strong { color: #1e3c72; }
        .spinner { display: none; width: 20px; height: 20px; border: 3px solid #f3f3f3; 
            border-top: 3px solid #1e3c72; border-radius: 50%; animation: spin 1s linear infinite; 
            margin: 0 auto; }
        @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
        .success { display: none; text-align: center; color: #27ae60; }
    </style>
</head>
<body>
    <div class="container">
        <div class="logo">
            <svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg">
                <circle cx="50" cy="50" r="45" fill="none" stroke="#1e3c72" stroke-width="4"/>
                <path d="M30 50 Q50 30 70 50 Q50 70 30 50" fill="none" stroke="#1e3c72" stroke-width="3"/>
                <circle cx="50" cy="50" r="15" fill="#1e3c72"/>
            </svg>
        </div>
        <h1>Filament Dryer ESP32</h1>
        <p class="subtitle">Configure a conexão WiFi para iniciar</p>
        
        <form id="wifiForm">
            <div class="form-group">
                <label for="ssid">Nome da Rede (SSID)</label>
                <input type="text" id="ssid" name="ssid" required autocomplete="off" placeholder="MinhaRedeWiFi">
            </div>
            <div class="form-group">
                <label for="password">Senha</label>
                <input type="password" id="password" name="password" autocomplete="new-password" placeholder="Senha da rede">
            </div>
            <button type="submit" class="btn" id="submitBtn">Conectar</button>
            <div class="spinner" id="spinner"></div>
        </form>
        
        <div class="success" id="successMsg">
            ✓ Credenciais salvas! O dispositivo vai reiniciar a conexão...
        </div>
        
        <div class="info">
            <strong>Informações:</strong>
            <ul style="margin: 10px 0 0 20px; padding: 0;">
                <li>AP padrão: <strong>philarmony</strong> / <strong>philarmony</strong></li>
                <li>IP do AP: <strong>192.168.4.1</strong></li>
                <li>Após conectar, acesse via WebSocket no IP do dispositivo</li>
            </ul>
        </div>
    </div>
    
    <script>
        document.getElementById('wifiForm').addEventListener('submit', async (e) => {
            e.preventDefault();
            const btn = document.getElementById('submitBtn');
            const spinner = document.getElementById('spinner');
            const success = document.getElementById('successMsg');
            
            btn.disabled = true;
            btn.textContent = '';
            spinner.style.display = 'block';
            
            const formData = new FormData(e.target);
            
            try {
                const response = await fetch('/api/wifi/config', {
                    method: 'POST',
                    body: formData
                });
                
                const result = await response.json();
                
                if (result.status === 'success') {
                    spinner.style.display = 'none';
                    success.style.display = 'block';
                    btn.style.display = 'none';
                } else {
                    throw new Error(result.message || 'Erro desconhecido');
                }
            } catch (err) {
                alert('Erro: ' + err.message);
                btn.disabled = false;
                btn.textContent = 'Conectar';
                spinner.style.display = 'none';
            }
        });
    </script>
</body>
</html>
)rawliteral";
}