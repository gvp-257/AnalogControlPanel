#ifndef ANALOG_CONTROLPANEL_M328P_H
#define ANALOG_CONTROLPANEL_M328P_H

// GvP 2025-08.
// https://github.com/gvp-257/analogcontrolpanel

// Values for the "reference" function. (Like Arduino's analogReadReference().)
#ifndef DEFAULT
#define DEFAULT 0
#endif
#ifndef INTERNAL
#define INTERNAL 1
#endif
#ifndef EXTERNAL
#define EXTERNAL 2
#endif

#ifndef cli
#define cli() __asm__ __volatile__ ("cli" ::: "memory")
#endif
#ifndef sei
#define sei()  __asm__ __volatile__ ("sei" ::: "memory")
#endif

/* Sections correspond to the "seven 'S'es" of using an ADC:
 *
 * STATE: on/off
 * SCALE: max voltage/reference setting
 * SENSITIVITY or Bit Depth or Precision or Resolution
 * SOURCE: which pin
 * SPEED: ADC clock rate -> max sampling rate.
 * STARTING = set triggering mode, manual, free-running  or on an event.
 * SAMPLING: doing/getting the readings. Including specials.
*/
// One instance, called InternalADC
struct _M328P_ADC
{
public:
    void begin(void);
    void end(void);
    // STATE: ON OR OFF
    bool isOn(void);
    bool isOff(void);

    void powerOn(void);
    void powerOff(void);


    // SCALE: ADC VOLTAGE REFERENCE


    void reference(void);
    void reference(const int);

    // Default = ATmega's supply voltage.
    void referenceDefault(void);

    // Internal bandgap reference, nominal 1.1V.
    void referenceInternal(void);
    void setInternalReferenceVoltage(const float);

    // External voltage reference IC on AREF pin, e.g. a TL431 or LM4040.
    void referenceExternal(void);

    void setScale(const int);
    void setScale(void);


    // SENSITIVITY/RESOLUTION/PRECISION: BIT DEPTH SETTING
    void bitDepth8(void);
    void bitDepth10(void);
    void readResolution(const uint8_t);  // 8 or 10 - Arduino contravening own guidelines
    void readResolution();               // default 10.


    // SOURCE: INPUT SELECTION

    // Connect a Pin to the ADC input.
    void usePin(uint8_t pin);
    // Sets ADC's input to internal ground connection.
    void freePin(uint8_t pin);

    void reconnectPinDigitalInput(uint8_t pin);
    void disconnectPinDigitalInput(uint8_t pin);
    inline void disconnectAllDigitalInputs() {DIDR0 = 0x3F;}
    inline void reconnectAllDigitalInputs()  {DIDR0 = 0;}



    // SPEED: SAMPLE RATES

    void speed1x(void);  // ==  rate9k, clock125k
    void speed2x(void);  // == rate18k, clock250k
    void speed4x(void);  // == rate37k, clock500k
    void speed8x(void);  // == rate75k, clock1M


    void rate75k(void);  // too fast: inaccurate (wrong not just imprecise)
    void rate37k(void);  // slight loss of precision
    void rate18k(void);  // Fine
    void rate9k(void);   // Fine, lowest possible with 16MHz sys clock.
    void rate4k(void);   // Better with 1MHz clock?

    void clock1M(void);
    void clock500k(void);
    void clock250k(void);
    void clock125k(void);
    void clock62k5(void);


    // STARTING / TRIGGERING config: When to START to take a reading

    // Manual sampling where you use startReading() each time.
    // (Or if you use one of the read() functions, which do that internally.)
    void singleReadingMode(void);

    // Set the ADC to take samples continuously.
    // After first startReadinge(), can use getLastReading() repeatedly.
    // The ADC still takes 27 or more microseconds per sample, though, so
    // you'll just get the same sample if you re-read before it's done the next.
    // (See "When will the ADC finish? below.)
    void freeRunMode(void);

    // Uses Timer 1's input capture flag bit
    void triggerOnInputCapture(void);

    // External interrupt 0 flag bit
    // Can use this with RTC 32 kHz output or GPS PPS output.
    void triggerOnInterrupt0(void);

    void triggerOnTimer0Overflow(void);

    void triggerOnTimer1CompareB(void);
    void triggerOnTimer1Overflow(void);


    // SIGNALING: NOTIFICATION OF COMPLETION

    // When will the ADC finish?
    // Readings take 13 ADC clock cycles, except the first after powerOn, which
    // takes 25 ADC clock cycles. At clock125k, that's 104 or 200 microseconds.
    // With manual triggering (singleReadingMode), there will on average be
    // half an ADC clock cycle extra.

    // 1. Set up "ADC Conversion Complete" Interrupt Handling
    void interruptOnDone(void);
    void noInterruptOnDone(void);

    void attachDoneInterruptFunction(void(*fn)(void));
    void detachDoneInterruptFunction(void);

    // Define your own function to be called as soon as the ADC says "done!":
    // void myADCProcessingFunction(void) { ... ;}
    // InternalADC::attachDoneInterruptFunction(myADCProcessingFunction);
    // InternalADC::interruptOnDone(); // makes the ADC call your function.



    // 2. See if the ADC has finished processing a reading.
    bool readingReady(void);



    // TAKING READINGS == SAMPLES WITH THE ADC

    // 0. Get the ADC's latest reading - don't start a new one.
    int     getLastReading(void);
    uint8_t getLastReading8Bit(void);


    // 1. BLOCKING READS.
    // These wait for the ADC to finish, can't do anything meanwhile.

    int     analogRead(const uint8_t pin); // Like Arduino's analogRead(pin).


    // Use 'usePin(pin)' before using read(), read8Bit(), or sleepRead().

    int     read(void);  // 0..1023 for bitDepth10, 0..255 for bitDepth8.

    // 0..255 in a single byte. Smaller for arrays of readings.
    uint8_t read8Bit(void);

    // With sleepRead(), the CPU is stopped. Lower noise.
    int     sleepRead(void);


    // 2. NON-BLOCKING READS.
    // The cycle is:
    // power on, set bit depth, speed, reference
    // usePin(sensorPin)
    // start; loop(if readingReady(): getLastReading and start next, use last).
    // freePin(sensorPin)

    // In triggered modes, don't use startReading().
    // (triggerOnInterrupt0(), etc.). An event (interrupt) will start the ADC.

    void startReading(void);


    // 3. SPECIALS: Read Internal Sensors - no pin selected.

    // Raw ADC reading 0..1023 from internal ground connection.
    // Blocking read. Should always be 0!
    int readGround(void);

    // Raw 10-bit 0..1023 ADC reading from internal bandgap voltage ref.
    // Blocking read.
    int readInternalReference(void);

    // Raw 10-bit ADC reading from internal temperature sensor. (around 220?)
    int readTempSensor(void);


    // Get voltage at AVCC (ATmega's battery voltage)
    // **as a floating point number**
    // Unit is volts.

    float getSupplyVoltage(void);


private:
    float bandgapV = 1.1;
    uint8_t _psbitsfromclock(const unsigned long);
    void    _setClock(const unsigned long);

}; // struct _M328P_ADC

extern struct _M328P_ADC InternalADC;


#endif
