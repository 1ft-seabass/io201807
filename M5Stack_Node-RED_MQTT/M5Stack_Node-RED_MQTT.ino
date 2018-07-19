#include <WiFiClient.h>
#include <PubSubClient.h>
#include <M5Stack.h>
#include <ArduinoJson.h>

// Wi-FiのSSID
char *ssid = "***Wi-FiのSSID***";
// Wi-Fiのパスワード
char *password = "***Wi-Fiのパスワード***";
// MQTTの接続先のIP
const char *endpoint = "***MQTTの接続先のIP***";
// MQTTのポート
const int port = 1883;
// デバイスID
char *deviceID = "M5Stack3";  // デバイスIDは機器ごとにユニークにします
// メッセージを知らせるトピック
char *pubTopic = "pub/M5Stack3";
// メッセージを待つトピック
char *subTopic = "sub/M5Stack3";

////////////////////////////////////////////////////////////////////////////////
  
WiFiClient httpsClient;
PubSubClient mqttClient(httpsClient);
  
void setup() {
  Serial.begin(115200);
  
  // Initialize the M5Stack object
  M5.begin();

  // START
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);
  
  // Start WiFi
  Serial.println("WiFi Connecting to ");
  M5.Lcd.println("WiFi Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      M5.Lcd.printf(".");
  }

  // WiFi Connected
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(10, 40);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);
  M5.Lcd.printf("WiFi Connected.");

  // 接続情報
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 100);
  M5.Lcd.printf("endpoint: ");
  M5.Lcd.println(endpoint);
  M5.Lcd.printf("deviceID: ");
  M5.Lcd.println(deviceID);
  M5.Lcd.printf("sub: ");
  M5.Lcd.println(subTopic);
  M5.Lcd.printf("pub: ");
  M5.Lcd.println(pubTopic);
  
  mqttClient.setServer(endpoint, port);
  mqttClient.setCallback(mqttCallback);

}
  
void connectMQTT() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect(deviceID)) {
            Serial.println("Connected.");
            int qos = 0;
            mqttClient.subscribe(subTopic, qos);
            Serial.println("Subscribed.");
        } else {
            Serial.print("Failed. Error state=");
            Serial.print(mqttClient.state());
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}
  
long messageSentAt = 0;
int count = 0;
char pubMessage[128];
int flag,bg_r,bg_g,bg_b,txt_r,txt_g,txt_b;
  
void mqttCallback (char* topic, byte* payload, unsigned int length) {

    String str = "";
    Serial.print("Received. topic=");
    Serial.println(topic);
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        str += (char)payload[i];
    }
    Serial.print("\n");

    StaticJsonBuffer<300> jsonBuffer;
    
    JsonObject& root = jsonBuffer.parseObject(str);
  
    // パースが成功確認
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }
    // JSONデータ
    const char* message = root["message"];
    flag = root["flag"];
    bg_r = root["bg_r"];
    bg_g = root["bg_g"];
    bg_b = root["bg_b"];
    txt_r = root["txt_r"];
    txt_g = root["txt_g"];
    txt_b = root["txt_b"];

    if( flag == 1 ){
      // 文字色RGBカラー uint16_t に変換
      uint16_t TXT_RGB = ((txt_r>>3)<<11) | ((txt_g>>2)<<5) | (txt_b>>3);
      // 背景色RGBカラー uint16_t に変換
      uint16_t BG_RGB = ((bg_r>>3)<<11) | ((bg_g>>2)<<5) | (bg_b>>3);
      // 背景色カラー反映
      M5.Lcd.fillRect(0, 0, 320, 240, BG_RGB);
      // テキスト反映
      M5.Lcd.setCursor(10, 120);
      M5.Lcd.setTextColor(TXT_RGB);
      M5.Lcd.setTextSize(5);
      M5.Lcd.printf(message);
    } else {
      // 消灯
      M5.Lcd.fillScreen(BLACK);
    }

    delay(300);
    
}

void loop() {

  // 常にMQTTチェックして切断されたら復帰できるように
  if (!mqttClient.connected()) {
      connectMQTT();
  }
  mqttClient.loop();
  
  // Aボタンを押したらメッセージw飛ばす
  if(M5.BtnA.wasPressed()) {
    Serial.print("M5.BtnA.wasPressed");
    
    // カウントアップ
    count++;
    // 表示
    M5.Lcd.fillScreen(GREEN);
    M5.Lcd.setCursor(0, 80);
    M5.Lcd.setTextColor(BLACK);
    M5.Lcd.setTextSize(4);
    M5.Lcd.println("Button A");
    M5.Lcd.print("count:");
    M5.Lcd.println(count);
    // MQTT送信
    sprintf(pubMessage, "{\"count\": %d}", count);
    mqttClient.publish(pubTopic, pubMessage);
    Serial.print(pubMessage);
    delay(500);
  }
 
  M5.update();
}
