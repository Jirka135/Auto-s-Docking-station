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
#include <wire.h>
#include <cstdio>

 int Pa, Pd, Pp, La, Lp, Ld;

uint8_t broadcastAddress[] = {0x94, 0xB9, 0x7E, 0xAD, 0x45, 0xD4};

Bala bala;

using namespace std;
TFT_eSprite display = TFT_eSprite(&M5.Lcd);
// Funkce příchod dat
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len)
{
  //větší převod = rychlejší auto (max 5 - min 1)
  int prevod = 5;
  std::stringstream data;
  std::string prichozi = (const char*)incomingData;
  Serial.println(prichozi.c_str());
  data.str(prichozi);
  Serial.println(data.str().c_str());

  data >> Pa >> Pd >> Pp >> La >> Lp >> Ld;

  
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

  int16_t prava = Pd*prevod;
  int16_t leva = Ld*prevod;

  bala.SetSpeed(leva,prava);

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

void setup()
{
  // Init Serial Monitor
  Serial.begin(115200);
  vypis("Start",10,10);
  M5.begin();
  Wire.begin();
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

void loop()
{

  // Send a message

  display.fillSprite(TFT_BLACK);
  vypis("Povidame si",10,10);
  float batteryLevel = M5.Power.getBatteryLevel();
  Serial.print("Battery level: ");
  char myText[16];
  sprintf(myText, "%.2f", batteryLevel);
  vypis(myText,10,40);
  Serial.println(batteryLevel);
  /*const char* outgoingData = "1";
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t*) outgoingData, strlen(outgoingData) + 1);
  if (result == ESP_OK) {
    //Serial.println("odesláno");
  }
  */
  // Wait for 5 seconds before sending the next message
  delay(1000);
}