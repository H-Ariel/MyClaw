#pragma once

#include "framework.h"


class BaseBuffer
{
public:
	BaseBuffer() : _data(nullptr), _size(0), _idx(0), _allocated(false) {}
	BaseBuffer(const vector<uint8_t>& buffer) : BaseBuffer(buffer.data(), buffer.size()) {}
	BaseBuffer(const void* data, size_t size, bool alloc = true)
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

	virtual ~BaseBuffer()
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

	void setIndex(int64_t newIdx)
	{
		_idx = newIdx;
	}

	int64_t getIndex() const
	{
		return _idx;
	}

	// skip `n` bytes. if `n<0` it goes back.
	void skip(int64_t n)
	{
		if (n < 0 && _idx + n < 0)
		{
			throw Exception(__FUNCTION__ ": invalid index");
		}
		setIndex(_idx + n);
	}

	const uint8_t* const getCData() const
	{
		return _data;
	}
	vector<uint8_t> getData() const
	{
		return vector<uint8_t>(_data, _data + _size);
	}
	size_t getSize() const
	{
		return _size;
	}


protected:
	uint8_t* _data;
	size_t _size;
	size_t _idx;
	bool _allocated;
};
