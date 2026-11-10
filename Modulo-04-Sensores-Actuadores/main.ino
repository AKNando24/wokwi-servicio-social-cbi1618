/*
  Módulo 4: Sensores y Actuadores de Uso Común
  Proyecto: Estacionamiento inteligente
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Qué hace:
  - HC-SR04: detecta un vehículo frente a la barrera (distancia < 30 cm).
  - Servomotor: levanta y baja la barrera con movimiento suave.
  - Motor a pasos + A4988: avanza un cuarto de vuelta para "entregar el boleto".
  - DHT22: mide temperatura y humedad del estacionamiento.
  - Relevador: enciende el extractor cuando la temperatura supera el umbral
    (con histéresis para que no oscile).
  Todo funciona sin delay() bloqueantes: cada tarea usa su propio temporizador.
*/

#include <Servo.h>
#include <DHT.h>

// ---------- Pines ----------
const uint8_t PIN_TRIG  = 7;
const uint8_t PIN_ECHO  = 6;
const uint8_t PIN_DHT   = 4;
const uint8_t PIN_SERVO = 9;
const uint8_t PIN_RELAY = 8;
const uint8_t PIN_STEP  = 2;
const uint8_t PIN_DIR   = 3;

// Nivel que activa el relevador (en Wokwi: transistor "npn" = activo en ALTO).
// Muchos módulos reales son activos en BAJO: invierte estas dos constantes.
const uint8_t RELAY_ON  = HIGH;
const uint8_t RELAY_OFF = LOW;

// ---------- Parámetros ----------
const float         DIST_VEHICULO_CM   = 30.0;   // umbral de detección
const unsigned long RETARDO_CIERRE_MS  = 3000;   // espera sin vehículo antes de cerrar
const unsigned long INTERVALO_SONAR_MS = 100;
const unsigned long INTERVALO_DHT_MS   = 2000;
const unsigned long INTERVALO_SERVO_MS = 15;     // velocidad de la barrera
const float         TEMP_EXTRACTOR_ON  = 30.0;   // °C
const float         TEMP_HISTERESIS    = 2.0;    // °C
const int           ANGULO_CERRADA     = 0;
const int           ANGULO_ABIERTA     = 90;
const int           PASOS_BOLETO       = 50;     // 50 pasos = 1/4 de vuelta (200 pasos/vuelta)
const unsigned long PERIODO_PASO_US    = 4000;

enum EstadoBarrera { CERRADA, ABIERTA };

Servo servoBarrera;
DHT dht(PIN_DHT, DHT22);

EstadoBarrera estado = CERRADA;
int  anguloActual = ANGULO_CERRADA;
int  anguloObjetivo = ANGULO_CERRADA;
int  pasosPendientes = 0;
bool extractorOn = false;
float temperatura = NAN, humedad = NAN, distanciaCm = -1;

unsigned long tSonar = 0, tDHT = 0, tServo = 0, tPaso = 0, tUltimoVehiculo = 0, tReporte = 0;

// ---------- Prototipos ----------
float medirDistanciaCm();
void  abrirBarrera();
void  cerrarBarrera();
void  moverServoSuave();
void  avanzarStepper();
void  leerClima();
void  controlarExtractor();
void  reportar();

void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_STEP, OUTPUT);
  pinMode(PIN_DIR, OUTPUT);
  digitalWrite(PIN_RELAY, RELAY_OFF);
  digitalWrite(PIN_DIR, HIGH);

  servoBarrera.attach(PIN_SERVO);
  servoBarrera.write(ANGULO_CERRADA);
  dht.begin();

  Serial.println(F("--- Módulo 4: Estacionamiento inteligente ---"));
}

