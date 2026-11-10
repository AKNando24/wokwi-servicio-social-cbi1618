# Módulo 10: Proyecto Integrador Final y Publicación

## Objetivo
Construir una **estación meteorológica IoT simulada** (sensores + pantalla + MQTT) en dos etapas, y prepararla para llevarla a hardware real, versionarla en GitHub e incrustarla en una web.

## Contenido
| Carpeta | Qué contiene |
|---|---|
| [`10.1_prototipo-sin-red/`](10.1_prototipo-sin-red) | ESP32 + DHT22 + LDR + OLED + 2 botones + LED de alarma. Filtro, mínimos/máximos, histéresis, 3 páginas de pantalla y salida CSV |
| [`10.2_estacion-completa/`](10.2_estacion-completa) | Lo anterior + Wi-Fi no bloqueante, NTP, MQTT (telemetría, LWT y comandos `RESET` / `UMBRAL <°C>`), `PASO_A_HARDWARE_REAL.md` y `embed.html` |

## Conexiones (ESP32 DevKit C v4)
| Componente | Pin |
|---|---|
| DHT22 | GPIO4 |
| LDR (AO) | GPIO34 (ADC1) |
| OLED SSD1306 (0x3C) | SDA → GPIO21, SCL → GPIO22 |
| Botón de página / reinicio | GPIO18 / GPIO19 (pull-up interna) |
| LED de alarma | GPIO5 → 220 Ω → GND |

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 10.1 — Proyecto integrador, parte 1: estación meteorológica (prototipo sin red)](guiones/10.1_estacion-meteorologica-prototipo.md) | ≈ 10:30 min | _pendiente_ |
| [Guion 10.2 — Proyecto integrador, parte 2: MQTT, paso a hardware real, GitHub y publicación](guiones/10.2_estacion-completa-mqtt-hardware-real-y-github.md) | ≈ 11:00 min | _pendiente_ |

## Notas de verificación
- **Este código NO se pudo compilar en el entorno donde se generó** (no hay toolchain de ESP32): pruébalo en Wokwi. Los pines de ambos `diagram.json` se validaron contra la documentación oficial.
- **Cambia `ID_ESTACION`** en 10.2 antes de ejecutar: el broker público es compartido.
- Verifica en Wokwi si el valor del LDR sube o baja con la luz y, si es necesario, invierte el `map()` de `luzPct`.
