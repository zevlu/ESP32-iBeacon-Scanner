# ESP32 iBeacon Scanner & nRF52840 Beacon 發送器

本專案實作了一個完整的物聯網架構：由 Nordic nRF52840 作為發送端（iBeacon），並由 ESP32 作為接收端（Gateway），將捕捉到的 RSSI 數據透過 MQTT 協定傳輸至雲端。

## 📂 資料夾結構說明
* **/nRF52840_iBeacon_TX**: 包含 Nordic 端的發送邏輯原始碼 (main.c)。
* **/ESP32_MQTT_Scanner**: 包含 ESP32 的掃描與 MQTT 傳輸程式碼 (.ino)。

## 📡 發送端技術細節 (nRF52840)
針對廣播機制與頻率進行了專業設定：
1. **核心函式**: 使用 Zephyr SDK 的 `bt_le_adv_start()` 啟動廣播。
2. **廣播模式**: `BT_LE_ADV_NCONN` (不可連線模式)，專為 iBeacon 設計。
3. **廣播間隔 (Interval)**: 預設設定於 **100ms**，確保接收端能高頻率捕捉訊號。
4. **封包內容**:
   - **Company ID**: `0x4c, 0x00` (Apple)
   - **Type**: `0x02, 0x15` (iBeacon)

## 🔍 接收端技術細節 (ESP32)
* **掃描邏輯**: 使用非阻塞式掃描 (`pBLEScan->start(1, false)`)，在維持 MQTT 連線的同時持續偵測藍牙訊號。
* **數據處理**: 自動過濾包含 `0x4C 0x00 0x02 0x15` 的封包，並提取 RSSI 強度。
* **雲端對接**: 數據打包成 JSON 格式上傳至 `test.mosquitto.org`。

## 🚀 第三週目標
* 修改 Nordic 原始碼中的 **UUID** 欄位，實作個人身分識別功能。
