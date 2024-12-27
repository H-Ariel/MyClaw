#pragma once

#include "Framework.h"
#include "DynamicArray.hpp"


/// <summary>
/// Represents a buffer of raw data that supports reading and writing operations.
/// </summary>
class Buffer
{
public:
	/// <summary>
	/// Default constructor that initializes an empty buffer.
	/// </summary>
	Buffer();

	/// <summary>
	/// Constructs a buffer from a given DynamicArray.
	/// </summary>
	/// <param name="buffer">The data to initialize the buffer with.</param>
	Buffer(const DynamicArray<uint8_t>& buffer);

	/// <summary>
	/// Constructs a buffer from raw data, allocating memory if needed.
	/// </summary>
	/// <param name="data">Pointer to the raw data.</param>
	/// <param name="size">The size of the data in bytes.</param>
	/// <param name="alloc">Flag indicating whether to allocate memory for the buffer (default is true).</param>
	Buffer(const void* data, size_t size, bool alloc = true);

	/// <summary>
	/// Constructs a buffer from a file stream with the specified size.
	/// </summary>
	/// <param name="ifs">Pointer to the input file stream.</param>
	/// <param name="size">The size of the data to read from the file.</param>
	Buffer(ifstream* ifs, size_t size);

	/// <summary>
	/// Copy constructor that creates a buffer by copying another buffer.
	/// </summary>
	/// <param name="buf">The buffer to copy.</param>
	Buffer(const Buffer& buf);

	/// <summary>
	/// Destructor that releases the allocated memory.
	/// </summary>
	~Buffer();

	/// <summary>
	/// Assignment operator that copies the contents of another buffer.
	/// </summary>
	/// <param name="buf">The buffer to copy from.</param>
	/// <returns>The current buffer instance with copied contents.</returns>
	Buffer& operator=(const Buffer& buf);

	/// <summary>
	/// Skips `n` bytes in the buffer. If `n` is negative, it moves backwards.
	/// </summary>
	/// <param name="n">The number of bytes to skip. Can be negative to move backwards.</param>
	void skip(int64_t n);

	/// <summary>
	/// Sets the current index to a new value.
	/// </summary>
	/// <param name="newIdx">The new index to set.</param>
	void setIndex(int64_t newIdx);

	/// <summary>
	/// Gets the current index in the buffer.
	/// </summary>
	/// <returns>The current index.</returns>
	int64_t getIndex() const { return _idx; }

	/// <summary>
	/// Retrieves the raw data pointer of the buffer.
	/// </summary>
	/// <returns>A pointer to the buffer's data.</returns>
	const uint8_t* getCData() const { return _data; }

	/// <summary>
	/// Retrieves the data as a DynamicArray.
	/// </summary>
	/// <returns>A DynamicArray containing the buffer's data.</returns>
	DynamicArray<uint8_t> getData() const { return DynamicArray<uint8_t>(_data, _size); }

	/// <summary>
	/// Retrieves the size of the buffer.
	/// </summary>
	/// <returns>The size of the buffer in bytes.</returns>
	size_t getSize() const { return _size; }


	// Reading methods

	/// <summary>
	/// Reads an object of type T from the buffer.
	/// </summary>
	/// <param name="t">The object to read the data into.</param>
	template <class T>
	void read(T& t);

	/// <summary>
	/// Reads an object of type T from the buffer and returns it.
	/// </summary>
	/// <returns>The object read from the buffer.</returns>
	template <class T>
	T read();

	/// <summary>
	/// Reads a string of a specified length from the buffer.
	/// </summary>
	/// <param name="len">The length of the string to read.</param>
	/// <returns>The string read from the buffer.</returns>
	string readString(size_t len);

	/// <summary>
	/// Reads a null-terminated string from the buffer.
	/// </summary>
	/// <returns>The null-terminated string read from the buffer.</returns>
	string readString();

	/// <summary>
	/// Reads a specified number of bytes from the buffer.
	/// </summary>
	/// <param name="n">The number of bytes to read.</param>
	/// <param name="readAnyway">If true, will attempt to read even if there are insufficient bytes.</param>
	/// <returns>A DynamicArray containing the bytes read from the buffer.</returns>
	DynamicArray<uint8_t> readBytes(size_t n, bool readAnyway = false);


	// Writing methods

	/// <summary>
	/// Writes an object of type T to the buffer.
	/// </summary>
	/// <param name="value">The value to write to the buffer.</param>
	template <class T>
	void write(const T& value);

	/// <summary>
	/// Writes an array of objects to the buffer.
	/// </summary>
	/// <param name="data">The array of data to write to the buffer.</param>
	/// <param name="len">The number of elements in the array.</param>
	template <class T>
	void write(const T data[], size_t len);


protected:
	/// <summary>
	/// Extends the buffer by the specified length.
	/// </summary>
	/// <param name="length">The number of bytes to add to the buffer.</param>
	void extend(size_t length);

	/// <summary>
	/// The raw data stored in the buffer.
	/// </summary>
	uint8_t* _data;

	/// <summary>
	/// The size of the buffer in bytes.
	/// </summary>
	size_t _size;

	/// <summary>
	/// The current index in the buffer.
	/// </summary>
	size_t _idx;

	/// <summary>
	/// A flag indicating whether the buffer was allocated dynamically.
	/// </summary>
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
