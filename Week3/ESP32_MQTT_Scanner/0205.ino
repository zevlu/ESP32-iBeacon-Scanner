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
const char* topic = "C113118234/ibeacon/data";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMqttUpdate = 0;

void setupWiFi() {
  delay(10);
  Serial.print("連線至 "); Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi 已連線");
}

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

// --- iBeacon 掃描回傳處理 ---
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
        uint8_t* payload = advertisedDevice.getPayload();
        size_t len = advertisedDevice.getPayloadLength();

        // [要求 3] 印出 Raw Payload
        Serial.print("\n[Raw Payload]: ");
        for (int j = 0; j < len; j++) Serial.printf("%02X ", payload[j]);
        Serial.println();

        // --- 修正點：加上長度檢查，避免存取越界 (len < 30 的封包直接跳過) ---
        if (len < 30) {
            return; 
        }

        for (int i = 0; i <= (len - 25); i++) {
            // 確認 iBeacon 標頭 (0x4C 0x00 0x02 0x15)
            if (payload[i] == 0x4C && payload[i+1] == 0x00 && 
                payload[i+2] == 0x02 && payload[i+3] == 0x15) {
                
                // 比對 Major/Minor
                if (payload[i+20] == 0xAA && payload[i+21] == 0xAA &&
                    payload[i+22] == 0xBB && payload[i+23] == 0xBB) {
                    
                    // 偵測到 Zev 加密組 (5A 65 76)
                    if (payload[i+4] == 0x5A && payload[i+5] == 0x65 && payload[i+6] == 0x76) {
                        String name = "Zev";
                        int rssi = advertisedDevice.getRSSI();
                        Serial.printf(">>> 狀態: 偵測到加密組 | 解密姓名: %s | RSSI: %d\n", name.c_str(), rssi);
                        Serial.printf(">>> Free Heap: %d bytes\n", ESP.getFreeHeap());

                        if (millis() - lastMqttUpdate > 1000) {
                            String msg = "{\"id\":\"C113118234\", \"user\":\"" + name + "\", \"rssi\":" + String(rssi) + "}";
                            client.publish(topic, msg.c_str());
                            lastMqttUpdate = millis();
                        }
                    } 
                    // 偵測到普通輪播組 (12 34 56)
                    else if (payload[i+4] == 0x12 && payload[i+5] == 0x34) {
                        Serial.println(">>> 狀態: 偵測到普通輪播組 (UUID: 1234...)");
                    }
                }
            }
        }
    }
};

void setup() {
  Serial.begin(115200);
  setupWiFi();
  client.setServer(mqtt_server, 1883);

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  BLEDevice::getScan()->start(1, false); 
}
