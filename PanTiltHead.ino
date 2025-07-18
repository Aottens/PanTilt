#include <Arduino.h>
#include <AccelStepper.h>
#include <Bluepad32.h>
#include <AS5600.h>
#include <Preferences.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoOTA.h>
#include "webui.h"

// Stepper pins
#define PAN_STEP 18
#define PAN_DIR  19
#define PAN_EN   16
#define TILT_STEP 17
#define TILT_DIR  21
#define TILT_EN    4

// Steps per degree (arbitrary)
#define STEPS_PER_DEG 100

// AS5600 channels (0:pan,1:tilt) if MUX used
#ifdef USE_I2C_MUX
static const uint8_t MUX=0x70;
static void muxSelect(uint8_t ch){Wire.beginTransmission(MUX);Wire.write(1<<ch);Wire.endTransmission();}
#endif

AccelStepper pan(AccelStepper::DRIVER,PAN_STEP,PAN_DIR);
AccelStepper tilt(AccelStepper::DRIVER,TILT_STEP,TILT_DIR);
AS5600 encPan; // Wire
AS5600 encTilt; // Wire1
Preferences prefs;
AsyncWebServer server(80);
AsyncEventSource events("/events");
GamepadPtr pad;

struct Keyframe{float pan;float tilt;};
Keyframe kf[4];
float dwell=1.0f;
float panMin=-180,panMax=180,tiltMin=-90,tiltMax=90;
float velTrim=1.0f;
bool seq=false;
uint8_t seqIdx=0;
unsigned long holdStart=0;

class Callbacks: public BP32::Callbacks{
  void onConnectedGamepad(GamepadPtr gp) override{pad=gp;}
  void onDisconnectedGamepad(GamepadPtr gp) override{if(pad==gp) pad=nullptr;}
} cb;

float clamp(float v,float lo,float hi){return v<lo?lo:(v>hi?hi:v);} 

void readEncoders(){
#ifdef USE_I2C_MUX
  muxSelect(0);
#endif
  float p=encPan.getAngleDegrees()-180.0f;
#ifdef USE_I2C_MUX
  muxSelect(1);
#endif
  float t=encTilt.getAngleDegrees()-180.0f;
  pan.setCurrentPosition(p*STEPS_PER_DEG);
  tilt.setCurrentPosition(t*STEPS_PER_DEG);
}

void loadPrefs(){
  prefs.begin("pt",true);
  prefs.getBytes("kf",&kf,sizeof(kf));
  dwell=prefs.getFloat("dwell",1.0f);
  panMin=prefs.getFloat("panMin",-180);
  panMax=prefs.getFloat("panMax",180);
  tiltMin=prefs.getFloat("tiltMin",-90);
  tiltMax=prefs.getFloat("tiltMax",90);
  prefs.end();
}

void savePrefs(){
  prefs.begin("pt",false);
  prefs.putBytes("kf",&kf,sizeof(kf));
  prefs.putFloat("dwell",dwell);
  prefs.putFloat("panMin",panMin);
  prefs.putFloat("panMax",panMax);
  prefs.putFloat("tiltMin",tiltMin);
  prefs.putFloat("tiltMax",tiltMax);
  prefs.end();
}

void rumble(uint8_t patt){
  if(!pad) return;
  switch(patt){
    case 0: pad->playDualRumble(0x40,0x40,80); break; // short
    case 1: pad->playDualRumble(0x40,0x40,80); delay(120); pad->playDualRumble(0x80,0x80,200); break;
    case 2: for(int i=0;i<3;i++){pad->playDualRumble(0x80,0x80,70); delay(150);} break;
    case 3: pad->playDualRumble(0x80,0x80,400); break; // wipe
  }
}

void storeKF(uint8_t idx){kf[idx]={pan.currentPosition()/STEPS_PER_DEG,tilt.currentPosition()/STEPS_PER_DEG};savePrefs();rumble(1);} 
void recallKF(uint8_t idx){pan.moveTo(clamp(kf[idx].pan,panMin,panMax)*STEPS_PER_DEG);tilt.moveTo(clamp(kf[idx].tilt,tiltMin,tiltMax)*STEPS_PER_DEG);rumble(0);} 

void processPad(){
  if(!pad||!pad->isConnected()) return;
  int lx=pad->axisX();
  int ly=pad->axisY();
  float vx=powf(lx/512.0f,3)*velTrim*600;
  float vy=powf(ly/512.0f,3)*velTrim*600;
  pan.setSpeed(vx); pan.runSpeed();
  tilt.setSpeed(vy); tilt.runSpeed();
  if(pad->dpadLeft())  pan.move(-50);
  if(pad->dpadRight()) pan.move( 50);
  if(pad->dpadUp())    tilt.move( 50);
  if(pad->dpadDown())  tilt.move(-50);
  if(pad->buttonR3()) { pan.moveTo(0); tilt.moveTo(0); }
  velTrim=0.2f+2.8f*(pad->throttleR()+1)/2.0f;
  if(pad->buttonStart() && pad->buttonSelect() && millis()-holdStart>10000){ memset(kf,0,sizeof(kf)); savePrefs(); rumble(3); holdStart=millis(); }
}

