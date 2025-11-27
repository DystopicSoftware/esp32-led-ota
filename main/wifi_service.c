#include "wifi_service.h"
#include "http_service.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "string.h"
#include "esp_sntp.h"
#include "time.h"
#include "esp_mac.h" // <--- AGREGADO: Necesario para manejar direcciones MAC

static const char *TAG = "WIFI_SVC";

// --- CONFIGURACIÓN AP ---
#define AP_SSID      "ESP32_TED"
#define AP_PASS      "12345678"
#define AP_CHANNEL   1
#define AP_MAX_CONN  4

// --- ZONA HORARIA (Ej: Colombia/Perú -5) ---
#define TIME_ZONE    "EST5" 

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Iniciando SNTP...");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();
    
    setenv("TZ", TIME_ZONE, 1);
    tzset();
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    app_context_t *ctx = (app_context_t *)arg;

    // 1. Estación (Cliente Router)
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ctx->wifi_connected = false;
        esp_wifi_connect(); 
        ESP_LOGI(TAG, "Reintentando conectar al Router...");
    } 
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Conectado a Internet. IP: " IPSTR, IP2STR(&event->ip_info.ip));
        ctx->wifi_connected = true;
        initialize_sntp();
    }
    
    // 2. Punto de Acceso (AP)
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        
        // --- ARREGLO DEL ERROR AQUÍ ---
        // En lugar de usar la macro compleja, usamos el formato hexadecimal directo:
        ESP_LOGI(TAG, "Cliente conectado al AP. MAC: %02x:%02x:%02x:%02x:%02x:%02x, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_service_init(app_context_t *ctx) {
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, (void*)ctx, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, (void*)ctx, NULL));

    // Configuración Wifi Casa
    wifi_config_t wifi_config_sta = {
        .sta = { .threshold.authmode = WIFI_AUTH_WPA2_PSK },
    };
    if (ctx->wifi_ssid != NULL) {
        strncpy((char*)wifi_config_sta.sta.ssid, ctx->wifi_ssid, sizeof(wifi_config_sta.sta.ssid));
        strncpy((char*)wifi_config_sta.sta.password, ctx->wifi_pass, sizeof(wifi_config_sta.sta.password));
    }

    // Configuración Wifi Propio (AP)
    wifi_config_t wifi_config_ap = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .channel = AP_CHANNEL,
            .password = AP_PASS,
            .max_connection = AP_MAX_CONN,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = { .required = true },
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config_sta));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config_ap));
    
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Modo Hibrido (AP + STA) Iniciado");
    http_service_start(ctx);
}