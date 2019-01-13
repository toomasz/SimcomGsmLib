#include "SequenceDetector.h"

SequenceDetector::SequenceDetector()
{
	_sequence = nullptr;
	_state = 0;
	_length = 0;
}
SequenceDetector::SequenceDetector(const char* sequence):
	_sequence(sequence),
	_state(0)
{
	_length = strlen(sequence);
}

SequenceDetector::SequenceDetector(const char * sequence, int length):
	_sequence(sequence), 
	_state(0), 
	_length(length)
{
}

void SequenceDetector::SetSequence(const char * sequence, int length)
{
	_sequence = sequence;
	_state = 0;
	_length = length;
}
/* returns true of sequence is detected */
bool SequenceDetector::NextChar(char c)
{
	if (_sequence == nullptr)
	{
		return false;
	}
	const char cByte = _sequence[_state];
	if (cByte == c)
	{
		_state++;
	}
	else
	{
		_state = 0;
		if (c == _sequence[_state])
		{
			_state++;
		}
	}
	if (_state == _length)
	{
		_state = 0;
		return true;
	}
	return false;
}
