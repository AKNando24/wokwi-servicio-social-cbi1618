# Módulo 5: Interfaces de Visualización e Interacción

## Objetivo
Mostrar una misma medición del DHT22 en cuatro pantallas: **LCD I2C**, **OLED SSD1306**, **matriz MAX7219** y **display de 7 segmentos de 4 dígitos** (multiplexado), con un botón para cambiar el valor mostrado.

## Componentes
DHT22, LCD1602 (I2C), OLED SSD1306, matriz 8x8 MAX7219, display de 7 segmentos de 4 dígitos (ánodo común), 7 resistencias de 220 Ω, botón.

## Conexiones
| Componente | Conexión |
|---|---|
| DHT22 | SDA → 12 |
| LCD (0x27) y OLED (0x3C) | SDA → A4, SCL → A5 (bus compartido) |
| MAX7219 | DIN → 11, CLK → 13, CS → 10 |
| 7 segmentos | A–G → pines 2–8 (cada uno con 220 Ω); DIG1–DIG4 → 9, A0, A1, A2 |
| Botón de modo | A3 → GND (INPUT_PULLUP) |

## Cómo ejecutarlo en Wokwi
1. Entra a [wokwi.com](https://wokwi.com) y crea un proyecto nuevo de **Arduino Uno**.
2. Pega el contenido de `main.ino` en la pestaña `sketch.ino`.
3. Reemplaza la pestaña `diagram.json` con el archivo de esta carpeta.
4. Si hay un `libraries.txt`, pega también su contenido en la pestaña del mismo nombre.
5. Presiona **Start Simulation**.

## Librerías
`DHT sensor library`, `Adafruit Unified Sensor`, `LiquidCrystal I2C`, `Adafruit SSD1306`, `Adafruit GFX Library` (ver `libraries.txt`). El MAX7219 se controla escribiendo sus registros a mano, sin librería.

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 5.1 — Pantallas LCD alfanuméricas 16x2 y 20x4: bus directo e I2C](guiones/5.1_pantallas-lcd-16x2-y-20x4.md) | ≈ 8:50 min | _pendiente_ |
| [Guion 5.2 — Pantalla OLED SSD1306 con Adafruit_GFX](guiones/5.2_oled-ssd1306-y-adafruit-gfx.md) | ≈ 9:30 min | _pendiente_ |
| [Guion 5.3 — Displays de 7 segmentos y matrices de LEDs con MAX7219](guiones/5.3_siete-segmentos-y-matriz-max7219.md) | ≈ 9:30 min | _pendiente_ |

## Notas de verificación
- Compila sin errores para Arduino Uno: 22 190 bytes de programa (67,7 %) y 768 bytes de RAM estática. La OLED reserva además 1 024 bytes de búfer al iniciar, así que la RAM del Uno queda muy ajustada: no agregues variables grandes.
- Los pines de `diagram.json` se validaron contra la documentación oficial.
- **Verifica en Wokwi** la orientación de la matriz MAX7219 (si la barra aparece girada, invierte el orden de las filas en `actualizarMatriz()`).
- No se pudo ejecutar la simulación en el entorno donde se generó.
