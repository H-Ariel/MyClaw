#pragma once

// class of dynamic array (simple than std::vector)
template <class T>
class DynamicArray {
public:
	DynamicArray() : _size(0), arr(nullptr) {}
	DynamicArray(size_t size) : _size(size) { arr = DBG_NEW T[size]; }
	DynamicArray(size_t size, T defVal) : _size(size) {
		arr = DBG_NEW T[size];
		for (size_t i = 0; i < size; arr[i++] = defVal);
	}
	DynamicArray(const T* data, size_t size) : _size(size) {
		arr = DBG_NEW T[size];
		memcpy(arr, data, _size * sizeof(T));
	}
	DynamicArray(std::initializer_list<T> data) : _size(data.size()){
		arr = DBG_NEW T[_size];
		memcpy(arr, data.begin(), _size * sizeof(T));
	}
	DynamicArray(const DynamicArray& other) : _size(other._size) {
		arr = DBG_NEW T[_size];
		memcpy(arr, other.arr, _size * sizeof(T));
	}
	DynamicArray(DynamicArray&& other) : _size(other._size), arr(other.arr) {
		other.arr = nullptr;
	}
	~DynamicArray() { delete[] arr; }

	DynamicArray<T>& operator=(const DynamicArray<T>& other) {
		if (this != &other) {
			delete[] arr;
			_size = other._size;
			arr = DBG_NEW T[_size];
			memcpy(arr, other.arr, _size * sizeof(T));
		}
		return *this;
	}
	DynamicArray<T>& operator=(DynamicArray<T>&& other) noexcept {
		if (this != &other) {
			delete[] arr;
			_size = other._size;
			arr = other.arr;
			other.arr = nullptr;
		}
		return *this;
	}

	size_t size() const { return _size; }
	
	T& operator[](size_t index) { return arr[index]; }
	const T& operator[](size_t index) const { return arr[index]; }

	T* begin() { return arr; }
	const T* begin() const { return arr; }
	T* end() { return arr + _size; }
	const T* end() const { return arr + _size; }

	T* data() { return arr; }
	const T* data() const { return arr; }

	void clear() {
		delete[] arr;
		arr = nullptr;
		_size = 0;
	}

private:
	T* arr;
	size_t _size;
};
