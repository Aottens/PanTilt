#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFiUdp.h>
#include <HTTPClient.h>
#include <vector>

#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define JOY_X_PIN 34
#define JOY_Y_PIN 35
#define JOY_BTN_PIN 0 // boot button can be used

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
WebServer server(80);
Preferences prefs;
WiFiUDP udp;

struct ReceiverInfo {
    IPAddress ip;
    String name;
};
std::vector<ReceiverInfo> receivers;
IPAddress selectedReceiver;

void showStatus(const String &msg) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println(msg);
    display.display();
}

void saveCredentials(const String &ssid, const String &pass) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
}

void handleRoot() {
    int n = WiFi.scanNetworks();
    String options;
    for (int i = 0; i < n; i++) {
        options += "<option>" + WiFi.SSID(i) + "</option>";
    }
    String page = "<html><body><h1>Controller WiFi Setup</h1>";
    page += "<form method='POST' action='/save'>";
    page += "SSID:<br><select name='ssid'>" + options + "</select><br>";
    page += "Password:<br><input name='pass' type='password'><br>";
    page += "<input type='submit' value='Save'></form></body></html>";
    server.send(200, "text/html", page);
}

void handleSave() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        String ssid = server.arg("ssid");
        String pass = server.arg("pass");
        saveCredentials(ssid, pass);
        server.send(200, "text/plain", "Saved. Rebooting...");
        delay(1000);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Missing fields");
    }
}

void startConfigAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Controller-Setup");
    IPAddress ip = WiFi.softAPIP();
    showStatus(String("AP: ") + ip.toString());

    server.on("/", handleRoot);
    server.on("/save", HTTP_POST, handleSave);
    server.begin();

    while (true) {
        server.handleClient();
        delay(10);
    }
}

bool connectWiFi() {
    prefs.begin("wifi", true);
    String ssid = prefs.getString("ssid", "");
    String pass = prefs.getString("pass", "");
    prefs.end();

    if (ssid.isEmpty()) {
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
        delay(500);
        showStatus("Connecting...\n" + ssid);
    }
    return WiFi.status() == WL_CONNECTED;
}

void discoverReceivers() {
    receivers.clear();
    const char *msg = "DISCOVER_PTZ";
    udp.begin(4210);
    udp.beginPacket(IPAddress(255,255,255,255), 4210);
    udp.write((const uint8_t*)msg, strlen(msg));
    udp.endPacket();

    unsigned long start = millis();
    while (millis() - start < 3000) {
        int p = udp.parsePacket();
        if (p) {
            ReceiverInfo info;
            info.ip = udp.remoteIP();
            String name;
            while (udp.available()) {
                name += (char)udp.read();
            }
            info.name = name;
            receivers.push_back(info);
        }
        delay(10);
    }
    udp.stop();
}

void showReceivers() {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println("Receivers:");
    for (size_t i=0; i<receivers.size() && i<4; ++i) {
        display.setCursor(0, 10 + i*10);
        display.print(i+1);
        display.print(": ");
        display.print(receivers[i].ip);
    }
    display.display();
    if (!receivers.empty()) {
        selectedReceiver = receivers[0].ip;
    }
}

void sendControl() {
    if (selectedReceiver == IPAddress()) return;
    int x = analogRead(JOY_X_PIN);
    int y = analogRead(JOY_Y_PIN);
    int pan = map(x, 0, 4095, -1000, 1000);
    int tilt = map(y, 0, 4095, -1000, 1000);
    String url = String("http://") + selectedReceiver.toString() + "/move?pan=" + pan + "&tilt=" + tilt;
    HTTPClient http;
    http.begin(url);
    http.GET();
    http.end();
}

void setup() {
    Serial.begin(115200);
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);
    Wire.begin(OLED_SDA, OLED_SCL);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
    }
    showStatus("Booting...");

    if(digitalRead(JOY_BTN_PIN) == LOW || !connectWiFi()) {
        startConfigAP();
    }

    showStatus("WiFi Connected\nIP: " + WiFi.localIP().toString());
    discoverReceivers();
    showReceivers();
}

unsigned long lastSend = 0;
void loop() {
    if (millis() - lastSend > 200) {
        sendControl();
        lastSend = millis();
    }
    delay(10);
}

