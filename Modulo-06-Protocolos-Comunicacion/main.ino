/*
  Módulo 6: Protocolos de Comunicación Serial (UART, I2C y SPI)
  Proyecto: Consola de comandos que controla tres buses a la vez
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  - UART:  la consola (Monitor Serie) envía comandos de texto y recibe respuestas.
  - I2C:   LCD 16x2 (0x27) y OLED SSD1306 (0x3C) comparten SDA/SCL; un escáner
           muestra qué direcciones responden.
  - SPI:   registro de desplazamiento 74HC595 que maneja 8 LEDs (MOSI, SCK y CS = latch).
           MOSI está unido a MISO con un puente para hacer una prueba de eco (loopback).

  Comandos (escribe en el Monitor Serie y presiona Enter):
    HELP                 lista de comandos
    SCAN                 escáner I2C
    LED <0-7> <ON|OFF|T> enciende, apaga o invierte un LED (SPI)
    BAR <0-8>            enciende N LEDs en forma de barra (SPI)
    SPI <00-FF>          escribe un byte en hexadecimal (SPI)
    LCD <texto>          escribe en el LCD (I2C)
    OLED <texto>         escribe en la OLED (I2C)
    STATUS               estado de los LEDs y de los buses
*/

#include <Wire.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

const uint8_t PIN_LATCH = 10;          // CS / STCP del 74HC595
const uint32_t VELOCIDAD_SPI = 1000000UL;

LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_SSD1306 oled(128, 64, &Wire, -1);

char buffer[40];
uint8_t indice = 0;
uint8_t mascaraLeds = 0;
bool oledListo = false;

// ---------- Prototipos ----------
void procesarComando(char *linea);
void escanearI2C();
uint8_t escribirSPI(uint8_t valor);
void pruebaLoopbackSPI();
void mostrarLCD(const char *texto);
void mostrarOLED(const char *texto);
void mayusculas(char *s);
void ayuda();
void estado();

void setup() {
  Serial.begin(115200);                       // UART
  Wire.begin();                               // I2C
  SPI.begin();                                // SPI
  pinMode(PIN_LATCH, OUTPUT);
  digitalWrite(PIN_LATCH, HIGH);

  lcd.init();
  lcd.backlight();
  lcd.print(F("Modulo 6 - Bus"));
  oledListo = oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  mostrarOLED("UART + I2C + SPI");

  escribirSPI(0x00);
  Serial.println(F("--- Módulo 6: consola de protocolos ---"));
  escanearI2C();
  pruebaLoopbackSPI();
  ayuda();
  Serial.print(F("> "));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      buffer[indice] = '\0';
      indice = 0;
      procesarComando(buffer);
      Serial.print(F("> "));
    } else if (indice < sizeof(buffer) - 1) {
      buffer[indice++] = c;
    }
  }
}

void procesarComando(char *linea) {
  if (linea[0] == '\0') return;
  char *cmd = strtok(linea, " ");
  char *arg1 = strtok(NULL, " ");
  char *resto = arg1 ? arg1 + strlen(arg1) + 1 : NULL;   // texto completo para LCD/OLED
  mayusculas(cmd);

  if (!strcmp(cmd, "HELP")) {
    ayuda();
  } else if (!strcmp(cmd, "SCAN")) {
    escanearI2C();
  } else if (!strcmp(cmd, "LED")) {
    char *accion = strtok(NULL, " ");
    if (!arg1 || !accion) { Serial.println(F("Uso: LED <0-7> <ON|OFF|T>")); return; }
    int n = atoi(arg1);
    if (n < 0 || n > 7) { Serial.println(F("LED fuera de rango (0-7)")); return; }
    mayusculas(accion);
    if (!strcmp(accion, "ON")) mascaraLeds |= (1 << n);
    else if (!strcmp(accion, "OFF")) mascaraLeds &= ~(1 << n);
    else if (!strcmp(accion, "T")) mascaraLeds ^= (1 << n);
    else { Serial.println(F("Acción inválida")); return; }
    escribirSPI(mascaraLeds);
    Serial.print(F("OK, máscara = 0x")); Serial.println(mascaraLeds, HEX);
  } else if (!strcmp(cmd, "BAR")) {
    if (!arg1) { Serial.println(F("Uso: BAR <0-8>")); return; }
    int n = constrain(atoi(arg1), 0, 8);
    mascaraLeds = (n == 0) ? 0 : (0xFF >> (8 - n));
    escribirSPI(mascaraLeds);
    Serial.print(F("Barra de ")); Serial.print(n); Serial.println(F(" LEDs"));
  } else if (!strcmp(cmd, "SPI")) {
    if (!arg1) { Serial.println(F("Uso: SPI <00-FF>")); return; }
    mascaraLeds = (uint8_t)strtol(arg1, NULL, 16);
    escribirSPI(mascaraLeds);
    Serial.print(F("Enviado por SPI: 0x")); Serial.println(mascaraLeds, HEX);
  } else if (!strcmp(cmd, "LCD")) {
    mostrarLCD(resto ? resto : "");
    Serial.println(F("Texto enviado al LCD (I2C)"));
  } else if (!strcmp(cmd, "OLED")) {
    mostrarOLED(resto ? resto : "");
    Serial.println(F("Texto enviado a la OLED (I2C)"));
  } else if (!strcmp(cmd, "STATUS")) {
    estado();
  } else {
    Serial.println(F("Comando desconocido. Escribe HELP"));
  }
}

