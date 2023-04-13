// XMI file parser, based on "libwap" (see at OpenClaw: https://github.com/pjasicek/OpenClaw/blob/master/libwap/XmiFile.cpp)

#include "BufferReader.h"
#include "BufferWriter.h"


// TODO: use BufferWriter and BufferReader
class MemoryBuffer
{
public:
	MemoryBuffer()
		: m_pData(nullptr), m_pPointer(nullptr), m_pEnd(nullptr), m_pBufferEnd(nullptr)
	{
	}

	MemoryBuffer(char* pData, size_t iLength)
	{
		m_pData = m_pPointer = (char*)pData;
		m_pEnd = m_pData + iLength;
		m_pBufferEnd = nullptr;
	}

	~MemoryBuffer()
	{
		if (m_pBufferEnd != nullptr)
			delete[] m_pData;
	}

	vector<uint8_t> takeData()
	{
		vector<uint8_t> res(m_pEnd - m_pData);
		memcpy(res.data(), m_pData, m_pEnd - m_pData);
		delete[] m_pData;
		m_pData = m_pPointer = m_pEnd = m_pBufferEnd = nullptr;
		return res;
	}

	size_t tell() const { return m_pPointer - m_pData; }

	bool seek(size_t position)
	{
		if (m_pData + position > m_pEnd)
		{
			if (!_realloc(position))
				return false;
		}
		m_pPointer = m_pData + position;
		return true;
	}

	bool skip(int distance)
	{
		if (distance < 0)
		{
			if (m_pPointer + distance < m_pData)
				return false;
		}
		return seek(m_pPointer - m_pData + distance);
	}

	bool scanTo(const void* pData, size_t iLength)
	{
		for (; m_pPointer + iLength <= m_pEnd; ++m_pPointer)
		{
			if (memcmp(m_pPointer, pData, iLength) == 0)
				return true;
		}
		return false;
	}

	const char* getPointer() const { return m_pPointer; }

	template <class T>
	bool read(T& value) { return read(&value, 1); }

	template <class T>
	bool read(T* values, size_t count)
	{
		if (m_pPointer + sizeof(T) * count > m_pEnd)
			return false;
		memcpy(values, m_pPointer, sizeof(T) * count);
		m_pPointer += sizeof(T) * count;
		return true;
	}

	unsigned int readBigEndianUInt24()
	{
		uint8_t iByte0, iByte1, iByte2;
		if (read(iByte0) && read(iByte1) && read(iByte2))
			return (((iByte0 << 8) | iByte1) << 8) | iByte2;
		return 0;
	}

	unsigned int readUIntVar()
	{
		unsigned int iValue = 0;
		uint8_t iByte;
		for (int i = 0; i < 4; ++i)
		{
			if (!read(iByte))
				return false;
			iValue = (iValue << 7) | static_cast<unsigned int>(iByte & 0x7F);
			if ((iByte & 0x80) == 0)
				break;
		}
		return iValue;
	}

	template <class T>
	bool write(const T& value) { return write(&value, 1); }

	template <class T>
	bool write(const T* values, size_t count)
	{
		if (!skip(static_cast<int>(sizeof(T) * count)))
			return false;
		memcpy(m_pPointer - sizeof(T) * count, values, sizeof(T) * count);
		return true;
	}

	bool writeBigEndianUInt16(uint16_t iValue) { return write(_byteSwap(iValue)); }

	bool writeBigEndianUInt32(uint32_t iValue) { return write(_byteSwap(iValue)); }

	bool writeUIntVar(unsigned int iValue)
	{
		int iByteCount = 1;
		unsigned int iBuffer = iValue & 0x7F;
		for (; iValue >>= 7; ++iByteCount)
		{
			iBuffer = (iBuffer << 8) | 0x80 | (iValue & 0x7F);
		}
		for (int i = 0; i < iByteCount; ++i)
		{
			uint8_t iByte = iBuffer & 0xFF;
			if (!write(iByte))
				return false;
			iBuffer >>= 8;
		}
		return true;
	}

	bool isEOF() const { return m_pPointer == m_pEnd; }

protected:
	template <class T>
	static T _byteSwap(T value)
	{
		T swapped = 0;
		for (int i = 0; i < static_cast<int>(sizeof(T)) * 8; i += 8)
			swapped |= ((value >> i) & 0xFF) << (sizeof(T) * 8 - 8 - i);
		return swapped;
	}

	bool _realloc(size_t size)
	{
		if (m_pData + size <= m_pBufferEnd)
		{
			m_pEnd = m_pData + size;
			return true;
		}

		char* pNewData = DBG_NEW char[size * 2];
		if (pNewData == nullptr)
			return false;
		size_t iOldLength = m_pEnd - m_pData;
		memcpy(pNewData, m_pData, size > iOldLength ? iOldLength : size);
		m_pPointer = m_pPointer - m_pData + pNewData;
		if (m_pBufferEnd != nullptr)
			delete[] m_pData;
		m_pData = pNewData;
		m_pEnd = pNewData + size;
		m_pBufferEnd = pNewData + size * 2;
		return true;
	}

