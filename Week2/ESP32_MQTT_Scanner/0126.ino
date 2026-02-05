#include <WiFi.h>
#include <PubSubClient.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// --- 設定區 ---
const char* ssid = "lu1101";
const char* password = "lu638047";
const char* mqtt_server = "test.mosquitto.org";
const char* topic = "C113118234/ibeacon/data";  // 你的專屬主題

// --- 物件初始化 ---
WiFiClient espClient;
PubSubClient client(espClient);
BLEScan* pBLEScan;
unsigned long lastMqttUpdate = 0;

// --- iBeacon 掃描回傳處理 ---
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        uint8_t* payload = advertisedDevice.getPayload();
        size_t len = advertisedDevice.getPayloadLength();

        // 🔍 自動掃描封包，尋找 0x4C 0x00 0x02 0x15
        for (int i = 0; i < (len - 4); i++) {
            if (payload[i] == 0x4C && payload[i+1] == 0x00 && 
                payload[i+2] == 0x02 && payload[i+3] == 0x15) {
                
                int rssi = advertisedDevice.getRSSI();
                Serial.printf("🎯 成功捕捉到 iBeacon! RSSI: %d \n", rssi);

                // 限制 1 秒發送一次，避免 MQTT 塞車
                if (millis() - lastMqttUpdate > 1000) {
                    String message = "{\"student_id\":\"C113118234\", \"rssi\":" + String(rssi) + "}";
                    bool published = client.publish(topic, message.c_str());
                    
                    if(published) {
                        Serial.println("✅ MQTT 數據發送成功！");
                    } else {
                        Serial.println("❌ MQTT 發送失敗，請檢查連線。");
                    }
                    lastMqttUpdate = millis();
                }
                return; // 找到就跳出迴圈
            }
        }
    }
};

// --- WiFi 連線函式 ---
void setupWiFi() {
  delay(10);
  Serial.print("連線至 "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi 已連線");
}

// --- MQTT 重新連線函式 ---
void reconnect() {
  while (!client.connected()) {
    Serial.print("嘗試 MQTT 連線...");
    if (client.connect("ESP32_Scanner_C113118234")) {
      Serial.println("連線成功");
    } else {
      Serial.print("失敗, rc="); Serial.print(client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, 1883);

  // 初始化藍牙掃描器
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); 
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); // 主動掃描較耗電但資訊較準確
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99); 
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 啟動非阻塞掃描 (讓 MQTT 維持連線)
  pBLEScan->start(1, false); 
}
