#include "arduino_secrets.h"
#include "thingProperties.h"
#include "lcdgfx.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TimeLib.h>

#define ONE_WIRE_BUS 9
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

uint8_t sensor1[8] = { 0x28, 0x4D, 0x1E, 0x1B, 0x0F, 0x00, 0x00, 0x0C };
uint8_t sensor2[8] = { 0x28, 0xBA, 0x02, 0x1C, 0x0F, 0x00, 0x00, 0x20 };
uint8_t sensor3[8] = { 0x28, 0x1A, 0xE1, 0x1B, 0x0F, 0x00, 0x00, 0x6E };
uint8_t sensor4[8] = { 0x28, 0x5F, 0x24, 0x1B, 0x0F, 0x00, 0x00, 0x00 };
uint8_t sensor5[8] = { 0x28, 0xFB, 0xB5, 0x1B, 0x0F, 0x00, 0x00, 0xFA };
uint8_t sensor6[8] = { 0x28, 0x17, 0xC0, 0x1B, 0x0F, 0x00, 0x00, 0x11 };
uint8_t sensor7[8] = { 0x28, 0x9A, 0x0F, 0x1B, 0x0F, 0x00, 0x00, 0xFC };

DisplaySSD1306_128x64_I2C display(-1);

char s[40];
char t[40];
char u[40];
char v[40];
int tempExt = 999;
float tempS1L = 0;
float tempS1H = 0;
int tempS1 = 0;
float tempS2L = 0;
float tempS2H = 0;
int tempS2 = 0;
float tempS3L = 0;
float tempS3H = 0;
int tempS3 = 0;
int tempFin = 0;
float tempDiffS1 = 0;
float tempDiffS2 = 0;
float tempDiffS3 = 0;
unsigned long ventTime = 0;
/*bool startDelay = true;*/
unsigned long TimeH = 0;
unsigned long Time = 0;
unsigned long StartTime = 0;
unsigned long TimeCount = 0;
unsigned long EndTime = 0;
unsigned long getCloudTime = 1;
/*unsigned long BlinkPreviousMillis = 0;
const long  BlinkIntervall = 250;
const long  BlinkTime = 10000;
const int Blinker = 9;
int BlinkState = LOW;*/
bool counter = true;
bool counter1 = false;
bool on = false;
const long DisplayClearIntervall = 10000;
const long CheckCloudConnectionIntervall = 600000;
unsigned long DisplayClearIntervallPreviousMillis = 0;
unsigned long CheckCloudConnectionPreviousMillis = 0;
bool ondelay = false;
unsigned long OndelayPreviousMillis = 0;
const long OndelayIntervall = 1200000 ;
unsigned long tempCheckPreviousMillis = 0;
unsigned long tempCheckIntervall = 5000;

void setup() {
  Serial.begin(9600);
  delay(1500);
  pinMode(4, INPUT);
  pinMode(8, INPUT);
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  sensors.begin();
  display.begin();
  display.fill(0x00);
  display.setFixedFont(ssd1306xled_font6x8);
  display.printFixedN (13,  30, "FU POWER", STYLE_BOLD, FONT_SIZE_2X);
  delay(5000);
  display.clear();
  digitalWrite(LED_BUILTIN, HIGH);
  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(1);
  ArduinoCloud.printDebugInfo();
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::CONNECT, doThisOnConnect);
  ArduinoCloud.addCallback(ArduinoIoTCloudEvent::DISCONNECT, doThisOnDisconnect);
}

