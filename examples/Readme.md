## Analog Control Panel Examples

### Read LDR

Basic example, reading the voltage at the junction of an LDR and ordinary resistor to estimate light level.

### Read Battery

Shows the readBattery() function.

### Measure Internal Reference

TODO Puts the internal reference voltage on the "AREF" pin so you can measure it with a multi-meter.

### Measure Capacitor

TODO Uses the R-C time constant formula with a known resistance to estimate the value of a capacitor.

### One Pin Many Buttons

TODO shows how to use a resistor ladder and an analog input pin to detect which of five buttons was pressed.

### Bit Depth 12

TODO Demonstrates a technique called "oversampling and decimation" to get more precision out of the 10 bit ADC in AVR chips. It's not easy and there is no free lunch, though. Extra hardware needed.

### Test Reading Scatter

Tests reading repeatability - does reading the same voltage always give the same answer? If not, how big are the differences?

This sketch does repeated readings on a simple resistive divider to produce a simple "stem-and-leaf plot" and a summary figure-of-merit for "reading scatter" for different reading methods, speeds, and voltage references. By default only the summary tables are printed.

Uses [SendOnlySerial](https://github.com/gvp-257/SendOnlySerial) instead of `Serial` to report back to the Arduino serial monitor. SendOnlySerial has a couple of useful macros.

Sample output (sleepR is sleepRead(), freerun is freeRunningMode with getLastReading()).

With an Uno clone on USB power:

    Summary tables of scatter scores (lower is better)

    Supply as voltage reference
    Method-speed     1x      2x      4x
    read()           53      59      49
    sleepR           34      16      63
    freerun          56      56      78

    Internal voltage reference
    Method-speed     1x      2x      4x
    read()           13      16       8
    sleepR           38      24       1
    freerun          31      27       8


## Calibrate ADC

TODO
