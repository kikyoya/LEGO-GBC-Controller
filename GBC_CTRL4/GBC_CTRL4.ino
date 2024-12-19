// GBCコントローラー with BaseX by Kikyoya(Tamotsu Kamigaki) (C) 2022-23.
// 2023/01/05 M5Unified対応, 表示座標の調整
// 2023/10/20 起動時にすべて接続状態にする
// 2023/11/25 緊急停止ボタン、起動時誤動作防止追加
// 2024/12/18 PbHubライブラリの更新

#include <M5Unified.h>  // M5Unified version
const lgfx::v1::IFont* FontJP=&fonts::lgfxJapanGothic_24; // 日本語フォント
//const lgfx::v1::IFont* FontJP=&fonts::efontJA_24_b;  // efontはでかいのでPartition SchemeをNo OTAとかにする(1.5M位になる)
const lgfx::v1::IFont* Font16F=&fonts::AsciiFont8x16;  // 16dot等幅フォント
int Lcd_w=0, Lcd_h=0;   // LCD画面の幅と高さ

#include "M5UnitPbHub.h"
M5UnitPbHub pbhub;

#include "batIcon.h"  // バッテリーアイコン画像
/* Coreはバッテリーの有無を検出できず、無しでも100%充電中と判別される
  createBtIcon();   // 初期化：バッテリーアイコン用スプライト作成
  void pushBtIcon(int x,int y,Color bg=BLUE); // バッテリー情報を読み出しアイコンを書き出し
  M5.Lcd.printf("%d",M5.Power.getBatteryVoltage());   // バッテリー電圧
*/
#include "BaseXUni.h"
BASE_X base_x = BASE_X();

void setup()
{
  M5.begin();
// pbHubの初期化 Coreなら何も指定せずともよいがCore2はアドレスが違うので要指定
  pbhub.begin(&Wire, UNIT_PBHUB_I2C_ADDR, M5.Ex_I2C.getSDA(), M5.Ex_I2C.getSCL());  
// 液晶ディスプレイの設定
  M5.Lcd.setRotation(1);      // LCDの向き(0-3)
  Lcd_w=M5.Display.width(); Lcd_h=M5.Display.height();    // 画面の大きさ
  M5.Lcd.setBrightness(60);  // バックライトの明るさ(0-255)
  M5.Lcd.fillScreen(BLUE);    // バックの色
// ファンクションキー
  M5.Lcd.fillRect(0,209,Lcd_w,Lcd_h,BLACK);  // 下は黒
  M5.Lcd.setFont(FontJP);
  M5.Lcd.setTextColor(BLACK,YELLOW);// ファンクションキーの色
  M5.Lcd.drawString("　←　",31, 214);
  M5.Lcd.drawString(" 接続 ",125, 214);
  M5.Lcd.drawString("　→　",219, 214);
// メイン画面
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(YELLOW,BLUE);// 文字の色,文字背景の色
  M5.Lcd.printf("GBCコントローラー");
  M5.Lcd.setCursor(5,30);
  M5.Lcd.setTextColor(WHITE,BLUE);  // 色を元に戻す
  M5.Lcd.printf("Motor| M1 | M2 | M3 | M4");  // 見出し
// バッテリーアイコン
  createBtIcon();
// BASEXの初期設定
  for(uint8_t i = 1; i < 5; i++){
      base_x.SetMode(i, NORMAL_MODE);
  }
// スライダーの初期設定
  for(uint8_t i = 0; i < 4; i++){
    pbhub.setLEDBrightness(i, 20);
//    pbhub.setLEDColor(i,3,0x003000); // 位置は変えられない(PbHubのバグ) ch,num,RGB
  }
}

