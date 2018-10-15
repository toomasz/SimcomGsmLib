#include "CircularDataBuffer.h"
#include <string.h>

CircularDataBuffer::CircularDataBuffer()
{
	Clear();
}

void CircularDataBuffer::Clear()
{
	dataBufferHead = dataBufferTail = 0;
	memset(_dataBuffer, 0, DATA_BUFFER_SIZE);
}

bool CircularDataBuffer::DataAvailable()
{
	return dataBufferHead != dataBufferTail;
}

int CircularDataBuffer::UnwriteDataBuffer()
{
	if (dataBufferTail == 0)
		dataBufferTail = DATA_BUFFER_SIZE - 1;
	else
		dataBufferTail--;
	return _dataBuffer[dataBufferTail];
}

void CircularDataBuffer::WriteDataBuffer(char c)
{
	int tmp = dataBufferTail + 1;
	if (tmp == DATA_BUFFER_SIZE)
		tmp = 0;
	if (tmp == dataBufferHead)
	{
	//	Log_P(F("Buffer overflow"));
		return;
	}
	//ds.print(F("Written ")); this->PrintDataByte(c);
	_dataBuffer[dataBufferTail] = c;
	dataBufferTail = tmp;
}

int CircularDataBuffer::ReadDataBuffer()
{
	if (dataBufferHead != dataBufferTail)
	{
		int ret = _dataBuffer[dataBufferHead];
		dataBufferHead++;
		if (dataBufferHead == DATA_BUFFER_SIZE)
			dataBufferHead = 0;
		return ret;
	}
	return -1;
}
