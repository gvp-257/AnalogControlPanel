#include <Arduino.h>
/*
 Test Reading Scatter 
 (variation in the result between readings of the same thing)

 https://github.com/gvp-257/AnalogControlPanel example.

 Circuit:  see the Test-schematic.png alongside this sketch.
 A resistive voltage divider stabilised with a capacitor, connected
 to analog pin A3. Arrange for the junction to be below 1.1V so that
 the internal reference can be tested also.

 See notes in the Readme.md alongside this sketch.

 The method is:
 First get an "average" reading. Then for 100 readings, see how 
 much each reading differs from the average, and count the number in 
 each group - same, off by +1, off by -1, etc.

 Print the results and calculate a weighted score.

 At the end print the results for all the variables.

 Variables:   Speed (sample rate): 1x, 2x, 4x.
                Voltage reference: Default (supply), Internal Ref.
                   reading method: read(), sleepRead(), free-running.

*/

#include "AnalogControlPanel.h"

#include "SendOnlySerial.h" // Lightweight serial for output only. 
// Has printVar macro. https://github.com/gvp-257/SendOnlySerial



const int numReadMethods = 3;   // read, sleepRead, freeRunning
const int numSpeeds = 3;        // speed1x, speed2x, speed4x
const int numVrefs = 2;         // default, internal.

const bool printDetails = false; // print the counts for each 'bin' in the test?


// Convenience macro to define strings stored in flash memory, labels for numbers:-
#define LABEL(x, y) static const char x[] PROGMEM = y; 

LABEL(defaultReferenceLabel,  "Supply as voltage reference");
LABEL(internalReferenceLabel, "Internal voltage reference ");

const char * referencelabels[numVrefs] = {defaultReferenceLabel, internalReferenceLabel};

LABEL(methodread, "read()\t");
LABEL(methodsleepread, "sleepRd\t");
LABEL(methodfreerun, "freerun\t");

const char * methodlabels[numReadMethods] = {methodread, methodsleepread, methodfreerun};

LABEL(speed1xLabel, "speed1x");
LABEL(speed2xLabel, "speed2x");
LABEL(speed4xLabel, "speed4x");

const char * speedlabels[numSpeeds] = {speed1xLabel, speed2xLabel, speed4xLabel};


// Labels for detail print, counts of each 'bin'.
LABEL(readScatterLabel,      "read() scatter: count by difference from average");
LABEL(sleepReadScatterLabel, "sleepRead() scatter");
LABEL(freeRunScatterLabel,   "free-running read scatter");

const char * methodbinsheading[numReadMethods] =
  {readScatterLabel, sleepReadScatterLabel, freeRunScatterLabel};

LABEL(columnsLabel, "count\t-4\t-3\t-2\t-1\t0\t+1\t+2\t+3\t+4+\n");


// Summary Table labels:
LABEL(summaryheader,"Summary tables of scatter scores (lower is better)");
LABEL(tableheader,"Method-speed \t1x\t2x\t4x")

//==============================================================================
// Testing function: test the selected read method at the selected speed
// print the test details if requested.