void loop() {
  unsigned long ahora = millis();

  // 1) Sonar: detecta vehículo y decide el estado de la barrera
  if (ahora - tSonar >= INTERVALO_SONAR_MS) {
    tSonar = ahora;
    distanciaCm = medirDistanciaCm();
    bool hayVehiculo = (distanciaCm > 0 && distanciaCm < DIST_VEHICULO_CM);

    if (hayVehiculo) {
      tUltimoVehiculo = ahora;
      if (estado == CERRADA) abrirBarrera();
    } else if (estado == ABIERTA && ahora - tUltimoVehiculo >= RETARDO_CIERRE_MS) {
      cerrarBarrera();
    }
  }

  // 2) Clima y extractor
  if (ahora - tDHT >= INTERVALO_DHT_MS) {
    tDHT = ahora;
    leerClima();
    controlarExtractor();
  }

  // 3) Actuadores con movimiento no bloqueante
  moverServoSuave();
  avanzarStepper();

  // 4) Reporte por Serial cada segundo
  if (ahora - tReporte >= 1000) {
    tReporte = ahora;
    reportar();
  }
}

// Dispara el HC-SR04 y convierte el eco a centímetros (eco_us / 58).
float medirDistanciaCm() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  unsigned long eco = pulseIn(PIN_ECHO, HIGH, 25000UL);  // timeout ~4 m
  if (eco == 0) return -1;                               // sin eco
  return eco / 58.0;
}

void abrirBarrera() {
  estado = ABIERTA;
  anguloObjetivo = ANGULO_ABIERTA;
  pasosPendientes += PASOS_BOLETO;                       // entrega de boleto
  Serial.println(F(">> Vehículo detectado: abriendo barrera y entregando boleto"));
}

void cerrarBarrera() {
  estado = CERRADA;
  anguloObjetivo = ANGULO_CERRADA;
  Serial.println(F(">> Sin vehículo: cerrando barrera"));
}

// Mueve el servo 1° cada INTERVALO_SERVO_MS hasta llegar al objetivo.
void moverServoSuave() {
  if (anguloActual == anguloObjetivo) return;
  unsigned long ahora = millis();
  if (ahora - tServo < INTERVALO_SERVO_MS) return;
  tServo = ahora;
  anguloActual += (anguloObjetivo > anguloActual) ? 1 : -1;
  servoBarrera.write(anguloActual);
}

// Un pulso en STEP = un paso del motor (el A4988 genera las fases).
void avanzarStepper() {
  if (pasosPendientes <= 0) return;
  unsigned long ahoraUs = micros();
  if (ahoraUs - tPaso < PERIODO_PASO_US) return;
  tPaso = ahoraUs;
  digitalWrite(PIN_STEP, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_STEP, LOW);
  pasosPendientes--;
}

void leerClima() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) {                            // manejo de errores
    Serial.println(F("[!] Error de lectura del DHT22, se conserva el último valor"));
    return;
  }
  temperatura = t;
  humedad = h;
}

void controlarExtractor() {
  if (isnan(temperatura)) return;
  if (!extractorOn && temperatura >= TEMP_EXTRACTOR_ON) {
    extractorOn = true;
    digitalWrite(PIN_RELAY, RELAY_ON);
    Serial.println(F(">> Temperatura alta: extractor ENCENDIDO"));
  } else if (extractorOn && temperatura <= TEMP_EXTRACTOR_ON - TEMP_HISTERESIS) {
    extractorOn = false;
    digitalWrite(PIN_RELAY, RELAY_OFF);
    Serial.println(F(">> Temperatura normal: extractor APAGADO"));
  }
}

void reportar() {
  Serial.print(F("Dist: "));
  if (distanciaCm < 0) Serial.print(F("---")); else Serial.print(distanciaCm, 1);
  Serial.print(F(" cm | Barrera: "));
  Serial.print(estado == ABIERTA ? F("ABIERTA") : F("CERRADA"));
  Serial.print(F(" | T: "));  Serial.print(temperatura, 1);
  Serial.print(F(" C | H: ")); Serial.print(humedad, 0);
  Serial.print(F(" % | Extractor: "));
  Serial.println(extractorOn ? F("ON") : F("OFF"));
}
