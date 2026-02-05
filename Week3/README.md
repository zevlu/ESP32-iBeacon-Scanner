# Week 3: iBeacon 輪播與身分解密實作

## 實作目標
1. **Nordic (nRF52840)**: 實現兩組 iBeacon UUID 輪播，包含一組 ASCII 加密身分 ("Zev")。
2. **ESP32**: 實現 BLE 掃描、Payload 過濾防護、UUID 自動解密並透過 MQTT 上傳雲端。

## 技術重點
- **動態輪播**: 廣播端每 3 秒自動切換 UUID。
- **安全防護**: 加入 `len < 30` 檢查，防止非法封包造成系統崩潰。
- **身分解密**: 精確定位 UUID 欄位，將 `5A 65 76` 還原為字串。
- **資源監測**: 
  - Nordic RAM: 25.15 KB / ROM: 15.28 KB
  - ESP32 Free Heap: 穩定於 ~87 KB
