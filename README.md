# ESP32 iBeacon Scanner & nRF52840 Broadcaster

本專案實作了一個完整的物聯網架構：由 **Nordic nRF52840** 作為發送端（Broadcaster），並由 **ESP32** 作為接收端（Gateway），將捕捉到的數據透過 MQTT 協定傳輸至雲端。

## 🛠 系統架構與技術細節

### **發送端 (nRF52840)**
* **核心機制**: 使用 Zephyr SDK `bt_le_adv_start()` 實作。
* **廣播模式**: `BT_LE_ADV_NCONN` (不可連線模式)，專為 iBeacon 設計以節省耗能。
* **封包規格**: 遵循 Apple iBeacon 標準，包含 Company ID (`0x4C00`) 與 Type (`0x0215`)。

### **接收端 (Gateway - ESP32)**
* **掃描防護**: 實作 `len < 30` 封包長度檢查，有效過濾非法封包防止系統崩潰 (Panic)。
* **身分還原**: 針對特定 UUID 進行 ASCII 對位解密（如：`5A 65 76` -> `Zev`）。
* **雲端對接**: 數據打包成 JSON 格式並發送至 MQTT Broker (`test.mosquitto.org`)。

---

## 📅 開發實驗紀錄

### **[Week 3] 輪播、解密與系統防護**
* **技術重點**:
    * **Nordic**: 實作 3 秒動態輪播機制，在「普通組」與「Zev加密組」之間切換。
    * **ESP32**: 實作 Heap Memory 監測，系統運作時穩定維持於約 **87KB** 剩餘空間。
* **實驗報告**: [📑 查看本週完整 PDF 報告](./Week3/Nordic實作第三周.pdf)

### **[Week 2] 基礎掃描與廣播**
* **ESP32**: 基礎 BLE 掃描與 MQTT 數據轉發。
* **Nordic**: 靜態 iBeacon 廣播實作。

---

## 📊 系統時序圖
本專案的資料流向如下圖所示：

1. **Nordic** 輪流廣播不同 UUID 的 iBeacon 訊號。
2. **ESP32** 掃描並過濾 Major/Minor 匹配之設備。
3. **ESP32** 執行解密並監測記憶體後，將 JSON 數據送往 **MQTT Broker**。
