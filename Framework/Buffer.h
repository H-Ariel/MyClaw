#pragma once

#include "Framework.h"
#include "DynamicArray.hpp"


class Buffer
{
public:
	Buffer();
	Buffer(const DynamicArray<uint8_t>& buffer);
	Buffer(const void* data, size_t size, bool alloc = true);
	Buffer(ifstream* ifs, size_t size);
	Buffer(const Buffer& buf);
	~Buffer();

	Buffer& operator=(const Buffer& buf);

	void skip(int64_t n); // skip `n` bytes. if `n<0` it goes back.
	void setIndex(int64_t newIdx);
	int64_t getIndex() const { return _idx; }

	const uint8_t* getCData() const { return _data; }
	DynamicArray<uint8_t> getData() const { return DynamicArray<uint8_t>(_data, _size); }
	size_t getSize() const { return _size; }


	// read methods

	template <class T>
	void read(T& t);
	template <class T>
	T read();

	string readString(size_t len);
	string readString(); // read null-terminated string
	DynamicArray<uint8_t> readBytes(size_t n, bool readAnyway = false); // read `n` bytes. if `readAnyway` is `false`, it will throw exception if it can't read `n` bytes.


	// write methods

	template <class T>
	void write(const T& value);
	template <class T>
	void write(const T data[], size_t len);


protected:
	void extend(size_t length); // add `length` bytes to `_data`

	uint8_t* _data;
	size_t _size;
	size_t _idx;
	bool _allocated;
};


template <class T>
void Buffer::read(T& t)
{
	if (_idx + sizeof(t) > _size)
		throw Exception("unable to read data (reached to end of buffer)");

	memcpy(&t, _data + _idx, sizeof(t));
	_idx += sizeof(t);
}

template<class T>
inline T Buffer::read()
{
	T t; read(t);
	return t;
}


template <class T>
void Buffer::write(const T& value)
{
	while (_idx + sizeof(T) >= _size + sizeof(T))
	{
		extend(1024);
	}
	memcpy(_data + _idx, &value, sizeof(T));
	_idx += sizeof(T);
}

template <class T>
void Buffer::write(const T data[], size_t len)
{
	for (size_t i = 0; i < len; write(data[i++]));
}
