#ifdef __cplusplus
extern "C" {
#endif

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_system.h"

void WiFi_InitStation(char *UserWifiSSID_, char *UserWifiPassWord_, SemaphoreHandle_t *wifiConnectedSemaphore);

#ifdef __cplusplus
}
#endif
