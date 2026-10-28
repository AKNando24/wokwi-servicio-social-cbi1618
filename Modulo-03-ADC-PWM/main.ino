/*
  Módulo 3: ADC, PWM y Acondicionamiento de Señal
  Servicio Social CBI-1618 - Ing. en Computación
  Prestador: Pérez Sánchez Fernando Alberto

  Demostración:
  - Lectura multicanal ADC (10 bits: 0-1023).
  - Acondicionamiento con Filtro de Promedio Móvil Exponencial (EMA).
  - Conversión B-Parameter (Beta) para sensor NTC.
  - Modulación por Ancho de Pulsos (PWM 8 bits: 0-255) en LED.
  - Síntesis de frecuencia variable con Buzzer según diferencial térmico.
*/

#include <math.h>

// Pines de Entradas Analógicas
const int PIN_POT = A0;
const int PIN_LDR = A1;
const int PIN_NTC = A2;

// Pines de Salidas PWM
const int PIN_LED_PWM = 5;
const int PIN_BUZZER = 3;

// Parámetros NTC (Ecuación Beta)
const float BETA = 3950.0;
const float ROOM_TEMP_K = 298.15; // 25°C en Kelvin

// Factor de suavizado EMA (0.0 a 1.0)
const float ALPHA = 0.15;
float filteredNTC = 0.0;

// Ventana de tiempo (no bloqueante)
unsigned long previousMillis = 0;
const long interval = 100; // Muestreo cada 100 ms

void setup()
{
    Serial.begin(115200);

    pinMode(PIN_LED_PWM, OUTPUT);
    pinMode(PIN_BUZZER, OUTPUT);

    // Lectura inicial para estabilizar el filtro EMA
    filteredNTC = analogRead(PIN_NTC);

    Serial.println("==================================================");
    Serial.println("  MÓDULO 3: PROCESAMIENTO ADC, FILTRADO Y PWM");
    Serial.println("==================================================");
}

void loop()
{
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
        previousMillis = currentMillis;

        // 1. Lectura ADC Cruda
        int rawPot = analogRead(PIN_POT);
        int rawLDR = analogRead(PIN_LDR);
        int rawNTC = analogRead(PIN_NTC);

        // 2. Acondicionamiento de Señal: Filtro Exponencial (EMA)
        filteredNTC = (ALPHA * rawNTC) + ((1.0 - ALPHA) * filteredNTC);

        // 3. Conversión de NTC a Grados Celsius
        float tempC = calculateNTC(filteredNTC);

        // 4. Establecer Setpoint de Umbral Térmico con Potenciómetro (20°C a 80°C)
        float tempThreshold = map(rawPot, 0, 1023, 20, 80);

        // 5. Control PWM de LED según Nivel de Luz (LDR)
        // Invierte el valor: menor luz -> mayor brillo de LED
        int pwmBrightness = map(rawLDR, 0, 1023, 255, 0);
        pwmBrightness = constrain(pwmBrightness, 0, 255);
        analogWrite(PIN_LED_PWM, pwmBrightness);

        // 6. Lógica de Alarma Acústica adaptativa con Buzzer (PWM / Frecuencia)
        if (tempC > tempThreshold)
        {
            // Frecuencia dinámica basada en el exceso de temperatura
            float deltaTemp = tempC - tempThreshold;
            int toneFreq = constrain(440 + (deltaTemp * 100), 440, 2000);
            tone(PIN_BUZZER, toneFreq);
        }
        else
        {
            noTone(PIN_BUZZER);
        }

        // Trazador Serial (Serial Plotter)
        Serial.print("NTC_Crudo:");
        Serial.print(rawNTC);
        Serial.print(" NTC_Filtrado:");
        Serial.print(filteredNTC);
        Serial.print(" Temp_C:");
        Serial.print(tempC);
        Serial.print(" Umbral_C:");
        Serial.print(tempThreshold);
        Serial.print(" LED_PWM:");
        Serial.println(pwmBrightness);
    }
}

// Función para aproximación de temperatura mediante el modelo Beta del NTC
float calculateNTC(float adcValue)
{
    if (adcValue <= 0 || adcValue >= 1023)
        return 0.0;
    float resistance = (1023.0 / adcValue) - 1.0;
    resistance = 10000.0 / resistance; // Para módulo NTC de 10k
    float tempK = 1.0 / (log(resistance / 10000.0) / BETA + (1.0 / ROOM_TEMP_K));
    return tempK - 273.15; // Convertir a Celsius
}