#pragma once

#include "Framework/BufferReader.h"


class RezArchive;
class RezDirectory;


class RezFile
{
public:
	RezFile(RezDirectory* parent, ifstream* ownerFileStream);

	DynamicArray<uint8_t> getFileData() const;
	shared_ptr<BufferReader> getBufferReader() const;
	string getFullPath() const;


	string name;
	char extension[4];

private:
	const RezDirectory* parent;
	ifstream* ownerFileStream;
	const uint32_t size;
	const uint32_t offset;


	friend class RezArchive;
	friend class RezDirectory;
};

class RezDirectory
{
public:
	RezDirectory(RezDirectory* parent);
	~RezDirectory();

	RezFile* getFile(const string& rezFilePath);
	RezDirectory* getDirectory(const string& rezDirectoryPath);
	string getFullPath() const;


	string name;
	map<string, RezDirectory*> rezDirectories; // [name]=RezDirectory
	map<string, RezFile*> rezFiles; // [name.ext]=RezFile
	
private:
	RezFile* getChildFile(const string& fileName);


	const RezDirectory* parent;


	friend class RezFile;
	friend class RezArchive;
};


class RezArchive
{
public:
	RezArchive(const string& rezFilePath);
	~RezArchive();

	RezFile* getFile(const string& rezFilePath);
	RezDirectory* getDirectory(const string& rezDirectoryPath);

private:
	void readDirectory(RezDirectory* rezDirectory, int32_t dirOffset, int32_t dirSize);


	RezDirectory* rootDirectory;
	ifstream* fileStream;


	friend class RezFile;
};
