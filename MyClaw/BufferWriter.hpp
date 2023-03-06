#pragma once

#include "BaseBuffer.hpp"


class BufferWriter : public BaseBuffer
{
public:
	BufferWriter() {}

	void setIndex(int64_t newIdx)
	{
		while (newIdx > (int64_t)_size)
		{
			extend(1024);
		}
		BaseBuffer::setIndex(newIdx);
	}

	template <class T>
	void write(const T& value)
	{
		while (_idx + sizeof(T) >= _size + sizeof(T))
		{
			extend(1024);
		}
		memcpy(_data + _idx, &value, sizeof(T));
		_idx += sizeof(T);
	}
	template <class T>
	void write(const T data[], size_t len)
	{
		for (size_t i = 0; i < len; write(data[i++]));
	}


private:
	// add `length` bytes to `_data`
	void extend(size_t length)
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
};
