# Módulo 7: Depuración Avanzada e Instrumentación Virtual

## Objetivo
Capturar con el **analizador lógico** de Wokwi una trama conocida (UART, I2C, SPI y PWM), exportarla en **VCD** y analizarla con PulseView, GTKWave y un script de Python.

## Componentes
LCD1602 (I2C), 74HC595 con barra de LEDs, analizador lógico de 8 canales.

## Conexiones
| Canal | Señal | Pin del Uno |
|---|---|---|
| D0 | TX (UART) | 1 |
| D1 / D2 | SDA / SCL (I2C) | A4 / A5 |
| D3 / D4 / D5 | MOSI / SCK / CS (SPI) | 11 / 13 / 10 |
| D6 | PWM (~490 Hz) | 9 |
| D7 | TRIG (disparador) | 8 |

## Cómo ejecutarlo en Wokwi
1. Entra a [wokwi.com](https://wokwi.com) y crea un proyecto nuevo de **Arduino Uno**.
2. Pega el contenido de `main.ino` en la pestaña `sketch.ino`.
3. Reemplaza la pestaña `diagram.json` con el archivo de esta carpeta.
4. Si hay un `libraries.txt`, pega también su contenido en la pestaña del mismo nombre.
5. Presiona **Start Simulation**.

## Librerías
Ninguna (`libraries.txt` queda vacío).

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 7.1 — El analizador lógico de Wokwi: capturar UART, I2C y SPI](guiones/7.1_analizador-logico-de-wokwi.md) | ≈ 9:20 min | _pendiente_ |
| [Guion 7.2 — Archivos VCD: análisis con PulseView y GTKWave](guiones/7.2_archivos-vcd-pulseview-gtkwave.md) | ≈ 10:10 min | _pendiente_ |

## Notas de verificación
- Al detener la simulación, Wokwi descarga `modulo7-captura.vcd`. Luego: `python3 analisis_vcd.py modulo7-captura.vcd`.
- `analisis_vcd.py` se probó con una captura sintética (bit de 104,2 µs ≈ 9600 baudios, SCK de 1 MHz y PWM de 490 Hz). No se probó con una captura real de Wokwi: si el formato difiere, ajusta el lector `leer_vcd()`.
- El sketch compila sin errores para Arduino Uno (5 348 bytes de programa, 428 bytes de RAM estática).
- Los atributos del analizador (`bufferSize`, `channelNames`, `filename`, `triggerMode`, `triggerPin`, `triggerLevel`) coinciden con la documentación oficial.
