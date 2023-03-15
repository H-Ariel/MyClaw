#include "BufferReader.h"


BufferReader::BufferReader(const vector<uint8_t>& buffer) 
	: BaseBuffer(buffer) 
{
}
BufferReader::BufferReader(const void* data, size_t size, bool alloc) 
	: BaseBuffer(data, size, alloc)
{
}
BufferReader::BufferReader(ifstream* ifs, size_t size)
{
	_size = size;
	_data = DBG_NEW uint8_t[size];
	ifs->read((char*)_data, size);
	_allocated = true;
}

string BufferReader::ReadString(size_t len)
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
string BufferReader::ReadNullTerminatedString()
{
	string str;
	char c;
	for (read(c); c != 0; read(c))
		str += c;
	return str;
}

vector<uint8_t> BufferReader::ReadVector(size_t n)
{
	if (_idx + n > _size)
		throw Exception("unable to read data (reached to end of buffer)");

	vector<uint8_t> vec(n);
	memcpy(vec.data(), _data + _idx, n);
	_idx += n;
	return vec;
}
