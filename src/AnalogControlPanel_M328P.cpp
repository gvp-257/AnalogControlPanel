
// GvP 2025-08.
// https://github.com/gvp-257/analogcontrolpanel

#include <avr/io.h>         // AVR-libc header files. "*bit_is_*" macros.
#include <avr/sleep.h>      // for sleepRead()
#include <util/delay.h>     // for _delay_us().


#include "AnalogControlPanel_M328P.h"
#include "ACP_M328P_interrupt.h"

//------------------------------------------------------------------------------

// A framework for the ADC control functions.
// Seven groups
//   power on - off
//   set sample rate <-> ADC clock
//   configure bit depth (8 bit - 10 bit)
//   set the trigger mode - single-shot, auto on interrupt, or free-running
//   select input pin
//   take readings, various ways
//   specialised no-pin readings - internal temperature sensor and
//     voltage reference, and supply voltage estimate.

// Arduino-style begin-end-readAPin functions
void _M328P_ADC::begin()
{
    // Set state, scale, resolution, speed, triggering
    powerOn();
    reference(DEFAULT);
    bitDepth10();
    speed1x();
    singleReadingMode();
}

void _M328P_ADC::end() {powerOff();}


bool _M328P_ADC::isOn()
    {return (bit_is_clear(PRR, PRADC) && bit_is_set(ADCSRA, ADEN));}
    // bit_is_* macros are from avr/sfr_defs.h, included from avr/io.h.

bool _M328P_ADC::isOff()
    {return (bit_is_clear(ADCSRA, ADEN) || bit_is_set(PRR, PRADC));}

void _M328P_ADC::powerOn()
{
    cli();
    ADCSRA = 0;             // ensure disabled before changing PRR
    PRR   &= ~ _BV(PRADC);  // enable ADC clock
    ADCSRA = 0x96;
    // 9=enable, no start conversion, disable autotrigger, clear pdg. interrupts,
    // 6=disable interrupts, prescale 6 = divide by 64.
    // ensure ADC disabled before changing PRR bit.
    ADCSRB = 0;             // no autotrigger source
    ADMUX  = 0x4f;
    // 4=Ref=AVCC, ADLAR = 0,
    // f=Input = internal Gnd
    sei();

}

void _M328P_ADC::powerOff()
{
    cli();
    ADCSRA |=  (1<<ADIF);    // clear pending interrupt
    ADCSRA &= ~(1<<ADEN);    // disable ADC
    PRR |= _BV(PRADC);       // turn off its clock
    DIDR0 = 0;               // all analog pins usable for digital I/O.
    sei();
}


void _M328P_ADC::bitDepth8()
{
    cli();
    ADCSRA &= ~((1<<ADATE)|(1<<ADIE));  //disable autotrigger and interrupts
    ADMUX |= (1<<ADLAR);     //left-adjust: upper 8 bits in ADCH register.
    sei();
}

void _M328P_ADC::bitDepth10()
{
    cli();
    ADCSRA &= ~((1<<ADATE)|(1<<ADIE));  //disable autotrigger and interrupts
    ADMUX &= ~(1<<ADLAR);               // on change of bit depth
    sei();
}

InternalADCSettings _M328P_ADC::saveSettings(void)
{
    InternalADCSettings s;
    cli();
    s.adcsra = ADCSRA;
    s.adcsrb = ADCSRB;
    s.admux  = ADMUX;
    sei();
    return s;
}

void _M328P_ADC::restoreSettings(InternalADCSettings s)
{
    cli();
    ADMUX  = s.admux;
    ADCSRB = s.adcsrb;
    ADCSRA = s.adcsra;
    sei();
}

void _M328P_ADC::readResolution(const uint8_t bits)
{
    if (bits == 8) bitDepth8();
    else bitDepth10();
}
void _M328P_ADC::readResolution() {bitDepth10();}


