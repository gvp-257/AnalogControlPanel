#include <Arduino.h>
/*
 Test Reading Scatter (Variation from one read to the next)

 Test the "scatter" in repeated readings of the same pin. 
 First get an "average" reading. Then for 100 readings, see how 
 much each reading differs from the average, and count the number in 
 each group - same, off by +1, off by -1, etc.

 High scatter may indicate a problem with your power supply being unstable.
 Battery power should give good results with very little scatter.

 For this test you need two resistors of 1K or more in series from 5V to ground,
 with the junction connected to pin A3 (or whatever analog pin you would
 like to use).

                    +-------------|Pin A3|
                    |                  
   |5V|---| R1 |----+----| R2 |-----|GND|

*/

#include "AnalogControlPanel.h"

#include "SendOnlySerial.h" // Has printReg, printVal macros.


#define LABEL(x, y) static const char x[] PROGMEM = y; 

LABEL(r9klbl,  "\tRate 9k");
LABEL(r18klbl, "\tRate 18k");
LABEL(r37klbl, "\tRate 37k");

LABEL(intReflbl, "Using internal voltage reference ");
LABEL(dftReflbl, "Using Supply as voltage reference ");

void printRegisters(void)
{
  printReg(ADCSRA);
  printReg(ADCSRB);
  printReg(ADMUX);
  SendOnlySerial.println();
}

void PrintResults(const char* rdglabel, const int rdg)
{
  SendOnlySerial.printP(rdglabel);
  SendOnlySerial.println(rdg);
  SendOnlySerial.println();
}

void PrintTestTime(const char * ratelabel, long time)
{
  SendOnlySerial.printlnP(ratelabel);
  LABEL(timingtext, "Time taken (microseconds): ");
  SendOnlySerial.printP(timingtext);
  SendOnlySerial.println((time));
  SendOnlySerial.flush();
}

void PrintTimedResults(const char* rdglabel, const int rdg, const char* ratelabel, long testtime)
{
  PrintResults(rdglabel, rdg);
  PrintTestTime(ratelabel, testtime);
  SendOnlySerial.flush();
}


void timedtestRead(const char* testlabel, const char* ratelabel)
{
  long timetaken;
  timetaken = micros();
  int reading = InternalADC.read();
  timetaken = micros() - timetaken;
  PrintTimedResults(testlabel, reading, ratelabel, timetaken);
  SendOnlySerial.println();
  SendOnlySerial.flush();
}

void testSample(const char* testlabel, const char* ratelabel)
{
  InternalADC.startReading();
  while (!InternalADC.readingReady());
  int reading = InternalADC.getLastReading();
  PrintResults(testlabel, reading);
  SendOnlySerial.printP(ratelabel);
  SendOnlySerial.flush();
}

void timedtestSample(const char* testlabel, const char* ratelabel)
{
  long timetaken;
  timetaken = micros();
  InternalADC.startReading();
  while (!InternalADC.readingReady());
  int reading = InternalADC.getLastReading();
  timetaken = micros() - timetaken;
  PrintTimedResults(testlabel, reading, ratelabel, timetaken);
  SendOnlySerial.flush();
}

void binstestRead(void)
{
  printRegisters();

  //First get an "average reading"
  long sum = 0;
  for (short i = 0; i < 16; i++) sum +=(long) InternalADC.read();
  int av = (int)((sum + 8) / 16);

  printVar(sum);
  printVar(av);
  SendOnlySerial.flush();


  // Now 'bin' 100 readings' difference from average
  int bins[9];  for (short i = 0; i < 9; i++) bins[i] = 0;

  short bin; 

  const short middlebin = 4;

  for(short i = 0; i < 100; i++)
  {
    _delay_us(100);
    int diff = InternalADC.read() - av;
    if (abs(diff) > 3)
    {
      if (diff < 0) bin = 0;  // less than -3
      else bin = 8;           // greater than +3
    }
    else {bin = middlebin + diff;}

    (bins[bin])++;  // increment count of readings in that bin
  } 
  // Print out the frequency
  // then freq distrib of differences from av:
  LABEL(columns, "\n\ncount\t-4\t-3\t-2\t-1\t0\t+1\t+2\t+3\t+4+\n");
  SendOnlySerial.printP(columns);
  // SendOnlySerial.Flush();
  for (bin = 0; bin < 9; bin++)
    {SendOnlySerial.print('\t'); SendOnlySerial.print(bins[bin]);}

  SendOnlySerial.println(); SendOnlySerial.flush();

}

