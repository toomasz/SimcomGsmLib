#ifndef  _CIRCULAR_DATA_BUFFER_H
#define _CIRCULAR_DATA_BUFFER_H

const int DATA_BUFFER_SIZE = 40;

class CircularDataBuffer
{
	char _dataBuffer[DATA_BUFFER_SIZE];
	int dataBufferHead;
	int dataBufferTail;
public:
	CircularDataBuffer();
	void Clear();
	bool DataAvailable();
	int UnwriteDataBuffer();
	void WriteDataBuffer(char c);
	int ReadDataBuffer();

	bool commandBeforeRN; // todo figure out what this is and refactor
};

#endif