// internal: translate from desired clock rate to prescaler setting.
uint8_t _M328P_ADC::_psbitsfromclock(const unsigned long clk)
{
    uint8_t bits;
    uint8_t ps = (uint8_t)(F_CPU / clk);
    // TODO find a log2 function that I understand.
    switch (ps)
    {
        case   2: bits = 0x01; break;
        case   4: bits = 0x02; break;
        case   8: bits = 0x03; break;
        case  16: bits = 0x04; break;
        case  32: bits = 0x05; break;
        case  64: bits = 0x06; break;
        case 128: bits = 0x07; break;
        default:  bits = 0x07; break;
    }
    return bits;
}

// Internal: set ADC clock according to desired rate.
void _M328P_ADC::_setClock(const unsigned long clk)
{
    uint8_t psbits = _psbitsfromclock(clk);
    cli();
    ADCSRA &= 0b11111000;  // all prescale bits off
    ADCSRA |= (psbits & 0x07);
    sei();
}

// visible functions
void _M328P_ADC::clock1M(void)   {_setClock(1000000UL);}
void _M328P_ADC::clock500k(void) {_setClock( 500000UL);}
void _M328P_ADC::clock250k(void) {_setClock( 250000UL);}
void _M328P_ADC::clock125k(void) {_setClock( 125000UL);}
void _M328P_ADC::clock62k5(void) {_setClock(  62500UL);}

void _M328P_ADC::rate75k(void)   {_setClock(1000000UL);} // ADC clock 1MHz.OK for 8bit maybe.
void _M328P_ADC::rate37k(void)   {_setClock( 500000UL);} // OK-ish for 10bit
void _M328P_ADC::rate18k(void)   {_setClock( 250000UL);} // Fine
void _M328P_ADC::rate9k(void)    {_setClock( 125000UL);} // Fine, lowest with 16MHz sys clock.
void _M328P_ADC::rate4k(void)    {_setClock(  62500UL);} // Better with 1MHz clock?

void _M328P_ADC::speed1x(void)   {_setClock( 125000UL);} // Arduino compatible functions.
void _M328P_ADC::speed2x(void)   {_setClock( 250000UL);}
void _M328P_ADC::speed4x(void)   {_setClock( 500000UL);}
void _M328P_ADC::speed8x(void)   {_setClock(1000000UL);}



// SCALE: ADC VOLTAGE REFERENCE

// The reference would be a reading of 1024 if the ADC could go past 1023.
// or 256 for 8-bit 0..255 mode.

void _M328P_ADC::reference(const int ref)
{
    if (ref == DEFAULT)  {referenceDefault(); return;}
    if (ref == INTERNAL) {referenceInternal();return;}
    if (ref == EXTERNAL) {referenceExternal();return;}
    // error.
    referenceDefault();
}
void _M328P_ADC::reference() {reference(DEFAULT);}

// Default = supply voltage is maximum/reference.
void _M328P_ADC::referenceDefault()
{
    ADMUX &= ~(1<<REFS1);   // ref selection = 01.
    ADMUX |=  (1<<REFS0);
}

/* Internal bandgap reference, nominal 1.1V.
    * NOTE: If the bandgap reference was not in use before, it takes up to 70
    * microseconds to stabilise when selected.
    */
void _M328P_ADC::referenceInternal()
{
    ADMUX |= (1<<REFS0) | (1<<REFS1);  // ref bits 11
    _delay_us(70);                     // stabilise internal BG ref.
}

void _M328P_ADC::setInternalReferenceVoltage(float newV){bandgapV = newV;}

/* External voltage reference IC on AREF pin, e.g. a TL431 or LM4040.*/
void _M328P_ADC::referenceExternal()
{
    ADMUX &= ~(1<<REFS1);  // ref bits 00.
    ADMUX &= ~(1<<REFS0);
}

void _M328P_ADC::setScale( const int ref)
{
    reference(ref);
}

void _M328P_ADC::setScale()
{
    reference(DEFAULT);
}


// SOURCE: INPUT SELECTION
// =======================

// 1. External Pins.

