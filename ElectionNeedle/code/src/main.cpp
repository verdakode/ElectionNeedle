#include <ESP32Servo.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Arduino_JSON.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>
#include <Preferences.h>
#include <DNSServer.h>
#include "html_templates.h"

// Pin Definitions
const int SERVO_PIN = 13;
const int MIN_ANGLE = 180;
const int MAX_ANGLE = 0;
const int DEFAULT_ANGLE = 90;

// Network Constants
const char* HOSTNAME = "electionneedle";
const char* AP_SSID = "ElectionNeedleConfig";
const char* BASE_API_URL = "https://api.manifold.markets/v0/slug/";
const unsigned long WIFI_TIMEOUT = 20000;  // 20 seconds timeout for WiFi connection
const unsigned long UPDATE_INTERVAL = 5000; // 5 seconds between probability checks


// Global Objects
Servo servo;
WebServer server(80);
DNSServer dnsServer;
Preferences preferences;
WiFiClientSecure client;

// State Variables
struct DeviceState {
    String clientSsid;
    String clientPassword;
    String marketSlug = "will-trump-win-the-2024-election"; // Default slug
    double currentProbability = 0.5;
    bool isAPMode = true;
    unsigned long lastUpdate = 0;
} state;

// Function Prototypes
void setupAP();
void setupMainWebServer();
bool connectWiFi();
void updateProbability();
void log(const String& message);
void handleRoot();
void handleStatus();
void handleConfigure();
void handleUpdateSlug();
String getMainHTML();

void setup() {
    Serial.begin(115200);
    delay(1000);
    log("\n\n=== STARTING UP ===");

     // Allocate PWM timers for servo
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

 // Initialize servo for SG90
    servo.setPeriodHertz(50);  // 50 Hz for SG90
    if (servo.attach(SERVO_PIN, 500, 2400)) {  // SG90 specific pulse width range
        log("SG90 servo initialized");
        servo.write(DEFAULT_ANGLE);
    } else {
        log("Failed to initialize SG90 servo on pin " + String(SERVO_PIN));
    }

    // Initialize preferences
    preferences.begin("wifi-config", false);
    state.clientSsid = preferences.getString("ssid", "");
    state.clientPassword = preferences.getString("password", "");
    state.marketSlug = preferences.getString("slug", state.marketSlug);

    // Configure SSL client
    client.setInsecure();

    // Set hostname before attempting connection
    WiFi.setHostname(HOSTNAME);
    delay(100);

    // Attempt WiFi connection or start AP
    if (state.clientSsid.length() > 0 && connectWiFi()) {
        log("WiFi Connected");
        log("IP: " + WiFi.localIP().toString());

        // Initialize mDNS
        if (MDNS.begin(HOSTNAME)) {
            MDNS.addService("http", "tcp", 80);
            log("mDNS started - hostname: " + String(HOSTNAME) + ".local");
        } else {
            log("mDNS failed to start");
        }

        setupMainWebServer();
    } else {
        log("Starting AP mode");
        setupAP();
    }
}

void loop() {
    if (state.isAPMode) {
        dnsServer.processNextRequest();
    }

    server.handleClient();

    // Update probability if in client mode
    if (!state.isAPMode && (millis() - state.lastUpdate >= UPDATE_INTERVAL)) {
        updateProbability();
        state.lastUpdate = millis();
    }

    // Check WiFi connection
    if (!state.isAPMode && WiFi.status() != WL_CONNECTED) {
        log("WiFi disconnected, attempting reconnection");
        if (!connectWiFi()) {
            log("Reconnection failed, switching to AP mode");
            setupAP();
            state.isAPMode = true;
        }
    }

    delay(10);
}

void setupAP() {
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(1000);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);

    IPAddress IP = WiFi.softAPIP();
    log("AP IP address: " + IP.toString());

    // Start DNS server for captive portal
    dnsServer.start(53, "*", IP);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/configure", HTTP_POST, handleConfigure);
    // Add this for captive portal functionality
    server.onNotFound([]() {
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString(), true);
        server.send(302, "text/plain", "");
    });

    server.begin();
    state.isAPMode = true;
    log("AP Mode active with captive portal");
}

