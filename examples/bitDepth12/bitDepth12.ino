#include <Arduino.h>
/*
  Example of increasing "bit depth" (precision) by 2 bits by 16-fold oversampling.

  Also, separately, illustrates using the "ADC conversion complete" interrupt
  to accumulate ADC readings and maybe do simple processing of them, while
  leaving the main loop free to do other things.

  12-bit depth works by oversampling and decimation, taking 16 readings and then
  averaging them to return a value between 0 and 4092. It is slow, and ideally
  used with input voltages that change very slowly.

  (In general for N bits extra, you have to accumulate 4 to the power of N readings.)

  Oversampling and decimation also requires a noise source, also known as dither.

  **Dither Required with 12-bit**
    ---------------------------
  For oversampling and decimation to work, the readings must vary a little and be
  averaged. This means that a small amount of electrical 'noise' must be added to
  the signal that you are measuring.

  One traditional way of doing this is with a small triangle wave.
  On the Arduino this can be done by using the `tone()` function with an 
  attenuator/integrator circuit, applying it to the ADC input pin along with
  the sensor voltage. 

  (Of course it can also be done with a 555. Everything can be done with a 555.)

  At the default sample rate (`Rate9k()`), 16 samples are taken in 1728
  microseconds, equivalent to a frequency of 578.7 Hz.
  So we'd use `tone(TONE_PIN, 579)` before starting and 
  `notone()` when the sample is ready.


              DC Block    Attenuator/Integrator  Summing point

        +--+    10 nF      +-------+                          +--+
        |  |------||-------| 250 K |----+-------------+-------|  |  ANALOG_PIN
        +--+               +-------+    |             |       +--+  (A0 .. A7)
        TONE_PIN (D4)                   |            +-+
        579 Hz                          |            | | 2K2
                               1 uF    ===           +-+
                               (105)    |             |       +--+
                                        |             +-------|  |  SENSOR_OUT
        Gnd                             |                     +--+  from module
        --------------------------------+-------------------------

        Example Dither Circuit. Credit: "Qwerty" on the Freetronics forum.

  This will provide about 6 millivolts peak-to-peak from TONE_PIN at
  the sampling pin, using tone() from a 3.3V Arduino Pro mini running on
  batteries at 3.1V. That is about two LSB (least significant bits) dither
  using the 3.3V supply as the reference voltage.

  If you change the tone frequency significantly, the 1uF capacitor will have
  to be changed also.

*/
#include "AnalogControlPanel.h"

#define TONE_PIN 4
#define ANALOG_PIN A3

#define REFERENCE_VOLTS 5.153

#define READING_INTERVAL_MILLIS 1000UL
// Interval between readings of ANALOG_PIN in milliseconds

// For the ISR to tell loop that the reading is ready:
volatile bool readingDone = false;

volatile int  readSum = 0;
// readSum accumulates the 10-bit readings by the ADC.
// For 16 readings an int is OK.
// For 64 (13-bit precision) or more we need a long (int32_t).


// Definitions for the ADC conversion complete interrupt:
#define numReadingsFor12Bit 1 * 4 * 4  // One " * 4" for each bit extra over 10
volatile short numReadings = 0;

// Our function that hardware calls when the ADC completes one (10-bit) reading:
void accumulateReadingsISR(void)
{
  // accumulate 16 readings.
  if (numReadings < numReadingsFor12Bit) 
  {
    readSum += InternalADC.getLastReading();
    numReadings++;
  }
  if (numReadings == numReadingsFor12Bit)
  {readingDone = true;}
}

void startReadings(void)
{
  // Reset the sum and count of readings, start the ADC
  readSum = 0;
  numReadings = 0;
  InternalADC.freeRunningMode();  // Get the ADC to take readings continuously.
  InternalADC.interruptOnDone();
  InternalADC.startReading();     // start the ADC going.
}

void stopReadings (void)
{
  // Stop the ADC.
  InternalADC.noInterruptOnDone();
  InternalADC.singleReadingMode();
}

// "Decimate" the accumulated reading and process the result
void processReading(void)
{
  int r12bit = (readSum + 8) >> 2;
  // r12bit has a value between 0 and 4094.

  // Do something with the value.
  // Here we'll just print it and the voltage  to the Arduino Serial Monitor.
  Serial.print("12 bit reading value: ");
  Serial.println(r12bit);

  float volts = ((float)r12bit * REFERENCE_VOLTS) / 4094.0;
  Serial.print("Voltage: ");
  Serial.println(volts, 4);
}


void setup()
{
  Serial.begin(9600);

  // Start a source of electrical noise as above.
  tone(TONE_PIN, 579);
  delay(1000); // wait for tone signal to stabilise


  // InternalADC.powerOn();
  // InternalADC.singleReadingMode();
  // InternalADC.bitDepth10();
  // InternalADC.speed1x();
  // InternalADC.referenceDefault();
  // InternalADC.noInterruptOnDone();

  InternalADC.begin();
  // begin() does the above six things and connects the ADC's input to internal ground.

  InternalADC.speed2x();
  InternalADC.attachDoneInterruptFunction(accumulateReadingsISR);
  InternalADC.usePin(ANALOG_PIN);

}

void loop()
{
  static bool wantReading = true;
  static unsigned long nextReadingTime;

  // Limit output on the serial monitor to 5 cycles
  static short numCyclesToPrint = 5;
  if (numCyclesToPrint == 0)
  {
    // We have finished the example.
    // Stop the noise generator.
    noTone(TONE_PIN);

    wantReading = false;
    InternalADC.powerOff();
  }

  // Process completed reading accumulation
  if (wantReading && readingDone)
  {
    stopReadings(); // stop the hardware calling the accumulate ISR
    // because we have finished accumulating the 16 10-bit readings
    nextReadingTime = millis() + READING_INTERVAL_MILLIS;
    processReading();
    numCyclesToPrint--;
  }

  // Start a 12-bit reading if it is time
  if (wantReading && millis() >= nextReadingTime)
  {
    readingDone = false;  // set to true by the ISR.
    startReadings();
    // Readings take place in the background but still take
    // 16 x 54 = 864 microseconds, plus overhead.
  }

  /*
    Meanwhile:-
    
    Do other things, like write to an LCD display.
    Or write to an SD card.
    Or read a GPS over softwareSerial.
    Or read an accelerometer over I2C (Wire) or SPI.
    Turn off ADC if readings not required for a while:
    wantReadings = false;
    Here we'll simulate doing work with a delay:
    Note: delay has to smaller than the reading interval.
  */
  delay(167);

}