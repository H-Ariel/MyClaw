#include "RezArchive.h"


#define fileRead(t) _rezFileStream.read((char*)&t, sizeof(t));


// split `str` by `delim`
static vector<string> splitStringIntoTokens(string input, char delim)
{
	stringstream sstr(input);
	string tok;
	vector<string> toksList;

	while (getline(sstr, tok, delim))
	{
		if (!tok.empty())
		{
			toksList.push_back(tok);
		}
	}

	return toksList;
}


RezFile::RezFile(uint32_t size, uint32_t offset, const string& name, RezDirectory* parent, ifstream* ownerFile)
	: name(name), _parent(parent), _ownerFile(ownerFile), _size(size), _offset(offset)
{
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
	for (RezDirectory* dir = _parent; dir != nullptr; dirPath = dir->_name + '/' + dirPath, dir = dir->_parent);
	return dirPath + name; // ignore the first character ('/')
}


RezDirectory::RezDirectory(uint32_t size, const string& name, RezDirectory* parent)
	: _size(size), _name(name), _parent(parent)
{
}
const RezFile* RezDirectory::getFile(const string& filePath) const
{
	const RezFile* file = nullptr;

	size_t fileNamePos = filePath.find_last_of('/');
	if (fileNamePos == string::npos) // in case `filePath` is only filename
		file = getChildFile(filePath);
	else // try to retrieve file from its directory
		file = getDirectory(filePath.substr(0, fileNamePos))->getChildFile(filePath.substr(fileNamePos + 1));

	if (file == nullptr)
		throw Exception("file not found. filePath=" + filePath);
	
	return file;
}
const RezDirectory* RezDirectory::getDirectory(const string& dirPath) const
{
	const RezDirectory* currDir = this;
	for (const string& dirname : splitStringIntoTokens(dirPath, '/'))
	{
		if (currDir->_directories.count(dirname))
			currDir = currDir->_directories.at(dirname).get();
		else
			throw Exception("directory not found. dirPath=" + dirPath);
	}
	return currDir;
}
const RezFile* RezDirectory::getChildFile(const string& filename) const
{
	return _files.count(filename) ? _files.at(filename).get() : nullptr;
}

RezArchive::RezArchive(const string& filename)
	: _rezFileStream(filename, ios::binary)
{
	if (!_rezFileStream.is_open())
	{
		throw Exception("unable to open \"" + filename + '"');
	}

	uint32_t offset = 0, size = 0;
	_rezFileStream.ignore(131); // 127 header + 4 version
	fileRead(offset); // read offset of first directory
	fileRead(size); // read size of first directory

	// checksum, offset to last archive + its node size should be equal to file size
	_rezFileStream.seekg(0, ios::end); // get actual size of file we opened
	if (offset + size == _rezFileStream.tellg())
	{
		_root.reset(DBG_NEW RezDirectory(size, "", nullptr));
		readRezDirectory(_root, offset);
	}
}

// change PID filenames to number (e.g. "FRAME000" -> "000")
string fixFileName(const string& filename)
{
	size_t i = 0;
	while (!isdigit(filename[i]) && i < filename.length()) i += 1;
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
	uint32_t currentOffset = dirOffset, remainingBytes = dir->_size;
	uint32_t bytesRead = 0, isDirectory = 0, offset = 0, size = 0;
	char ext[4] = {};

	while (0 < remainingBytes)
	{
		_rezFileStream.seekg(currentOffset);
		fileRead(isDirectory);
		fileRead(offset);
		fileRead(size);

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
			fileRead(ext);
			_strrev(ext);
			for (_rezFileStream.ignore(4); _rezFileStream.peek() != 0; name += _rezFileStream.get());
			_rezFileStream.ignore(1); // another null

			string newName = name;
			if (strcmp(ext, "PID") == 0)
				newName = fixFileName(name);

			newName = newName + '.' + ext;

			dir->_files[newName].reset(DBG_NEW RezFile(size, offset, newName, dir.get(), &_rezFileStream));
			bytesRead = 13;
		}

		bytesRead += 17 + (uint32_t)name.length();
		remainingBytes -= bytesRead;
		currentOffset += bytesRead;
		name.clear();
	}
}