// Disconnect digital input circuitry on the selected pin to reduce current
// consumption, as recommended by the data sheet.

void _M328P_ADC::disconnectPinDigitalInput(uint8_t pin)
{
    if (pin > 13) pin -= 14;
    switch (pin)
    {
        case 0:
            DIDR0 |= (1<<ADC0D); break;
        case 1:
            DIDR0 |= (1<<ADC1D); break;
        case 2:
            DIDR0 |= (1<<ADC2D); break;
        case 3:
            DIDR0 |= (1<<ADC3D); break;
        case 4:
            DIDR0 |= (1<<ADC4D); break;
        case 5:
            DIDR0 |= (1<<ADC5D); break;
        default:
            ; // nothing to do.
    };
}
void _M328P_ADC::reconnectPinDigitalInput(uint8_t pin)
{
    if (pin > 13) pin -= 14;
    switch (pin)
    {
        case 0:
            DIDR0 &= ~(1<<ADC0D); break;
        case 1:
            DIDR0 &= ~(1<<ADC1D); break;
        case 2:
            DIDR0 &= ~(1<<ADC2D); break;
        case 3:
            DIDR0 &= ~(1<<ADC3D); break;
        case 4:
            DIDR0 &= ~(1<<ADC4D); break;
        case 5:
            DIDR0 &= ~(1<<ADC5D); break;
        default:
            ; // no can do.
    };
}

void _M328P_ADC::usePin(uint8_t pin)
{
    if (pin > 13) pin -= 14; // Arduino A0 = 14, etc. MUX needs 0..7.
    ADMUX &= 0xf0;           // reset MUX3..MUX0 bits to zero
    ADMUX += (pin & 0x07);   // pin number 0 .. 7 is MUX value (ADC input)
    disconnectPinDigitalInput(pin);
}

void _M328P_ADC::freePin(uint8_t pin)
{
    ADMUX  |= 0x0f;         // set ADC input to internal ground connection.
    reconnectPinDigitalInput(pin);
}

// TRIGGER: When to start to take a reading
// ========================================


// "Normal" sampling where you use startReading() each time.
// (Or, use one of the read() functions, which do that internally.)

void _M328P_ADC::singleReadingMode()
{
    ADCSRA &= ~(1<<ADATE);  // disable autotrigger
    ADCSRA &= ~(1<<ADIE);   // disable interrupts
    // so: check ADSC==0 for conversion complete
    // (SampleReady does this).
}


// Set the ADC to take samples continuously.
// After first startReading(), can use getLastReading() repeatedly.
// The ADC still takes at least 27 microseconds per sample, though, so
// you'll just get the same sample if you re-read sooner than that.

void _M328P_ADC::freeRunningMode()
{
    ADCSRB  = 0x00;         // trigger source 0 = "ADC conversion complete".
    ADCSRA |= (1<<ADATE);   // enable autotrigger
}

void _M328P_ADC::triggerOnInputCapture()
{
    ADCSRB = 0x07;           // trigger source 7 = "Timer1 Capture Event Flag".
    ADCSRA |= (1<<ADATE);    // autotrigger enable.
}

// Can use this with RTC 32 kHz output or GPS PPS output.
void _M328P_ADC::triggerOnInterrupt0()
{
    ADCSRB  = 0x02;          // trigger source 2 = "External IRQ 0 flag".
    ADCSRA |= (1<<ADATE);    // autotrigger enable.
}

void _M328P_ADC::triggerOnTimer0Overflow()
{
    cli();
    noInterruptOnDone();
    ADCSRB = 0x04;           // trigger source 4 = "Timer0 overflow flag".
    ADCSRA |= (1<<ADATE);    // autotrigger enable.
    sei();
}

