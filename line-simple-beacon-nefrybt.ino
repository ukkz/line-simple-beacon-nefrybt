/**
 * Nefry BT(R2/R3) にGroveの光センサーを接続し、照度変化があったらアドバタイズします。
 * Groveのブザーはアドバタイズされていることをわかりやすくするためのデバッグ用なので
 * 実際に使用するときは接続しないほうがよいです。
 * コンパイルのボード指定は、NefryBTではなく "ESP32 Dev Module" にしてください。
 * （大分類が Nefry(ESP32)Module ではなく ESP32 Arduino 内のもの）
 * 以下のページに従って、ボードマネージャから espressif/arduino-esp32 を
 * インストールしてください。
 * https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md
 */

#include "BLEDevice.h"
#include "LineSimpleBeacon.h"

/**
 * LINEビーコン用ハードウェアID
 * あらかじめBotアカウント(LINE公式アカウント)を作成した状態で
 * 以下のページにアクセスしてHWIDを発行してください。
 * https://admin-official.line.me/beacon/register#/
 * (Issue LINE Simple Beacon Hardware ID をクリック)
 */
const char hwid[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

#define Nefry_A0                32  // NefryのA0ピン(照度センサ)
#define Nefry_D0                21  // NefryのD0ピン(ブザー)
#define DETECTING_DURATION      2   // this秒間の間にLIGHT_LEVEL_THRESHOLD以上の変化があったらアドバタイズ
#define ADVERTISING_DURATION    15  // アドバタイズパケットを送信する秒数(1回あたり)
#define SENSOR_VALUE_INTEGRAL   100 // 照度測定値を平均化するときの積算回数
#define LIGHT_LEVEL_THRESHOLD   64  // DETECTING_DURATION秒間の間にthis以上の変化があったらアドバタイズ
#define AUTO_RESET_MINUTES      15  // この分数経過したら強制的に再起動して1度だけアドバタイズする
int light_prev = 0;
int minutesAfterStarted = 0;



void advertise(String message) {
  char payload[13];
  memset(&payload[0], 0, 13);
  message.toCharArray(payload, 13);

  LineSimpleBeacon LineBeacon = LineSimpleBeacon(hwid);
  LineBeacon.setMessage(payload, (char)message.length());
  
  BLEAdvertisementData advData = BLEAdvertisementData();
  BLEAdvertisementData scnData = BLEAdvertisementData();
  advData.addData(LineBeacon.getAdvPacket());

  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->setAdvertisementData(advData);
  advertising->setScanResponseData(scnData);

  advertising->start(); Serial.printf("Advertizing start... %s", payload);
  for (unsigned long i=0; i<ADVERTISING_DURATION; i++) {
    digitalWrite(Nefry_D0, HIGH); delay(50);
    digitalWrite(Nefry_D0, LOW);  delay(50);
    digitalWrite(Nefry_D0, HIGH); delay(50);
    digitalWrite(Nefry_D0, LOW);  delay(50);
    delay(800);
  }
  advertising->stop();  Serial.println(" ...stop");
}


int getLightLevel() {
  unsigned long sum = 0;
  for (int i=0; i<SENSOR_VALUE_INTEGRAL; i++) {
    sum += analogRead(Nefry_A0);
    delay(1);
  }
  return (int)(sum/SENSOR_VALUE_INTEGRAL);
}


void setup() {
  Serial.begin(115200);
  pinMode(Nefry_D0, OUTPUT);
  BLEDevice::init("LineBeacon/NefryBT");
  light_prev = getLightLevel();
  
  /**
   * アドバタイズパケットに載せる文字列は任意ですが13bytesまでです。
   * ここでは、"(照度センサ値)/(照度変化量)" という文字列にしていて
   * サーバ側ではphpのexplodeで分割取得しています。
   */
  advertise(String(light_prev) + String("/") + String(0));
  // 自動リセット含む初回起動時に照度変化0でadvertiseすることで「定期送信」としています。
}


void loop() {
  // 起動してからの分数
  minutesAfterStarted = (int)(millis()/60000UL);
  
  // 検出値と前回との差分
  int light_now = getLightLevel();
  int diff = light_now - light_prev;
  light_prev = light_now; // 測定値保持
  
  // 変化量がLIGHT_LEVEL_THRESHOLD以上であったときのみadvertising
  if (diff >= LIGHT_LEVEL_THRESHOLD or diff <= (-1)*LIGHT_LEVEL_THRESHOLD) {
    if (diff > 0) Serial.printf("Detected LIGHT UP (delta %d) - current level is %d.",   diff, light_now); // 明るさUP検知
    else          Serial.printf("Detected LIGHT DOWN (delta %d) - current level is %d.", diff, light_now); // 明るさDOWN検知
    Serial.println("");
    advertise(String(light_now) + String("/") + String(diff)); // 同期処理のためADVERTISING_DURATION秒間かかる
  }

  // 連続起動していたら一定のところでソフトウェアリセット
  if (minutesAfterStarted >= AUTO_RESET_MINUTES) {
    Serial.println("");
    Serial.print(AUTO_RESET_MINUTES); Serial.println("minutes elapsed!");
    Serial.println("ESP32 software reset: Restarting...");
    Serial.println("");
    ESP.restart(); // 強制再起動
  }
  
  // 次のループまで待つ
  delay(1000LL * DETECTING_DURATION);
}
