idf_component_register(SRCS "WiFiConfig.c"
                    INCLUDE_DIRS "include"
                    REQUIRES  esp_wifi esp_eth  mdns
                    esp_http_server esp_http_client nvs_flash spiffs 
                    Utilities esp_psram heap)