	char* m_pData, *m_pPointer, *m_pEnd, *m_pBufferEnd;
};

struct MidiToken
{
	int iTime;
	unsigned int iBufferLength;
	const char* pBuffer;
	uint8_t iType, iData;

	bool operator<(const MidiToken& oRight) { return iTime < oRight.iTime; }
};

class MidiTokensList : public vector<MidiToken>
{
public:
	MidiToken* append(int iTime, uint8_t iType)
	{
		push_back(MidiToken());
		MidiToken* pToken = &back();
		pToken->iTime = iTime;
		pToken->iType = iType;
		return pToken;
	}
};

vector<uint8_t> xmiToMidi(vector<uint8_t> xmiFileData)
{
	MemoryBuffer bufInput((char*)xmiFileData.data(), xmiFileData.size());
	MemoryBuffer bufOutput;

	if (!bufInput.scanTo("EVNT", 4) || !bufInput.skip(8))
		return {};

	MidiTokensList lstTokens;
	MidiToken* pToken;
	int iTokenTime = 0;
	int iTempo = 500000;
	bool bTempoSet = false;
	bool bEnd = false;
	uint8_t iTokenType, iExtendedType;

	while (!bufInput.isEOF() && !bEnd)
	{
		while (true)
		{
			if (!bufInput.read(iTokenType))
				return {};

			if (iTokenType & 0x80)
				break;
			else
				iTokenTime += static_cast<int>(iTokenType) * 3;
		}
		pToken = lstTokens.append(iTokenTime, iTokenType);
		pToken->pBuffer = bufInput.getPointer() + 1;
		switch (iTokenType & 0xF0)
		{
		case 0xC0:
		case 0xD0:
			if (!bufInput.read(pToken->iData))
				return {};
			pToken->pBuffer = nullptr;
			break;

		case 0x80:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			if (!bufInput.read(pToken->iData))
				return {};
			if (!bufInput.skip(1))
				return {};
			break;

		case 0x90:
			if (!bufInput.read(iExtendedType))
				return {};
			pToken->iData = iExtendedType;
			if (!bufInput.skip(1))
				return {};
			pToken = lstTokens.append(iTokenTime + bufInput.readUIntVar() * 3, iTokenType);
			pToken->iData = iExtendedType;
			pToken->pBuffer = "\0";
			break;

		case 0xF0:
			iExtendedType = 0;
			if (iTokenType == 0xFF)
			{
				if (!bufInput.read(iExtendedType))
					return {};

				if (iExtendedType == 0x2F)
					bEnd = true;
				else if (iExtendedType == 0x51)
				{
					if (!bTempoSet)
					{
						bufInput.skip(1);
						iTempo = bufInput.readBigEndianUInt24() * 3;
						bTempoSet = true;
						bufInput.skip(-4);
					}
					else
					{
						lstTokens.pop_back();
						if (!bufInput.skip(bufInput.readUIntVar()))
							return {};
						break;
					}
				}
			}
			pToken->iData = iExtendedType;
			pToken->iBufferLength = bufInput.readUIntVar();
			pToken->pBuffer = bufInput.getPointer();
			if (!bufInput.skip(pToken->iBufferLength))
				return {};
			break;
		}
	}

	if (lstTokens.empty())
		return {};
	if (!bufOutput.write("MThd\0\0\0\x06\0\0\0\x01", 12))
		return {};
	if (!bufOutput.writeBigEndianUInt16((iTempo * 3) / 25000))
		return {};
	if (!bufOutput.write("MTrk\xBA\xAD\xF0\x0D", 8))
		return {};

	sort(lstTokens.begin(), lstTokens.end());

	iTokenTime = 0;
	iTokenType = 0;
	bEnd = false;

	for (auto& itr : lstTokens)
	{
		if (bEnd) break;

		if (!bufOutput.writeUIntVar(itr.iTime - iTokenTime))
			return {};
		iTokenTime = itr.iTime;
		if (itr.iType >= 0xF0)
		{
			if (!bufOutput.write(iTokenType = itr.iType))
				return {};
			if (iTokenType == 0xFF)
			{
				if (!bufOutput.write(itr.iData))
					return {};
				if (itr.iData == 0x2F)
					bEnd = true;
			}
			if (!bufOutput.writeUIntVar(itr.iBufferLength))
				return {};
			if (!bufOutput.write(itr.pBuffer, itr.iBufferLength))
				return {};
		}
		else
		{
			if (itr.iType != iTokenType)
			{
				if (!bufOutput.write(iTokenType = itr.iType))
					return {};
			}
			if (!bufOutput.write(itr.iData))
				return {};
			if (itr.pBuffer)
			{
				if (!bufOutput.write(itr.pBuffer, 1))
					return {};
			}
		}
	}

	uint32_t iLength = static_cast<uint32_t>(bufOutput.tell() - 22);
	bufOutput.seek(18);
	bufOutput.writeBigEndianUInt32(iLength);

	return bufOutput.takeData();
}
