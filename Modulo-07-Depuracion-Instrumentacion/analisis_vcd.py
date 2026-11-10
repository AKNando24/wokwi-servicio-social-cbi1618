#!/usr/bin/env python3
"""
analisis_vcd.py - Mide bit rate (UART) y frecuencia de reloj (SPI/I2C/PWM) en un archivo VCD
exportado por el analizador lógico de Wokwi. Solo usa la biblioteca estándar de Python.

Uso:
    python3 analisis_vcd.py modulo7-captura.vcd

Servicio Social CBI-1618 - Pérez Sánchez Fernando Alberto
"""
import sys
from statistics import median

ESCALAS = {"fs": 1e-15, "ps": 1e-12, "ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}


def leer_vcd(ruta):
    """Devuelve (nombres, cambios): cambios[nombre] = [(tiempo_en_segundos, valor), ...]"""
    ids, cambios, escala, t = {}, {}, 1e-9, 0
    with open(ruta) as f:
        tokens = f.read().split()
    i = 0
    while i < len(tokens):
        tk = tokens[i]
        if tk == "$timescale":
            valor = ""
            i += 1
            while tokens[i] != "$end":
                valor += tokens[i]
                i += 1
            numero = "".join(c for c in valor if c.isdigit())
            unidad = "".join(c for c in valor if c.isalpha())
            escala = int(numero) * ESCALAS[unidad]
        elif tk == "$var":
            # $var wire 1 <id> <nombre> $end
            ids[tokens[i + 3]] = tokens[i + 4]
            cambios[tokens[i + 4]] = []
            i += 5
        elif tk.startswith("#"):
            t = int(tk[1:])
        elif tk[0] in "01xXzZ" and tk[1:] in ids:
            cambios[ids[tk[1:]]].append((t * escala, tk[0]))
        i += 1
    return cambios


def flancos(serie, nivel):
    """Tiempos de los flancos que llegan a 'nivel' ('1' = subida, '0' = bajada)."""
    return [t for t, v in serie if v == nivel]


def ancho_minimo(serie):
    """Duración del pulso más corto = tiempo de bit en una señal UART."""
    anchos = [b[0] - a[0] for a, b in zip(serie, serie[1:]) if a[1] in "01" and b[0] > a[0]]
    return min(anchos) if anchos else None


def frecuencia(serie):
    """Frecuencia a partir del periodo mediano entre flancos de subida consecutivos."""
    subidas = flancos(serie, "1")
    periodos = [b - a for a, b in zip(subidas, subidas[1:])]
    # se descartan los saltos grandes entre tramas (más de 10 veces la mediana)
    if not periodos:
        return None
    m = median(periodos)
    utiles = [p for p in periodos if p < 10 * m]
    return 1.0 / median(utiles)


def main():
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(1)
    cambios = leer_vcd(sys.argv[1])
    print("Canales encontrados:", ", ".join(f"{n} ({len(v)} cambios)" for n, v in cambios.items()))

    if "TX" in cambios and cambios["TX"]:
        ancho = ancho_minimo(cambios["TX"])
        if ancho:
            print(f"\nUART  (TX): bit = {ancho * 1e6:.1f} us  ->  ~{1 / ancho:.0f} baudios")
    for nombre in ("SCK", "SCL", "PWM"):
        if nombre in cambios and len(cambios[nombre]) > 2:
            f = frecuencia(cambios[nombre])
            if f:
                unidad, valor = ("kHz", f / 1e3) if f >= 1e3 else ("Hz", f)
                print(f"{nombre:<5}: frecuencia = {valor:.2f} {unidad}")


if __name__ == "__main__":
    main()
