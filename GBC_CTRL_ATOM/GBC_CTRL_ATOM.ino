// GBCコントローラー with H-Driver by Kikyoya(Tamotsu Kamigaki) (C) 2022-23.
// 2023/11/22 H-Driverバージョン 

#include <M5Atom.h>  // M5UnifiedではM5.begin()で一瞬モータが回転するため断念

/* 
Groveにスライダーボリューム取り付け
ATOM本体のボタンで緊急停止、リセットで再起動
// ATOM I/O
// GND 5V G25 G21       // PL:G25
// G33 G23 G19 G22 3V3  // SP:G19-G33(GND)
// Grove G32 G26 5V GND
// HAT GND 5V G22 G33 G19 NC 3V3 NC
*/
#define MaxPort 2
#define SLEDi 27    // 内蔵RGB LED（実装済みLED）ATOM/Stamp PICO:G27 C3/U:G2
#define SLEDs 26    // Grove Slider RGB LED
#define slPin 32    // Grove Slider VR
//////////////////////////////////////////////////////////////////////////
// for SK6812
//
#include <Adafruit_NeoPixel.h>
// 内蔵LED用
//#define SLEDi 27    // 内蔵RGB LED（実装済みLED）ATOM/Stamp PICO:G27 C3/U:G2
Adafruit_NeoPixel Pxi(1, SLEDi);  // 実装済みsLED C3/U:2 PICO:27
#define InitLEDi()    Px.begin()  // initialize the pixel
#define SetLEDi(col)  Pxi.setPixelColor(0, col)
#define ShowLEDi()    Pxi.show()
#define DimLEDi(x)    Pxi.setBrightness(x)  // 全体の輝度を調整0-255
// 外部LED用
//#define SLEDs 32    // Grove RGB LED（LED）ATOM/Stamp PICO:G32, Stamp C3/U:G1
#define NUM_LEDS 14   // スライダー内蔵LEDの個数
Adafruit_NeoPixel Px(NUM_LEDS, SLEDs, NEO_GRB + NEO_KHZ800);
#define InitLEDs()  Px.begin()  // initialize the pixel
#define SetLEDs(n,col)  Px.setPixelColor(n, col)
#define ShowLEDs()  Px.show()
#define DimLEDs(x) Px.setBrightness(x)  // 全体の輝度を調整0-255
// RGB
const uint32_t LED_GREEN = 0x001f00;
const uint32_t LED_YELLOW = 0x0f0f00;
const uint32_t LED_RED = 0x1f0000;
const uint32_t LED_BLUE = 0x00001f;
uint32_t LedCol = LED_GREEN;

// Motor Driver関連
const int IN1_PIN = 19;
const int IN2_PIN = 23;
int freq          = 10000;  // これ以上低いとキーン音が気になるが低速時のトルクはある
int ledChannel1   = 0;
int ledChannel2   = 1;
int resolution    = 7;
int VIN_PIN       = 33;
int FAULT_PIN     = 22;
void mdInit(){
    ledcSetup(ledChannel1, freq, resolution);
    ledcSetup(ledChannel2, freq, resolution);
    ledcAttachPin(IN1_PIN, ledChannel1);
    ledcAttachPin(IN2_PIN, ledChannel2);
    pinMode(VIN_PIN, INPUT);
    pinMode(FAULT_PIN, INPUT);
}
void mdSetMotorSpeed(int8_t sp){
  if(sp==0){   // Break
    ledcWrite(ledChannel1, 128);
    ledcWrite(ledChannel2, 128);
  }else if(sp>0){   // 正転
    ledcWrite(ledChannel1, sp);
    ledcWrite(ledChannel2, 0);
  }else{      // 反転
    ledcWrite(ledChannel1, 0);
    ledcWrite(ledChannel2, -sp);
  }
}

void setup()
{
  M5.begin(false,false,false);  // Serial, I2C, FastLED
  mdInit();
  pinMode(slPin, INPUT);
  InitLEDi();
  DimLEDi(100);
  InitLEDs();
  DimLEDs(100);
  SetLEDi(LED_RED);
  ShowLEDi();
  SetLEDs(3,LED_RED);
  ShowLEDs();
}

void loop()
{
  static bool onoff=false; // 接続中か？
  M5.update();
  int adcValue = analogRead(slPin);
  int8_t spd = 0;   // 0-4095 => -127-0-127, 1778-2317は0にしてセンターをとる（補正）
//    if(adcValue<1778) spd = adcValue/14 - 127;  // 0-1778 => -127-0
//    else if(adcValue>2317) spd = (adcValue-2317)/14;  // 2317-4095 => 0-127
  if(adcValue<1663) spd = adcValue/13 - 127;  // 0-1663 => -127-0
//    else if(adcValue>2432) spd = (adcValue-2432)/13;  // 2432-4095 => 0-127
  else if(adcValue>2560) spd = (adcValue-2560)/12;  // 2560-4095 => 0-127
  if(spd==0) onoff=true;
  if(onoff){   // 接続中ならモーターON、LED ON
    SetLEDi(LED_GREEN);
    ShowLEDi();
    SetLEDs(3,LED_GREEN);
    ShowLEDs();
    mdSetMotorSpeed(spd);
  }else{
    mdSetMotorSpeed(0);
  }
//    Serial.printf("%4d",spd);//base_x.GetMotorSpeed(i+1));
  if(M5.Btn.wasPressed()){
      onoff = false;  // 緊急停止
      mdSetMotorSpeed(0);
      SetLEDi(LED_RED);
      ShowLEDi();
      SetLEDs(3,LED_RED);
      ShowLEDs();
  }
  if(digitalRead(FAULT_PIN) == 0){  // 過電流などでモータドライバ停止
      SetLEDi(LED_RED);
      ShowLEDi();
  }
  delay(10);
}
