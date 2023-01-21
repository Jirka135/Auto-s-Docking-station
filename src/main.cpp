#include <M5Stack.h>
#include "bala.h"
#include "PinDefinitionsAndMore.h" 
#include <ir/IRremote.hpp>

Bala bala;

int mode = 0; 
bool jede = false;
bool hleda = false;
int speed1 = 0;
int speed2 = 0;
char vratka;
int zalozni_speed = 0;

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

void text(bool jede,int mode,int batery,bool hleda,char vratka,int speed1,int speed2){
    TFT_eSprite s = TFT_eSprite(&M5.Lcd);
    s.createSprite(280,180);
    s.fillSprite(TFT_BLACK);
    s.setTextSize(3);
    s.setTextFont(1);
    s.setCursor(10,0);
    if(jede == false){
        s.print("Nejede");
    }
    if(jede == true){
        s.print("Jede");
    }
    s.setCursor(10,30);
    s.print(mode);
    s.setCursor(10,60);
    s.print(batery);
    s.setCursor(10,90);
    if(hleda == 0){
        s.print("Nehleda");
    }else{
        s.print("Hledam");
    }
    s.setCursor(10,120);
    s.setTextSize(2);
    s.print("rychlosti: ");
    s.setCursor(130,120);
    s.print(speed1);
    s.setCursor(180,120);
    s.print(speed2);
    s.setCursor(10,150);
    s.print(vratka);
    s.pushSprite(0,0);
    s.deleteSprite();
}

void setup() {
    M5.begin();
    Serial.begin(115200);
    delay(500);
    Wire.begin();
    // Just to know which program is running on my Arduino
    Serial.println("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE);

    // Start the receiver and if not 3. parameter specified, take LED_BUILTIN pin from the internal boards definition as default feedback LED
    IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);

    Serial.print("Ready to receive IR signals of protocols: ");
    printActiveIRProtocols(&Serial);
    M5.Lcd.setTextFont(1);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE);
}

void loop() {
    if (IrReceiver.decode()) {

        // Print a short summary of received data
        IrReceiver.printIRResultShort(&Serial);
        IrReceiver.printIRSendUsage(&Serial);
        if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
            Serial.println("Received noise or an unknown (or not yet enabled) protocol");
            // We have an unknown protocol here, print more info
            IrReceiver.printIRResultRawFormatted(&Serial, true);
        }

        IrReceiver.resume(); // Enable receiving of the next value
        int command = IrReceiver.decodedIRData.command;
        if (command == 0x19) {
            if(mode == 10){
                mode = 0;
            }else{
                mode++;
            }
            command = 0;
        }if (command == 0x1D) {
            if(mode == 0){
                mode = 10;
            }else{
                mode--;
            }
            command = 0;
        }
        delay(200);
        /* mody
        aktivace modu příkaz 0xA
        -0 stop
        -1 setting speed každé kolo
        -2 setting obě kola
        -3 ovládání
        -4 vyhledej baterku
        -5 prostě jedu
        */        
        if (mode == 0){
            bala.SetSpeed(0,0);
            jede = false;
        }
        if (mode == 1){
            //leve kolo
            if(IrReceiver.decodedIRData.command == 0x1A){
                if(speed1 == 900){
                    speed1 = 0;
                }else{
                    speed1 += 100;
                }
            }
            if(IrReceiver.decodedIRData.command == 0x1E){
                if(speed1 == -900){
                    speed1 = 0;
                }else{
                    speed1 -= 100;
                }
            }
            //prave kolo
            if(IrReceiver.decodedIRData.command == 0x1B){
                if(speed2 == 900){
                    speed2 = 0;
                }else{
                    speed2 += 100;
                }
            }
            if(IrReceiver.decodedIRData.command == 0x1F){
                if(speed2 == -900){
                    speed2 = 0;
                }else{
                    speed2 -= 100;
                }
            }
        }
        if (mode == 2){
            if(IrReceiver.decodedIRData.command == 0x1A){
                if(zalozni_speed == 900){
                    zalozni_speed = 0;
                }else{
                    zalozni_speed += 100;
                }
            }
            if(IrReceiver.decodedIRData.command == 0x1E){
                if(zalozni_speed == -900){
                    zalozni_speed = 0;
                }else{
                    zalozni_speed -= 100;
                }
            }
            speed1 = zalozni_speed;
            speed2 = zalozni_speed;

            
        }
        if(mode == 5){
            int command  = IrReceiver.decodedIRData.command;
            if(jede == false && command  == 0xA){
                bala.SetSpeed(speed1,speed2);
                jede = true;
                Serial.print("nejede");
                command = 0;
            }if(jede == true && command== 0xA){
                bala.SetSpeed(0,0);
                jede = false;
                Serial.print("jede");
                command = 0;
            }
            delay(1000);
        }
    }

    vratka = IrReceiver.decodedIRData.command;
    text(jede,mode,getBatteryLevel(),hleda,vratka,speed1,speed2);
}