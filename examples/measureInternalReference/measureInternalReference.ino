#include <Arduino.h>
/*
 * (Get Supply Voltage Read Battery) example.
 *
 */

#include "AnalogControlPanel.h"

#include "SendOnlySerial.h"

#define LABEL(x, y) static const char x[] PROGMEM = y;

void setup()
{
    // InternalADC.powerOn(); // Necessary to set up AREF and clock
    // InternalADC.rate9k();
    InternalADC.begin();      // Combined power on, set rate and reference.

    SendOnlySerial.begin();   // default baud rate 9600.

    SendOnlySerial.print("Estimated Arduino supply voltage: ");

    // Read several times to settle down the internal voltage reference.
    for (short i = 0; i < 10; i++) { InternalADC.getSupplyVoltage();}

    //Now print a reading.
    SendOnlySerial.println(InternalADC.getSupplyVoltage());

    SendOnlySerial.flush();
}

void loop()
{

}