#include <Arduino.h>
/* 
  Median filter: smooth data by selecting middle value of 3 neighboring values.

  This is not a filter in the normal DSP sense, but a non-linear smoothing
  "trick" to improve raw data quality by removing spikes, single readings that
  are a lot different to their neigbours. These often occur in environments that
  are electrically noisy, for example with big motors switching on and off.

  It potentially changes every reading though, so it is best used with data
  that is expected to change smoothly.

  This just demonstrates the technique. It should be possible to optimise the code
  and to vary it for your application.

  This example does not depend on Analog Control Panel functions.

   GvP-257 2025-09.  Public domain.
*/

// Return the value that isn't the biggest nor the smallest:
// the middle one, the median.
int median3(const int value1, const int value2, const int value3)
{
	int smallestValue, biggestValue;
	unsigned short theSmallestOne, theBiggestOne;
 
	smallestValue = biggestValue = value1;
	theSmallestOne = theBiggestOne = 1;

	if (value2 < smallestValue)
		{smallestValue = value2; theSmallestOne = 2;}
	if (value2 > biggestValue)
		{biggestValue  = value2; theBiggestOne = 2;}

	if (value3 < smallestValue)
		{smallestValue = value3; theSmallestOne = 3;}
	if (value3 > biggestValue)
		{biggestValue  = value3; theBiggestOne = 3;}

	// If all are the same, just return that value. Otherwise,
	// return the one that isn't the smallest and isn't the biggest.

	if (smallestValue == biggestValue) return smallestValue;

	if ((3 != theSmallestOne) && (3 != theBiggestOne)) return value3;
	if ((2 != theSmallestOne) && (2 != theBiggestOne)) return value2;
	return value1;
}

void medianFilter(int * inputData, int * outputData, const size_t numElements)
{
	size_t i, lastElem;
	lastElem = numElements - 1;

	//first element
	outputData[0] 	= median3(inputData[0], inputData[1], inputData[2]);
	//middle elements
	for (i = 1; i < lastElem; i++ )
	{
		outputData[i] = median3(inputData[i-1], inputData[i], inputData[i+1]);
	}
	//last element
	outputData[lastElem] = median3(inputData[lastElem -2], inputData[lastElem - 1],
							  inputData[lastElem]);
}

void printArray(const int * array, const size_t len)
{
	Serial.print(array[0]);
	for (size_t i = 1; i < len ; i++)
	{
		Serial.print('\t');
		Serial.print(array[i]);
	}
	Serial.println('.');
}

void setup()
{
	// A data set with 3 spikes (6, 1003, 17). Most readings are around 130.
	int data[10] = {127, 134, 6, 135, 134, 135, 1003, 137, 126, 17};

	const size_t len = sizeof(data) / sizeof(data[0]);
	
	// Array to hold the filtered data.
	int filteredData[len];

	medianFilter(data, filteredData, len);


	// Print original data and results.
	Serial.begin(9600);
	Serial.print(F("Original data:"));
	printArray(data, len);
	Serial.print(F("Filtered data:"));
	printArray(filteredData, len);

	Serial.println(F(" ------- End of medianfilter example -------"));
	Serial.flush();
	exit(0);

	// Results:-
	// Original data:127	134	  6	    135	  134	  135	  1003	 137 	 126	  17.
	// Filtered data:127	127	  134	  134	  135	  135	  137	   137	 126	  126.
	// ------- End of medianfilter example -------

}

void loop()
{

}