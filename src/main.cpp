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

const char* ssid = "jirka";
const char* password = "jirkajebest";

int Pa,Pd,Pp,La,Ld,Lp;

using namespace std;

TFT_eSprite display = TFT_eSprite(&M5.Lcd);

WiFiServer server(80);

Bala bala;

void vypis(const char *text){
  display.fillSprite(TFT_BLACK);
  display.setCursor(10, 10);
  display.drawString(text, 0, 0);
  display.pushSprite(0, 0);
}

void setup() {
  M5.begin();
  Wire.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  display.createSprite(300,100);
  display.fillSprite(TFT_BLACK);
  display.setTextColor(TFT_WHITE);
  display.setTextSize(3);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    vypis("pripojuji");
  }
  
  server.begin();
  Serial.println("zapinam server");
  vypis("zapinam server");
  Serial.println(WiFi.localIP());
  
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    vypis("Klient není pripojen");
    return;
  }

  while (client.connected()) {
    vypis("klient pripojen");
    if (client.available()) {
      String incomingMessage = client.readStringUntil('\n');
      std::stringstream data(incomingMessage.c_str());

      /*
      A = úhel
      D = délka
      P = stav tlačítka
      */

      data >> Pa>>Pd>>Pp>>La>>Ld>>Lp;
      //dopředu rozsah 270-90
      //dozadu 268-90
      /*
      if (Pa > 270 && Pa < 90){
        Pd = Pd*1;
      }else if(Pa < 270 && Pa > 90){
        Pd = Pd*-1;
      }else if(Pa == 0){
        Pd = 0;
      }

      if (La > 270 && La < 90){
        Ld = Ld*1;
      }else if(La < 270 && La > 90){
        Ld = Ld*-1;
      }else if(La == 0){
        Ld = 0;
      }
      */
      int16_t prava = Pd*5;
      int16_t leva = Ld*5;
      bala.SetSpeed(leva,prava);
      Serial.print(Pa);
      Serial.print(",");
      Serial.print(Pd);
      Serial.print("           ");
    }
  }
}