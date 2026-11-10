# Cambios hechos sobre lo que ya tenías (Módulos 1 a 3)

Al revisar tu repositorio con el código real y con la documentación oficial de Wokwi, estos fueron los ajustes:

| Archivo | Cambio | Motivo |
|---|---|---|
| `Modulo-02-.../main.ino` | Se quitó `IRAM_ATTR` de `handleButtonInterrupt()` y se agregaron dos prototipos de función | `IRAM_ATTR` solo existe en ESP32; en Arduino Uno **provoca error de compilación**. Los prototipos son buena práctica. Si algún día portas este código a ESP32, vuelve a poner `IRAM_ATTR` |
| `Modulo-02-.../diagrama.json` → `diagram.json` | Renombrado y corregidos los pines del botón: `btn1:1.N.1` → `btn1:1.l` y `btn1:2.N.1` → `btn1:2.l` | Wokwi exige el nombre `diagram.json`, y el pulsador solo tiene los pines `1.l`, `1.r`, `2.l` y `2.r`; con los nombres anteriores el botón no se conectaba |
| `Modulo-03-.../diagrama.json` → `diagram.json` | Renombrado | Mismo motivo |
| `ReadME.md` / `ReadMe.md` → `README.md` (Módulos 1, 2 y 3) | Renombrado, contenido intacto | GitHub distingue mayúsculas: conviene un solo nombre en todo el repo |

**No se modificó** `main.ino` de los Módulos 1 y 3, ni el contenido de sus README.

## Observaciones sin cambiar
- El antirrebote del Módulo 2 se aplica después de la interrupción: un rebote durante la secuencia peatonal puede volver a levantar la bandera. Es una limitación conocida y está explicada en el guion 2.3 como mejora.
- Tu `README.md` raíz en GitHub solo tiene dos líneas; el archivo `INDICE_DEL_CURSO.md` trae un índice completo que puedes copiar ahí.

## Qué se verificó y qué no
- Compilados con avr-gcc para Arduino Uno sin errores: Módulos 1 a 7 y el proyecto 9.2 (el Módulo 3 con prototipos agregados solo para la prueba, porque el IDE de Arduino los genera solo).
- Todos los nombres de pines de todos los `diagram.json` se validaron contra la documentación oficial de Wokwi.
- **No compilado** (no hay toolchain de ESP32 en el entorno): Módulos 8, 10.1 y 10.2. `main.py` del 9.1 solo pasó verificación de sintaxis.
- **Ninguna simulación se ejecutó en Wokwi.** Antes de grabar cada video, abre el proyecto en Wokwi y confirma que funciona.
