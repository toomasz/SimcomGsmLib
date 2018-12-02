
#ifndef SEQUENCEDETECTOR_H_
#define SEQUENCEDETECTOR_H_

#include <inttypes.h>
#include <WString.h>

class SequenceDetector
{
public:
	const char* _sequence;
	uint8_t state;
	uint8_t length;
	SequenceDetector(const char * sequence);
	bool NextChar(char c);
};
#endif
