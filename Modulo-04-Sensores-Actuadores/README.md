# Módulo 4: Sensores y Actuadores de Uso Común

## Objetivo
Medir distancia, temperatura y humedad y mover actuadores (servo, motor a pasos, relevador) en un proyecto integrado: un **estacionamiento inteligente** sin `delay()` bloqueantes.

## Componentes
HC-SR04, DHT22, servomotor, driver A4988 + motor a pasos, módulo relevador, LED con resistencia.

## Conexiones
| Componente | Conexión |
|---|---|
| HC-SR04 | TRIG → 7, ECHO → 6, VCC → 5V, GND |
| DHT22 | SDA → 4, VCC → 5V, GND |
| Servomotor | PWM → 9, V+ → 5V, GND |
| A4988 | STEP → 2, DIR → 3, RESET unido a SLEEP, VDD y VMOT → 5V |
| Relevador | IN → 8, COM → 5V, NO → resistencia → LED → GND |

## Cómo ejecutarlo en Wokwi
1. Entra a [wokwi.com](https://wokwi.com) y crea un proyecto nuevo de **Arduino Uno**.
2. Pega el contenido de `main.ino` en la pestaña `sketch.ino`.
3. Reemplaza la pestaña `diagram.json` con el archivo de esta carpeta.
4. Si hay un `libraries.txt`, pega también su contenido en la pestaña del mismo nombre.
5. Presiona **Start Simulation**.

## Librerías
`DHT sensor library` y `Adafruit Unified Sensor` (ver `libraries.txt`). `Servo` ya viene incluida.

## Videos y guiones
| Guion | Duración estimada | Video |
|---|---|---|
| [Guion 4.1 — Sensor ultrasónico HC-SR04: medir distancia](guiones/4.1_sensor-ultrasonico-hc-sr04.md) | ≈ 8:50 min | _pendiente_ |
| [Guion 4.2 — Sensor DHT22: temperatura y humedad](guiones/4.2_sensor-dht22-temperatura-humedad.md) | ≈ 9:10 min | _pendiente_ |
| [Guion 4.3 — Actuadores: servomotor, motor a pasos y relevador](guiones/4.3_servo-motor-a-pasos-y-relevador.md) | ≈ 9:40 min | _pendiente_ |

## Notas de verificación
- Compila sin errores para Arduino Uno (avr-gcc): 10 138 bytes de programa (30,9 %) y 432 bytes de RAM estática.
- Todos los nombres de pines de `diagram.json` se validaron contra la documentación oficial de Wokwi.
- **Verifica en Wokwi** qué nivel activa el módulo relevador (la documentación de la parte lo describe con ambigüedad): si el LED del extractor queda invertido, intercambia `RELAY_ON` y `RELAY_OFF`.
- No se pudo ejecutar la simulación en el entorno donde se generó: prueba el circuito completo antes de grabar.