int testReadScatter(const int selectedMethod, const int selectedSpeed, const bool printDetail)
{
  // Do lots of reads to see how much they vary from "average".
  // Returns a scatter score from 0 up. Lower is better.

  if (selectedSpeed == 0) {InternalADC.speed1x();}
  if (selectedSpeed == 1) {InternalADC.speed2x();}
  if (selectedSpeed == 2) {InternalADC.speed4x();}

  //First get an "average reading".
  long sum = 0;
  if (selectedMethod == 0) 
  {
    InternalADC.singleReadingMode();
    for (short i = 0; i < 16; i++) {sum +=(long)InternalADC.read();}
  }
  if (selectedMethod == 1)
  {
    InternalADC.singleReadingMode();
    for (short i = 0; i < 16; i++) {sum +=(long)InternalADC.sleepRead();}
  }
  if (selectedMethod == 2)
  {
    InternalADC.freeRunningMode();
    InternalADC.startReading();
    delayMicroseconds(200);
    for (short i = 0; i < 16; i++)
    {
      delayMicroseconds(120); // Allow plenty of time for ADC to get a new reading
      sum +=(long)InternalADC.getLastReading();
    }
  }

  int average = (int)((sum + 8) / 16);  // round the readings: add 0.5 to each.

  // Now 'bin' 100 readings' difference from average
  // The nine bins are:
  // 4 or more below--3 below--2 below--1 below--==average--1 above--2 above--3 above--4 or more
  //       0             1        2        3          4         5        6        7        8

  int bins[9]; for (short i = 0; i < 9; i++) bins[i] = 0;

  short bin; const short middlebin = 4;

  for(short i = 0; i < 100; i++)
  {
    int diff = 0;
    if (selectedMethod == 0) {diff = InternalADC.read() - average;}
    if (selectedMethod == 1) {diff = InternalADC.sleepRead() - average;}
    if (selectedMethod == 2) {diff = InternalADC.getLastReading() - average;}
    if (abs(diff) > 3)
    {
      if (diff < 0) bin = 0;  // less than -3: bin 0
      else bin = 8;           // greater than +3: bin 8
    }
    else {bin = middlebin + diff;}  // bins 1 .. 7
    (bins[bin])++;  // increment count of readings in that bin
    delayMicroseconds(120);
  }

  if (printDetail)
  {
    // Print a line with the bin counts as described above
    SendOnlySerial.printP(methodlabels[selectedMethod]);
    SendOnlySerial.printlnP(speedlabels[selectedSpeed]);
    printVar(sum); printVar(average);
    SendOnlySerial.printlnP(methodbinsheading[selectedMethod]);
    SendOnlySerial.printP(columnsLabel);
    for (bin = 0; bin < 9; bin++)
      {SendOnlySerial.print("\t "); SendOnlySerial.print(bins[bin]);}
    SendOnlySerial.println();
  }
  
  // Calculate the weighted score: sum of (count x abs(diff from av))
  sum = 0;  // re-use sum
  for (short i = 0; i < 9; i++)
  {
    int binScore = abs(i - middlebin) * bins[i];
    sum += binScore;
  }
  if (printDetail)
  {
    SendOnlySerial.print("Score: ");
    SendOnlySerial.println(sum);
    SendOnlySerial.println();
    SendOnlySerial.flush();
  }
  return (int)sum;
}



//==============================================================================


void setup()
{
  SendOnlySerial.begin();

  InternalADC.powerOn(); // Necessary to set up AREF and clock
  InternalADC.usePin(A3);

  int scores[numVrefs][numReadMethods][numSpeeds];
  int currVref, currReadMethod, currSpeed;


  // 1. Using ref = AVCC (supply voltage)
  currVref = 0;
  InternalADC.referenceDefault();
  for (currReadMethod = 0; currReadMethod < numReadMethods; currReadMethod++)
  {
    for (currSpeed = 0; currSpeed < numSpeeds; currSpeed++)
    {
      scores[currVref][currReadMethod][currSpeed] = 
          testReadScatter(currReadMethod, currSpeed, printDetails);
    }
  }

  // readScatterLabel with  Ref = internal bandgap voltage reference
  currVref = 1;
  InternalADC.referenceInternal();
  //Take some readings to get the reference to settle down
  InternalADC.speed1x();
  for (short i = 0; i < 50; i++) InternalADC.read();

  for (currReadMethod = 0; currReadMethod < numReadMethods; currReadMethod++)
  {
    for (currSpeed = 0; currSpeed < numSpeeds; currSpeed++)
    {
      scores[currVref][currReadMethod][currSpeed] = 
          testReadScatter(currReadMethod, currSpeed, printDetails);
    }
  }

  // Print the scores
  SendOnlySerial.printlnP(summaryheader); SendOnlySerial.println();
  // table for default reference then one for internal reference
  //   readmethod  speed1x speed2x speed4x

  for ( currVref = 0; currVref < numVrefs; currVref++)
  {
    SendOnlySerial.printlnP(referencelabels[currVref]);
    SendOnlySerial.printlnP(tableheader);
    for (currReadMethod = 0; currReadMethod < numReadMethods; currReadMethod++)
    {
      SendOnlySerial.printP(methodlabels[currReadMethod]);
      for (currSpeed = 0; currSpeed < numSpeeds; currSpeed++)
      {
        SendOnlySerial.print('\t');
        SendOnlySerial.print(scores[currVref][currReadMethod][currSpeed]);
      }
      SendOnlySerial.println();
    }
    SendOnlySerial.println();
  }
  SendOnlySerial.flush();
}

void loop() {}