void _M328P_ADC::triggerOnTimer1CompareB()
{
    cli();
    noInterruptOnDone();
    ADCSRB = 0x05;           // trigger source 5 = "Timer1 compare match B".
    ADCSRA |= (1<<ADATE);    // autotrigger enable.
    sei();
}
void _M328P_ADC::triggerOnTimer1Overflow()
{
    cli();
    noInterruptOnDone();
    ADCSRB = 0x06;           // trigger source 5 = "Timer1 overflow flag".
    ADCSRA |= (1<<ADATE);    // autotrigger enable.
    sei();
}

// SIGNALING: NOTIFICATION OF COMPLETION
// =====================================


// 1. ADC Conversion Complete Interrupt Handling
// =============================================

void _M328P_ADC::interruptOnDone()
{
    ADCSRA |= (1<<ADIF);    // Clear any pending interrupts
    ADCSRA |= (1<<ADIE);    // Enable interrupts.
}

void _M328P_ADC::noInterruptOnDone()
{
    ADCSRA |= (1<<ADIF);    // Clear any pending interrupts
    ADCSRA &= ~(1<<ADIE);   // Disable interrupts.
}

void _M328P_ADC::attachDoneInterruptFunction(void(*fn)(void))
    {_ADCDoneFunc = fn;}
void _M328P_ADC::detachDoneInterruptFunction()
//{_ADCDoneFunc = _ADCdefaultISR;}
{_ADCDoneFunc = 0;}

// How to see if the ADC is finished.
bool _M328P_ADC::readingReady()
{
    if (bit_is_clear(ADCSRA, ADATE))   // if not autotrigger mode i.e. "normal"
        return bit_is_clear(ADCSRA, ADSC);  // ADC clears ADSC on completion
    /*else: auto-trigger mode selected*/
    if (bit_is_clear(ADCSRA, ADIE))    //  autotrigger, interrupts not enabled
    {
        bool done = bit_is_set(ADCSRA, ADIF); // ADC done when IF set.
        if  (done) {ADCSRA |= (1<<ADIF);}  // clear IF ready for next sample

        return done;    // true if ADC finished.
    }
    /* else: auto-trigger mode (ADATE == 1) and interrupts enabled ADIE == 1
    * ADSC is not set, and since an interrupt routine exists (above),
    * the interrupt flag ADIF is cleared by firmware, so we don't know
    * here in this function whether the ADC is done.
    * The user ISR function should be setting a flag and the user code
    * checking that flag, not calling readingReady().
    */
    return _adcdone;
}



// TAKING READINGS == SAMPLES WITH THE ADC
// =======================================

// 0. Get the ADC's latest reading - don't start a new one.
// I.e., get the contents of its data registers ADCH and ADCL.

int _M328P_ADC::getLastReading(void)
{
    if (bit_is_set(ADMUX, ADLAR)) return (int)ADCH; // 8-bit mode
    return (int)ADC;
}
uint8_t _M328P_ADC::getLastReading8Bit() {return ADCH;}

/*
    * Blocking Reads.
    * Wait for the ADC to finish, can't do anything meanwhile.
    */

// Like Arduino's analogRead() function.
int _M328P_ADC::analogRead(const uint8_t pin)
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX  = ADMUX;
    bool wasOff = isOff();
    if (wasOff) begin();
    singleReadingMode();
    usePin(pin);
    ADCSRA |= (1<<ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    freePin(pin);
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    if (wasOff) end();
    return getLastReading();
}

// Use 'usePin()' before using the bare read().

// Returns 0..1023 if 10-bit resolution, 0..255 if 8-bit, in an int.
int _M328P_ADC::read(void)
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX  = ADMUX;
    singleReadingMode();       // disable event-based triggering
    ADCSRA |= (1<<ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    return getLastReading();
}


// Returns 0..255 in a single byte. Useful for arrays of readings.
uint8_t _M328P_ADC::read8Bit(void)
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX  = ADMUX;
    ADMUX  |= (1<<ADLAR);
    singleReadingMode();
    ADCSRA |= (1<<ADSC);
    loop_until_bit_is_clear(ADCSRA, ADSC);
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    return ADCH;
}


