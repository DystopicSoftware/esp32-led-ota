#ifndef APP_CONTEXT_H
#define APP_CONTEXT_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_http_server.h"

// Comandos para el LED
typedef enum {
    LED_CMD_TOGGLE,
    LED_CMD_SET_RGB
} led_cmd_type_t;

// Mensaje que viaja en la cola del LED
typedef struct {
    led_cmd_type_t type;
    uint8_t r;
    uint8_t g;
    uint8_t b;
} led_msg_t;

// --- LA ESTRUCTURA MAESTRA ---
typedef struct {
    // Colas (Mecanismo de comunicación)
    QueueHandle_t led_queue;
    
    // Handles del Sistema
    httpd_handle_t http_server_handle;

    // Estado de la aplicación
    bool wifi_connected;
    
    // Configuración (Hardcoded por ahora, idealmente vendría de NVS)
    const char* wifi_ssid;
    const char* wifi_pass;

} app_context_t;

#endif