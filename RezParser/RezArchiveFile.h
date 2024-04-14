#pragma once

#include "Framework/Framework.h"
#include "Framework/BufferReader.h"


/***************************************************************/
/********************* REZ FORMAT ******************************/
/***************************************************************/

class RezArchive;
class RezDirectory;


class RezFile
{
public:
	RezFile();
	~RezFile();

	vector<uint8_t> getFileData() const;
	shared_ptr<BufferReader> getBufferReader() const;
	string getFullPath() const;


	string name;
	char extension[4];

private:
	RezDirectory* parent;
	RezArchive* owner;
	uint32_t size;
	uint32_t offset;


	friend class RezArchive;
	friend class RezDirectory;
};

class RezDirectory
{
public:
	RezDirectory();
	~RezDirectory();

	RezFile* getFile(const string& rezFilePath);
	RezDirectory* getDirectory(const string& rezDirectoryPath);
	string getFullPath() const;


	string name;
	map<string, RezDirectory*> rezDirectories; // [name]=RezDirectory
	map<string, RezFile*> rezFiles; // [name.ext]=RezFile
	
private:
	RezFile* getChildFile(const string& fileName);


	RezDirectory* parent;


	friend class RezFile;
	friend class RezArchive;
};


class RezArchive
{
public:
	RezArchive(const string& rezFilePath = "../CLAW.REZ");
	~RezArchive();

	RezFile* getFile(const string& rezFilePath);
	RezDirectory* getDirectory(const string& rezDirectoryPath);

private:
	void readDirectory(RezDirectory* rezDirectory, int32_t dirOffset, int32_t dirSize);


	map<const RezFile*, vector<uint8_t>> rezFilesCache;
	RezDirectory* rootDirectory;
	ifstream* fileStream;


	friend class RezFile;
};
