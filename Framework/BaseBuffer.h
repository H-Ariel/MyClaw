#pragma once

#include "Framework.h"
#include "DynamicArray.hpp"


// TODO: combine BufferReader and BufferWriter to this class ? (NOTE: these classes same to iostream...)
class BaseBuffer
{
public:
	BaseBuffer();
	BaseBuffer(const DynamicArray<uint8_t>& buffer);
	BaseBuffer(const void* data, size_t size, bool alloc = true);
	BaseBuffer(const BaseBuffer& buf);
	virtual ~BaseBuffer();

	BaseBuffer& operator=(const BaseBuffer& buf);

	void skip(int64_t n); // skip `n` bytes. if `n<0` it goes back.
	void setIndex(int64_t newIdx);
	int64_t getIndex() const { return _idx; }
	const uint8_t* getCData() const { return _data; }
	DynamicArray<uint8_t> getData() const { return DynamicArray<uint8_t>(_data, _size); }
	size_t getSize() const { return _size; }


protected:
	uint8_t* _data;
	size_t _size;
	size_t _idx;
	bool _allocated;
};
