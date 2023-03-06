#pragma once

#include "BaseBuffer.hpp"


class BufferReader : public BaseBuffer
{
public:
	BufferReader(const vector<uint8_t>& buffer) : BaseBuffer(buffer) {}
	BufferReader(const void* data, size_t size, bool alloc = true) : BaseBuffer(data, size, alloc) {}
	BufferReader(ifstream* ifs, size_t size) // initialize from existing file. make sure you set the offset before call this method.
	{
		_size = size;
		_data = DBG_NEW uint8_t[size];
		ifs->read((char*)_data, size);
		_allocated = true;
	}

	bool isEOF()
	{
		return _idx > _size;
	}

	template <class T>
	void read(T& t)
	{
		if (_idx + sizeof(t) > _size)
			throw Exception("unable to read data (reached to end of buffer)");

		memcpy(&t, _data + _idx, sizeof(t));
		_idx += sizeof(t);
	}

	uint8_t readByte()
	{
		uint8_t u;
		read(u);
		return u;
	}

	string ReadString(size_t len)
	{
		if (_idx + len > _size)
			throw Exception("unable to read data (reached to end of buffer)");

		char* pStr = DBG_NEW char[len + 1];
		memcpy(pStr, _data + _idx, len);
		pStr[len] = 0;
		_idx += len;
		string str(pStr);
		delete[] pStr;
		return str;
	}
	string ReadNullTerminatedString()
	{
		string str;
		char c;
		for (read(c); c != 0; read(c))
			str += c;
		return str;
	}

	// read `n` bytes
	vector<uint8_t> ReadVector(size_t n)
	{
		if (_idx + n > _size)
			throw Exception("unable to read data (reached to end of buffer)");

		vector<uint8_t> vec(n);
		memcpy(vec.data(), _data + _idx, n);
		_idx += n;
		return vec;
	}
};
