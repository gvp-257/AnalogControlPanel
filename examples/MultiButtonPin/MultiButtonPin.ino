#include <Arduino.h>

/*

Multi Button Pin - use one pin to detect which of five buttons is pressed.
Use with the associated schematic shown in the Readme. Five resistors and
five buttons.

 Button   Ideal Reading    Range
   None    1023          950 - 1023
   SW1        0          0   -  40
   SW2       93          50  - 150
   SW3      297          250 - 350
   SW4      573          450 - 650
   SW5      789          700 - 870

*/

#define BUTTONPIN A3
#define NBR_BUTTONS 6  // includes "none".

#include "AnalogControlPanel.h"

// Which button was pressed?
// 0 = None, 1 - 5 as above.  -1 = unknown, inconclusive reading.
int whichButton(void)
{
    static const int upperBound[6] = {1023, 40, 150, 350, 650, 870};
    static const int lowerBound[6] = { 950,  0,  50, 250, 450, 700};

    InternalADC.usePin(BUTTONPIN);
    InternalADC.speed4x();

    int reading = 0;
    for (short rdgs = 0; rdgs < 4; rdgs++)
    {
        reading += InternalADC.read();
    }
    reading = (reading + 2) / 4;  // average of 4 rounded readings.

    int button = 0;
    for (button = 0; button < NBR_BUTTONS; button++)
    {
        if ((reading >= lowerBound[button]) && (reading <= upperBound[button]))
        {return button;}
    }
    return -1;
}

void setup()
{
    Serial.begin(9600);
    InternalADC.begin();
}


void loop()
{
    static unsigned long timeToCheck = millis() + 1000L;
    if (millis() >= timeToCheck)
    {
        int button = whichButton();
        if (button > 0)
        {
            Serial.print("Button ");
            Serial.print(button);
            Serial.println(" is pressed.");
        }
        else if (button == 0)
        {
            // Too many messages. Don't print the default state.
            // Serial.println("No button is pressed.");
        }
        else // button == -1
        {
            Serial.println("??? inconclusive reading.");
        }
        timeToCheck = millis() + 1000L;
    }
}
