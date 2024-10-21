#include "Buffer.h"


Buffer::Buffer()
	: _idx(0), _allocated(false), _size(0), _data(nullptr)
{
}
Buffer::Buffer(const DynamicArray<uint8_t>& buffer)
	: Buffer(buffer.data(), buffer.size())
{
}
Buffer::Buffer(const void* data, size_t size, bool alloc)
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
Buffer::Buffer(ifstream* ifs, size_t size)
	: _idx(0), _allocated(true), _size(size)
{
	_data = DBG_NEW uint8_t[size];
	ifs->read((char*)_data, size);
}
Buffer::Buffer(const Buffer& buf)
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

Buffer::~Buffer()
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

Buffer& Buffer::operator=(const Buffer& buf)
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


void Buffer::skip(int64_t n)
{
	if (n < 0 && _idx + n < 0)
		throw Exception("invalid buffer index");
	setIndex(_idx + n);
}

void Buffer::setIndex(int64_t newIdx)
{
	if (newIdx < 0)
		throw Exception("invalid buffer index");
	_idx = (size_t)newIdx;
}


string Buffer::readString(size_t len)
{
	if (_idx + len > _size)
		throw Exception("unable to read data (reached to end of buffer)");

	char* tmp = DBG_NEW char[len + 1];
	memcpy(tmp, _data + _idx, len);
	tmp[len] = 0;
	_idx += len;
	string str(tmp);
	delete[] tmp;
	return str;
}
string Buffer::readString()
{
	string str;
	char c;
	for (read(c); c != 0; read(c))
		str += c;
	return str;
}

DynamicArray<uint8_t> Buffer::readBytes(size_t n, bool readAnyway)
{
	if (_idx + n > _size)
	{
		if (readAnyway)
		{
			LOG("[Warning] unable to read data (reached to end of buffer)\n");
			n = _size - _idx;
		}
		else
			throw Exception("unable to read data (reached to end of buffer)");
	}

	DynamicArray<uint8_t> vec(_data + _idx, n);
	_idx += n;
	return vec;
}


void Buffer::extend(size_t length)
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
