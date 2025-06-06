#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>
#include <WiFiUdp.h>


#define PAN_ENCODER_ADDR  0x36
#define TILT_ENCODER_ADDR 0x37
#define ZOOM_ENCODER_ADDR 0x38

#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WebServer server(80);
Preferences prefs;
WiFiUDP udp;

#define DISCOVERY_PORT 4210
#define WIFI_BTN_PIN 0
const char *DEVICE_NAME = "PTZ-Receiver";

// Stepper driver pins
#define PAN_STEP_PIN 32
#define PAN_DIR_PIN 33
#define PAN_EN_PIN 25

#define TILT_STEP_PIN 26
#define TILT_DIR_PIN 27
#define TILT_EN_PIN 14

#define ZOOM_STEP_PIN 12
#define ZOOM_DIR_PIN 13
#define ZOOM_EN_PIN 15

AccelStepper panStepper(AccelStepper::DRIVER, PAN_STEP_PIN, PAN_DIR_PIN);
AccelStepper tiltStepper(AccelStepper::DRIVER, TILT_STEP_PIN, TILT_DIR_PIN);
AccelStepper zoomStepper(AccelStepper::DRIVER, ZOOM_STEP_PIN, ZOOM_DIR_PIN);

uint16_t panAngle = 0;
uint16_t tiltAngle = 0;
uint16_t zoomAngle = 0;

uint16_t readAS5600(uint8_t addr) {
    Wire.beginTransmission(addr);
    Wire.write(0x0C); // RAW ANGLE MSB
    if (Wire.endTransmission(false) != 0) return 0;
    Wire.requestFrom(addr, (uint8_t)2);
    if (Wire.available() < 2) return 0;
    uint16_t value = (Wire.read() << 8) | Wire.read();
    return value;
}

void showStatus(const String &msg) {
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println(msg);
    display.display();
}

void initSteppers() {
    pinMode(PAN_EN_PIN, OUTPUT);
    pinMode(TILT_EN_PIN, OUTPUT);
    pinMode(ZOOM_EN_PIN, OUTPUT);

    digitalWrite(PAN_EN_PIN, LOW);
    digitalWrite(TILT_EN_PIN, LOW);
    digitalWrite(ZOOM_EN_PIN, LOW);

    panStepper.setMaxSpeed(2000);
    panStepper.setAcceleration(1000);

    tiltStepper.setMaxSpeed(2000);
    tiltStepper.setAcceleration(1000);

    zoomStepper.setMaxSpeed(2000);
    zoomStepper.setAcceleration(1000);
}

void saveCredentials(const String &ssid, const String &pass) {
    prefs.begin("wifi", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();
}

void handleRoot() {
    String page = "<html><body><h1>PanTilt WiFi Setup</h1>";
    page += "<form method='POST' action='/save'>";
    page += "SSID:<br><input name='ssid'><br>";
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

void handleMove() {
    if (server.hasArg("pan")) {
        long p = server.arg("pan").toInt();
        panStepper.moveTo(p);
    }
    if (server.hasArg("tilt")) {
        long t = server.arg("tilt").toInt();
        tiltStepper.moveTo(t);
    }
    if (server.hasArg("zoom")) {
        long z = server.arg("zoom").toInt();
        zoomStepper.moveTo(z);
    }
    server.send(200, "text/plain", "OK");
}

void startConfigAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("PanTilt-Setup");
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

void setup() {
    Serial.begin(115200);
    Wire.begin(OLED_SDA, OLED_SCL);
    pinMode(WIFI_BTN_PIN, INPUT_PULLUP);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
    }

    showStatus("Booting...");
    initSteppers();

    panAngle = readAS5600(PAN_ENCODER_ADDR);
    tiltAngle = readAS5600(TILT_ENCODER_ADDR);
    zoomAngle = readAS5600(ZOOM_ENCODER_ADDR);
  
    if(digitalRead(WIFI_BTN_PIN) == LOW || !connectWiFi()) {

    if(!connectWiFi()) {

        startConfigAP();
    }

    showStatus("WiFi Connected\nIP: " + WiFi.localIP().toString());
    server.on("/", [](){ server.send(200, "text/plain", "Controller endpoint"); });
    server.on("/move", HTTP_GET, handleMove);
    server.begin();
    udp.begin(DISCOVERY_PORT);
}

void loop() {
    server.handleClient();
    int p = udp.parsePacket();
    if (p) {
        String msg;
        while (udp.available()) {
            msg += (char)udp.read();
        }
        if (msg == "DISCOVER_PTZ") {
            udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.write((const uint8_t*)DEVICE_NAME, strlen(DEVICE_NAME));
            udp.endPacket();
        }
    }

    panStepper.run();
    tiltStepper.run();
    zoomStepper.run();

    panAngle = readAS5600(PAN_ENCODER_ADDR);
    tiltAngle = readAS5600(TILT_ENCODER_ADDR);
    zoomAngle = readAS5600(ZOOM_ENCODER_ADDR);
    delay(1);
}

