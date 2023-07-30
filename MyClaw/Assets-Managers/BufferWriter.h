#pragma once

#include "BaseBuffer.h"


class BufferWriter : public BaseBuffer
{
public:
	BufferWriter() = default;

	void setIndex(int64_t newIdx);

	template <class T>
	void write(const T& value);
	template <class T>
	void write(const T data[], size_t len);


private:
	// add `length` bytes to `_data`
	void extend(size_t length);
};


template <class T>
void BufferWriter::write(const T& value)
{
	while (_idx + sizeof(T) >= _size + sizeof(T))
	{
		extend(1024);
	}
	memcpy(_data + _idx, &value, sizeof(T));
	_idx += sizeof(T);
}

template <class T>
void BufferWriter::write(const T data[], size_t len)
{
	for (size_t i = 0; i < len; write(data[i++]));
}
