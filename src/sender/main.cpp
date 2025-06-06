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
#define BTN_UP_PIN    16
#define BTN_DOWN_PIN  17
#define BTN_SELECT_PIN 18
#define BTN_BACK_PIN   19

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

enum class MenuState {
    NONE,
    SELECT_RECEIVER,
    MAIN_MENU,
    MODES_MENU,
    SETTINGS_MENU,
    SPEED_MENU,
    ACCEL_MENU
};

enum class ControlMode {
    FREE_MOVE,
    KEYFRAME,
    TIMELAPSE
};

MenuState menuState = MenuState::SELECT_RECEIVER;
ControlMode currentMode = ControlMode::FREE_MOVE;
size_t menuIndex = 0;
float moveSpeed = 2000.0f;
float moveAccel = 1000.0f;

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
    if (!receivers.empty()) {
        selectedReceiver = receivers[0].ip;
    }
}

void sendControl() {
    if (selectedReceiver == IPAddress()) return;
    static float filtX = analogRead(JOY_X_PIN);
    static float filtY = analogRead(JOY_Y_PIN);
    float rawX = analogRead(JOY_X_PIN);
    float rawY = analogRead(JOY_Y_PIN);
    // simple exponential moving average to smooth joystick noise
    const float alpha = 0.3f;
    filtX = filtX * (1.0f - alpha) + rawX * alpha;
    filtY = filtY * (1.0f - alpha) + rawY * alpha;
    int pan = map((int)filtX, 0, 4095, -1000, 1000);
    int tilt = map((int)filtY, 0, 4095, -1000, 1000);

    String url = String("http://") + selectedReceiver.toString() + "/move?pan=" + pan + "&tilt=" + tilt;
    HTTPClient http;
    http.begin(url);
    http.GET();
    http.end();
}

void sendSettings() {
    if (selectedReceiver == IPAddress()) return;
    String url = String("http://") + selectedReceiver.toString() + "/settings?speed=" +
                 String((int)moveSpeed) + "&accel=" + String((int)moveAccel);
    HTTPClient http;
    http.begin(url);
    http.GET();
    http.end();
}

struct Keyframe { int pan; int tilt; };
std::vector<Keyframe> keyframes{{0,0},{500,500},{-500,0}};
size_t keyframeIndex = 0;

void handleKeyframeMode() {
    if (selectedReceiver == IPAddress()) return;
    Keyframe kf = keyframes[keyframeIndex];
    String url = String("http://") + selectedReceiver.toString() + "/move?pan=" +
                 String(kf.pan) + "&tilt=" + String(kf.tilt);
    HTTPClient http;
    http.begin(url);
    http.GET();
    http.end();
    keyframeIndex = (keyframeIndex + 1) % keyframes.size();
}

bool lastUp = false, lastDown = false, lastSelect = false, lastBack = false;

void displayMenu() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);

    switch(menuState) {
        case MenuState::SELECT_RECEIVER:
            display.println("Select PTZ:");
            for(size_t i=0;i<receivers.size() && i<4;i++) {
                display.setCursor(0,10+i*10);
                if(i==menuIndex) display.print("> "); else display.print("  ");
                display.print(receivers[i].name);
            }
            break;
        case MenuState::MAIN_MENU:
            display.println("Menu:");
            display.setCursor(0,10); display.print(menuIndex==0?"> ":"  "); display.print("Modes");
            display.setCursor(0,20); display.print(menuIndex==1?"> ":"  "); display.print("Settings");
            break;
        case MenuState::MODES_MENU:
            display.println("Modes:");
            display.setCursor(0,10); display.print(menuIndex==0?"> ":"  "); display.print("Free Move");
            display.setCursor(0,20); display.print(menuIndex==1?"> ":"  "); display.print("Keyframes");
            display.setCursor(0,30); display.print(menuIndex==2?"> ":"  "); display.print("Timelapse");
            break;
        case MenuState::SETTINGS_MENU:
            display.println("Settings:");
            display.setCursor(0,10); display.print(menuIndex==0?"> ":"  "); display.print("WiFi Setup");
            display.setCursor(0,20); display.print(menuIndex==1?"> ":"  "); display.print("Speed");
            display.setCursor(0,30); display.print(menuIndex==2?"> ":"  "); display.print("Acceleration");
            break;
        case MenuState::SPEED_MENU:
            display.print("Speed: "); display.println((int)moveSpeed);
            break;
        case MenuState::ACCEL_MENU:
            display.print("Accel: "); display.println((int)moveAccel);
            break;
        case MenuState::NONE:
        default:
            display.print("PTZ: ");
            if(selectedReceiver != IPAddress()) display.println(selectedReceiver);
            else display.println("none");
            display.setCursor(0,10);
            switch(currentMode){
                case ControlMode::FREE_MOVE: display.println("Mode: Free"); break;
                case ControlMode::KEYFRAME: display.println("Mode: Keyframe"); break;
                case ControlMode::TIMELAPSE: display.println("Mode: Timelapse"); break;
            }
            break;
    }
    display.display();
}

