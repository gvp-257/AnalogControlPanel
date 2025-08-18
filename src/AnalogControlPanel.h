#ifndef ANALOG_CONTROL_PANEL_H
#define ANALOG_CONTROL_PANEL_H

// GvP 2025-08.
// https://github.com/gvp-257/analogcontrolpanel

/*
 * Use the ATmega328P's analog-to-digital coverter more flexibly.
 * For those times when Arduino's `analogRead(pin)` is limiting or annoying.
 *
 * Power on/off, speed up sampling rate, non-blocking samples,
 * automatically sample at precise times (a GPS unit's pulse-per-second pin,
 * say), read internal sensors, estimate supply voltage (useful for
 * battery-powered projects).
 *
 * For the ATmega168/328/P/PA (Arduino Uno, Duemilanove, Pro Mini, Nano).
 *
 * Use Case: Environmental Data Loggers, environmental science instruments.
 *
 */

#ifndef cli
#define cli() __asm__ __volatile__ ("cli" ::: "memory")
#endif
#ifndef sei
#define sei()  __asm__ __volatile__ ("sei" ::: "memory")
#endif

#if defined (__AVR_ATmega328P__)  || defined (__AVR_ATmega168P__) \
 || defined (__AVR_ATmega328)     || defined (__AVR_ATmega_168__) \
 || defined (__AVR_ATmega328PA__) || defined (__AVR_ATmega328PB__)
#include "AnalogControlPanel_M328P.h"

#else // Chip not recognised

#error "Analog Control Panel has not been set up for this type of chip."

#endif

#endif  //ANALOG_CONTROL_PANEL_H