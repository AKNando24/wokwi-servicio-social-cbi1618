/*
  Módulo 10.1: Estación meteorológica (prototipo sin red)
  Proyecto Integrador - Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Hardware (ESP32): DHT22 (GPIO4), LDR (GPIO34, ADC1), OLED SSD1306 (I2C: SDA 21 / SCL 22),
                    botón de página (GPIO18), botón de reinicio de mínimos/máximos (GPIO19),
                    LED de alarma (GPIO5).
  Integra lo aprendido en los módulos anteriores:
    - entradas digitales con antirrebote (M2), ADC (M3), sensores (M4),
    - pantalla OLED con gráficos (M5), bus I2C (M6), salida CSV por UART (M6/M7).
  Páginas de la pantalla: 1) valores actuales, 2) mínimos y máximos, 3) gráfica de temperatura.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"

// ---------- Pines ----------
const uint8_t PIN_DHT = 4;
const uint8_t PIN_LDR = 34;
const uint8_t PIN_LED = 5;
const uint8_t PIN_BTN_PAGINA = 18;
const uint8_t PIN_BTN_RESET = 19;

// ---------- Parámetros ----------
const float TEMP_ALARMA = 30.0;
const float HISTERESIS = 1.5;
const float ALFA = 0.3;                          // filtro exponencial: 0 = muy suave, 1 = sin filtro
const unsigned long PERIODO_LECTURA_MS = 2000;
const uint8_t N_PUNTOS = 64;

// ---------- Objetos ----------
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
DHTesp dht;

struct Boton { uint8_t pin; bool previo; unsigned long t; };
Boton btnPagina = {PIN_BTN_PAGINA, HIGH, 0};
Boton btnReset = {PIN_BTN_RESET, HIGH, 0};

// ---------- Estado ----------
float temp = NAN, tempFiltrada = NAN, hum = NAN;
int luzPct = 0;
float tMin = 1000, tMax = -1000;
float historial[N_PUNTOS];
uint8_t nPuntos = 0;
bool alarmaActiva = false;
uint8_t pagina = 0;
unsigned long tLectura = 0;

// ---------- Prototipos ----------
bool presionado(Boton &b);
void leerSensores();
void evaluarAlarma();
void dibujar();
void imprimirCSV();

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_BTN_PAGINA, INPUT_PULLUP);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  dht.setup(PIN_DHT, DHTesp::DHT22);

  Wire.begin(21, 22);
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) Serial.println(F("No se encontró la OLED"));
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(F("Estacion CBI-1618"));
  oled.println(F("Iniciando..."));
  oled.display();

  Serial.println(F("tiempo_s,temp_c,temp_filtrada_c,hum_pct,luz_pct,alarma"));
}

void loop() {
  if (presionado(btnPagina)) { pagina = (pagina + 1) % 3; dibujar(); }
  if (presionado(btnReset)) { tMin = 1000; tMax = -1000; nPuntos = 0; dibujar(); }

  if (millis() - tLectura >= PERIODO_LECTURA_MS) {
    tLectura = millis();
    leerSensores();
    evaluarAlarma();
    imprimirCSV();
    dibujar();
  }
}

// Devuelve true una sola vez por pulsación (con antirrebote de 40 ms)
bool presionado(Boton &b) {
  bool actual = digitalRead(b.pin);
  bool evento = false;
  if (actual != b.previo && millis() - b.t > 40) {
    b.t = millis();
    evento = (actual == LOW);
    b.previo = actual;
  }
  return evento;
}

void leerSensores() {
  TempAndHumidity d = dht.getTempAndHumidity();
  if (dht.getStatus() != 0) { Serial.println(F("# error DHT22")); return; }
  temp = d.temperature;
  hum = d.humidity;
  tempFiltrada = isnan(tempFiltrada) ? temp : ALFA * temp + (1 - ALFA) * tempFiltrada;
  luzPct = map(analogRead(PIN_LDR), 0, 4095, 0, 100);

  if (temp < tMin) tMin = temp;
  if (temp > tMax) tMax = temp;
  if (nPuntos == N_PUNTOS) {                       // desplaza el historial
    for (uint8_t i = 1; i < N_PUNTOS; i++) historial[i - 1] = historial[i];
    nPuntos--;
  }
  historial[nPuntos++] = tempFiltrada;
}

void evaluarAlarma() {
  if (!alarmaActiva && tempFiltrada >= TEMP_ALARMA) alarmaActiva = true;
  else if (alarmaActiva && tempFiltrada <= TEMP_ALARMA - HISTERESIS) alarmaActiva = false;
  digitalWrite(PIN_LED, alarmaActiva ? HIGH : LOW);
}

void imprimirCSV() {
  Serial.print(millis() / 1000); Serial.print(',');
  Serial.print(temp, 1); Serial.print(',');
  Serial.print(tempFiltrada, 1); Serial.print(',');
  Serial.print(hum, 0); Serial.print(',');
  Serial.print(luzPct); Serial.print(',');
  Serial.println(alarmaActiva ? 1 : 0);
}

void dibujar() {
  oled.clearDisplay();
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(1);
  oled.setCursor(0, 0);
  oled.print(F("Pag ")); oled.print(pagina + 1); oled.print(F("/3"));
  if (alarmaActiva) { oled.setCursor(80, 0); oled.print(F("! ALARMA")); }

  if (pagina == 0) {
    oled.setTextSize(2);
    oled.setCursor(0, 14); oled.print(tempFiltrada, 1); oled.print(F(" C"));
    oled.setTextSize(1);
    oled.setCursor(0, 38); oled.print(F("Humedad: ")); oled.print(hum, 0); oled.print(F(" %"));
    oled.setCursor(0, 50); oled.print(F("Luz: ")); oled.print(luzPct); oled.print(F(" %"));
  } else if (pagina == 1) {
    oled.setCursor(0, 16); oled.print(F("Minima: ")); oled.print(tMin, 1); oled.print(F(" C"));
    oled.setCursor(0, 30); oled.print(F("Maxima: ")); oled.print(tMax, 1); oled.print(F(" C"));
    oled.setCursor(0, 48); oled.print(F("Boton 2: reiniciar"));
  } else {
    oled.drawRect(0, 12, 128, 52, SSD1306_WHITE);
    for (uint8_t i = 1; i < nPuntos; i++) {
      int y0 = 62 - constrain(map((int)(historial[i - 1] * 10), 100, 450, 0, 48), 0, 48);
      int y1 = 62 - constrain(map((int)(historial[i] * 10), 100, 450, 0, 48), 0, 48);
      oled.drawLine((i - 1) * 2 + 1, y0, i * 2 + 1, y1, SSD1306_WHITE);
    }
  }
  oled.display();
}
