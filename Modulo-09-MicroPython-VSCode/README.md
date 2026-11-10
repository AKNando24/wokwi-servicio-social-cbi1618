# Módulo 9: Entornos Profesionales y Alternativas de Programación

## Objetivo
Programar en **MicroPython** (Raspberry Pi Pico y ESP32) y pasar a un flujo profesional con **VS Code + PlatformIO + extensión de Wokwi**, con una **librería propia** reutilizable.

## Contenido
| Carpeta | Qué contiene |
|---|---|
| [`9.1_micropython/`](9.1_micropython) | `main.py` único para Pico y ESP32, con un `diagram.json` por placa (`pico/` y `esp32/`) |
| [`9.2_platformio_vscode/`](9.2_platformio_vscode) | Proyecto PlatformIO para Arduino Uno: `platformio.ini`, `wokwi.toml`, `diagram.json`, `src/main.cpp` y la librería `lib/EstacionLib/` |

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 9.1 — MicroPython en ESP32 y Raspberry Pi Pico](guiones/9.1_micropython-en-esp32-y-raspberry-pi-pico.md) | ≈ 10:20 min | _pendiente_ |
| [Guion 9.2 — Visual Studio Code, PlatformIO, extensión de Wokwi y librerías propias](guiones/9.2_vscode-platformio-y-librerias-propias.md) | ≈ 10:50 min | _pendiente_ |

## Notas de verificación
- 9.2: `src/main.cpp` y `EstacionLib` compilan sin errores con avr-gcc para Arduino Uno. No se ejecutó `pio run` ni la extensión de Wokwi en el entorno donde se generó.
- 9.1: `main.py` pasó la verificación de sintaxis de Python; no se ejecutó en MicroPython. Los pines de ambos `diagram.json` se validaron contra la documentación oficial.
- La extensión de Wokwi para VS Code requiere una licencia (F1 → «Wokwi: Request a new License»); revisa las condiciones vigentes.