// Recorre las 127 direcciones: si endTransmission() devuelve 0, alguien contestó (ACK).
void escanearI2C() {
  Serial.println(F("Escaneando bus I2C..."));
  uint8_t encontrados = 0;
  for (uint8_t dir = 1; dir < 127; dir++) {
    Wire.beginTransmission(dir);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  Dispositivo en 0x"));
      if (dir < 16) Serial.print('0');
      Serial.println(dir, HEX);
      encontrados++;
    }
  }
  Serial.print(encontrados); Serial.println(F(" dispositivo(s) encontrado(s)"));
}

// Transfiere un byte por SPI y devuelve lo que llegó por MISO. CS (latch) sube al final.
uint8_t escribirSPI(uint8_t valor) {
  SPI.beginTransaction(SPISettings(VELOCIDAD_SPI, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_LATCH, LOW);
  uint8_t recibido = SPI.transfer(valor);
  digitalWrite(PIN_LATCH, HIGH);
  SPI.endTransaction();
  return recibido;
}

// Con MOSI unido a MISO, lo que se envía debe regresar idéntico.
void pruebaLoopbackSPI() {
  uint8_t enviado = 0xA5;
  uint8_t recibido = escribirSPI(enviado);
  Serial.print(F("Loopback SPI: enviado 0x")); Serial.print(enviado, HEX);
  Serial.print(F(", recibido 0x")); Serial.print(recibido, HEX);
  Serial.println(recibido == enviado ? F("  -> OK") : F("  -> FALLO (revisa el puente MOSI-MISO)"));
  escribirSPI(mascaraLeds);
}

void mostrarLCD(const char *texto) {
  lcd.clear();
  lcd.setCursor(0, 0);
  for (uint8_t i = 0; texto[i] && i < 16; i++) lcd.print(texto[i]);
  if (strlen(texto) > 16) {
    lcd.setCursor(0, 1);
    for (uint8_t i = 16; texto[i] && i < 32; i++) lcd.print(texto[i]);
  }
}

void mostrarOLED(const char *texto) {
  if (!oledListo) return;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(F("Modulo 6"));
  oled.setTextSize(2);
  oled.setCursor(0, 20);
  oled.print(texto);
  oled.display();
}

void mayusculas(char *s) { for (; *s; s++) *s = toupper(*s); }

void ayuda() {
  Serial.println(F("Comandos: HELP | SCAN | LED n ON/OFF/T | BAR n | SPI hh | LCD txt | OLED txt | STATUS"));
}

void estado() {
  Serial.print(F("LEDs (Q7..Q0): "));
  for (int8_t i = 7; i >= 0; i--) Serial.print((mascaraLeds >> i) & 1);
  Serial.print(F("  0x")); Serial.println(mascaraLeds, HEX);
  Serial.println(oledListo ? F("OLED: lista") : F("OLED: no detectada"));
}
