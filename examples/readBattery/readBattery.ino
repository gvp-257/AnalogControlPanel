#include <Arduino.h>
/*
 * (Get Supply Voltage Read Battery) example.
 *
 */

#include "AnalogControlPanel.h"

#define  INTERNAL_REF_V  1.100F  // 1.100V, change this value if known


void setup()
{
    // InternalADC.powerOn(); // Necessary to set up AREF and clock
    // InternalADC.rate9k();
    InternalADC.begin();      // Combined power on, set rate and reference.

    Serial.begin();   // default baud rate 9600.

    InternalADC.setInternalReferenceVoltage(INTERNAL_REF_V);
    Serial.print(" Using internal reference voltage ");
    Serial.println(INTERNAL_REF_V);
    Serial.println();

    Serial.print("Estimated Arduino supply voltage: ");

    // Read several times to settle down the internal voltage reference.
    for (short i = 0; i < 7; i++) {InternalADC.getSupplyVoltage();}

    //Now print a reading.
    Serial.println(InternalADC.getSupplyVoltage());

    Serial.flush();
}

void loop()
{

}