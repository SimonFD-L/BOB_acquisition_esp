
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// =====================================
// CONFIGURATION DES BROCHES
// =====================================

#define ENCODER_A 34
#define ENCODER_B 35
#define TEMP_PIN 32

// =====================================
// PARAMETRES D'ACQUISITION
// =====================================

const float PPR = 1000.0;  // Impulsions/tour sur A

const unsigned long SAMPLE_TIME = 100; // ms

// =====================================
// ENCODEUR
// =====================================

volatile uint32_t pulseCount = 0;

void IRAM_ATTR countPulse() {
    pulseCount++;
}

// =====================================
// TEMPERATURE DS18B20
// =====================================

OneWire oneWire(TEMP_PIN);

DallasTemperature sensors(&oneWire);

float temperatureC = NAN;

// =====================================
// VARIABLES TEMPORELLES
// =====================================

unsigned long previousTime = 0;

// =====================================
// SETUP
// =====================================

void setup() {

    Serial.begin(115200);

    // Encodeur
    pinMode(ENCODER_A, INPUT);
    pinMode(ENCODER_B, INPUT);

    attachInterrupt(
        digitalPinToInterrupt(ENCODER_A),
        countPulse,
        RISING
    );

    // Temperature
    sensors.begin();

    // Resolution 10 bits : conversion en 187,5 ms
    sensors.setResolution(10);

    // Mode non bloquant
    sensors.setWaitForConversion(false);

    // Lancement premiere conversion
    sensors.requestTemperatures();

    previousTime = millis();


}

// =====================================
// LOOP
// =====================================

void loop() {

    unsigned long currentTime = millis();

    if (currentTime - previousTime >= SAMPLE_TIME) {

        // ---------------------------------
        // 1. LECTURE ENCODEUR
        // ---------------------------------

        noInterrupts();

        uint32_t pulses = pulseCount;
        pulseCount = 0;

        interrupts();

        float dt = (currentTime - previousTime) / 1000.0;

        float rpm = (pulses / PPR) * (60.0 / dt);

        // ---------------------------------
        // 2. LECTURE TEMPERATURE
        // ---------------------------------

        float newTemperature = sensors.getTempCByIndex(0);

        if (newTemperature != DEVICE_DISCONNECTED_C &&
            newTemperature >= -55.0 &&
            newTemperature <= 125.0) {

            temperatureC = newTemperature;
        }

        // Lancer la conversion suivante
        sensors.requestTemperatures();

        // ---------------------------------
        // 3. TRANSMISSION SERIE
        // ---------------------------------

        Serial.print(currentTime);
        Serial.print(",");

        Serial.print(rpm, 2);
        Serial.print(",");

        if (isnan(temperatureC)) {
            Serial.println("ERROR");
        } else {
            Serial.println(temperatureC, 2);
        }

        previousTime = currentTime;
    }
}
