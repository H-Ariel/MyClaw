#include "BaseBuffer.h"


BaseBuffer::BaseBuffer()
	: _data(nullptr), _size(0), _idx(0), _allocated(false) 
{
}
BaseBuffer::BaseBuffer(const vector<uint8_t>& buffer) 
	: BaseBuffer(buffer.data(), buffer.size()) 
{
}
BaseBuffer::BaseBuffer(const void* data, size_t size, bool alloc)
	: _idx(0), _allocated(alloc), _size(size)
{
	if (alloc)
	{
		_data = DBG_NEW uint8_t[size];
		memcpy(_data, data, size);
	}
	else
	{
		_data = (uint8_t*)data;
	}
}
BaseBuffer::BaseBuffer(const BaseBuffer& buf)
	: _idx(0), _allocated(buf._allocated), _size(buf._size)
{
	if (_allocated)
	{
		_data = DBG_NEW uint8_t[_size];
		memcpy(_data, buf._data, _size);
	}
	else
	{
		_data = buf._data;
	}
}

BaseBuffer::~BaseBuffer()
{
	if (_data)
	{
		if (_allocated)
		{
			delete[] _data;
		}
		_data = nullptr;
		_size = 0;
	}
}

BaseBuffer& BaseBuffer::operator=(const BaseBuffer& buf)
{
	if (&buf == this)
	{
		_idx = 0;
		_allocated = buf._allocated;
		_size = buf._size;

		if (_allocated)
		{
			delete[] _data;
			_data = DBG_NEW uint8_t[_size];
			memcpy(_data, buf._data, _size);
		}
		else
		{
			_data = buf._data;
		}
	}

	return *this;
}


void BaseBuffer::skip(int64_t n)
{
	if (n < 0 && _idx + n < 0)
	{
		throw Exception("invalid buffer index");
	}
	setIndex(_idx + n);
}

void BaseBuffer::setIndex(int64_t newIdx)
{
	if (newIdx < 0) throw Exception(__FUNCTION__ ": newIdx < 0");
	_idx = (size_t)newIdx;
}
