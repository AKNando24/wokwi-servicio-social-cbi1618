/*
  Módulo 1: Introducción a Wokwi y Estructura de diagram.json
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto
*/

const int LED_PIN = 13;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("--- Módulo 1: Entorno Wokwi iniciado ---");
  Serial.println("Placas soportadas: Arduino Uno/Nano/Mega, ESP32, Raspberry Pi Pico y STM32.");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Estado LED: ENCENDIDO");
  delay(1000);

  digitalWrite(LED_PIN, LOW);
  Serial.println("Estado LED: APAGADO");
  delay(1000);
}