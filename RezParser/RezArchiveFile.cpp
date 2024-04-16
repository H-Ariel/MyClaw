#include "libwap.h"


#define DIRECTORY_SEPARATOR '/'



template<typename T>
static inline void FileRead(T& dest, ifstream* file)
{
	file->read((char*)&dest, sizeof(T));
}

static inline vector<string> splitPath(const string& input)
{
	string token;
	vector<string> tokensVector;
	for (stringstream ss(input); getline(ss, token, DIRECTORY_SEPARATOR); tokensVector.push_back(token));
	return tokensVector;
}

// change PID filenames to number (e.g. "FRAME000" -> "000")
static inline string fixFileName(const string& filename)
{
	size_t i = 0;
	while (!isdigit(filename[i]) && i < filename.length()) i += 1;
	if (i < filename.length()) // found a number in filename
	{
		int num = stoi(filename.substr(i));
		char str[4];
		sprintf(str, "%03d", num);
		return str;
	}
	return filename;
}


RezFile::RezFile()
	: size(0), offset(0), parent(nullptr), owner(nullptr)
{
	memset(extension, 0, sizeof(extension));
}
RezFile::~RezFile()
{
	auto it = owner->rezFilesCache.find(this);
	if (it != owner->rezFilesCache.end())
		owner->rezFilesCache.erase(it);
}
vector<uint8_t> RezFile::getFileData() const
{
	if (owner == nullptr)
		return vector<uint8_t>();

	if (owner->rezFilesCache.count(this) == 0)
	{
		owner->rezFilesCache[this] = vector<uint8_t>(size);
		owner->fileStream->seekg(offset, ios::beg);
		owner->fileStream->read((char*)owner->rezFilesCache[this].data(), size);
	}

	return owner->rezFilesCache[this];
}
shared_ptr<BufferReader> RezFile::getBufferReader() const
{
	owner->fileStream->seekg(offset);
	return make_shared<BufferReader>(owner->fileStream, size);
}
string RezFile::getFullPath() const
{
	string dirPath;
	for (RezDirectory* parentDir = parent; parentDir != nullptr; parentDir = parentDir->parent)
		dirPath = parentDir->name + DIRECTORY_SEPARATOR + dirPath;
	return fixPath(dirPath + name + '.' + extension);
}



RezDirectory::RezDirectory()
	: parent(nullptr) {}
RezDirectory::~RezDirectory()
{
	for (auto& i : rezDirectories) delete i.second;
	for (auto& i : rezFiles) delete i.second;
}
RezFile* RezDirectory::getFile(const string& rezFilePath)
{
	string path = fixPath(rezFilePath);
	size_t fileNamePos = path.find_last_of(DIRECTORY_SEPARATOR);
	if (fileNamePos == string::npos)
		return getChildFile(rezFilePath);

	string fullFileName = path.substr(fileNamePos + 1);
	string directoryPath = path.substr(0, fileNamePos);

	RezDirectory* searchedFileDirectory = getDirectory(directoryPath.c_str());
	if (searchedFileDirectory == nullptr)
		throw Exception("Failed to find directory: " + directoryPath);

	return searchedFileDirectory->getChildFile(fullFileName);
}
RezDirectory* RezDirectory::getDirectory(const string& rezDirectoryPath)
{
	vector<string> directoriesTokens = splitPath(fixPath(rezDirectoryPath));
	if (directoriesTokens.empty())
		return this;

	RezDirectory* curr = this;
	for (const string& dirName : directoriesTokens)
	{
		auto i = curr->rezDirectories.find(dirName);
		if (i == curr->rezDirectories.end())
			throw Exception("Failed to find directory: " + dirName);
		curr = i->second;
	}

	return curr;
}
RezFile* RezDirectory::getChildFile(const string& fileName)
{
	auto i = rezFiles.find(fileName);
	if (i == rezFiles.end())
		throw Exception("Failed to find file: " + fileName);
	return i->second;
}
string RezDirectory::getFullPath() const
{
	string dirPath;
	for (RezDirectory* parentDir = parent; parentDir != nullptr; parentDir = parentDir->parent)
		dirPath = parentDir->name + DIRECTORY_SEPARATOR + dirPath;
	return dirPath + name;
}



