/*
  Módulo 8: Internet de las Cosas con ESP32
  Proyecto: Nodo IoT que consulta una API, publica telemetría y recibe comandos
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Flujo del programa:
   1) Se conecta a la red Wi-Fi virtual de Wokwi ("Wokwi-GUEST", sin contraseña, canal 6).
   2) Sincroniza la hora por NTP.
   3) HTTPS GET: consulta la temperatura exterior a la API pública Open-Meteo (sin llave).
   4) HTTP POST: envía una lectura en JSON a httpbin.org, que devuelve lo recibido (eco).
   5) MQTT: publica telemetría JSON cada 5 s y escucha comandos para el LED.

  Topics MQTT (cambia ID_UNICO: el broker público es compartido por todo el mundo):
    cbi1618/<ID_UNICO>/telemetria   (publica el ESP32)
    cbi1618/<ID_UNICO>/comando      (suscrito: "ON", "OFF" o "TOGGLE")
    cbi1618/<ID_UNICO>/estado       (online / offline, con mensaje de última voluntad)

  Nota para ESP32: el LDR va en GPIO34 (ADC1). Los pines de ADC2 no funcionan con Wi-Fi activo.
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHTesp.h"
#include <time.h>

// ---------- Configuración ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int   WIFI_CANAL = 6;                 // acelera la conexión en Wokwi

const char* MQTT_BROKER = "broker.hivemq.com";
const uint16_t MQTT_PUERTO = 1883;
const char* ID_UNICO = "fperez-demo";       // <-- CÁMBIALO por algo propio

// Coordenadas para la consulta de clima (cámbialas por tu ciudad)
const float LATITUD = 19.43;
const float LONGITUD = -99.13;
const long  ZONA_HORARIA_S = -6 * 3600;     // UTC-6

const uint8_t PIN_DHT = 4;
const uint8_t PIN_LDR = 34;
const uint8_t PIN_LED = 18;

const unsigned long PERIODO_TELEMETRIA_MS = 5000;
const unsigned long PERIODO_CLIMA_MS = 60000;

// ---------- Objetos y estado ----------
DHTesp dht;
WiFiClient clienteWifi;
PubSubClient mqtt(clienteWifi);

char topicTelemetria[64], topicComando[64], topicEstado[64];
float tempExterior = NAN;
bool ledEncendido = false;
unsigned long tTelemetria = 0, tClima = 0, tReintentoMqtt = 0;

// ---------- Prototipos ----------
void conectarWiFi();
void sincronizarHora();
void consultarClimaExterno();
void enviarPost(float temp, float hum);
bool conectarMQTT();
void alRecibirMensaje(char* topic, byte* payload, unsigned int longitud);
void publicarTelemetria();
void fijarLed(bool encendido);
String horaActual();

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.setup(PIN_DHT, DHTesp::DHT22);

  snprintf(topicTelemetria, sizeof(topicTelemetria), "cbi1618/%s/telemetria", ID_UNICO);
  snprintf(topicComando, sizeof(topicComando), "cbi1618/%s/comando", ID_UNICO);
  snprintf(topicEstado, sizeof(topicEstado), "cbi1618/%s/estado", ID_UNICO);

  Serial.println(F("--- Módulo 8: Nodo IoT ESP32 ---"));
  conectarWiFi();
  sincronizarHora();

  mqtt.setServer(MQTT_BROKER, MQTT_PUERTO);
  mqtt.setCallback(alRecibirMensaje);

  consultarClimaExterno();
  TempAndHumidity d = dht.getTempAndHumidity();
  if (dht.getStatus() == 0) enviarPost(d.temperature, d.humidity);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectarWiFi();

  // Reconexión MQTT sin bloquear el programa (reintenta cada 5 s)
  if (!mqtt.connected()) {
    if (millis() - tReintentoMqtt > 5000) {
      tReintentoMqtt = millis();
      conectarMQTT();
    }
  } else {
    mqtt.loop();
  }

  unsigned long ahora = millis();
  if (ahora - tTelemetria >= PERIODO_TELEMETRIA_MS) {
    tTelemetria = ahora;
    publicarTelemetria();
  }
  if (ahora - tClima >= PERIODO_CLIMA_MS) {
    tClima = ahora;
    consultarClimaExterno();
  }
}

void conectarWiFi() {
  Serial.print(F("Conectando a Wi-Fi "));
  Serial.print(WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CANAL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print('.');
  }
  Serial.print(F(" conectado. IP: "));
  Serial.println(WiFi.localIP());
}

void sincronizarHora() {
  configTime(ZONA_HORARIA_S, 0, "pool.ntp.org", "time.nist.gov");
  struct tm t;
  if (getLocalTime(&t, 10000)) {
    Serial.print(F("Hora sincronizada: "));
    Serial.println(horaActual());
  } else {
    Serial.println(F("No se pudo sincronizar la hora por NTP"));
  }
}

String horaActual() {
  struct tm t;
  if (!getLocalTime(&t, 100)) return "sin-hora";
  char buf[24];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
  return String(buf);
}

// HTTPS GET + lectura de JSON con ArduinoJson
void consultarClimaExterno() {
  WiFiClientSecure seguro;
  seguro.setInsecure();   // solo para el curso: en un producto real valida el certificado
  HTTPClient http;
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(LATITUD, 2) +
               "&longitude=" + String(LONGITUD, 2) + "&current=temperature_2m";
  if (!http.begin(seguro, url)) { Serial.println(F("[GET] no se pudo iniciar")); return; }
  int codigo = http.GET();
  if (codigo == HTTP_CODE_OK) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getString());
    if (!err) {
      tempExterior = doc["current"]["temperature_2m"] | NAN;
      Serial.print(F("[GET] Temperatura exterior: ")); Serial.print(tempExterior); Serial.println(F(" C"));
    } else {
      Serial.print(F("[GET] JSON inválido: ")); Serial.println(err.c_str());
    }
  } else {
    Serial.print(F("[GET] error, código ")); Serial.println(codigo);
  }
  http.end();
}

// HTTP POST con cuerpo JSON; httpbin devuelve en su respuesta lo que recibió.
void enviarPost(float temp, float hum) {
  HTTPClient http;
  http.begin("http://httpbin.org/post");
  http.addHeader("Content-Type", "application/json");
  char cuerpo[96];
  snprintf(cuerpo, sizeof(cuerpo), "{\"temp\":%.1f,\"hum\":%.1f,\"origen\":\"esp32-wokwi\"}", temp, hum);
  int codigo = http.POST(cuerpo);
  Serial.print(F("[POST] código ")); Serial.println(codigo);
  if (codigo > 0) Serial.println(http.getString());
  http.end();
}

bool conectarMQTT() {
  char clientId[40];
  snprintf(clientId, sizeof(clientId), "cbi1618-%s-%04X", ID_UNICO, (unsigned)random(0xFFFF));
  Serial.print(F("Conectando a MQTT... "));
  // Último deseo (LWT): si el ESP32 se cae, el broker publica "offline"
  if (mqtt.connect(clientId, NULL, NULL, topicEstado, 0, true, "offline")) {
    Serial.println(F("OK"));
    mqtt.publish(topicEstado, "online", true);
    mqtt.subscribe(topicComando);
    return true;
  }
  Serial.print(F("falló, rc=")); Serial.println(mqtt.state());
  return false;
}

// Se ejecuta cada vez que llega un mensaje a un topic suscrito
void alRecibirMensaje(char* topic, byte* payload, unsigned int longitud) {
  char msg[16];
  unsigned int n = min(longitud, (unsigned int)(sizeof(msg) - 1));
  memcpy(msg, payload, n);
  msg[n] = '\0';
  Serial.print(F("[MQTT] ")); Serial.print(topic); Serial.print(F(" -> ")); Serial.println(msg);
  if (!strcasecmp(msg, "ON")) fijarLed(true);
  else if (!strcasecmp(msg, "OFF")) fijarLed(false);
  else if (!strcasecmp(msg, "TOGGLE")) fijarLed(!ledEncendido);
}

void fijarLed(bool encendido) {
  ledEncendido = encendido;
  digitalWrite(PIN_LED, encendido ? HIGH : LOW);
}

void publicarTelemetria() {
  TempAndHumidity d = dht.getTempAndHumidity();
  if (dht.getStatus() != 0) { Serial.println(F("[!] Error leyendo DHT22")); return; }
  int luz = analogRead(PIN_LDR);                         // 0-4095

  JsonDocument doc;
  doc["hora"] = horaActual();
  doc["temp"] = round(d.temperature * 10) / 10.0;
  doc["hum"] = round(d.humidity * 10) / 10.0;
  doc["luz"] = luz;
  if (!isnan(tempExterior)) doc["temp_ext"] = tempExterior;
  doc["led"] = ledEncendido;
  char json[200];
  serializeJson(doc, json, sizeof(json));

  Serial.print(F("[MQTT] publica: ")); Serial.println(json);
  if (mqtt.connected()) mqtt.publish(topicTelemetria, json);
}
