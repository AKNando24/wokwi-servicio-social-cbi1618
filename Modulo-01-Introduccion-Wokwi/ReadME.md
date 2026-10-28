# Módulo 1: Introducción a la Simulación de Hardware y Entorno Wokwi

## Puntos Clave del Módulo
1. **Ventajas de la simulación:** Validación rápida de circuito y código sin riesgo de daño físico ni requerimiento de componentes reales.
2. **Placas soportadas:** Arduino (Uno/Nano/Mega), ESP32, Raspberry Pi Pico y STM32.
3. **Análisis de `diagram.json`:**
   * `parts`: Arreglo de componentes con `type`, `id`, posición (`top`, `left`) y atributos (`attrs`).
   * `connections`: Arreglo de pares de pines interconectados con especificación de color de cable.

## Instrucciones para ejecutar en Wokwi
1. Abrir [Wokwi.com](https://wokwi.com).
2. Crear un proyecto nuevo de **Arduino Uno**.
3. Reemplazar el código de `sketch.ino` y la pestaña `diagram.json` con los archivos correspondientes a esta carpeta.
4. Hacer clic en **Start Simulation**.