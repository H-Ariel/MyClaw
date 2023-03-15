#pragma once

#include "BaseBuffer.h"


class BufferReader : public BaseBuffer
{
public:
	BufferReader(const vector<uint8_t>& buffer);
	BufferReader(const void* data, size_t size, bool alloc = true);
	BufferReader(ifstream* ifs, size_t size); // initialize from existing file. make sure you set the offset before call this method.

	bool isEOF() const { return _idx > _size; }

	template <class T>
	void read(T& t);
	
	template <class T>
	T read();

	string ReadString(size_t len);
	string ReadNullTerminatedString();

	vector<uint8_t> ReadVector(size_t n); // read `n` bytes
};


template <class T>
void BufferReader::read(T& t)
{
	if (_idx + sizeof(t) > _size)
		throw Exception("unable to read data (reached to end of buffer)");

	memcpy(&t, _data + _idx, sizeof(t));
	_idx += sizeof(t);
}

template<class T>
inline T BufferReader::read()
{
	T t; read(t);
	return t;
}