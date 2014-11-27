#include "SequenceDetector.h"


SequenceDetector::SequenceDetector(const char *sequence):sequence(sequence)
{
	state = 0;
	length = strlen_P(sequence);
}
/* returns true of sequence is detected */
bool SequenceDetector::NextChar(char c)
{
	char cByte = pgm_read_byte(&sequence[state]);
	if (cByte == c)
		state++;
	else
	{
		state = 0;
		if (c == pgm_read_byte(&sequence[state]))
			state++;
	}
	if (state == length)
	{
		state = 0;
		return true;
	}
	return false;
}