RezArchive::RezArchive(const string& rezFilePath)
{
	char header[127];
	uint32_t version;

	memset(header, 0, sizeof(header));

	fileStream = DBG_NEW ifstream(rezFilePath, ifstream::binary);
	if (!fileStream->is_open())
	{
		delete fileStream;
		fileStream = nullptr;
		throw Exception("Failed to open REZ file: " + rezFilePath);
	}

	rootDirectory = DBG_NEW RezDirectory;

	fileStream->read(header, 127);
	FileRead(version, fileStream);
	uint32_t dirOffset, dirSize;
	FileRead(dirOffset, fileStream);
	FileRead(dirSize, fileStream);

	// Checksum, offset to last archive + its node size should == file size
	uint32_t expectedRezArchiveSize = dirOffset + dirSize;
	// Get actual size of file we opened
	fileStream->seekg(0, ios::end);
	streamoff actualLoadedFileSize = fileStream->tellg();
	// If this check fails, we did not load valid REZ file
	if (expectedRezArchiveSize != actualLoadedFileSize)
	{
		throw Exception("Failed to load REZ file: " + rezFilePath);
	}

	readDirectory(rootDirectory, dirOffset, dirSize);
}
RezArchive::~RezArchive()
{
	delete fileStream;
	delete rootDirectory;
}
RezFile* RezArchive::getFile(const string& rezFilePath)
{
	return rootDirectory->getFile(rezFilePath);
}
RezDirectory* RezArchive::getDirectory(const string& rezDirectoryPath)
{
	if (rezDirectoryPath.empty())
		return rootDirectory;
	return rootDirectory->getDirectory(rezDirectoryPath);
}
void RezArchive::readDirectory(RezDirectory* parent, int32_t dirOffset, int32_t dirSize)
{
	// Keeps track of how many more bytes need to be read in this directory - it
	// specifies how much more of files/directories are remaining to be read
	int32_t remainingBytes = dirSize;
	uint32_t currentOffset = dirOffset;
	uint32_t bytesRead, isDirectoryFlag;
	uint32_t size, offset;
	char c;

	while (remainingBytes > 0)
	{
		bytesRead = 0;

		// Determine whether next element is file or another directory
		fileStream->seekg(currentOffset, ios::beg);
		FileRead(isDirectoryFlag, fileStream);

		if (isDirectoryFlag)
		{
			// Create new rez directory
			RezDirectory* newDir = DBG_NEW RezDirectory;
			newDir->parent = parent;

			FileRead(offset, fileStream);
			FileRead(size, fileStream);
			fileStream->ignore(4); // date and time

			for (; (c = fileStream->get()) != 0; newDir->name += c);

			bytesRead = uint32_t(17 + newDir->name.length());

			readDirectory(newDir, offset, size);

			parent->rezDirectories[newDir->name] = newDir;
		}
		else
		{
			// Create new rez file
			RezFile* newFile = DBG_NEW RezFile;
			newFile->parent = parent;
			newFile->owner = this;

			FileRead(newFile->offset, fileStream);
			FileRead(newFile->size, fileStream);
			fileStream->ignore(8); // date and time, file id
			fileStream->read(newFile->extension, 4); // extension in reverse order
			reverse(newFile->extension, newFile->extension + strlen(newFile->extension));
			fileStream->ignore(4); // unknown

			for (; (c = fileStream->get()) != 0; newFile->name += c); // Read file name. File name is terminated by null character

			c = fileStream->get(); // Random NULL char

			bytesRead = uint32_t(30 + newFile->name.length());

			if (strcmp(newFile->extension, "PID") == 0 || strcmp(newFile->extension, "PCX") == 0)
				newFile->name = fixFileName(newFile->name);

			parent->rezFiles[newFile->name + '.' + newFile->extension] = newFile;
		}

		remainingBytes -= bytesRead;
		currentOffset += bytesRead;
	}
}
