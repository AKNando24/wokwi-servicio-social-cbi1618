# Estación completa con Wi-Fi y MQTT

1. Crea un proyecto **ESP32** en [wokwi.com](https://wokwi.com).
2. Pega `main.ino` en la pestaña `sketch.ino`, `diagram.json` y `libraries.txt` en sus pestañas.
3. Presiona **Start Simulation**. Botón GPIO18: cambia de página. Botón GPIO19: reinicia mínimos y máximos.

Topics MQTT: `cbi1618/<ID_ESTACION>/telemetria`, `.../estado` (retenido, con LWT) y `.../comando` (`RESET` o `UMBRAL 32.5`).
