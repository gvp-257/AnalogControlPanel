# ANALOG CONTROL PANEL

Arduino is great at keeping things simple, but after a while you might want more control. Analog Control Panel gives you full control of analog readings using the analog-to-digital converter (ADC) that is built in to AVR microcontrollers, the one used by `analogRead()`.

**Note:** Analog Control Panel (ACP) is all about configuring analog **reads** using the built-in ADC. It has no functionality for `analogWrite()`. There are many add-on ADC boards and libraries for them in the Arduino ecosystem. ACP can't help you with those.

## Compatibility

Analog Control Panel is compatible with Arduino boards based on the AVR ATmega328P microcontroller: the Uno, Nano, and Pro Mini (bot 5 volt and 3 volt versions). It will also work with "breadboard Arduinos" using the ATmega328P running at 16 MHz, 8MHz, or 1MHz.

## Installation

On the  [Github page](https://github.co/gvp-257/AnalogControlPanel), click the green Code button, and choose download zip. Extract the zip file into the "libraries" folder inside your sketchbook folder, and rename the extracted folder from "AnalogControlPanel-main" to "AnalogControlPanel".

## BACKGROUND

I'm going to assume you're familiar with using Arduino's `analogRead` function and its supporting function `analogReference`, but want to know more and do more. If the next sections are confusing, refer to the [Arduino documentation for `analogRead`](https://docs.arduino.cc/language-reference/en/functions/analog-io/analogRead/).

### What Happens Inside `analogRead()`?

Inside every chip that can do an `analogRead()` there is a specialised circuit module called an "analog to digital converter", ADC.

The ADC's job is to convert the voltage on a pin to a number that the chip's CPU can process and store in memory. That's what's happening inside `analogRead()`. Analog Control Panel controls this internal ADC module in the ATmega328P chip used by the Uno, Nano, and Pro Mini.

## SIMPLE USAGE

You can use Analog Control Panel pretty much the same way as `analogRead()` if you want. Just prefix your function calls with "`InternalADC.`":-

    #include "AnalogControlPanel.h"

    // All functions belong to the `InternalADC` object.

    InternalADC.begin();     // optional: InternalADC.analogRead() will do it if needed.

    int reading = InternalADC.analogRead(A3); // same as Arduino's "analogRead(A3);"

    InternalADC.end();       // turn off the InternalADC, save batteries.

also:-

    InternalADC.speed2x();   // variations: speed1x (default), speed4x()

    // 8-bit readings that fit in a byte. Values 0 to 255:-

    InternalADC.readResolution(8);
    byte smallreading = InternalADC.analogRead(A4);

    InternalADC.readResolution(10);
    // back to readings in 0..1023 range, still speed2x.

    InternalADC.reference(INTERNAL); // other values: DEFAULT, EXTERNAL
    // as in Arduino's analogReference(). See that documentation for details.

    int refReading = InternalADC.readInternalReference();
    // With reference(INTERNAL): gives around 1020. (should be 1023.)
    // With reference(DEFAULT) on an Uno, about 225.

    float batteryVoltage = InternalADC.getSupplyVoltage();
    // For Uno, is near 5.00. More useful for unregulated, battery powered projects.


So you can use Analog Control Panel and `InternalADC.analogRead()` just like Arduino's `analogRead()`, with a few bells and whistles, if you want.

There is more, however. Before we get into that, we'll need to cover a bit of background and introduce some terminology.

## WHAT DOES THE CONTROL PANEL...CONTROL?

The ADC is not magic. It takes time to do its job, and it can't measure infinite voltages or measure them infinitely precisely - numbers that big would fill up the Arduino's memory, anyway.

So there are choices that have to be made. Arduino's `analogRead()` has been making a bunch of those choices for you.

Over time, the choices have been grouped into seven questions:-

 1. (Source) Which pin do you want to read? Or, instead of a pin, do you want one of the specialised internal readings - the on-chip temperature sensor, say, or the supply voltage of the Arduino?
 2. (Scale or Reference) What is the maximum voltage you want to be able to read? The largest number the ADC can give you is 1023, but what does 1023 mean? -- 5 volts, or 1.1 volts?
 3. (Speed) How fast do you want to take a reading? Mostly, fast is good. But for a given design of hardware, beyond a certain point, faster means less accurate: greater likelihood of the answer being wrong, not just imprecise.
 4. (Precision or Resolution or Bit Depth) How precise do the results have to be? There's a tradeoff here: longer numbers can hold more precise results, but they take more space in memory. Also, quite often extra precision is an illusion, because of fluctuations in the sensor you're reading. The ADC can't provide better data than its source.
 5. (Triggering) When do you want to take the reading--what triggers a reading? Your code, when you write `InternalADC.analogRead(A3)`? Or, say, do you want to take a reading _exactly_ when a pulse is received or some other event happens--"trigger on event" mode.
 6. (Notification) Do you want to wait around for the ADC while it does its work, or could you other things while you wait? The ADC takes between 13 and 110 microseconds for each reading, depending on speed setting. 110 microseconds equals 1,760 CPU clock cycles, enough for some arithmetic and array access. With `analogRead()`, you wait. If you want to work instead, then when it's convenient, you can check a flag to tell you whether the ADC is finished and your reading is ready.
 7. (State) Turning the ADC off and on. In Arduino, of course, it's always on. You might prefer to save your batteries for other things.

Some people like to change a few names and call these the seven 'S'es of ADCs: *S*ource, *S*cale, *S*peed, *S*ensitivity, *S*tarting, *S*ignaling and *S*tate.

In Arduino you get to choose Scale with the `analogReference(...)` function, and the Source by specifying the pin in `analogRead(...)`: `analogRead(LDR_PIN)`. Arduino chooses the others for you.

There is one speed (`speed1x`), on an Uno there is one precision (0 to 1023 in an `int`), you have to tell it to take every reading--the ADC can't automatically take readings based on an event, you have to wait for the ADC every time, and you can't turn it off.

Analog Control Panel lets you choose.

The bad news is that you can't choose anything you want for each of these things. You can only choose from the options that the hardware designers gave us.  Still, at least you can get some choice.

### A Look Ahead

With those concepts covered, we can look at the functions in Analog Control Panel. First up there is a summary, listing the functions of Analog Control Panel according to the seven S categories, then a more code-like usage reference. But first of all, an interlude. (You're probably ready for a break.)

Here's a picture of a cat:-
![Photo of a cat, public domain](cat-publicdomain.jpg)


## ANALOG CONTROL PANEL SUMMARY

All functions belong to **InternalADC**, i.e. must be prefixed with `InternalADC.` ("Internal"..? There are many libraries for external ADC modules that one can buy for more advanced needs. This library is for the built-in ADC.)

  * **State**: `isOff()`, `isOn()`, `powerOn()`, `powerOff()`; or more Arduino-ish: `begin()`, `end()`.

  * **Source** Input Pin selection that is decoupled from taking readings: `usePin(pin)`, `freePin(pin)`. After `usePin()`, readings via `InternalADC.read()` and auto-triggered readings will use that pin.

  * **Scale**: set full-scale (reference) voltage input:  `referenceDefault()` (ATmega's supply), `referenceInternal()` (ATmega's internal reference, 1.1V), `referenceExternal()` (for example, to use a TL431 or LM4040 voltage reference chip).

  * **Speed**: `speed1x()`, `speed2x()`, and `speed4x()`. In ADC world speed is usually called sample rate, so there are `rate9k()` (Arduino and ACP default, the same as `speed1x()`), `rate18k()`, `rate37k()`, `rate74k()` - for both 16MHz Uno and 8 MHz Pro Mini 3.3V: sets the maximum rate at which samples can be taken, the rate at which the ADC automatically takes samples in free-running mode. Also speeds up single-shot readings.

  * **Precision or Sensitivity**: Set the bit depth of samples: `bitDepth8()` (readings from 0 to 255), `bitDepth10()` (readings from 0 to 1023). There is also `readResolution(8)`, `readResolution(10)` since Arduino has a similar function that can be used on other boards.


  * **Starting** readings. First, **manually**: Single-shot or free-running mode: `singleReadingMode()`, `freeRunMode()`

  * **Starting automatically**,  auto-triggering, take samples at precise times/rates: `triggerOnInterrupt0()` - use with GPS PPS pin (pulse-per-second) or other external pulse source on pin 2. `triggerOnInputCapture()`: advanced, triggers the ADC on an input capture event on pin 8. There's no way to set that up in the core Arduino language, but there might be a library. Similarly for `triggerOnTimer1CompareB()`: triggers on the internal Timer1 hardware reaching a specified value. You must configure the timer yourself.

  * **Signalling** or **Notification**: **Blocking reads**: `analogRead(pin)` (like Arduino's built in `analogRead(pin)`), `usePin(pin)` ... `read()` or `read8Bit()` (`read8Bit()` after `bitDepth8()`).  When the function finishes you have a reading.

  * **Sleep-mode** reading for lower noise from the CPU: `sleepRead()` (The signal is: the CPU is running again, your code is executing.)

  * **Non-blocking reads**: use the cycle `startReading()` ... `readingReady()`: if true, the reading is done so `getLastReading()` or `getLastReading8Bit()`.

  * **"ADC Conversion Complete" interrupt handling**: define a function to get the ADC reading as soon as it's done, and control when that function is used. `interruptOnDone()` and `noInterruptOnDone()` to enable/disable the interrupt; `attachDoneInterruptFunction(function-name)`, `detachDoneInterruptFunction()` to set the function to be called when the ADC has done the reading.

  * **Special Reads**: read the AVR's internal voltage reference, or the internal temperature sensor, or the internal ground connection in the ATmega328P: `readInternalReference()`, `readTemperature()`, `readGround()`. These are "raw" readings, 0 to 1023.

  * **Suppply Voltage** Get an estimate of the ATmega's supply voltage - useful for battery powered projects: `getSupplyVoltage()`. Returns voltage as a floating-point number.


# USAGE REFERENCE

    #include "AnalogControlPanel.h"

## STATE: On/Off Control

    bool InternalADC.isOff()
    bool InternalADC.isOn()

Is the ADC enabled and is its clock running?

    InternalADC.powerOn()
    InternalADC.powerOff()
    InternalADC.begin()
    InternalADC.end()

Start and stop the InternalADC. Stopping it reduces current consumption by about 300 microamps while the processor is awake, which may be useful for battery powered projects that only use the ADC now and then, for example to read the battery voltage. The ADC must also be stopped before going into power-off sleep, or it will continue to consume current.


## SCALE: Voltage Reference for Max Input Voltage

    InternalADC.referenceDefault()     // Arduino default, default after powerOn() or begin().
    InternalADC.referenceInternal()    // Arduino's INTERNAL. 1100 millvolts nominal
    InternalADC.referenceExternal()    // If you know what you are doing.

    InternalADC.reference(REF)         // REF is one of DEFAULT, INTERNAL, EXTERNAL.

Set the ADC's reference "full scale" voltage for readings (equal to a reading
of 1024, if the ADC could output a number bigger than 1023 in 10-bit mode).

`referenceDefault()` is the chip's supply voltage on the AVCC pin. It can vary quite a bit, especially on batteries.

`referenceInternal()` is an internal circuit with a nominal voltage of 1.100 V,
plus or minus 10%. A bit more stable than batteries. See readInternalReference() below to measure it on each chip.

The ATmega turns off its internal reference when not in use, so after selecting
it, wait 70 microseconds for it to stabilise. Or do other things taking that
long, like reading a real-time clock module over I2C.

To specify the internal reference's exact voltage in your sketches after measuring it, use

    InternalADC.setInternalReferenceVoltage(1.094); // will be remembered for that sketch.

The default value is 1.100 (1.1 volts).

WARNING: I have heard that some Arduino clone suppliers bridge the AREF pin to AVCC on the printed circuit board. You can only use referenceDefault() with such boards.

Any external voltage reference, for example a TL431 or LM4040, must be between 1 volt and AVCC. `referenceExternal()` is for knowledgeable circuit designers.


## Resolution: Bit Depth Of Samples

    InternalADC.bitDepth8()
    InternalADC.bitDepth10()  // Default after On(), Arduino value.

    // also:-
    InternalADC.readResolution(8)
    // Allowed values: 8, 10. Like Arduino's analogReadResolution() function.

8-bit depth is used with read8Bit() and getLastReading8Bit() below.
Reading with 8-bit depth returns a value between 0 and 255.

Read() and SampleValue() will return correct results, but in a 16-bit integer.

10-bit depth is the default and returns a value between 0 and 1023.


## ADC SPEED

Speed is also referred to as Sample Rate (symbol: sa/s, samples per second).

Easy mode functions:-

    InternalADC.speed1x()   // Arduino standard, the default rate after begin().
    InternalADC.speed2x()
    InternalADC.speed4x()


More nerdy:-

    InternalADC.rate9k()    // Equal to speed1x().
    InternalADC.rate18k()   // speed2x().
    InternalADC.rate37k()   // speed4x().
    InternalADC.rate74k()   // Usually gives poor results.

    InternalADC.rate4k()    // 4k only usable with 1MHz breadboard arduinos
                            // possibly poor results.


Set the ADC's maximum sample rate (reading rate), the rate at which it takes readings in free-running mode. According to Nick Gammon's experiments (see References), rates up to 37000 samples/second (37k) do not affect accuracy or precision very much, at least with signal sources within specification (under 10 kohm source impedance).

At 74k, accuracy is compromised, although you might get lucky with your chip.

The number of settable rates is small. Internally, the ADC uses a fraction of the master clock frequency (16 MHz for an Uno) to control taking readings. Only a limited number of these fractions are available.

To take readings at other, slower rates (down to once per month?), see `triggerOnInterrupt0()`, `triggerOnTimer1CompareB()`, and `triggerOnInputCapture()` below.


## TRIGGERING (STARTING)

    InternalADC.singleReadingMode() [default]
    InternalADC.freeRunningMode()


    InternalADC.triggerOnInterrupt0()
    InternalADC.triggerOnInputCapture()
    InternalADC.triggerOnTimer1CompareB()
    InternalADC.triggerOnTimer1Overflow()

### Single Reading Mode and Free-Running Mode

`singleReadingMode()` in **single-reading mode**, the ADC starts taking one sample (reading) after you do `startReading()` (see below under "non-blocking reads"). The `read()` functions (below) do `startReading()` internally to take a single reading.

`freeRunningMode()` in **free-running mode**, the ADC starts taking the next sample as soon as it has finished the previous one. So after doing `startSample()` once, you can just read values with `getLastReading()` or `getLastReading8Bit()` when desired.

To go back to single reading mode use `singleReadingMode()`, or use `end()` to stop the ADC entirely.


### Event-Based Triggering / Initiation

    InternalADC.triggerOnInterrupt0()

Start a sample when the INTF0 flag bit in the EIFR
register is first set by a change on pin 2, after attachInterrupt(0).

For example, the PPS (pulse per second) output from a GPS module could be used with `triggerOnInterrupt0()` to take one sample every second. Or the 32 kHz output from a DS3232 RTC module could be used instead, to take a sample at 32,768 samples per second, or fractions of this (16384, 8192, .., 8, 4, 2, 1).


    InternalADC.triggerOnInputCapture()

Start taking a sample when an input capture interrupt occurs due to a rising or falling voltage on pin 8 after Timer 1's Input Capture unit has been configured. (Advanced: no functionality in the base Arduino system.)

`triggerOnInterrupt0()` and `triggerOnInputCapture()` also allow random event based sampling, taking a reading whenever a particular event occurs on pins D2 or D8.


    InternalADC.triggerOnTimer1CompareB(), triggerOnTimer1Overflow()

You can configure Timer1 to count at various rates and use these modes to take regular samples. (Advanced: no functionality in the base Arduino system.)




## SOURCE: INPUT SELECTION

    InternalADC.usePin(pin)
    InternalADC.freePin(pin)

`usePin()` connects the ADC's input to the specified pin ready to take samples. It also disconnects the AVR's digital circuitry from the pin to reduce power consumption and possible electrical "noise" while taking readings, as recommended by AVR/Microchip in the data sheet for the ATmega328P.

Arduino's `digitalRead()` and `digitalWrite()` will not work on the pin selected with `usePin()`.

Pin must be one of the analog pins on an Arduino, A0 .. A7. (The Uno does not have A6 or A7.)

`freePin()` reconnects the used pin's digital circutry, allowing
`digitalRead()` and `digitalWrite()` to work, and sets the ADC's input to its internal ground connection, so no pin is used by the InternalADC.


Internally-used functions:-


    InternalADC.disconnectPinDigitalInput(pin)
    InternalADC.reconnectPinDigitalInput(pin)

    InternalADC.disconnectAllDigitalInputs()
    InternalADC.reconnectAllDigitalInputs()

Disconnect / reconnect the digital input circuitry for an analog pin or all six* of them, for lower power consumption with voltages around half of supply voltage. These may be useful separately from `usePin()` and `freePin()`.

NOTE: A4 and A5 are used digitally for Wire (the built-in I2C peripheral).

\* A6 and A7, which exist on Nanos and Pro Minis, don't have digital circuitry.




## SIGNALING: COLLECTING READINGS WHEN DONE

### 1. VIA THE "ADC CONVERSION COMPLETE" INTERRUPT

You probably want to skip this section on a first read.

This may be useful when using the 'auto' modes  (freeRunMode, triggerOnInterrupt0, etc.).

    InternalADC.interruptOnADCDone()
    InternalADC.noInterruptOnADCDone()

Enable or disable the "ADC Conversion Complete" interrupt. The interrupt will only work if Arduino's `interrupts()` is used to enable interrupts of any kind.


    InternalADC.attachDoneInterruptFunction(interrupt_function)
    InternalADC.detachDoneInterruptFunction()


Write a function to be called as soon as the ADC has finished taking a sample. This might be useful if you want to accumulate samples while the CPU is doing other things, for example writing to an SD card. The function must be of form

   void ADCdoneFunction() { ... }

that is, take no parameters and not return a value, instead changing global
variables.

The usual warnings apply for functions in the ISR (interrupt service routine)
context: make it short, don't use delay() or Serial, Wire, or SPI; and any
variables used both in the interrupt_function and other places must be declared
`volatile`.

Probably one such variable should be a flag to tell your other code in loop() that the ADC has a new reading ready.

Maybe another is the reading itself or an array of them. Or you could use `InternalADC.getLastReading()` from `loop()` instead, if it's OK to lose a reading now and then - see below under non-blocking sampling.


### 2. BLOCKING SAMPLING (LIKE `analogRead`)

"Blocking" means that the ATmega can't do anything else until the ADC
is finished. (27ish microseconds if speed4x() is used before `read()`, 110-ish microseconds with the default `speed1x()`. Double those for the first read after starting.)


    int InternalADC.analogRead(const uint8_t pin)

This corresponds to Arduino's analogRead(pin) function.


    int InternalADC.read()

You must do `usePin(pin)` beforehand. Repeated `read()`s continue to use the same pin and other settings. Results depend your chosen bit depth:-

* After `bitDepth8()`,  returns 0..255.
* After `bitDepth10()`, returns 0..1023.


    uint16_t InternalADC.sleepRead()

For lower noise: turns off the CPU, timers, and other parts of the chip while the ADC works, and turns them back on when it's finished. Wake-up takes about 10 CPU-clock cycles on top of the ADC's processing time.


    uint8_t InternalADC.read8Bit()

Low-RAM read, e. g. for storing hundreds of readings for a simple oscilloscope. Returns 0 .. 255 in a single byte. Assumes you have done `bitDepth8()` beforehand: does not check.



### 3. NON-BLOCKING READS

Non-blocking means that the ATmega starts the ADC, then continues processing while the ADC is working in the background.

    InternalADC.startReading()

    bool InternalADC.readingReady()

    uint16_t InternalADC.getLastReading()

    uint8_t  InternalADC.getLastReading8Bit()

Here's an example with `singleReadingMode()`, the default. In `singleReadingMode()`, you have to initiate every reading:-


    void setup() {
        ... other setup stuff...
        InternalADC.powerOn();
        InternalADC.usePin(SENSORPIN);
        InternalADC.speed2x();
        ...
    }

    void loop() {

        // Static variables: value is remembered from one loop to the next.
        // We might go around the loop several times before the ADC has finished.

        static bool waitingforADC = false, wantADCreading = true;

        uint16_t adcSample;  // variable to hold the ADC reading

        if (wantADCReading && ! waitingforADC) {

             InternalADC.startReading();
             waitingforADC = true;
        }

        ... do other work ...
        ... while waiting ...

        if (waitingforADC && InternalADC.readingReady()) {

            waitingforADC = false;
            adcSample = InternalADC.getLastReading();

            // start the next sample if wanted.
            InternalADC.startReading();
            waitingforADC = true;

            // otherwise: wantADCReading = false;

            ... do things with adcSample ...
        }
        ..... more code ...
    }

---


#### With `freeRunningMode()`

In free running mode, after the first `startReading()`, the ADC itself starts the next as soon as the last is finished. Basically one sample every 27 microseconds (assuming you've used `speed4x()`).

Just carry on with other stuff, and `getLastReading()` whenever you want:-

    void setup() {
        ...
        ...
        InternalADC.powerOn();
        InternalADC.speed4x();
        InternalADC.bitDepth8();        // only want 8-bit values.
        InternalADC.usePin(SENSORPIN);
        InternalADC.freeRunMode();      // ADC triggers itself once started.
        InternalADC.startReading();     // Trigger the first sample.
        delayMicroseconds(54);          // First sample takes double time.
    }

    void loop() {
        uint8_t adcval = InternalADC.getLastReading8Bit();
        ...
        ... more code, using adcval...
        ... meanwhile, the ADC is working in the background
        ... producing a new reading every 27 microseconds.
    }



### Specials: Internal Sensors

    InternalADC.readGround()
    InternalADC.readInternalReference()
    InternalADC.readTemperature()

Raw 0-1023 ADC readings, blocking, no pins used.

`readGround()` hopefully returns 0.

`readInternalReference()` is useful for battery monitoring.
With InternalReference used as a reference, hopefully it returns 1023.

NOTE: `ReadTemperature()` does `referenceInternal()`, i.e. changes the ADC's voltage reference to the internal voltage reference, and has a wait of 6 ms.

Repeated readings of the temperature sensor increase up to 1015 or so,
so just read it now and then. The temperature sensor is not well calibrated
or controlled -- there is a lot of variation between chips. An external
temperature sensor (if necessary, glued to the chip) will be a better bet.



### Power Supply Voltage Estimate


    float InternalADC.getSupplyVoltage()

Return an estimate of the battery voltage as a floating point number, in volts. Uses `referenceDefault()` and `ReadInternalReference()` to calculate and return the voltage on AVCC as a floating-point number in volts. Blocking function.


## References and Further Information

Nick Gammon's most excellent page on the [AVR ATmega328P ADC](https://www.gammon.com.au/adc), and of course the ATmega328P data sheet.

For an introduction to using Arduino boards for science and engineering projects, see *Arduino for Projects in Scientific Measurement* by Randy Normann. On Measurement, LLC, 2018. ISBN-13 978-0-9997536-1-3.  Discusses anti-aliasing and signal conditioning; oversampling vs smoothing; noise, interference and grounding; input protection; calibration; and more.

For a fully realised, thoroughly documented project, see Ed Mallon's [Cave Pearl Project](https://thecavepearlproject.org) wet-environment data loggers. Many practical issues thoroughly treated: battery budgeting, pre-deployment and post-deployment testing and group normalising, weatherproofing, cable and connector durability, chemical, insect and plant damage, temperature compensation, ...


#### NOTE

There are quite a few "gotchas" with analog readings. If you must have good reliable results, you need to know about input protection and buffering, anti-aliasing, filtering and signal level shifting, star grounding and shielding, power supply de-noising, and voltage reference stabilisation. The real world is awkward, and the saying "garbage in, garbage out" applies very strongly.

Good luck!



TODO
====

In Progress
-----------

 Documentation, examples


Maybe
-----

*   Calibration Functionality.

    InternalADC.StoreCalibration(const int calibparams[])

Store parameters for a calibration/reading correction formula.
Need to work out and document a calibration procedure first.

    int InternalADC.CorrectedValue(int adcreading)

Function using the stored calibration to correct the raw reading.

Issues: calibration can vary depending on the power supply, the  selected voltage reference, and on the temperature (which varies!) and the characteristics of the individual chip.



*    Variants for ATtiny84, ATmega2560, ATmega1284P.

These chips have extra features like more input pins, differential readings,
and/or selectable amplification of signals ("programmable gain"). But they're
all slightly different in features and arrangements.
