//https://homemadegarbage.com/pov-esp32-03
#include <SPI.h>
#include <Adafruit_DotStar.h>
#include "WiFi.h"
#include "AsyncUDP.h"


const char * ssid = "ssid";
const char * password = "pass";

AsyncUDP udp;


#define NUMPIXELS 24
#define Div 100 //pythonと合わせる

#define itrPin 33

uint32_t pic[Div][NUMPIXELS] = {0, };


int numDiv = 0;
int stateRot = 0;
int stateDiv = 0;
float rpm;
#define th_SENSORS 1000

unsigned long rotTime, timeOld, timeNow;

#define DATAPIN    23
#define CLOCKPIN   18
Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);

char chararrayDiv[] = "0x00";
char chararrayColor[] = "0xffffff";


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi Failed");
    while (1) {
      delay(1000);
    }
  }

  //UDP受信
  if (udp.listen(1234)) {
    Serial.print("UDP Listening on IP: ");
    Serial.println(WiFi.localIP());
    udp.onPacket([](AsyncUDPPacket packet) {
      chararrayDiv[2] = packet.data()[0];
      chararrayDiv[3] = packet.data()[1];
      Serial.print("strtoul=");
      Serial.println(int(strtoul(chararrayDiv, NULL, 16)));

      for (int i = 0; i < NUMPIXELS ; i++) {
        for (int j = 0; j < 6 ; j++) {
          chararrayColor[j + 2] = packet.data()[2 + i * 6 + j];
        }
        pic[int(strtoul(chararrayDiv, NULL, 16))][i] = strtoul(chararrayColor, NULL, 16);
      }
    });
  }

  strip.begin();
  strip.clear();
  strip.show();

  pinMode(itrPin, INPUT);
  delay(500);
}

void loop() {
  if (stateRot == 0 && analogRead(33) < th_SENSORS) {
    timeNow = micros();
    timeOld = timeNow;
    stateRot = 1;
  } else if (stateRot == 2 && analogRead(33) < th_SENSORS) {
    stateRot = 3;
  } else if (stateRot == 4 && analogRead(33) < th_SENSORS) {
    stateRot = 0;
    timeNow = micros();
    rotTime = timeNow - timeOld;

//    フォトリフレクタからrpm計算
//    Serial.print("rotTime="); Serial.println(rotTime);
//    rpm = 60 * 1000 * 1000 * 1 / rotTime; //rotation per minute
//    Serial.print("rpm="); Serial.println(rpm);
  }
  if (stateRot == 1 && analogRead(33) > th_SENSORS) {
    stateRot = 2;
  }
  if (stateRot == 3 && analogRead(33) > th_SENSORS) {
    stateRot = 4;
  }
  
  if (stateDiv == 1 && micros() - timeOld > rotTime / Div * (numDiv)) {
    stateDiv = 0;
  }

  if (stateDiv == 0 && micros() - timeOld < rotTime / Div * (numDiv + 1)) {
    stateDiv = 1;

    strip.clear();

    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, pic[numDiv][i]);
    }

    strip.show();

    numDiv++;
    if (numDiv >= Div ) numDiv = 0;

  }
}
