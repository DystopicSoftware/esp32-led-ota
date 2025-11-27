#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"

// Includes de nuestra arquitectura
#include "app_context.h"
#include "led_service.h"
#include "wifi_service.h"

void app_main(void)
{
    // 1. Init NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. CREACIÓN DEL CONTEXTO (Heap)
    // Usamos calloc para que todo inicie en 0/NULL
    app_context_t *ctx = (app_context_t *)calloc(1, sizeof(app_context_t));

    // 3. Configurar Datos Iniciales
    ctx->wifi_ssid = "Teddy";  //"EstudiantesUN";     // Cambia esto 
    ctx->wifi_pass = "amoateddy";  //"RedEstudiantes"; // Cambia esto 
    
    // 4. Crear Recursos del Sistema (Colas)
    ctx->led_queue = xQueueCreate(10, sizeof(led_msg_t));

    // 5. Iniciar Servicios
    ESP_LOGI("MAIN", "Iniciando Servicios...");
    
    // Iniciamos LED primero (Consumidor)
    led_service_start(ctx);
    
    // Iniciamos WiFi (Productor de eventos de red)
    // El servidor HTTP se iniciará automáticamente dentro de wifi_service cuando tenga IP
    wifi_service_init(ctx);
    
    ESP_LOGI("MAIN", "Sistema Arrancado. Esperando IP...");
}