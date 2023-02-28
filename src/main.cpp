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
#include <TaskScheduler.h>

int Pa, Pd, Pp, La, Lp, Ld;
int hleda;
int command = 0;
int i = 0;
bool nasel = false;
uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xAD, 0x45, 0xD4};

SONIC_I2C sensor;

Task motor(100, TASK_FOREVER, &Motor);

Bala bala;

using namespace std;
TFT_eSprite display = TFT_eSprite(&M5.Lcd);

unsigned long lastChangeTime = 0;
const unsigned long debounceDelay = 2000;

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
int16_t prava;
int16_t leva;
// Funkce příchod dat

void Motor(int leva,int prava){
  bala.SetSpeed(leva,prava)
}

void BalaStop(){
  prava = 0;
  leva = 0;
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  //větší převod = rychlejší auto (max 5 - min 1)
  int prevod = 6;
  std::stringstream data;
  std::string prichozi = (const char*)incomingData;
  //Serial.println(prichozi.c_str());
  data.str(prichozi);
  //Serial.println(data.str().c_str());

  data >> Pa >> Pd >> Pp >> La >> Lp >> Ld;

  if(hleda == 0){
    if (Pa > 270 || Pa < 90) {
      Pd *= -1;
    } else {
      Pd *= 1;
    }

    if (La > 270 || La < 90) {
      Ld *= -1;
    } else {
      Ld *= 1;
    }

    prava= Pd*prevod;
    leva= Ld*prevod;
  }
  i++;
  /* DEBUG
  Serial.print("prichozi: ");
  Serial.print("prava: ");
  Serial.println(prava);
  Serial.print("leva: ");
  Serial.println(leva);
  Serial.print("data: ");
  Serial.println(data.str().c_str());
  */
}



void vypis(const char *text,int posx,int posy){
  display.setCursor(posx, posy);
  display.drawString(text, posx, posy);
  display.pushSprite(0, 0);
}

void hledani(int prikaz,int dalka){
  if(nasel == false){
    display.fillSprite(TFT_BLACK);
    vypis("hledam",10,10);
    bala.SetSpeed(800,-800);
    delay(100);
  }
  if(prikaz == 0xA && nasel == false){
    BalaStop();
    nasel = true;
    display.fillSprite(TFT_BLACK);                      
    vypis("neasel",10,10);
  }
  if (nasel == true && prikaz != 0x12 && dalka > 10){
    bala.SetSpeed(500,500);
  }
  if(prikaz == 0x12 && nasel == true){
    nasel = false;
  }
}

void setup()
{
  M5.begin();
  Wire.begin();
  sensor.begin();
  delay(500);
  motor.enable();
  Serial.println("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE);
  // Init Serial Monitor
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.begin(115200);
  Serial.print("Ready to receive IR signals of protocols: ");
  printActiveIRProtocols(&Serial);
  vypis("Start",10,10);
  display.createSprite(300,180);
  display.fillSprite(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(3);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  for (int ii = 0; ii < 6; ++ii )
  {
    peerInfo.peer_addr[ii] = (uint8_t) broadcastAddress[ii];
  }
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  display.fillSprite(TFT_BLACK);
  vypis("Hledam kamarada",10,10);
  esp_now_register_recv_cb(OnDataRecv);
}
int bat;
void loop()
{

  // Send a message
  float distance = sensor.getDistance();
  int cm = distance / 10;
  Serial.println(cm);

  // konec
  if (i >= 250){
    bat = getBatteryLevel();
    char baterka[10];
    sprintf(baterka, "%d", bat);
    const char* outgoingData = baterka;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*) outgoingData, strlen(outgoingData) + 1);
    i = 0;
    delay(100);
    display.fillSprite(TFT_BLACK);
    vypis(baterka,10,40);
    vypis("Povidame si",10,10);
  }

  if (IrReceiver.decode()) {
    IrReceiver.printIRResultShort(&Serial);
    IrReceiver.printIRSendUsage(&Serial);
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
        Serial.println("Received noise or an unknown (or not yet enabled) protocol");
        // We have an unknown protocol here, print more info
        IrReceiver.printIRResultRawFormatted(&Serial, true);
    }
    IrReceiver.resume(); // Enable receiving of the next value
    command = IrReceiver.decodedIRData.command;
  }

  if (Pp == 1 && Lp == 1 && hleda == 0 && millis() - lastChangeTime > debounceDelay) {
    BalaStop();
    Serial.println("změna");
    hleda = 1;
    lastChangeTime = millis();
  }

  if (Pp == 1 && Lp == 1 && hleda == 1 && millis() - lastChangeTime > debounceDelay) {
    hleda = 0;
    Serial.println("změna");
    lastChangeTime = millis();
  }


  Serial.print(hleda);
  if (hleda == 1){
    hledani(command,cm);
  }
  if (command != 0){
    Serial.print(command);
    command = 0;
  }
  bala.SetSpeed(leva,prava);
  // Wait for 10 seconds before sending the next message
  //delay(1000);
}