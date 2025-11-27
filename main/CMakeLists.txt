idf_component_register(
    SRCS "main.c" 
         "led_service.c" 
         "wifi_service.c" 
         "http_service.c"
    
    INCLUDE_DIRS "."
    
    REQUIRES nvs_flash esp_wifi esp_event esp_http_server driver esp_netif app_update esp_partition lwip
    
    EMBED_FILES "../web/index.html"
)