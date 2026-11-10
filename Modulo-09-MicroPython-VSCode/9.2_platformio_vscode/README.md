# 9.2 VS Code + PlatformIO + Wokwi

1. Instala **VS Code**, la extensión **PlatformIO IDE** y la extensión **Wokwi Simulator**; activa la licencia (F1 → *Wokwi: Request a new License*).
2. Abre esta carpeta en VS Code y compila con el botón *Build* de PlatformIO (o `pio run`).
3. F1 → *Wokwi: Start Simulator*. La extensión usa `wokwi.toml` y `diagram.json`.
4. Abre el monitor serie: verás una línea CSV por segundo (`tiempo_s,temp_c,temp_filtrada_c,limite_c,alarma`).

La librería propia está en `lib/EstacionLib/` (`FiltroMedia` y `Umbral`); PlatformIO la detecta sola.
**En el editor web de Wokwi** no existe `lib/`: agrega `EstacionLib.h` y `EstacionLib.cpp` como archivos del proyecto.
