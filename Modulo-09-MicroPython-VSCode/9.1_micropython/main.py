# Módulo 9.1: MicroPython en Raspberry Pi Pico y ESP32 (mismo programa, dos placas)
# Servicio Social CBI-1618 - Ing. en Computación
# Prestador: Pérez Sánchez Fernando Alberto
#
# - LED con brillo controlado por PWM desde un potenciómetro (ADC)
# - Botón que alterna entre brillo variable y parpadeo (interrupción + antirrebote)
# - DHT22 leído cada 2 s con manejo de errores
# Equivalente en C++: Módulos 2, 3 y 4 de este repositorio.

import sys
import time
from machine import Pin, ADC, PWM
import dht

# --- Pines según la placa (sys.platform: "rp2" = Pico, "esp32" = ESP32) ---
if sys.platform == "rp2":
    PIN_LED, PIN_BOTON, PIN_DHT, PIN_POT = 15, 14, 16, 26
else:
    PIN_LED, PIN_BOTON, PIN_DHT, PIN_POT = 18, 19, 4, 34

led = PWM(Pin(PIN_LED), freq=1000)                 # ~ analogWrite() de Arduino
boton = Pin(PIN_BOTON, Pin.IN, Pin.PULL_UP)        # ~ INPUT_PULLUP
pot = ADC(Pin(PIN_POT))
if sys.platform != "rp2":
    pot.atten(ADC.ATTN_11DB)                       # rango completo 0-3.3 V en ESP32
sensor = dht.DHT22(Pin(PIN_DHT))

modo_parpadeo = False
ultimo_toque = 0


def alternar_modo(pin):
    """Interrupción del botón con antirrebote de 200 ms."""
    global modo_parpadeo, ultimo_toque
    ahora = time.ticks_ms()
    if time.ticks_diff(ahora, ultimo_toque) > 200:
        modo_parpadeo = not modo_parpadeo
        ultimo_toque = ahora


boton.irq(trigger=Pin.IRQ_FALLING, handler=alternar_modo)

print("--- Módulo 9.1: MicroPython en", sys.platform, "---")

t_dht = time.ticks_ms()
t_blink = time.ticks_ms()
led_on = False

while True:
    ahora = time.ticks_ms()
    brillo = pot.read_u16()                        # 0-65535 en ambas placas

    if modo_parpadeo:
        if time.ticks_diff(ahora, t_blink) >= 250:
            t_blink = ahora
            led_on = not led_on
        led.duty_u16(brillo if led_on else 0)
    else:
        led.duty_u16(brillo)

    if time.ticks_diff(ahora, t_dht) >= 2000:
        t_dht = ahora
        try:
            sensor.measure()
            print("T = {:.1f} C | H = {:.0f} % | brillo = {:3d} % | modo = {}".format(
                sensor.temperature(), sensor.humidity(),
                brillo * 100 // 65535, "parpadeo" if modo_parpadeo else "variable"))
        except OSError:
            print("[!] Error leyendo el DHT22")

    time.sleep_ms(10)
