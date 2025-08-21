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

TODO Tests reading repeatability - does reading the same voltage always give the same answer? If not, how big are the differences?

This sketch does repeated readings on a simple resistive divider to produce a simple "stem-and-leaf plot" and a summary figure-of-merit for "reading scatter".

## Calibrate ADC

TODO
