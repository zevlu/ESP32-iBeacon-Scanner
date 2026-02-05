#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/kernel.h> // 為了使用 k_sleep

#ifndef IBEACON_RSSI
#define IBEACON_RSSI 0xc8
#endif

/* [要求 1] 定義兩組輪播的廣告資料 */

// 第一組：普通 Beacon (UUID: 1234...)
static const struct bt_data ad_normal[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
              0x4c, 0x00, 0x02, 0x15,
              0x12, 0x34, 0x56, 0x78, 0x00, 0x00, 0x00, 0x00,
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xAA, 0xAA, 0xBB, 0xBB, IBEACON_RSSI)
};

// 第二組：加密名字 Beacon (UUID 包含 Zev)
static const struct bt_data ad_zev[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
              0x4c, 0x00, 0x02, 0x15,
              0x5a, 0x65, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00, // Zev
              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
              0xAA, 0xAA, 0xBB, 0xBB, IBEACON_RSSI)
};

int main(void)
{
    int err;
    printk("Starting iBeacon Rotation Demo\n");

    /* 初始化藍牙 */
    err = bt_enable(NULL);
    if (err) {
        printk("Bluetooth init failed (err %d)\n", err);
        return 0;
    }
    printk("Bluetooth initialized\n");

    /* [要求 1 & 5] 進入輪播迴圈並監測資源 */
    while (1) {
        // 1. 開始廣播「普通組」
        err = bt_le_adv_start(BT_LE_ADV_NCONN, ad_normal, ARRAY_SIZE(ad_normal), NULL, 0);
        if (!err) {
            printk(">>> Advertising: Normal Beacon\n");
        }
        k_sleep(K_MSEC(3000)); // 廣播 3 秒
        bt_le_adv_stop();

        // 2. 開始廣播「Zev 加密組」
        err = bt_le_adv_start(BT_LE_ADV_NCONN, ad_zev, ARRAY_SIZE(ad_zev), NULL, 0);
        if (!err) {
            printk(">>> Advertising: Encrypted Name (Zev)\n");
        }
        k_sleep(K_MSEC(3000)); // 廣播 3 秒
        bt_le_adv_stop();
    }
    return 0;
}