// sleepRead(): Blocking read with most clocks off. Lower noise.
// Assumes pin already set with UsePin().
// Puts the AVR into "SLEEP_MODE_ADC" for less digital noise: CPU stopped.

int _M328P_ADC::sleepRead(void)
{
    _adcdone = false;
    ADCSRA |= (1<< ADIF);              // clear pending interrupt if any.
    uint8_t oldADCSRA = ADCSRA, oldADMUX = ADMUX;
    bitDepth10();
    singleReadingMode();               // clear auto-trigger flag
    cli();
    set_sleep_mode(SLEEP_MODE_ADC);
    ADCSRA |= (1<<ADSC) | (1<<ADIE); // enable ADC interrupt to wake CPU.
    sleep_enable();
    sei();
    sleep_cpu();

    sleep_disable();
    // Awake again, reading should be ready, but better make sure.
    // Maybe an external interrupt woke the CPU (timers off in ADC sleep).
    cli();
    loop_until_bit_is_clear(ADCSRA, ADSC);
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    return getLastReading();
}

// Non-Blocking Sampling.

// The cycle is:
// start, loop(if readingReady(): getLastReading(), start next, process last.)

// In interrupt-triggered modes don't use StartSample().
// (triggerOnInterrupt0(), etc.)

void _M328P_ADC::startReading() {_adcdone = false; ADCSRA |= (1<<ADSC);}
// startReading() not needed except in singleShot and freeRunning modes.
// For the others, an event (external interrupt, etc.) will start the ADC.



// SPECIALS: Read Internal Sensors - no pin selected.

// Raw ADC reading 0..1023 from internal ground connection.
// Blocking read.

int _M328P_ADC::readGround()
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX = ADMUX;
    ADCSRA &= 0x97;        // turn off ADATE and ADIE, leave prescale bits
    ADMUX  |= 0x0f;        // Ground is MUX3..MUX0 = 1111 (source 15)
    read();                // Discard first reading.
    ADCSRA |= (1<<ADSC);   // Start conversion
    loop_until_bit_is_clear(ADCSRA, ADSC);
    int reading = (int)ADC;
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    return reading;
}

// Raw 10-bit ADC reading from internal bandgap voltage ref.
// Blocking read.

int _M328P_ADC::readInternalReference()
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX = ADMUX;
    ADCSRA &= 0x97;        // turn off ADATE and ADIE, leave prescale bits
    ADMUX  &= 0xf0;        // reset source bits. leave reference bits.
    ADMUX  |= 0xce;        // turn off ADLAR, source 14 = internal ref.
    _delay_us(70);         // wait for bandgap reference to stabilise
    ADCSRA |= (1<<ADSC);   // start conversion
    loop_until_bit_is_clear(ADCSRA, ADSC);
    int reading = (int)ADC;
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    return reading;
}

// Raw ADC reading from internal temperature sensor. (around 220?)

int _M328P_ADC::readTempSensor()
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX = ADMUX;
    ADCSRA &= 0x97;     // turn off ADATE and ADIE, leave prescale bits
    ADMUX = 0xc8;       // bits 7,6 = internal ref., source 8 = Temp sensor.
    _delay_us(70);      // wait for internal reference to stabilise
    ADCSRA |= (1<<ADSC);   // start conversion
    loop_until_bit_is_clear(ADCSRA, ADSC);
    int reading = (int)ADC;
    ADMUX = oldADMUX; ADCSRA = oldADCSRA; // reset reference and selected pin.
    return reading;
}

// Get voltage at AVCC (ATmega's battery voltage) as a floating point number.
// Unit is volts.

float _M328P_ADC::getSupplyVoltage()
{
    uint8_t oldADCSRA = ADCSRA, oldADMUX = ADMUX;
    referenceDefault();
    singleReadingMode();
    int rdg = readInternalReference();
    ADMUX = oldADMUX; ADCSRA = oldADCSRA;
    if (rdg == 0) return 0;
    return (bandgapV * 1023.0 / (float)rdg);
}

struct _M328P_ADC InternalADC;