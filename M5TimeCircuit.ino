//
//
// https://lang-ship.com/blog/work/m5stickc-display4-lovyangfx/
// これがキモでした....
//
//
//
// M5StickC でタイムサーキットを作ってみる話
// M5TimeCirkit.ino
//

#define _M5DISPLAY_H_               // M5StickCとの共存用設定
class M5Display {};                 // 既存の描画関数は使えなくする
#include <M5StickC.h>               // M5StickCの読み込み

#include <time.h>
#include <WiFi.h>

// ネットワークへの接続
void setupWifi()
{
  //=============================================
  const char* ssid     = "********";
  const char* password = "********";
  //=============================================
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  configTime(9 * 3600L, 0, "ntp.nict.jp");
  configTzTime("JST-9", "ntp.nict.jp");
  struct tm timeInfo;
  getLocalTime(&timeInfo);
//  WiFi.disconnect(true, true);
//  WiFi.mode(WIFI_OFF);
}

// 描画ライブラリー読込
#include <LovyanGFX.hpp>
static LGFX lcd;
static LGFX_Sprite spr;

// 待つ
bool wait()
{
  bool rtn = false;
  static uint32_t tm_ = millis();
  if ((millis() - tm_) > 500) {
    tm_ = millis();
    rtn = true;
  }
  return rtn;
}

// 描画パラメータ
#define SCALEX (0.33)
#define SCALEY (0.49)
#define DESTINATION_TIME  (0)
#define PRESENT_TIME      (1)
#define LASTTIME_DEPARTED (2)

#define BACKCOLOR (0x0F0F0FU)

// 描画
void drawTime(int kind, bool stat)
{
  // 描画用スプライト生成
  spr.createSprite(48 * 16, 48);
  spr.setPivot(0, 0);

  time_t t;
  struct tm tm;
  struct tm *tmst;

  uint32_t col[3][2] = {
    0xFF0000, 0x8F0000,
    0x00FF00, 0x005F00,
    0xFFFF00, 0x5F5F00
  };
  char *mons[12] = {
    "JAN", "FEB", "MAR", "APL", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
  };

  int x, y;
  int rot = 0;
  switch (kind) {

    case DESTINATION_TIME:  // 目標時刻
      tm.tm_year = 2015 - 1900;
      tm.tm_mon  = 10 - 1;
      tm.tm_mday = 21;
      tm.tm_hour =  4;
      tm.tm_min  = 29;
      tm.tm_sec  =  0;
      t = mktime(&tm);
      x =  0;
      y =  0;
      break;

    case PRESENT_TIME:      // 現在時刻
      t = time(NULL);
      x =  0;
      y = 27;
      break;

    case LASTTIME_DEPARTED: // 出発時間
      tm.tm_year = 1985 - 1900;
      tm.tm_mon  = 10 - 1;
      tm.tm_mday = 26;
      tm.tm_hour =  1;
      tm.tm_min  = 20;
      tm.tm_sec  =  0;
      t = mktime(&tm);
      x =  0;
      y = 54;
      break;
  }

  tmst = localtime(&t);

  // 描画用スプライトに書く
  spr.fillScreen(BACKCOLOR);
  spr.setFont(&fonts::Font0);
  spr.setTextSize(6);
  spr.setTextColor(col[kind][1], BACKCOLOR);
  spr.setCursor(  2, 4); spr.printf("888");
  spr.setTextColor(col[kind][0]);
  spr.setCursor(  2, 4); spr.printf(mons[tmst->tm_mon]);
  spr.setFont(&fonts::Font7);
  spr.setTextSize(1);
  spr.setTextColor(col[kind][1], BACKCOLOR);
  spr.setCursor(116, 0); spr.printf("88 8888 :88:88");
  spr.setTextColor(col[kind][0]);
  spr.setCursor(116, 0); spr.printf("%02d %04d %1s%02d%s%02d",
                                    tmst->tm_mday,
                                    tmst->tm_year + 1900,
                                    (tmst->tm_hour > 12) ? ":" : ".", // 素敵な三項演算子
                                    tmst->tm_hour,
                                    (stat) ? ":" : " ",               // 素敵な三項演算子
                                    tmst->tm_min);
  // 描画用スプライトを転送する
  spr.pushRotateZoom(&lcd, x,  y, rot, SCALEX, SCALEY);
  // 描画用スプライト削除
  spr.deleteSprite();
}

static int _bright = 128;

// IMU呼び出し
void getImu()
{
  float gx, gy, gz;
  randomSeed(millis());
  M5.IMU.getGyroData(&gx, &gy, &gx);
  if (abs(gy + gy + gz)> 30) _bright = random(0, 255);

}

void setup() {
  M5.begin();
  M5.IMU.Init();
  Serial.begin( 115200 );
  setupWifi();
  lcd.init();               // LovyanGFX初期化
  lcd.setRotation(1);       // M5が右にくる方向
  lcd.setBrightness(_bright);   // 輝度半分くらい
}

void loop() {
  static bool stat_ = false;          // MSec 経過フラグ
  M5.update();

  getImu();
  lcd.setBrightness(_bright);         // ニキシー管なんで不安定

  if (!wait()) return;                // MSec 経過していない場合は書かない
  stat_ = !stat_;                     // MSec 経過フラグ更新
  drawTime(DESTINATION_TIME,  stat_); // 目標時刻
  drawTime(PRESENT_TIME,      stat_); // 現在時刻
  drawTime(LASTTIME_DEPARTED, stat_); // 出発時刻
}
