
12-bit depth works by oversampling and decimation, taking 16 readings and then
'averaging' them to return a value between 0 and 4092. It is slow, and ideally
used with input voltages that change very slowly. It also requires a noise source - see "Dither" below.


**Dither Required with 12-bit**

For oversampling and decimation to work, the readings must vary a little and be
averaged. This means that a small amount of electrical 'noise' must be added to
the signal that you are measuring.

One traditional way of doing this is with a small triangle wave.
On the Arduino this can be done by using the `tone()`
function with an attenuator/integrator circuit,  applying it to the ADC input
pin along with the sensor voltage.

At the default sample rate (`Rate9k()`), 16 samples are taken in 1728
microseconds, equivalent to a frequency of 578.7 Hz. So we'd use `tone(TONE_PIN, 579)` before starting and `notone()` when the sample is ready.


+--+    10 nF      +-------+                       +--+
|  |------||-------| 250 K |----+-------------+----|  | SAMPLING_PIN
+--+               +-------+    |             |    +--+  (A0 .. A7)
TONE_PIN (D4)                      |            +-+
579 Hz                            |            | | 2K2
1 uF    ===           +-+
(105)    |             |       +--+
|             +-------|  |  SENSOR_OUT
Gnd                             |                     +--+  from module
--------------------------------+----------------------

Example Dither Circuit. Credit: "Qwerty" on the Freetronics forum.

This will provide about 6 millivolts peak-to-peak from TONE_PIN at
the sampling pin, using tone() from a 3.3V Arduino Pro mini running on
batteries at 3.1V. That is about two least significant bits' dither.

If you change the tone frequency significantly, the 1uF capacitor will have
to be changed also.
