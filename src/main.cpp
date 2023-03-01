//MAC 24:0A:C4:F9:56:94
#include <esp_now.h>
#include <WiFi.h>
#include <M5Stack.h>
#include "bala.h"
#include "PinDefinitionsAndMore.h" 
#include <ir/IRremote.hpp>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <Wire.h>
#include <cstdio>
#include <Unit_Sonic.h>
#include <AsyncTaskLib.h>

//Proměnné

int Pa, Pd, Pp, La, Lp, Ld;
int command = 0;
int i = 0;
bool hleda = false;
bool nasel = false;
bool nabijim = false;
uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xAD, 0x45, 0xD4};

SONIC_I2C sensor;
Bala bala;

const unsigned long debounceDelay = 1000;
unsigned long lastActivationTime = 0;
bool previousHleda = false;

TFT_eSprite display = TFT_eSprite(&M5.Lcd);

int8_t getBatteryLevel()
{
  Wire.beginTransmission(0x75);
  Wire.write(0x78);
  if (Wire.endTransmission(false) == 0
  && Wire.requestFrom(0x75, 1)) {
    switch (Wire.read() & 0xF0) {
    case 0xE0: return 25;
    case 0xC0: return 50;
    case 0x80: return 75;
    case 0x00: return 100;
    default: return 0;
    }
  }
  return -1;
}

// Funkce příchod dat

int16_t leva;
int16_t prava;

//přepsání dat pro použítí funkcí
void s_motor(int16_t lleva,int16_t pprava){
  leva = lleva;
  prava = pprava;
}

//příjmání dat
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  std::stringstream data((const char*)incomingData);
  data >> Pa >> Pd >> Pp >> La >> Lp >> Ld;
  if (!hleda) {
    int prevod = 5;

    if (Pa > 270 || Pa < 90) {
      Pd *= -1;
    }

    if (La > 270 || La < 90) {
      Ld *= -1;
    }

    prava = Pd * prevod;
    leva = Ld * prevod;

    s_motor(leva, prava);
  }
  
  i++;
}

//funkce k ovládání rychlosti
void Motor(int16_t l,int16_t p) {
  bala.SetSpeed(l,p);
}
//asynchroní funkce
AsyncTask task(10, true, []() { Motor(leva, prava); });
//zastavení auta
void BalaStop(){
  prava = 0;
  leva = 0;
}
//výpis na display
void vypis(const char *text,int posx,int posy){
  display.setCursor(posx, posy);
  display.drawString(text, posx, posy);
  display.pushSprite(0, 0);
}
unsigned long lastExecutionTime = 0;
unsigned long executionInterval = 100;
int smer = 0; //0 = prava 1 =
void hledani(int prikaz,int dalka,int batery){

  if(prikaz == 0xA && !nasel){
    BalaStop();
    nasel = true;
    display.fillSprite(TFT_BLACK);                      
    vypis("nasel",10,10);
  }
  if (nasel && dalka > 10 && prikaz == 0xA && !nabijim){
    s_motor(-250,-250);

  }
  if(!nabijim && dalka <= 10){
    nabijim = true;
  }
  else{    
    unsigned long currentMillis = millis();
    if (currentMillis - lastExecutionTime > executionInterval) {
      display.fillSprite(TFT_BLACK);
      vypis("hledam",10,10);
      s_motor(-500,-500);
      lastExecutionTime = currentMillis;
    }
  }
  if(prikaz == 0x12){
    nasel = false;
    ESP.restart();
  }
}

void setup()
{

  M5.begin();
  Wire.begin();
  sensor.begin();
  WiFi.mode(WIFI_STA);

  // IR receiver
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.begin(115200);

  Serial.print("Ready to receive IR signals of protocols: ");
  printActiveIRProtocols(&Serial);

  // display
  display.createSprite(300, 180);
  display.fillSprite(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(3);

  // ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_peer_info_t peer_info;
  memset(&peer_info, 0, sizeof(peer_info));
  for (int i = 0; i < 6; ++i) {
    peer_info.peer_addr[i] = (uint8_t) broadcastAddress[i];
  }
  peer_info.channel = 0;
  peer_info.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peer_info) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  esp_now_register_recv_cb(OnDataRecv);

  task.Start();

  display.fillSprite(TFT_BLACK);
  vypis("Hledam kamarada", 10, 10);
  delay(1000);
}
int bat;
bool prvni = true;
void loop()
{
  task.Update();
  bat = getBatteryLevel();
  if (i >= 500 || prvni){
    char baterka[10];
    sprintf(baterka, "%d", bat);
    const char* outgoingData = baterka;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*) outgoingData, strlen(outgoingData) + 1);
    i = 0;
    display.fillSprite(TFT_BLACK);
    vypis(baterka,10,40);
    vypis("Povidame si",10,10);
    prvni = false;
  }

  if (IrReceiver.decode()) {
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.println("Received noise or an unknown (or not yet enabled) protocol");
      IrReceiver.printIRResultRawFormatted(&Serial, true);
    }
    IrReceiver.resume();
    command = IrReceiver.decodedIRData.command;
  }

  if (millis() - lastActivationTime > debounceDelay && Pp == 1 && Lp == 1 && !hleda) {
    BalaStop();
    Pp = 0;
    Lp = 0;
    Serial.println("hleda");
    hleda = true;
    previousHleda = true;
    lastActivationTime = millis();
  }
  if (millis() - lastActivationTime > debounceDelay && Pp == 1 && Lp == 1 && hleda && previousHleda) {
    hleda = false;
    Serial.println("nehleda");
    nasel = false;
    Pp = 0;
    Lp = 0;
    prvni = true;
    previousHleda = false;
    lastActivationTime = millis();
  }

  

  if (hleda) {
    float distance = sensor.getDistance();
    int cm = distance / 10;
    BalaStop();
    hledani(command, cm,bat);
  }

  if (command != 0) {
    Serial.print(command);
    command = 0;
  }
}