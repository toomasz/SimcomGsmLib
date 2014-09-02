
#ifndef SEQUENCEDETECTOR_H_
#define SEQUENCEDETECTOR_H_

#include <inttypes.h>
#include <Arduino.h>

class SequenceDetector
{
public:
	const char *sequence;
	uint8_t state;
	uint8_t length;
	SequenceDetector(const char  *sequence);
	bool NextChar(char c);
};
#endif
