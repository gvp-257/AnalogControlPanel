#include <Arduino.h>
/*
 * Measure internal reference voltage two ways
 *
 */

#include "AnalogControlPanel.h"

#define SUPPLY_V 5.300F   // Known supply voltage used for method 1

void setup()
{
    InternalADC.begin();      // Combined power on, set rate and reference.
    // InternalADC.referenceDefault();   // default in begin()
    // InternalADC.speed1x();            // default in begin()

    Serial.begin(9600);

    Serial.println("Measure Internal Reference Voltage");
    Serial.println("Two methods.");
    Serial.println("Method 1: Use known supply voltage to determine Internal Reference.");
    Serial.println("Method 2. Output Internal reference voltage on AREF pin.");


    Serial.println("Method 1:-");
    // Read several times to settle down the internal voltage reference.
    for (short i = 0; i < 10; i++) { InternalADC.getSupplyVoltage();}

    //Now print the measured supply voltage
    float supplyV = 0.0;
    for (short i = 0; i < 4; i++) { supplyV += InternalADC.getSupplyVoltage();}
    supplyV = supplyV / 4;  // average of 4 readings.
    Serial.print("Measured supply V with default internal reference: ");
    Serial.println(supplyV, 4);
    Serial.print("Known supply voltage is ");
    Serial,println(SUPPLY_V, 4);
    Serial.print("The ratio is ");

    float ratio = supplyV / SUPPLY_V;

    Serial.println(ratio, 4);
    Serial.print(" Calculated internal reference is ");
    float internalref = 1.100 / ratio;
    Serial.println(internalref, 3);
    Serial.println();


    Serial.println("Method 2:-");

    InternalADC.referenceInternal();
    delay(60);

    Serial.println("AREF has been set to internal reference.");
    Serial.println(" Measure it with a multimeter.");
    Serial.flush();
}

void loop()
{

}