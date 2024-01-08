#include "BufferWriter.h"


void BufferWriter::setIndex(int64_t newIdx)
{
	while (newIdx > (int64_t)_size)
	{
		extend(1024);
	}
	BaseBuffer::setIndex(newIdx);
}

void BufferWriter::extend(size_t length)
{
	uint8_t* newData = DBG_NEW uint8_t[_size + length];
	_allocated = true;
	if (_data)
	{
		memcpy(newData, _data, _size);
		if (_allocated)
		{
			delete[] _data;
		}
	}
	memset(newData + _size, 0, length);
	_data = newData;
	_size += length;
}
