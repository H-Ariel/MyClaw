#include "RezArchive.h"


#define DIRECTORY_SEPARATOR '/'


template<class T>
inline void fileRead(ifstream& file, T& t)
{
	file.read((char*)&t, sizeof(t));
}
template<class T, class ... Args>
inline void fileRead(ifstream& file, T& t, Args&...args)
{
	fileRead(file, t);
	fileRead(file, args...);
}


RezFile::RezFile(uint32_t size, uint32_t offset, const string& name, char extension[4], RezDirectory* parent, ifstream* ownerFile)
	: name(name), _parent(parent), _ownerFile(ownerFile), _size(size), _offset(offset), extension("")
{
	memcpy((char*)this->extension, extension, sizeof(this->extension));
}
vector<uint8_t> RezFile::getData() const
{
	vector<uint8_t> fileData(_size);
	_ownerFile->seekg(_offset, ios::beg);
	_ownerFile->read((char*)fileData.data(), _size);
	return fileData;
}
shared_ptr<BufferReader> RezFile::getBufferReader() const
{
	_ownerFile->seekg(_offset, ios::beg);
	return allocNewSharedPtr<BufferReader>(_ownerFile, _size);
}
string RezFile::getFullPath() const
{
	string dirPath;
	for (RezDirectory* dir = _parent; dir != nullptr; dirPath = dir->_name + DIRECTORY_SEPARATOR + dirPath, dir = dir->_parent);
	return dirPath.substr(1) + name + '.' + extension; // ignore the first character ('/')
}
bool RezFile::isPidFile() const
{
	return !memcmp(extension, "PID", 3);
}


RezDirectory::RezDirectory(uint32_t size, const string& name, RezDirectory* parent)
	: _size(size), _name(name), _parent(parent)
{
}
const RezFile* RezDirectory::getFile(string filePath) const
{
	auto getChildFile = [](const RezDirectory* srcdir, const string& filename) {
		return srcdir->_files.count(filename) ? srcdir->_files.at(filename).get() : nullptr;
	};

	RezFile* file;
	size_t fileNamePos = filePath.find_last_of(DIRECTORY_SEPARATOR);
	if (fileNamePos == string::npos)
	{
		// in case `filePath` is only filename
		file = getChildFile(this, filePath);
	}
	else
	{
		// try to retrieve file from its directory
		file = getChildFile(getDirectory(filePath.substr(0, fileNamePos)), filePath.substr(fileNamePos + 1));
	}

	if (file == nullptr)
	{
		throw Exception(__FUNCTION__ ": file not found. filePath=" + filePath);
	}
	return file;
}
const RezDirectory* RezDirectory::getDirectory(string dirPath) const
{
	const RezDirectory* currDir = this;
	for (const string& dirname : splitStringIntoTokens(dirPath, DIRECTORY_SEPARATOR))
	{
		if (currDir->_directories.count(dirname))
		{
			currDir = currDir->_directories.at(dirname).get();
		}
		else
		{
			throw Exception(__FUNCTION__ ": directory not found. dirPath=" + dirPath);
		}
	}
	return currDir;
}


RezArchive::RezArchive(string filename)
	: _rezFileStream(filename, ifstream::binary)
{
	if (!_rezFileStream.is_open())
	{
		throw Exception("unable to open \"" + filename + '"');
	}

	uint32_t offset, size;
	_rezFileStream.ignore(131); // 127 header + 4 version
	fileRead(_rezFileStream, offset, size); // read offset of first directory and its size

	// checksum, offset to last archive + its node size should be equal to file size
	_rezFileStream.seekg(0, ios::end); // get actual size of file we opened
	if (offset + size == _rezFileStream.tellg())
	{
		_root.reset(DBG_NEW RezDirectory(size, "", nullptr));
		readRezDirectory(_root, offset);
	}
}
const RezDirectory* RezArchive::getDirectory(string dirPath) const
{
	return _root->getDirectory(dirPath);
}
const RezFile* RezArchive::getFile(string filePath) const
{
	return _root->getFile(filePath);
}
vector<uint8_t> RezArchive::getFileData(string filePath)
{
	return getFile(filePath)->getData();
}
shared_ptr<BufferReader> RezArchive::getFileBufferReader(string filePath)
{
	return getFile(filePath)->getBufferReader();
}

// change PID filenames to number (e.g. "FRAME000" -> "000")
string fixFileName(const string& filename)
{
	size_t i = 0;
	while (!isdigit(filename[i]) && i<filename.length()) i += 1;
	if (i < filename.length())
	{
		int num = stoi(filename.substr(i));
		char str[4];
		sprintf(str, "%03d", num);
		return str;
	}
	return filename;
}

void RezArchive::readRezDirectory(unique_ptr<RezDirectory>& dir, uint32_t dirOffset)
{
	string name;
	uint32_t currentOffset = dirOffset, remainingBytes = dir->_size, bytesRead = 0, isDirectory, offset, size;
	char ext[4] = {};

	while (0 < remainingBytes)
	{
		_rezFileStream.seekg(currentOffset);
		fileRead(_rezFileStream, isDirectory, offset, size);

		if (isDirectory)
		{
			for (_rezFileStream.ignore(4); _rezFileStream.peek() != 0; name += _rezFileStream.get());
			dir->_directories[name].reset(DBG_NEW RezDirectory(size, name, dir.get()));
			readRezDirectory(dir->_directories[name], offset);
			bytesRead = 0;
		}
		else
		{
			_rezFileStream.ignore(8); // date and time + 4 file id
			fileRead(_rezFileStream, ext);
			_strrev(ext);
			for (_rezFileStream.ignore(4); _rezFileStream.peek() != 0; name += _rezFileStream.get());
			_rezFileStream.ignore(1); // another null

			string newName = name;
			if (strcmp(ext, "PID") == 0)
				newName = fixFileName(name);

			dir->_files[newName + '.' + ext].reset(DBG_NEW RezFile(size, offset, newName, ext, dir.get(), &_rezFileStream));
			bytesRead = 13;
		}

		bytesRead += 17 + (uint32_t)name.length();
		remainingBytes -= bytesRead;
		currentOffset += bytesRead;
		name.clear();
	}
}
