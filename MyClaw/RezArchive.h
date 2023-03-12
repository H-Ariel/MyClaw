#pragma once

#include "BufferReader.h"


class RezArchive;
class RezDirectory;


class RezFile
{
public:
	RezFile(uint32_t size, uint32_t offset, const string& name, char extension[4], RezDirectory* parent, ifstream* ownerFile);

	vector<uint8_t> getData() const;
	shared_ptr<BufferReader> getBufferReader() const;
	string getFullPath() const;
	bool isPidFile() const { return strcmp(extension, "PID") == 0; }

	const string name;
	const char extension[4];

private:
	RezDirectory* const _parent;
	ifstream* const _ownerFile;
	const uint32_t _offset, _size;

	friend class RezArchive;
	friend class RezDirectory;
};


class RezDirectory
{
public:
	RezDirectory(uint32_t size, const string& name, RezDirectory* parent);

	const RezFile* getFile(string filePath) const;
	const RezDirectory* getDirectory(string dirPath) const;


private:
	const string _name;
	map<string, unique_ptr<RezDirectory>> _directories; // [name]=directory
	map<string, unique_ptr<RezFile>> _files; // [name.extension]=file
	RezDirectory* const _parent;
	const uint32_t _size;

	friend class RezArchive;
	friend class RezFile;
	friend class AssetsManager;
	friend class AnimationsManager;
};


class RezArchive
{
public:
	RezArchive(string filename);

	const RezDirectory* getDirectory(string dirPath) const { return _root->getDirectory(dirPath); }
	const RezFile* getFile(string filePath) const { return _root->getFile(filePath); }
	vector<uint8_t> getFileData(string filePath) const { return getFile(filePath)->getData(); }
	shared_ptr<BufferReader> getFileBufferReader(string filePath) const { return getFile(filePath)->getBufferReader(); }

private:
	void readRezDirectory(unique_ptr<RezDirectory>& dir, uint32_t dirOffset);

	unique_ptr<RezDirectory> _root;
	ifstream _rezFileStream;
};
