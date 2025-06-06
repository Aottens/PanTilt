#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <AccelStepper.h>

#define OLED_SDA 21
#define OLED_SCL 22

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

WebServer server(80);
Preferences prefs;

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

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
    }

    showStatus("Booting...");
    initSteppers();

    if(!connectWiFi()) {
        startConfigAP();
    }

    showStatus("WiFi Connected\nIP: " + WiFi.localIP().toString());
    server.on("/", [](){ server.send(200, "text/plain", "Controller endpoint"); });
    server.on("/move", HTTP_GET, handleMove);
    server.begin();
}

void loop() {
    server.handleClient();
    panStepper.run();
    tiltStepper.run();
    zoomStepper.run();
    delay(1);
}

