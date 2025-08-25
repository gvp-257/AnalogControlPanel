#include <Arduino.h>

#include "AnalogControlPanel.h"

// =========================================================================
// Read LDR:  Simple example of using Analog Control Panel like
// analogRead().
//
// But I can't resist showing something else as well: pin powering, a
// way of saving energy.
//
// The LDR-resistor voltage divider is powered through pin 3
// for the amount of time it takes to read the voltage
// (and therefore get an idea of the light level).
// Note: We can only do this for things that take a small amount of current
// (less than 20 milliamps).  For larger loads we would use the pin to
// control a power transistor which acts as a switch for the circuit.
//
// Circuit:-
//
//  |pin 3|----| LDR |----+----|10K Resistor|----|GND|
//                        |
//  |pin A3|--------------+
//
// =========================================================================

#define LDR_SUPPLY_PIN  3
#define LDR_READ_PIN   A3

void setup()
{
    Serial.begin(9600);
    InternalADC.begin();
    InternalADC.speed2x();  // each read takes about 55 microseconds not 110.

}

void loop()
{
    // Supply power to the LDR
    pinMode(LDR_SUPPLY_PIN, OUTPUT);
    digitalWrite(LDR_SUPPLY_PIN, HIGH); // supply LDR with power

    // LDRs take a while to stabilise - milliseconds.
    delay(50);

    // Connect the Analog pin to the ADC internally.
    InternalADC.usePin(LDR_READ_PIN);

    // Get the average of four readings for better repeatability.
    int  reading = 0;
    for (short i = 0; i < 4; i++)
        {reading = reading + InternalADC.read();}

    reading = (reading + 2) / 4;        // round and average

    // Finished reading: disconnect the pins so that no power is being used.
    InternalADC.freePin(LDR_READ_PIN);              // analog pin can now be used for digital

    digitalWrite(LDR_SUPPLY_PIN, LOW);
    pinMode(LDR_SUPPLY_PIN, INPUT);     // stop supplying power

    // Print results.
    Serial.print(" Voltage at base of LDR is ");
    float voltage = ((float)reading + 0.5) / 1024.0;
    Serial.println(voltage, 3);
    Serial.println();

    // Delay 2 seconds between readings.
    // No power is being used by the LDR during this delay.
    delay(2000);

} // end of loop: repeat.