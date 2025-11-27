#include "led_service.h"
#include "driver/ledc.h"
#include "esp_log.h"


static const char *TAG = "LED_SVC";

// Pines (Ajusta a tus conexiones)
// Cambio  temporal para LED simple
#define LED_PIN_R 2   // Generalmente el LED integrado es el 2
// #define LED_PIN_R 21 (Lo dejas comentado para cuando tengas el RGB)
#define LED_PIN_G 22
#define LED_PIN_B 23

static void led_service_task(void *pvParameters) {
    app_context_t *ctx = (app_context_t *)pvParameters;
    led_msg_t msg;
    uint8_t current_state = 0; // Estado local para toggle

    // Configuración básica de GPIO (Simplificada para el ejemplo, usar LEDC para PWM real)
    gpio_reset_pin(LED_PIN_R); gpio_set_direction(LED_PIN_R, GPIO_MODE_OUTPUT);
    
    ESP_LOGI(TAG, "LED Task Started. Waiting for queue...");

    while (1) {
        // BLOQUEANTE: Espera mensaje infinito
        if (xQueueReceive(ctx->led_queue, &msg, portMAX_DELAY)) {
            
            if (msg.type == LED_CMD_TOGGLE) {
                current_state = !current_state;
                gpio_set_level(LED_PIN_R, current_state);
                ESP_LOGI(TAG, "LED Toggled to: %d", current_state);
            }
            else if (msg.type == LED_CMD_SET_RGB) {
                // Aquí implementarías ledc_set_duty con msg.r, msg.g, msg.b
                ESP_LOGI(TAG, "Setting RGB: %d %d %d", msg.r, msg.g, msg.b);
            }
        }
    }
}


void led_service_start(app_context_t *ctx) {
    xTaskCreate(led_service_task, "led_task", 4096, (void *)ctx, 5, NULL);
}