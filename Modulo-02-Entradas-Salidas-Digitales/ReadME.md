# Módulo 2: Entradas y Salidas Digitales / Manejo de Señales

## Conceptos Clave
1. **Configuración de Resistencias:**
   * **Pull-Up / Pull-Down:** Evitan estados flotantes (ruido eléctrico) en los pines digitales de entrada.
   * **`INPUT_PULLUP`:** Activa la resistencia interna del microcontrolador (~20kΩ-50kΩ) a VCC, simplificando las conexiones físicas al conectar el botón directamente a GND.
2. **Antirrebote (Debouncing) por Software:**
   * Técnica para filtrar los falsos pulsos mecánicos de un botón mediante una ventana de tiempo (`millis() > debounceDelay`).
3. **Interrupciones Básicas:**
   * Uso de `attachInterrupt()` para responder de forma asíncrona a eventos críticos en el circuito.