void loop() {
  ArduinoCloud.update();
  
  checkAndclearDisplay();
  
  
  
  tempCheck();
  
  onDelay();
  
  tempFin = map(analogRead(A0), 0, 1023, -5, 10);
  
  if (ArduinoCloud.connected()) {
   getCloudTime = ArduinoCloud.getInternalTime();
   checkCloudConnection();
  }
  
  setTime(getCloudTime);
  time_t Time = now();
  TimeH = hour(Time) + 1;
  
  unsigned long previusActiveTime = millis();
  
  if ((on == true) && (counter == true)) {
    StartTime = previusActiveTime;
    counter = false;
    counter1 = true;
    debugMessengeC = "VENTILATION ON";
  } 
  
  if ((on == false) && (counter1 == true)){
    EndTime = previusActiveTime;
    TimeCount = EndTime - StartTime;
    ventTime += TimeCount;
    counter = true;
    counter1 = false;
    debugMessengeC = "VENTILATION OFF";
    
  }
  
  tempS1 = (tempS1L + tempS1H) / 2;
  tempS2 = (tempS2L + tempS2H) / 2;
  tempS3 = (tempS3L + tempS3H) / 2;
  tempDiffS1 = (tempS1 - tempExt);
  tempDiffS2 = (tempS2 - tempExt);
  tempDiffS3 = (tempS3 - tempExt);
  
  TextC = tempExt;
  TempS1C = tempS1;
  TempS2C = tempS2;
  TempS3C = tempS3;
  VentTimeC = int ((ventTime / 1000) / 60);
  onTimeC = int (millis() / 3600000); 
  onC = on;
  if (digitalRead(4) == HIGH) {
    display.begin();
    display.fill(0x00);
    display.setFixedFont(ssd1306xled_font6x8);

    display.printFixed(0, 8, "Silo 1", STYLE_NORMAL);
    display.printFixed(110, 8, "C.", STYLE_NORMAL);
    sprintf(t, "%d", int(tempS1));
    display.printFixed(90, 8, t, STYLE_NORMAL);

    display.printFixed(0, 16, "Silo 2", STYLE_NORMAL);
    display.printFixed(110, 16, "C.", STYLE_NORMAL);
    sprintf(u, "%d", int(tempS2));
    display.printFixed(90, 16, u, STYLE_NORMAL);

    display.printFixed(0, 24, "Silo 3", STYLE_NORMAL);
    display.printFixed(110, 24, "C.", STYLE_NORMAL);
    sprintf(u, "%d", int(tempS3));
    display.printFixed(90, 24, u, STYLE_NORMAL);

    display.printFixed(0, 47, "Exterieur", STYLE_NORMAL);
    display.printFixed(110, 47, "C.", STYLE_NORMAL);
    sprintf(v, "%d", int(tempExt));
    display.printFixed(90, 47, v, STYLE_NORMAL);

    display.printFixed(0, 60, "Final", STYLE_NORMAL);
    display.printFixed(110, 60, "C.", STYLE_NORMAL);
    sprintf(s, "%d", int(tempFin));
    display.printFixed(95, 60, s, STYLE_NORMAL);

    /*delay(10000);
    display.printFixedN (13,  30, "FU POWER", STYLE_BOLD, FONT_SIZE_2X);
    delay(1000);*/
  }
  
  if ((TimeH >= 6) && (TimeH <= 22)) {
    
    if((tempDiffS1 >= 7) || (tempDiffS2 >= 7) || (tempDiffS3 >= 7)){
  
      if ((tempS1 > tempFin) || (tempS2 > tempFin) || (tempS3 > tempFin)){
        
        ondelay = true;
        on = true;
        /*if (startDelay == true) {
          
          unsigned long currentMillis = millis()
          
          display.begin();
          display.fill(0x00);
          display.setFixedFont(ssd1306xled_font6x8);
          display.printFixedN (5,  30, "ATTENTION!", STYLE_BOLD, FONT_SIZE_2X);
          display.clear();
          
          startDelay = false;
        }*/
      digitalWrite(7, HIGH);
      digitalWrite(6, HIGH);
      } 
      else if (ondelay == false) {
        digitalWrite(7, LOW); digitalWrite(6, LOW); on = false;/*startDelay = true;*/}
    }
    else if (ondelay == false) {
      digitalWrite(7, LOW); digitalWrite(6, LOW); on = false;}
  } 
  else if (ondelay == false) { 
    digitalWrite(6, LOW); digitalWrite(7, LOW); debugMessengeC = "HORAIRE VENTILATION OFF";on = false;}
    
  if (digitalRead(8) == HIGH) {
  digitalWrite(7, HIGH);
  digitalWrite(6, HIGH);
  on = true;
  } 
  
}
  
void doThisOnConnect(){
  display.begin();
  display.fill(0x00);
  display.setFixedFont(ssd1306xled_font6x8);
  display.printFixed (10,  30,"CONNECTED TO CLOUD     ", STYLE_NORMAL);
  debugMessengeC = "CONNECTED TO CLOUD";
  digitalWrite(LED_BUILTIN, LOW);
  checkAndclearDisplay();
}

void doThisOnDisconnect(){
  display.begin();
  display.fill(0x00);
  display.setFixedFont(ssd1306xled_font6x8);
  display.printFixed (10,  30,"CLOUD DISCONNECTED ", STYLE_NORMAL);
  debugMessengeC = "CLOUD DISCONNECTED";
  digitalWrite(LED_BUILTIN, HIGH);
}

void checkAndclearDisplay(){
  unsigned long currentMillis = millis();
  if (currentMillis - DisplayClearIntervallPreviousMillis >= DisplayClearIntervall) {
    DisplayClearIntervallPreviousMillis = currentMillis;
    display.clear();
  }
}
    
void checkCloudConnection(){
  unsigned long currentMillis1 = millis();
  if (currentMillis1 - CheckCloudConnectionPreviousMillis >= CheckCloudConnectionIntervall ) {
    CheckCloudConnectionPreviousMillis = currentMillis1;
    debugMessengeC = "CONNECTION CHECK";
  }
}
 
void onDelay(){
  unsigned long currentMillis2 = millis();
  
  if (currentMillis2 - OndelayPreviousMillis >= OndelayIntervall){
    OndelayPreviousMillis = currentMillis2;
    ondelay = false;
  }
}

void tempCheck(){
  unsigned long currentMillis3 = millis();
  if (currentMillis3 - tempCheckPreviousMillis >= tempCheckIntervall){
    tempCheckPreviousMillis = currentMillis3;
    sensors.requestTemperatures();
    tempS1L = sensors.getTempC(sensor1);
    tempS1H = sensors.getTempC(sensor2);
    tempS2L = sensors.getTempC(sensor3);
    tempS2H = sensors.getTempC(sensor4);
    tempS3L = sensors.getTempC(sensor5);
    tempS3H = sensors.getTempC(sensor6);
    tempExt = sensors.getTempC(sensor7);
  }
}  

void onSetTextCChange()  {
  tempExt = setTextC.toInt();
}