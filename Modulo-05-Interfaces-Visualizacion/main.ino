/*
  Módulo 5: Interfaces de Visualización e Interacción
  Proyecto: Panel de monitoreo multi-pantalla
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Una misma medición (DHT22) se muestra de cuatro formas distintas:
  - LCD 16x2 por I2C (PCF8574, dirección 0x27): valores y modo activo.
  - OLED SSD1306 128x64 por I2C (dirección 0x3C): gráfica del historial con Adafruit_GFX.
  - Matriz 8x8 con MAX7219 (registros escritos a mano con shiftOut): barra de nivel de temperatura.
  - Display de 7 segmentos de 4 dígitos con multiplexado directo.
  Un botón cambia el valor mostrado en los 7 segmentos (temperatura, humedad, segundos encendido).
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// ---------- Pines ----------
const uint8_t PIN_DHT = 12;
const uint8_t PIN_BOTON = A3;
const uint8_t PIN_MAX_DIN = 11, PIN_MAX_CLK = 13, PIN_MAX_CS = 10;
const uint8_t SEG[7] = {2, 3, 4, 5, 6, 7, 8};        // segmentos A..G
const uint8_t DIG[4] = {9, A0, A1, A2};               // dígitos 1..4

// ---------- Objetos ----------
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
DHT dht(PIN_DHT, DHT22);

// Patrones 0-9: bit0 = A ... bit6 = G (1 = segmento encendido)
const uint8_t DIGITOS[10] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

// ---------- Estado ----------
const uint8_t N_HIST = 64;
int8_t historial[N_HIST];           // temperatura entera de las últimas 64 lecturas
uint8_t nHist = 0;
float temperatura = NAN, humedad = NAN;
uint8_t modo = 0;                   // 0 = temp, 1 = humedad, 2 = segundos encendido
int valor7seg = 0;
uint8_t digitoActual = 0;

unsigned long tLectura = 0, tMux = 0, tBoton = 0;
bool botonPrevio = HIGH;

// ---------- Prototipos ----------
void leerSensor();
void actualizarLCD();
void actualizarOLED();
void actualizarMatriz();
void maxEscribir(uint8_t registro, uint8_t dato);
void maxIniciar();
void actualizarValor7seg();
void refrescar7seg();
void revisarBoton();

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(PIN_BOTON, INPUT_PULLUP);
  for (uint8_t i = 0; i < 7; i++) { pinMode(SEG[i], OUTPUT); digitalWrite(SEG[i], HIGH); }
  for (uint8_t i = 0; i < 4; i++) { pinMode(DIG[i], OUTPUT); digitalWrite(DIG[i], LOW); }

  lcd.init();
  lcd.backlight();
  lcd.print(F("Panel Modulo 5"));

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("No se encontro la OLED SSD1306"));
  }
  oled.clearDisplay();
  oled.display();

  maxIniciar();

  Serial.println(F("--- Módulo 5: Panel multi-pantalla iniciado ---"));
}

void loop() {
  unsigned long ahora = millis();

  revisarBoton();

  if (ahora - tLectura >= 2000 || tLectura == 0) {
    tLectura = ahora ? ahora : 1;
    leerSensor();
    actualizarLCD();
    actualizarOLED();
    actualizarMatriz();
  }
  actualizarValor7seg();
  refrescar7seg();
}

void leerSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (isnan(t) || isnan(h)) { Serial.println(F("[!] Error DHT22")); return; }
  temperatura = t;
  humedad = h;
  // Desplaza el historial y agrega la nueva muestra
  if (nHist < N_HIST) nHist++;
  for (uint8_t i = 0; i < nHist - 1; i++) historial[i] = historial[i + 1];
  historial[nHist - 1] = (int8_t)round(t);
  Serial.print(F("T=")); Serial.print(t, 1);
  Serial.print(F(" C  H=")); Serial.print(h, 0); Serial.println(F(" %"));
}

void actualizarLCD() {
  lcd.setCursor(0, 0);
  lcd.print(F("T:")); lcd.print(temperatura, 1); lcd.print(F("C H:")); lcd.print(humedad, 0); lcd.print(F("%  "));
  lcd.setCursor(0, 1);
  lcd.print(F("7seg: "));
  if (modo == 0) lcd.print(F("Temp   "));
  else if (modo == 1) lcd.print(F("Humedad"));
  else lcd.print(F("Tiempo "));
}

// Dibuja la gráfica del historial: eje en 10-45 °C mapeado a 0-39 píxeles de alto
void actualizarOLED() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.print(F("Temperatura (C)"));
  oled.setCursor(96, 0);
  oled.print(temperatura, 1);
  oled.drawRect(0, 16, 128, 48, SSD1306_WHITE);
  for (uint8_t i = 1; i < nHist; i++) {
    int x0 = (i - 1) * 2, x1 = i * 2;
    int y0 = 62 - constrain(map(historial[i - 1], 10, 45, 0, 44), 0, 44);
    int y1 = 62 - constrain(map(historial[i], 10, 45, 0, 44), 0, 44);
    oled.drawLine(x0 + 1, y0, x1 + 1, y1, SSD1306_WHITE);
  }
  oled.display();
}

// Barra de nivel: 15 °C = 0 columnas, 40 °C = 8 columnas
void actualizarMatriz() {
  int nivel = constrain(map((int)(temperatura * 10), 150, 400, 0, 8), 0, 8);
  for (uint8_t fila = 0; fila < 8; fila++) {
    maxEscribir(fila + 1, fila < nivel ? 0xFF : 0x00);   // registros 1..8 = filas
  }
}

// El MAX7219 recibe 16 bits: registro (8) + dato (8), validados con CS en alto.
void maxEscribir(uint8_t registro, uint8_t dato) {
  digitalWrite(PIN_MAX_CS, LOW);
  shiftOut(PIN_MAX_DIN, PIN_MAX_CLK, MSBFIRST, registro);
  shiftOut(PIN_MAX_DIN, PIN_MAX_CLK, MSBFIRST, dato);
  digitalWrite(PIN_MAX_CS, HIGH);
}

void maxIniciar() {
  pinMode(PIN_MAX_DIN, OUTPUT);
  pinMode(PIN_MAX_CLK, OUTPUT);
  pinMode(PIN_MAX_CS, OUTPUT);
  digitalWrite(PIN_MAX_CS, HIGH);
  maxEscribir(0x0F, 0x00);   // modo de prueba apagado
  maxEscribir(0x09, 0x00);   // sin decodificación BCD (control directo de LEDs)
  maxEscribir(0x0B, 0x07);   // escanea las 8 filas
  maxEscribir(0x0A, 0x04);   // intensidad media
  for (uint8_t r = 1; r <= 8; r++) maxEscribir(r, 0x00);
  maxEscribir(0x0C, 0x01);   // sale del modo apagado
}

void revisarBoton() {
  bool lectura = digitalRead(PIN_BOTON);
  if (lectura != botonPrevio && millis() - tBoton > 50) {      // antirrebote
    tBoton = millis();
    botonPrevio = lectura;
    if (lectura == LOW) { modo = (modo + 1) % 3; if (!isnan(temperatura)) actualizarLCD(); }
  }
}

void actualizarValor7seg() {
  if (modo == 0)      valor7seg = isnan(temperatura) ? 0 : (int)temperatura;
  else if (modo == 1) valor7seg = isnan(humedad) ? 0 : (int)humedad;
  else                valor7seg = (millis() / 1000UL) % 10000;
}

// Multiplexado: un dígito a la vez cada 3 ms (el ojo ve los cuatro encendidos)
void refrescar7seg() {
  if (millis() - tMux < 3) return;
  tMux = millis();
  for (uint8_t i = 0; i < 4; i++) digitalWrite(DIG[i], LOW);   // apaga todos
  int divisor[4] = {1000, 100, 10, 1};
  int d = (valor7seg / divisor[digitoActual]) % 10;
  bool apagar = (valor7seg < divisor[digitoActual] && digitoActual < 3);   // sin ceros a la izquierda
  uint8_t patron = apagar ? 0 : DIGITOS[d];
  for (uint8_t s = 0; s < 7; s++) digitalWrite(SEG[s], (patron >> s) & 1 ? LOW : HIGH);  // ánodo común: 0 = enciende
  digitalWrite(DIG[digitoActual], HIGH);
  digitoActual = (digitoActual + 1) % 4;
}
