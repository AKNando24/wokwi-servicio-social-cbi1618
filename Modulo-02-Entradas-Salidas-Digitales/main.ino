/*
  Módulo 2: Entradas y Salidas Digitales y Debouncing
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto
*/

const int PIN_RED = 12;
const int PIN_YELLOW = 11;
const int PIN_GREEN = 10;
const int PIN_BTN = 2;

// Variables para Debouncing por Software
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 50;
int lastButtonState = HIGH;
int buttonState = HIGH;

volatile bool requestPedestrian = false;

void IRAM_ATTR handleButtonInterrupt()
{
    requestPedestrian = true;
}

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_YELLOW, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);

    // Uso de resistencia Pull-Up interna (Botón activo en LOW)
    pinMode(PIN_BTN, INPUT_PULLUP);

    // Interrupción en el Pin 2 (Flanco de bajada / FALLING)
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), handleButtonInterrupt, FALLING);

    Serial.println("--- Módulo 2: Sistema de Tráfico con Debouncing Iniciado ---");
}

void loop()
{
    // Secuencia normal: Verde
    digitalWrite(PIN_GREEN, HIGH);
    digitalWrite(PIN_YELLOW, LOW);
    digitalWrite(PIN_RED, LOW);

    if (requestPedestrian)
    {
        processPedestrianSequence();
    }
}

void processPedestrianSequence()
{
    unsigned long currentTime = millis();

    // Filtrado Debouncing por software
    if ((currentTime - lastDebounceTime) > debounceDelay)
    {
        Serial.println("Solicitud peatonal detectada (Antirrebote validado).");

        // Transición a Amarillo
        digitalWrite(PIN_GREEN, LOW);
        digitalWrite(PIN_YELLOW, HIGH);
        delay(2000);

        // Transición a Rojo (Paso peatonal)
        digitalWrite(PIN_YELLOW, LOW);
        digitalWrite(PIN_RED, HIGH);
        delay(5000);

        requestPedestrian = false;
        lastDebounceTime = currentTime;
    }
}