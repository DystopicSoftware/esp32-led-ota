#include "http_service.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

#include <time.h>
#include <sys/time.h>

static const char *TAG = "HTTP_SVC";

// Referencia al archivo embebido
extern const uint8_t index_html_start[] asm("_binary_index_html_start");
extern const uint8_t index_html_end[]   asm("_binary_index_html_end");

/* HANDLER: GET / (Index) */
static esp_err_t index_handler(httpd_req_t *req) {
    httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);
    return ESP_OK;
}

/* HANDLER: POST /api/led (Control LED) */
static esp_err_t led_handler(httpd_req_t *req) {
    // 1. RECUPERAR CONTEXTO
    app_context_t *ctx = (app_context_t *)req->user_ctx;
    
    // 2. ENVIAR MENSAJE A LA COLA (No tocamos hardware aquí)
    led_msg_t msg = { .type = LED_CMD_TOGGLE };
    xQueueSend(ctx->led_queue, &msg, 0);
    
    httpd_resp_send(req, "LED Toggled", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* HANDLER: POST /OTAupdate (Firmware Update) */
static esp_err_t ota_update_handler(httpd_req_t *req) {
    esp_ota_handle_t update_handle = 0 ;
    const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
    char buf[1024];
    int received;
    
    ESP_LOGI(TAG, "Starting OTA...");
    
    if (esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle) != ESP_OK) {
        return ESP_FAIL;
    }

    while ((received = httpd_req_recv(req, buf, sizeof(buf))) > 0) {
        esp_ota_write(update_handle, buf, received);
    }

    if (esp_ota_end(update_handle) == ESP_OK) {
        if (esp_ota_set_boot_partition(update_partition) == ESP_OK) {
            httpd_resp_send(req, "OTA Success. Restarting...", HTTPD_RESP_USE_STRLEN);
            vTaskDelay(pdMS_TO_TICKS(1000));
            esp_restart(); // Reinicio limpio
        }
    }
    
    httpd_resp_send_500(req);
    return ESP_FAIL;
}

/* HANDLER: GET /api/time (Devuelve la hora del sistema) */
static esp_err_t time_handler(httpd_req_t *req) {
    time_t now;
    struct tm timeinfo;
    char time_str[64];
    char json_response[100];

    // 1. Leer hora del sistema
    time(&now);
    localtime_r(&now, &timeinfo);

    // 2. Verificar si la hora ya se sincronizó (Si el año es < 2020, aún no hay hora)
    if (timeinfo.tm_year < (2020 - 1900)) {
        sprintf(json_response, "{\"time\": \"Sincronizando...\"}");
    } else {
        // Formatear: HH:MM:SS
        strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
        sprintf(json_response, "{\"time\": \"%s\"}", time_str);
    }

    // 3. Enviar respuesta JSON
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_response, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

void http_service_start(app_context_t *ctx) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
    // *** INYECCIÓN DE DEPENDENCIA ***
    config.global_user_ctx = (void *)ctx; 
    config.global_user_ctx_free_fn = NULL;
    config.stack_size = 8192; // OTA necesita stack

    if (httpd_start(&ctx->http_server_handle, &config) == ESP_OK) {
        
        httpd_uri_t uri_index = { .uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = ctx };
        httpd_register_uri_handler(ctx->http_server_handle, &uri_index);

        httpd_uri_t uri_led = { .uri = "/api/led", .method = HTTP_POST, .handler = led_handler, .user_ctx = ctx };
        httpd_register_uri_handler(ctx->http_server_handle, &uri_led);
        
        httpd_uri_t uri_ota = { .uri = "/OTAupdate", .method = HTTP_POST, .handler = ota_update_handler, .user_ctx = ctx };
        httpd_register_uri_handler(ctx->http_server_handle, &uri_ota);
        
        ESP_LOGI(TAG, "Web Server Started");

        httpd_uri_t uri_time = { .uri = "/api/time", .method = HTTP_GET, .handler = time_handler, .user_ctx = ctx };
        httpd_register_uri_handler(ctx->http_server_handle, &uri_time);
    }

    
}