# Módulo 6: Protocolos de Comunicación Serial en Simulación

## Objetivo
Controlar tres buses desde una **consola de comandos por UART**: LCD y OLED por **I2C** (con escáner de direcciones) y ocho LEDs por **SPI** con un 74HC595, con prueba de eco (loopback) MOSI–MISO.

## Componentes
LCD1602 (I2C), OLED SSD1306, registro de desplazamiento 74HC595, barra de 8 LEDs con 8 resistencias de 220 Ω.

## Conexiones
| Bus | Conexión |
|---|---|
| I2C | LCD (0x27) y OLED (0x3C) en A4 (SDA) y A5 (SCL) |
| SPI | 74HC595: DS → 11 (MOSI), SHCP → 13 (SCK), STCP → 10 (CS/latch) |
| Loopback | puente entre el pin 11 (MOSI) y el 12 (MISO) |
| UART | monitor serie a 115200 baudios, terminador «Newline» |

## Cómo ejecutarlo en Wokwi
1. Entra a [wokwi.com](https://wokwi.com) y crea un proyecto nuevo de **Arduino Uno**.
2. Pega el contenido de `main.ino` en la pestaña `sketch.ino`.
3. Reemplaza la pestaña `diagram.json` con el archivo de esta carpeta.
4. Si hay un `libraries.txt`, pega también su contenido en la pestaña del mismo nombre.
5. Presiona **Start Simulation**.

## Librerías
`LiquidCrystal I2C`, `Adafruit SSD1306`, `Adafruit GFX Library` (ver `libraries.txt`). `Wire` y `SPI` ya vienen incluidas.

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 6.1 — UART: comunicación serial y consola de comandos](guiones/6.1_uart-comunicacion-serial.md) | ≈ 9:10 min | _pendiente_ |
| [Guion 6.2 — Bus I2C: maestro/esclavo y escáner de direcciones](guiones/6.2_bus-i2c-y-escaner-de-direcciones.md) | ≈ 9:20 min | _pendiente_ |
| [Guion 6.3 — Bus SPI: MOSI, MISO, SCK y CS con un registro 74HC595](guiones/6.3_bus-spi-y-registro-de-desplazamiento.md) | ≈ 9:30 min | _pendiente_ |

## Notas de verificación
- Comandos: `HELP`, `SCAN`, `LED <0-7> <ON|OFF|T>`, `BAR <0-8>`, `SPI <00-FF>`, `LCD <texto>`, `OLED <texto>`, `STATUS`.
- Compila sin errores para Arduino Uno: 19 556 bytes de programa (59,7 %) y 723 bytes de RAM estática.
- Los pines de `diagram.json` se validaron contra la documentación oficial.
- No se pudo ejecutar la simulación en el entorno donde se generó: comprueba que el escáner detecte 0x27 y 0x3C.
