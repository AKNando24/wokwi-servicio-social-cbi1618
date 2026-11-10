# Módulo 8: Internet de las Cosas (IoT) con ESP32

## Objetivo
Conectar un ESP32 a la red virtual `Wokwi-GUEST`, sincronizar la hora por **NTP**, consultar una API con **HTTPS GET**, enviar datos con **HTTP POST** y publicar telemetría / recibir comandos por **MQTT** (broker público HiveMQ).

## Componentes
ESP32 DevKit C v4, DHT22, módulo LDR, LED con resistencia.

## Conexiones
| Componente | Conexión |
|---|---|
| DHT22 | SDA → GPIO4, VCC → 3V3 |
| LDR (AO) | GPIO34 (ADC1: los pines ADC2 no funcionan con Wi-Fi) |
| LED | GPIO18 → 220 Ω → LED → GND |

## Cómo ejecutarlo en Wokwi
1. Entra a [wokwi.com](https://wokwi.com) y crea un proyecto nuevo de **ESP32 (ESP32 DevKit C v4)**.
2. Pega el contenido de `main.ino` en la pestaña `sketch.ino`.
3. Reemplaza la pestaña `diagram.json` con el archivo de esta carpeta.
4. Si hay un `libraries.txt`, pega también su contenido en la pestaña del mismo nombre.
5. Presiona **Start Simulation**.

## Librerías
`DHT sensor library for ESPx` (no la de Adafruit, por problemas de temporización en ESP32), `PubSubClient`, `ArduinoJson` (ver `libraries.txt`).

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 8.1 — Wi-Fi virtual de Wokwi y sincronización de hora por NTP](guiones/8.1_wifi-virtual-y-hora-por-ntp.md) | ≈ 9:40 min | _pendiente_ |
| [Guion 8.2 — Cliente HTTP/HTTPS: peticiones GET y POST a una API REST](guiones/8.2_peticiones-http-get-y-post.md) | ≈ 10:10 min | _pendiente_ |
| [Guion 8.3 — MQTT: publicar telemetría y recibir comandos](guiones/8.3_mqtt-telemetria-y-comandos.md) | ≈ 10:10 min | _pendiente_ |

## Notas de verificación
- **Cambia `ID_UNICO`** antes de ejecutar: el broker público es compartido por todo el mundo.
- Usa `setInsecure()` en HTTPS solo para el curso; en un producto real se valida el certificado.
- Los pines de `diagram.json` se validaron contra la documentación oficial.
- **Este código NO se pudo compilar en el entorno donde se generó** (no hay toolchain de ESP32). Pruébalo en Wokwi y corrige cualquier detalle de API antes de grabar. Si usas ArduinoJson 6 en lugar de la 7, reemplaza `JsonDocument` por `StaticJsonDocument<512>`.