void sleepReadBinsTest(void)
{
  printRegisters();

  //First get an "average reading"
  long sum = 0;
  for (short i = 0; i < 16; i++) sum +=(long) InternalADC.sleepRead();
  int av = (int)((sum + 8) / 16);

  printVar(sum);
  printVar(av);
  SendOnlySerial.flush();


  // Now 'bin' 100 readings' difference from average
  int bins[9];  for (short i = 0; i < 9; i++) bins[i] = 0;

  short bin; 

  const short middlebin = 4;

  for(short i = 0; i < 100; i++)
  {
    //_delay_us(100);
    int diff = InternalADC.sleepRead() - av;
    if (abs(diff) > 3)
    {
      if (diff < 0) bin = 0;  // less than -3
      else bin = 8;           // greater than +3
    }
    else {bin = middlebin + diff;}

    (bins[bin])++;  // increment count of readings in that bin
  } 
  // Print out the frequency
  // then freq distrib of differences from av:
  LABEL(columns, "\n\ncount\t-4\t-3\t-2\t-1\t0\t+1\t+2\t+3\t+4+\n");
  SendOnlySerial.printP(columns);
  // SendOnlySerial.Flush();
  for (bin = 0; bin < 9; bin++)
    {SendOnlySerial.print('\t'); SendOnlySerial.print(bins[bin]);}

  SendOnlySerial.println(); SendOnlySerial.flush();

}

void setup()
{
  InternalADC.powerOn(); // Necessary to set up AREF and clock
  InternalADC.rate9k();

  SendOnlySerial.begin();


  LABEL(gndlbl, "ReadGround(), value = ");
  LABEL(BGlabel, "Internal Ref value = ");
  LABEL(ana3lbl, "readPin(A3), value = ");

  InternalADC.referenceDefault();
  SendOnlySerial.printlnP(dftReflbl);

  // Test internal ground connection
  SendOnlySerial.printlnP(r9klbl);

  InternalADC.readGround(); // discard first reading
  int reading = InternalADC.readGround();
  PrintResults(gndlbl, reading);

  reading = InternalADC.readInternalReference();
  PrintResults(BGlabel, reading);

  reading = InternalADC.readPin(A3);
  PrintResults(ana3lbl, reading);

  InternalADC.rate37k();
  SendOnlySerial.printlnP(r37klbl);

  reading = InternalADC.readGround();
  PrintResults(gndlbl, reading);

  reading = InternalADC.readInternalReference();
  PrintResults(BGlabel, reading);

  reading = InternalADC.readPin(A3);
  PrintResults(ana3lbl, reading);


  // With Reference internal 1.1V
  InternalADC.referenceInternal();
  SendOnlySerial.printlnP(intReflbl);

  InternalADC.rate9k();
  SendOnlySerial.printlnP(r9klbl);

  InternalADC.readGround(); // discard first reading
  reading = InternalADC.readGround();
  PrintResults(gndlbl, reading);
  reading = InternalADC.readInternalReference();
  PrintResults(BGlabel, reading);
  reading = InternalADC.readPin(A3);
  PrintResults(ana3lbl, reading);

  InternalADC.rate37k();
  SendOnlySerial.printlnP(r37klbl);
  reading = InternalADC.readGround();
  PrintResults(gndlbl, reading);

  reading = InternalADC.readInternalReference();
  PrintResults(BGlabel, reading);

  reading = InternalADC.readPin(A3);
  PrintResults(ana3lbl, reading);


  // Test for variation in the readings with Read
  InternalADC.referenceDefault();
  SendOnlySerial.printP(dftReflbl);

  InternalADC.rate9k();
  SendOnlySerial.printlnP(r9klbl);


  LABEL(Jitter, "\n\nVariation in InternalADC.read() - counts of readings\n");
  SendOnlySerial.printP(Jitter);

  InternalADC.usePin(A3);
  binstestRead();


  // Jitter with  Ref = internal bandgap voltage reference
  InternalADC.referenceInternal();
  SendOnlySerial.printP(intReflbl);

  binstestRead();

  // Test for jitter with sleepRead
  LABEL(SRJitter, "\n\nJitter in InternalADC.sleepRead() readings\n");
  SendOnlySerial.printP(SRJitter);

  sleepReadBinsTest();

  // printRegisters();

  SendOnlySerial.flush();
}

void loop()
{

}