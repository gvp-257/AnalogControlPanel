## Analog Control Panel Examples

### Read LDR

TODO Basic example, reading the voltage at the junction of an LDR and ordinary resistor to estimate light level.

### Read Battery

TODO Shows the `getSupplyVoltage()` and `setInternalReference()` functions. The latter allows you to correct the voltage reported by `getSupplyVoltage()`, and all the other readings you do using the internal reference.

### Measure Internal Reference

TODO Puts the internal reference voltage on the "AREF" pin so you can measure it with a multi-meter.

### Measure Capacitor

TODO Uses the R-C time constant formula with a known resistance to estimate the value of a capacitor. Needs a resistor of known value around 10K, a small resistor of 220R to 330R (value not critical), and one or more capacitors to test.

### Multi Button Pin

TODO shows how to use one analog input pin to detect which of five buttons was pressed. The example requires five resistors and five button switches as shown in the Readme for MultiButtonPin. The technique can be adapted for fewer buttons and maybe one more than five.

### Bit Depth 12

TODO Demonstrates a technique called "oversampling and decimation" to get more precision out of the 10 bit ADC in AVR chips. It's not easy and there is no free lunch, though. Extra hardware needed.

### Test Reading Scatter

Tests reading repeatability - does reading the same voltage always give the same answer? If not, how big are the differences? How are the readings "scattered"?

And, do different reading methods and speeds give the same level of repeatability, or different?

This sketch does repeated readings on a simple resistive divider to produce a simple "stem-and-leaf plot" and  summary tables of "figures-of-merit" for reading scatter, for three reading methods, three speeds, and two voltage references (default and internal). Requires two resistors and a capacitor, as shown in the Readme for testReadingScatter.

By default only the summary tables are printed.

TestReadingScatter uses [SendOnlySerial](https://github.com/gvp-257/SendOnlySerial) instead of `Serial` to report back to the Arduino serial monitor. SendOnlySerial has a couple of useful macros.

Sample output (sleepR is sleepRead(), freerun is freeRunningMode with getLastReading()), with an Uno clone on USB power:

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

TODO.