void setupMainWebServer() {
    server.on("/", HTTP_GET, handleRoot);
    server.on("/status", HTTP_GET, handleStatus);
    server.on("/update-slug", HTTP_POST, handleUpdateSlug);
    server.begin();
}

bool connectWiFi() {
    WiFi.disconnect(true);
    delay(1000);
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(HOSTNAME);
    WiFi.begin(state.clientSsid.c_str(), state.clientPassword.c_str());

    log("Connecting to WiFi...");
    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < WIFI_TIMEOUT) {
        delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
        state.isAPMode = false;
        return true;
    }

    return false;
}

void updateProbability() {
    if (WiFi.status() != WL_CONNECTED) return;

    String apiUrl = String(BASE_API_URL) + state.marketSlug;
    HTTPClient http;
    http.begin(client, apiUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        JSONVar response = JSON.parse(payload);

        if (JSON.typeof(response) != "undefined") {
            double newProbability = double(response["probability"]);

            if (abs(newProbability - state.currentProbability) > 0.01) {
                state.currentProbability = newProbability;
                int angle = map(round(state.currentProbability * 100), 0, 100, MIN_ANGLE, MAX_ANGLE);
                servo.write(angle);
                log("Updated probability: " + String(state.currentProbability));
            }
        }
    }

    http.end();
}

String getMainHTML() {
    String html = MAIN_HTML;
    html.replace("%CURRENT_SLUG%", state.marketSlug);
    html.replace("%CURRENT_PROB%", String(state.currentProbability * 100, 1) + "%");
    html.replace("%CURRENT_ANGLE%", String(map(round(state.currentProbability * 100), 0, 100, MIN_ANGLE, MAX_ANGLE)));
    return html;
}

void handleRoot() {
    if (state.isAPMode) {
        server.send(200, "text/html", CONFIG_HTML);
    } else {
        server.send(200, "text/html", getMainHTML());
    }
}

void handleStatus() {
    JSONVar status;
    status["probability"] = state.currentProbability;
    status["angle"] = map(round(state.currentProbability * 100), 0, 100, MIN_ANGLE, MAX_ANGLE);
    status["slug"] = state.marketSlug;
    status["ip"] = WiFi.localIP().toString();
    server.send(200, "application/json", JSON.stringify(status));
}

void handleUpdateSlug() {
    JSONVar response;

    if (!server.hasArg("slug")) {
        response["success"] = false;
        response["message"] = "Missing slug";
        server.send(400, "application/json", JSON.stringify(response));
        return;
    }

    String newSlug = server.arg("slug");

    // Verify slug exists by making a test API call
    String testUrl = String(BASE_API_URL) + newSlug;
    HTTPClient http;
    http.begin(client, testUrl);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        state.marketSlug = newSlug;
        preferences.putString("slug", newSlug);

        response["success"] = true;
        response["message"] = "Market updated successfully";
        server.send(200, "application/json", JSON.stringify(response));

        // Force an immediate probability update
        updateProbability();
    } else {
        response["success"] = false;
        response["message"] = "Invalid market slug. Please check the URL and try again.";
        server.send(400, "application/json", JSON.stringify(response));
    }

    http.end();
}

void handleConfigure() {
    JSONVar response;

    if (!server.hasArg("ssid")) {
        response["success"] = false;
        response["message"] = "Missing SSID";
        server.send(400, "application/json", JSON.stringify(response));
        return;
    }

    state.clientSsid = server.arg("ssid");
    state.clientPassword = server.arg("password");

    if (connectWiFi()) {
        preferences.putString("ssid", state.clientSsid);
        preferences.putString("password", state.clientPassword);

        response["success"] = true;
        response["ip"] = WiFi.localIP().toString();
        response["hostname"] = String(HOSTNAME) + ".local";
        server.send(200, "application/json", JSON.stringify(response));

        delay(1000);
        ESP.restart();
    } else {
        response["success"] = false;
        response["message"] = "Could not connect to WiFi. Please check credentials.";
        server.send(400, "application/json", JSON.stringify(response));

        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_SSID);
    }
}

void log(const String& message) {
    Serial.println(message);
    delay(10);
}
