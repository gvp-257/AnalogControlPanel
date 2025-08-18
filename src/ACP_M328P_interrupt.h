#ifndef ACP_M328P_INTERRUPT_H
#define ACP_M328P_INTERRUPT_H

#include <avr/interrupt.h>  // ISR macro ADC_vect interrupt
#ifdef __cplusplus
extern "C"{
#endif
    volatile bool _adcdone;              // 1 byte RAM

    // The default function to call on 'ADC conversion complete' interrupt if enabled
    // void _ADCdefaultISR(void) {_adcdone = true;}

    // Global: pointer to 'ADC conversion complete' ISR function. 2 bytes RAM
    typedef void (*voidfnptr)();
    // volatile voidfnptr _ADCDoneFunc = _ADCdefaultISR;
    volatile voidfnptr _ADCDoneFunc;

    ISR(ADC_vect) {if (_ADCDoneFunc) (*_ADCDoneFunc)();}
#ifdef __cplusplus
};
#endif

#endif