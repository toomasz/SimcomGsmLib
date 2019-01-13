
#ifndef SEQUENCEDETECTOR_H_
#define SEQUENCEDETECTOR_H_

#include <inttypes.h>
#include <WString.h>

class SequenceDetector
{
	const char* _sequence;
	int _state;
	int _length;
public:	
	SequenceDetector();
	SequenceDetector(const char * sequence);
	SequenceDetector(const char * sequence, int length);
	void SetSequence(const char * sequence, int length);
	bool NextChar(char c);
};
#endif
