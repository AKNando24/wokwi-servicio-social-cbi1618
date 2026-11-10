# 9.1 MicroPython en Raspberry Pi Pico y ESP32

Un solo `main.py` funciona en ambas placas (detecta `sys.platform`).

| Función | Raspberry Pi Pico | ESP32 |
|---|---|---|
| LED (PWM) | GP15 | GPIO18 |
| Botón (pull-up) | GP14 | GPIO19 |
| DHT22 | GP16 | GPIO4 |
| Potenciómetro (ADC) | GP26 | GPIO34 |

**Cómo ejecutarlo:** crea en Wokwi un proyecto *MicroPython* para la placa elegida, pega `main.py` y el `diagram.json` de la subcarpeta `pico/` o `esp32/`, y presiona **Start Simulation**.
