#include "BufferReader.h"
#include "BufferWriter.h"


class ExtendedBufferReader : public BufferReader
{
public:
	ExtendedBufferReader(const vector<uint8_t>& buffer) : BufferReader(buffer) {}
	// scan until it find `buf`. `len` is length in bytes of `buf`
	void scanTo(const void* buf, size_t len) { for (; memcmp(_data + _idx, buf, len) && !isEOF(); _idx += 1); }
	const uint8_t* getPointer() const { return _data + _idx; }
	unsigned int readUIntVar()
	{
		unsigned int val = 0;
		for (uint8_t i = 0, iByte; i < 4; i++)
		{
			read(iByte);
			val = (val << 7) | (unsigned int)(iByte & 0x7F);
			if ((iByte & 0x80) == 0)
			{
				break;
			}
		}
		return val;
	}
	unsigned int readBigEndianUInt24() { return (((read<uint8_t>() << 8) | read<uint8_t>()) << 8) | read<uint8_t>(); }
};

class ExtendedBufferWriter : public BufferWriter
{
private:
	// reverses the bytes order of `obj`
	template <class T>
	static inline void reverseBytes(T& obj)
	{
		uint8_t* ptr = (uint8_t*)(&obj);
		reverse(ptr, ptr + sizeof(T));
	}
public:
	void writeBigEndianUInt16(uint16_t iValue) { reverseBytes(iValue); write(iValue); }
	void writeBigEndianUInt32(uint32_t iValue) { reverseBytes(iValue); write(iValue); }
	void writeUIntVar(unsigned int iValue)
	{
		int byteCount = 1;
		unsigned int buffer = iValue & 0x7F;
		uint8_t byte;
		for (; iValue >>= 7; ++byteCount)
		{
			buffer = (buffer << 8) | 0x80 | (iValue & 0x7F);
		}
		for (int i = 0; i < byteCount; ++i)
		{
			byte = buffer & 0xFF;
			write(byte);
			buffer >>= 8;
		}
	}
};

struct MidiToken
{
	const uint8_t* buffer;
	unsigned int bufferLength;
	int time;
	uint8_t type;
	uint8_t data;
};

class MidiTokenList : public vector<MidiToken>
{
public:
	MidiToken* append(int time, uint8_t type)
	{
		push_back(MidiToken());
		MidiToken* pToken = &back();
		pToken->time = time;
		pToken->type = type;
		return pToken;
	}
};


vector<uint8_t> xmiToMidi(vector<uint8_t> xmiFileData)
{
	ExtendedBufferReader bufIn(xmiFileData);
	ExtendedBufferWriter bufOut;

	bufIn.scanTo("EVNT", 4);
	bufIn.skip(8);

	MidiTokenList tokensList;
	MidiToken* pToken;
	int tokenTime = 0;
	int tempo = 500000;
	bool isTempoSet = false;
	bool stopLoop = false;
	uint8_t tokenType, extendedType;

	while (!bufIn.isEOF() && !stopLoop)
	{
		while (true)
		{
			bufIn.read(tokenType);
			if (tokenType & 0x80) break;
			tokenTime += (int)tokenType * 3;
		}

		pToken = tokensList.append(tokenTime, tokenType);
		pToken->buffer = bufIn.getPointer() + 1;
		switch (tokenType & 0xF0)
		{
		case 0xC0:
		case 0xD0:
			bufIn.read(pToken->data);
			pToken->buffer = nullptr;
			break;
		case 0x80:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			bufIn.read(pToken->data);
			bufIn.skip(1);
			break;
		case 0x90:
			bufIn.read(extendedType);
			pToken->data = extendedType;
			bufIn.skip(1);
			pToken = tokensList.append(tokenTime + bufIn.readUIntVar() * 3, tokenType);
			pToken->data = extendedType;
			pToken->buffer = (const uint8_t*)"\0";
			break;
		case 0xF0:
			extendedType = 0;
			if (tokenType == 0xFF)
			{
				bufIn.read(extendedType);

				if (extendedType == 0x2F)
				{
					stopLoop = true;
				}
				else if (extendedType == 0x51)
				{
					if (isTempoSet)
					{
						tokensList.pop_back();
						bufIn.skip(bufIn.readUIntVar());
						break;
					}
					else
					{
						bufIn.skip(1);
						tempo = bufIn.readBigEndianUInt24() * 3;
						isTempoSet = true;
						bufIn.skip(-4);
					}
				}
			}
			pToken->data = extendedType;
			pToken->bufferLength = bufIn.readUIntVar();
			pToken->buffer = bufIn.getPointer();
			bufIn.skip(pToken->bufferLength);
			break;
		}
	}

	if (tokensList.empty())
	{
		return {};
	}

	sort(tokensList.begin(), tokensList.end(), [](const MidiToken& a, const MidiToken& b) { return a.time < b.time; });
	tokenTime = 0;
	tokenType = 0;

	bufOut.write("MThd\0\0\0\x06\0\0\0\x01", 12);
	bufOut.writeBigEndianUInt16(tempo * 3 / 25000);
	bufOut.write("MTrk\xBA\xAD\xF0\x0D", 8);

	for (MidiToken& t : tokensList)
	{
		bufOut.writeUIntVar(t.time - tokenTime);
		tokenTime = t.time;
		if (t.type >= 0xF0)
		{
			bufOut.write(tokenType = t.type);

			if (tokenType == 0xFF)
			{
				bufOut.write(t.data);
				if (t.data == 0x2F)
				{
					break;
				}
			}
			bufOut.writeUIntVar(t.bufferLength);
			bufOut.write(t.buffer, t.bufferLength);
		}
		else
		{
			if (t.type != tokenType)
			{
				bufOut.write(tokenType = t.type);
			}
			bufOut.write(t.data);
			if (t.buffer)
			{
				bufOut.write(t.buffer, 1);
			}
		}
	}

	uint32_t length = (uint32_t)(bufOut.getIndex() - 22);
	bufOut.setIndex(18);
	bufOut.writeBigEndianUInt32(length);

	return bufOut.getData();
}
