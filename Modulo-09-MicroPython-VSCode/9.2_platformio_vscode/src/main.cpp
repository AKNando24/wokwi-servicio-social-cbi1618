/*
  Módulo 9.2: VS Code + PlatformIO + Wokwi, con biblioteca propia
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  El potenciómetro fija el límite de temperatura (20 a 40 °C). La temperatura del DHT22 pasa
  por un filtro de promedio móvil (EstacionLib) y, si supera el límite, se enciende la alarma
  con histéresis de 1.5 °C. Imprime una línea CSV por segundo, lista para graficar.
*/
#include <Arduino.h>
#include <DHT.h>
#include "EstacionLib.h"

const uint8_t PIN_DHT = 2;
const uint8_t PIN_POT = A0;
const uint8_t PIN_ALARMA = 8;

DHT dht(PIN_DHT, DHT22);
FiltroMedia filtro;
Umbral alarma(1.5);

unsigned long tLectura = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_ALARMA, OUTPUT);
  dht.begin();
  Serial.println(F("tiempo_s,temp_c,temp_filtrada_c,limite_c,alarma"));
}

void loop() {
  if (millis() - tLectura < 1000) return;
  tLectura = millis();

  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println(F("# error de lectura DHT22"));
    return;
  }
  filtro.agregar(t);
  float limite = 20.0 + (analogRead(PIN_POT) / 1023.0) * 20.0;   // 20-40 °C
  bool activa = alarma.evaluar(filtro.promedio(), limite);
  digitalWrite(PIN_ALARMA, activa ? HIGH : LOW);

  Serial.print(tLectura / 1000); Serial.print(',');
  Serial.print(t, 1); Serial.print(',');
  Serial.print(filtro.promedio(), 1); Serial.print(',');
  Serial.print(limite, 1); Serial.print(',');
  Serial.println(activa ? 1 : 0);
}
