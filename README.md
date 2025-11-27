# Sistema ESP32: Servidor Web, OTA y Sincronización NTP

Este proyecto implementa un firmware robusto para ESP32 utilizando **ESP-IDF v5.5** y **FreeRTOS**. Se destaca por utilizar una **arquitectura de software limpia sin variables globales**, basada en inyección de dependencias mediante un contexto de aplicación (`app_context`).

## 🚀 Características Principales

*   **Arquitectura Profesional:** Comunicación entre tareas mediante Colas (`xQueue`) y paso de contexto por punteros. Cero variables globales.
*   **WiFi Híbrido (AP + STA):**
    *   **Modo Estación:** Se conecta a un Router/Hotspot para tener acceso a Internet.
    *   **Modo Access Point:** Crea una red de respaldo (`ESP32_MASTER`) accesible sin router.
*   **Sincronización de Hora (NTP):** Obtiene la hora mundial de `pool.ntp.org` automáticamente al detectar Internet.
*   **Servidor Web Embebido:**
    *   Interfaz visual HTML/CSS/JS (embebida en el binario).
    *   Visualización de reloj en tiempo real (AJAX/Fetch).
    *   Control de LED remoto.
*   **Actualización Inalámbrica (OTA):** Capacidad de actualizar el firmware subiendo un archivo `.bin` desde el navegador.

## 🛠 Hardware Requerido

*   **ESP32 DevKitC** (o compatible).
*   **LED** (Opcional): Conectado al GPIO definido en `led_service.c` (Por defecto GPIO 2 para el LED integrado).
*   Cable USB para flasheo inicial.

## ⚙️ Configuración

Antes de compilar, edite el archivo `main/wifi_service.c` y `main/main.c` para ajustar sus credenciales:

1.  **Credenciales Router (STA):** En `main.c`.
2.  **Credenciales Punto de Acceso (AP):** En `wifi_service.c` (Default: `ESP32_MASTER` / `12345678`).
3.  **Zona Horaria:** En `wifi_service.c` (Default: `EST5` para UTC-5).

## 📦 Instalación y Uso

1.  **Compilar:**
    ```bash
    idf.py build
    ```
2.  **Flashear:**
    ```bash
    idf.py flash monitor
    ```
3.  **Acceso Web:**
    *   Si está conectado al Router: Mire el monitor serial para ver la IP asignada (ej: `http://192.168.1.15`).
    *   Si está conectado al AP del ESP32: Acceda a `http://192.168.4.1`.

## 📡 API Endpoints

El servidor expone los siguientes endpoints JSON:

| Método | Endpoint      | Descripción                           |
| :----- | :------------ | :------------------------------------ |
| `GET`  | `/api/time`   | Devuelve la hora actual del sistema.  |
| `POST` | `/api/led`    | Cambia el estado del LED (Toggle).    |
| `POST` | `/OTAupdate`  | Recibe el archivo `.bin` para update. |

## 📂 Estructura del Proyecto

```text
main/
├── app_context.h   # Estructura maestra de datos (Contexto)
├── main.c          # Punto de entrada e inyección de dependencias
├── wifi_service.c  # Gestión de AP, STA y SNTP
├── http_service.c  # Rutas Web y manejador OTA
├── led_service.c   # Tarea controladora de Hardware
└── web/            # Código Fuente Frontend (HTML/JS)