void handleButtons() {
    bool up = digitalRead(BTN_UP_PIN)==LOW;
    bool down = digitalRead(BTN_DOWN_PIN)==LOW;
    bool select = digitalRead(BTN_SELECT_PIN)==LOW;
    bool back = digitalRead(BTN_BACK_PIN)==LOW;

    if(menuState==MenuState::NONE) {
        if(back && !lastBack) {
            menuState=MenuState::MAIN_MENU; menuIndex=0; displayMenu();
        }
        if(currentMode==ControlMode::KEYFRAME && select && !lastSelect) {
            handleKeyframeMode();
        }
    } else if(menuState==MenuState::SELECT_RECEIVER) {
        if(up && !lastUp && menuIndex>0) { menuIndex--; displayMenu(); }
        if(down && !lastDown && menuIndex+1<receivers.size()) { menuIndex++; displayMenu(); }
        if(select && !lastSelect && !receivers.empty()) {
            selectedReceiver=receivers[menuIndex].ip; menuState=MenuState::MAIN_MENU; menuIndex=0; displayMenu();
        }
    } else if(menuState==MenuState::MAIN_MENU) {
        if(up && !lastUp && menuIndex>0){menuIndex--;displayMenu();}
        if(down && !lastDown && menuIndex<1){menuIndex++;displayMenu();}
        if(select && !lastSelect){
            menuState = (menuIndex==0)?MenuState::MODES_MENU:MenuState::SETTINGS_MENU;
            menuIndex=0; displayMenu();
        }
        if(back && !lastBack){menuState=MenuState::NONE; displayMenu();}
    } else if(menuState==MenuState::MODES_MENU) {
        if(up && !lastUp && menuIndex>0){menuIndex--;displayMenu();}
        if(down && !lastDown && menuIndex<2){menuIndex++;displayMenu();}
        if(select && !lastSelect){
            currentMode = (menuIndex==0)?ControlMode::FREE_MOVE:(menuIndex==1?ControlMode::KEYFRAME:ControlMode::TIMELAPSE);
            menuState=MenuState::NONE; displayMenu();
        }
        if(back && !lastBack){menuState=MenuState::MAIN_MENU; menuIndex=0; displayMenu();}
    } else if(menuState==MenuState::SETTINGS_MENU) {
        if(up && !lastUp && menuIndex>0){menuIndex--;displayMenu();}
        if(down && !lastDown && menuIndex<2){menuIndex++;displayMenu();}
        if(select && !lastSelect){
            if(menuIndex==0) { startConfigAP(); }
            else if(menuIndex==1) { menuState=MenuState::SPEED_MENU; displayMenu(); }
            else if(menuIndex==2) { menuState=MenuState::ACCEL_MENU; displayMenu(); }
        }
        if(back && !lastBack){menuState=MenuState::MAIN_MENU; menuIndex=0; displayMenu();}
    } else if(menuState==MenuState::SPEED_MENU) {
        if(up && !lastUp){ moveSpeed+=100; displayMenu(); }
        if(down && !lastDown && moveSpeed>100){ moveSpeed-=100; displayMenu(); }
        if(select && !lastSelect){ sendSettings(); menuState=MenuState::SETTINGS_MENU; displayMenu(); }
        if(back && !lastBack){ menuState=MenuState::SETTINGS_MENU; displayMenu(); }
    } else if(menuState==MenuState::ACCEL_MENU) {
        if(up && !lastUp){ moveAccel+=100; displayMenu(); }
        if(down && !lastDown && moveAccel>100){ moveAccel-=100; displayMenu(); }
        if(select && !lastSelect){ sendSettings(); menuState=MenuState::SETTINGS_MENU; displayMenu(); }
        if(back && !lastBack){ menuState=MenuState::SETTINGS_MENU; displayMenu(); }
    }

    lastUp=up; lastDown=down; lastSelect=select; lastBack=back;
}

void setup() {
    Serial.begin(115200);
    pinMode(JOY_BTN_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_SELECT_PIN, INPUT_PULLUP);
    pinMode(BTN_BACK_PIN, INPUT_PULLUP);
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
    displayMenu();
}

unsigned long lastSend = 0;
void loop() {
    handleButtons();

    if(menuState==MenuState::NONE) {
        if(currentMode==ControlMode::FREE_MOVE) {
            if(millis() - lastSend > 200) {
                sendControl();
                lastSend = millis();
            }
        }
    }

    delay(10);
}

