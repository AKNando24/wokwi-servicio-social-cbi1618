/*
  Módulo 10.2: Estación meteorológica completa (Wi-Fi + MQTT)
  Proyecto Integrador - Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Hardware (ESP32): DHT22 (GPIO4), LDR (GPIO34, ADC1), OLED SSD1306 (I2C: SDA 21 / SCL 22),
                    botón de página (GPIO18), botón de reinicio de mínimos/máximos (GPIO19),
                    LED de alarma (GPIO5).
  Integra lo aprendido en los módulos anteriores:
    - entradas digitales con antirrebote (M2), ADC (M3), sensores (M4),
    - pantalla OLED con gráficos (M5), bus I2C (M6), salida CSV por UART (M6/M7).
  Páginas de la pantalla: 1) valores actuales, 2) mínimos y máximos, 3) gráfica de temperatura.

  Novedades de la versión 10.2:
    - Wi-Fi (Wokwi-GUEST) y reconexión automática sin bloquear la estación.
    - Hora por NTP para marcar cada medición (UTC, formato ISO 8601).
    - MQTT: publica telemetría JSON, estado con mensaje de última voluntad (LWT) y recibe comandos.
        cbi1618/<ID_ESTACION>/telemetria  -> JSON con temp, hum, luz, min, max, alarma
        cbi1618/<ID_ESTACION>/estado      -> "online" / "offline" (retenido)
        cbi1618/<ID_ESTACION>/comando     -> "RESET" | "UMBRAL <°C>"
  La estación sigue midiendo y mostrando datos aunque la red falle.
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHTesp.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <time.h>

// ---------- Pines ----------
const uint8_t PIN_DHT = 4;
const uint8_t PIN_LDR = 34;
const uint8_t PIN_LED = 5;
const uint8_t PIN_BTN_PAGINA = 18;
const uint8_t PIN_BTN_RESET = 19;

// ---------- Parámetros ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int   WIFI_CANAL = 6;
const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PUERTO = 1883;
const char* ID_ESTACION = "fperez-estacion1";    // <-- CÁMBIALO: el broker es público y compartido
const unsigned long PERIODO_PUBLICACION_MS = 10000;
const unsigned long REINTENTO_RED_MS = 10000;

float TEMP_ALARMA = 30.0;                        // se puede cambiar por MQTT
const float HISTERESIS = 1.5;
const float ALFA = 0.3;                          // filtro exponencial: 0 = muy suave, 1 = sin filtro
const unsigned long PERIODO_LECTURA_MS = 2000;
const uint8_t N_PUNTOS = 64;

// ---------- Objetos ----------
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
DHTesp dht;
WiFiClient clienteWifi;
PubSubClient mqtt(clienteWifi);
char topicTelemetria[64], topicEstado[64], topicComando[64];
unsigned long tPublicacion = 0, tReintentoWifi = 0, tReintentoMqtt = 0;
bool wifiIniciado = false;

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
void gestionarRed();
void publicarTelemetria();
void alRecibirMensaje(char* topic, byte* payload, unsigned int longitud);
String marcaDeTiempo();

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

  snprintf(topicTelemetria, sizeof(topicTelemetria), "cbi1618/%s/telemetria", ID_ESTACION);
  snprintf(topicEstado, sizeof(topicEstado), "cbi1618/%s/estado", ID_ESTACION);
  snprintf(topicComando, sizeof(topicComando), "cbi1618/%s/comando", ID_ESTACION);
  mqtt.setServer(MQTT_BROKER, MQTT_PUERTO);
  mqtt.setCallback(alRecibirMensaje);
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println(F("tiempo_s,temp_c,temp_filtrada_c,hum_pct,luz_pct,alarma"));
}

void loop() {
  gestionarRed();
  if (presionado(btnPagina)) { pagina = (pagina + 1) % 3; dibujar(); }
  if (presionado(btnReset)) { tMin = 1000; tMax = -1000; nPuntos = 0; dibujar(); }

  if (millis() - tLectura >= PERIODO_LECTURA_MS) {
    tLectura = millis();
    leerSensores();
    evaluarAlarma();
    imprimirCSV();
    dibujar();
  }

  if (mqtt.connected() && millis() - tPublicacion >= PERIODO_PUBLICACION_MS) {
    tPublicacion = millis();
    publicarTelemetria();
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
  oled.setCursor(44, 0);
  oled.print(WiFi.status() == WL_CONNECTED ? F("W+") : F("W-"));
  oled.print(mqtt.connected() ? F(" M+") : F(" M-"));
  if (alarmaActiva) { oled.setCursor(92, 0); oled.print(F("ALARM")); }

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

// Mantiene Wi-Fi y MQTT sin bloquear: si algo falla, reintenta cada REINTENTO_RED_MS.
void gestionarRed() {
  if (WiFi.status() != WL_CONNECTED) {
    if (!wifiIniciado || millis() - tReintentoWifi > REINTENTO_RED_MS) {
      tReintentoWifi = millis();
      wifiIniciado = true;
      Serial.println(F("# Wi-Fi: conectando..."));
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CANAL);
    }
    return;
  }
  if (!mqtt.connected()) {
    if (millis() - tReintentoMqtt > REINTENTO_RED_MS) {
      tReintentoMqtt = millis();
      char clientId[40];
      snprintf(clientId, sizeof(clientId), "cbi1618-%s-%04X", ID_ESTACION, (unsigned)random(0xFFFF));
      if (mqtt.connect(clientId, NULL, NULL, topicEstado, 0, true, "offline")) {
        Serial.println(F("# MQTT: conectado"));
        mqtt.publish(topicEstado, "online", true);
        mqtt.subscribe(topicComando);
      } else {
        Serial.print(F("# MQTT: falló rc=")); Serial.println(mqtt.state());
      }
    }
  } else {
    mqtt.loop();
  }
}

String marcaDeTiempo() {
  struct tm t;
  if (!getLocalTime(&t, 50)) return "sin-hora";
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &t);
  return String(buf);
}

void publicarTelemetria() {
  if (isnan(temp)) return;
  char json[240];
  snprintf(json, sizeof(json),
           "{\"estacion\":\"%s\",\"ts\":\"%s\",\"temp\":%.1f,\"hum\":%.0f,\"luz\":%d,"
           "\"min\":%.1f,\"max\":%.1f,\"umbral\":%.1f,\"alarma\":%s}",
           ID_ESTACION, marcaDeTiempo().c_str(), tempFiltrada, hum, luzPct,
           tMin, tMax, TEMP_ALARMA, alarmaActiva ? "true" : "false");
  mqtt.publish(topicTelemetria, json);
  Serial.print(F("# MQTT publica: ")); Serial.println(json);
}

// Comandos: "RESET" reinicia mínimos/máximos; "UMBRAL 32.5" cambia el límite de la alarma.
void alRecibirMensaje(char* topic, byte* payload, unsigned int longitud) {
  char msg[24];
  unsigned int n = min(longitud, (unsigned int)(sizeof(msg) - 1));
  memcpy(msg, payload, n);
  msg[n] = '\0';
  Serial.print(F("# Comando recibido: ")); Serial.println(msg);
  if (!strcasecmp(msg, "RESET")) {
    tMin = 1000; tMax = -1000; nPuntos = 0;
  } else if (!strncasecmp(msg, "UMBRAL ", 7)) {
    float v = atof(msg + 7);
    if (v >= 10 && v <= 60) TEMP_ALARMA = v;           // valida el rango antes de aceptar
  }
}