void handleButtons(){
  if(!pad||!pad->isConnected()) return;
  static uint32_t press[4]={0};
  uint8_t btns[4]={pad->a(),pad->b(),pad->x(),pad->y()};
  for(int i=0;i<4;i++){
    if(btns[i] && !press[i]) press[i]=millis();
    else if(!btns[i] && press[i]){
      if(millis()-press[i]>1000) storeKF(i); else recallKF(i);
      press[i]=0;
    }
  }
  if(pad->buttonStart()){ if(!seq && millis()-holdStart>2000){ seq=true; seqIdx=0; recallKF(0); holdStart=millis(); } }
  else if(seq && (!pad->buttonStart() || abs(pad->axisX())>10 || abs(pad->axisY())>10)) seq=false;
}

void sequenceTask(){
  if(!seq) return;
  if(!pan.isRunning() && !tilt.isRunning() && millis()-holdStart>dwell*1000){
    seqIdx=(seqIdx+1)%4; recallKF(seqIdx); holdStart=millis();
  }
}

void sendState(){
  static unsigned long last=0; if(millis()-last<200) return; last=millis();
  String json="{"; json+="\"pan\":"+String(pan.currentPosition()/STEPS_PER_DEG)+","; json+="\"tilt\":"+String(tilt.currentPosition()/STEPS_PER_DEG)+"}"; events.send(json.c_str(),"state");
}

void setupWeb(){
  server.on("/",HTTP_GET,[](AsyncWebServerRequest*r){r->send_P(200,"text/html",index_html);});
  server.addHandler(&events);
  server.on("/api/state",HTTP_GET,[](AsyncWebServerRequest*r){String s="{";s+="\"seq\":"+(seq?"true":"false")+"}";r->send(200,"application/json",s);});
  server.on("/api/center",HTTP_POST,[](AsyncWebServerRequest*r){pan.moveTo(0);tilt.moveTo(0);r->send(200);});
  server.on("/api/sequence/start",HTTP_POST,[](AsyncWebServerRequest*r){seq=true;seqIdx=0;recallKF(0);r->send(200);});
  server.on("/api/sequence/stop",HTTP_POST,[](AsyncWebServerRequest*r){seq=false;r->send(200);});
  server.on("/api/keyframe/store/",HTTP_POST,[](AsyncWebServerRequest*r){int id=r->pathArg(0).toInt();if(id<4) storeKF(id);r->send(200);});
  server.on("/api/keyframe/recall/",HTTP_POST,[](AsyncWebServerRequest*r){int id=r->pathArg(0).toInt();if(id<4) recallKF(id);r->send(200);});
  server.on("/api/wipe",HTTP_POST,[](AsyncWebServerRequest*r){memset(kf,0,sizeof(kf));savePrefs();rumble(3);r->send(200);});
  server.on("/api/params",HTTP_POST,[](AsyncWebServerRequest*r){
    if(r->hasParam("panMin",true)) panMin=r->getParam("panMin",true)->value().toFloat();
    if(r->hasParam("panMax",true)) panMax=r->getParam("panMax",true)->value().toFloat();
    if(r->hasParam("tiltMin",true)) tiltMin=r->getParam("tiltMin",true)->value().toFloat();
    if(r->hasParam("tiltMax",true)) tiltMax=r->getParam("tiltMax",true)->value().toFloat();
    if(r->hasParam("dwell",true)) dwell=r->getParam("dwell",true)->value().toFloat();
    savePrefs(); r->send(200);});
  server.on("/api/params",HTTP_GET,[](AsyncWebServerRequest*r){String j="{";j+="\"panMin\":"+String(panMin)+",";j+="\"panMax\":"+String(panMax)+",";j+="\"tiltMin\":"+String(tiltMin)+",";j+="\"tiltMax\":"+String(tiltMax)+",";j+="\"dwell\":"+String(dwell)+"}";r->send(200,"application/json",j);});
  server.begin();
}

void setup(){
  Serial.begin(115200);
  Wire.begin(21,22);
  Wire1.begin(25,26);
  encPan.begin(Wire); encTilt.begin(Wire1);
  pan.setMaxSpeed(2000); pan.setAcceleration(1000); pinMode(PAN_EN,OUTPUT);digitalWrite(PAN_EN,LOW);
  tilt.setMaxSpeed(2000); tilt.setAcceleration(1000); pinMode(TILT_EN,OUTPUT);digitalWrite(TILT_EN,LOW);
  loadPrefs();
  readEncoders();
  BP32::setup(&cb);
  setupWeb();
  ArduinoOTA.begin();
}

void loop(){
  BP32::update();
  processPad();
  handleButtons();
  pan.run(); tilt.run();
  sequenceTask();
  sendState();
  ArduinoOTA.handle();
}
