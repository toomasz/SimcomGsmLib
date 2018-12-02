#include "SequenceDetector.h"


SequenceDetector::SequenceDetector(const char* sequence):_sequence(sequence)
{
	state = 0;
	length = strlen(sequence);
}
/* returns true of sequence is detected */
bool SequenceDetector::NextChar(char c)
{
	const char cByte = _sequence[state];
	if (cByte == c)
	{
		state++;
	}
	else
	{
		state = 0;
		if (c == _sequence[state])
		{
			state++;
		}
	}
	if (state == length)
	{
		state = 0;
		return true;
	}
	return false;
}
