/*
  Módulo 7: Depuración Avanzada e Instrumentación Virtual
  Proyecto: Generador de tramas para el analizador lógico de Wokwi
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Cada 500 ms el programa produce una "trama" con cuatro señales conocidas:
    UART : el texto "HOLA" a 9600 baudios                      -> canal D0 (TX)
    I2C  : dos bytes (0xA5, 0x5A) hacia el LCD en 0x27        -> canales D1 (SDA) y D2 (SCL)
    SPI  : dos bytes (0xAA, 0x55) hacia el 74HC595 a 1 MHz    -> canales D3 (MOSI), D4 (SCK), D5 (CS)
    PWM  : ciclo de trabajo que sube 12.5 % por trama (490 Hz) -> canal D6
  El pin 8 sube durante la trama y sirve como disparador (trigger) en el analizador (canal D7).

  Al detener la simulación, Wokwi descarga un archivo .vcd que se abre con PulseView o GTKWave.
  Como la secuencia es conocida, puedes comprobar que lo capturado coincide con lo programado.
*/

#include <Wire.h>
#include <SPI.h>

const uint8_t PIN_TRIGGER = 8;
const uint8_t PIN_PWM = 9;
const uint8_t PIN_CS = 10;
const uint8_t DIRECCION_I2C = 0x27;
const unsigned long PERIODO_TRAMA_MS = 500;

uint8_t duty = 0;
unsigned long tTrama = 0;

void enviarUART();
void enviarI2C();
void enviarSPI();

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Wire.setClock(100000);                    // 100 kHz (modo estándar)
  SPI.begin();
  pinMode(PIN_TRIGGER, OUTPUT);
  pinMode(PIN_CS, OUTPUT);
  digitalWrite(PIN_CS, HIGH);
  pinMode(PIN_PWM, OUTPUT);
}

void loop() {
  if (millis() - tTrama < PERIODO_TRAMA_MS) return;
  tTrama = millis();

  digitalWrite(PIN_TRIGGER, HIGH);          // inicio de trama: dispara la captura
  enviarUART();
  enviarI2C();
  enviarSPI();
  duty += 32;                               // 0, 32, 64 ... 224, 0 ... (8 pasos)
  analogWrite(PIN_PWM, duty);
  digitalWrite(PIN_TRIGGER, LOW);
}

void enviarUART() {
  Serial.println(F("HOLA"));                // 0x48 0x4F 0x4C 0x41 0x0D 0x0A
  Serial.flush();                           // espera a que termine de salir
}

void enviarI2C() {
  Wire.beginTransmission(DIRECCION_I2C);    // START + dirección 0x27 (escritura)
  Wire.write(0xA5);
  Wire.write(0x5A);
  Wire.endTransmission();                   // STOP
}

void enviarSPI() {
  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_CS, LOW);
  SPI.transfer(0xAA);
  SPI.transfer(0x55);
  digitalWrite(PIN_CS, HIGH);
  SPI.endTransaction();
}