void loop(){
  static int sel=0; // 0-3:選ばれているモータ番号
  static bool onoff[4]={true,true,true,true}; // 接続中か？
  static bool onStart[4]={true,true,true,true};  // 起動時や緊急停止復帰後に突然動き出すのを防止するためチェックする
// バッテリー状態の確認
//
  pushBtIcon(Lcd_w-40, 6);
  int bt = M5.Power.getBatteryLevel();   // バッテリーレベルのみ取得可(0,25,50,75,100%)
	M5.Lcd.setCursor(Lcd_w-40-8*5,6,Font16F);				// 最上行を指定
  M5.Lcd.setTextColor(BLUE,WHITE);
  M5.Lcd.printf("%3d%%",bt);
//
  M5.Lcd.setTextColor(WHITE,BLUE);
// スピードの制御
  M5.Lcd.setCursor(5,60,FontJP);
  M5.Lcd.printf("Speed");
  for(int i = 0; i < 4; i++){
    int adcValue = pbhub.analogRead(i);
// 中央付近は幅広く0にするための処理
    int8_t spd = 0;   // 0-4095 => -127-0-127, 中心付近は0にしてセンターをとる（補正）
//    if(adcValue<1778) spd = adcValue/14 - 127;  // 0-1778 => -127-0
//    else if(adcValue>2317) spd = (adcValue-2317)/14;  // 2317-4095 => 0-127
    if(adcValue<1663) spd = adcValue/13 - 127;  // 0-1663 => -127-0
//    else if(adcValue>2432) spd = (adcValue-2432)/13;  // 2432-4095 => 0-127
    else if(adcValue>2560) spd = (adcValue-2560)/12;  // 2560-4095 => 0-127
    if(spd==0) onStart[i]=false;
    M5.Lcd.printf("|");
    if(onoff[i]){   // 接続中ならモーターON、LED ON
      M5.Lcd.setTextColor(BLUE,WHITE); // 接続中は反転
      pbhub.setLEDColor(i,3,0x003000); // 緑
      if(onStart[i]){  // 起動直後or緊急停止中
        M5.Lcd.setTextColor(RED,WHITE); // 赤文字
        pbhub.setLEDColor(i,3,0x100500); // ほんのり赤
      }else{
        base_x.SetMotorSpeed(i+1, spd);
      }
    }else{
      base_x.SetMotorSpeed(i+1, 0);
      pbhub.setLEDColor(i,3,0x000020); // ほんのり青
    }
    M5.Lcd.printf("%4d",spd);//base_x.GetMotorSpeed(i+1));
    M5.Lcd.setTextColor(WHITE,BLUE);  // 色を元に戻す
  }
// 起動時誤動作警告＆緊急停止処理(赤ボタン)
  if(!pbhub.digitalRead(4,1)){   // 緊急停止
    for(int i = 0; i < 4; i++){
      onStart[i]=true;
      base_x.SetMotorSpeed(i+1, 0);
      pbhub.setLEDColor(i,3,0x300000);
    }
    M5.Lcd.setCursor(20,120);
    M5.Lcd.setTextColor(RED,WHITE);  // 警告色
    M5.Lcd.printf("　　　緊急停止中！　　　");delay(1000);
    M5.Lcd.setTextColor(WHITE,BLUE);  // 色を元に戻す
  }    
  M5.Lcd.setCursor(20,120);
  if(onStart[0]||onStart[1]||onStart[2]||onStart[3]){
    M5.Lcd.setTextColor(RED,WHITE);  // 警告色
    M5.Lcd.printf(" 一旦0に戻してください ");
    M5.Lcd.setTextColor(WHITE,BLUE);  // 色を元に戻す
  }else
    M5.Lcd.printf(" 　　　　　　　　　　　 ");
// 選択↑の表示
  M5.Lcd.setCursor(5,90);
  M5.Lcd.printf("      ");
  for(int i = 0; i < 4; i++)
    M5.Lcd.printf(" %2s  ",(sel==i)?"↑":"　");
// 本体ボタンと青ボタンの処理
  M5.update();
  if(M5.BtnB.wasPressed()||(!pbhub.digitalRead(4,0))){
    while(!pbhub.digitalRead(4,0)) delay(10);  // チャタリング防止
    onoff[sel] = !onoff[sel];
    if(onoff[sel]) base_x.SetEncoderValue(sel+1,0); // エンコーダ・リセット
  }
  if(M5.BtnA.wasPressed()){ // ←
      sel--;
      if(sel<0) sel=3;
  }
  if(M5.BtnC.wasPressed()){ // →
      sel++;
      if(sel>=4) sel=0;
  }
// エンコーダの値確認
  M5.Lcd.setCursor(0, 150);
  M5.Lcd.printf("回転角: 1:%6d, 2:%6d", base_x.GetEncoderValue(1),base_x.GetEncoderValue(2));
  M5.Lcd.setCursor(0, 180);
  M5.Lcd.printf("回転角: 3:%6d, 4:%6d", base_x.GetEncoderValue(3),base_x.GetEncoderValue(4));
  delay(10);
}