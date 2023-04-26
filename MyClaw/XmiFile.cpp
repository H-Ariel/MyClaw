// XMI file parser, based on "libwap" (see at OpenClaw: https://github.com/pjasicek/OpenClaw/blob/master/libwap/XmiFile.cpp)

#include "BufferReader.h"
#include "BufferWriter.h"


struct MidiToken
{
	int time;
	unsigned int len; // buffer length
	const uint8_t* buf;
	uint8_t type, data;

	bool operator<(const MidiToken& oRight) { return time < oRight.time; }
};

class MidiTokensList : public vector<MidiToken>
{
public:
	MidiToken* append(int iTime, uint8_t iType)
	{
		push_back(MidiToken());
		MidiToken* pToken = &back();
		pToken->time = iTime;
		pToken->type = iType;
		return pToken;
	}
};


class NewBufferReader : public BufferReader
{
public:
	NewBufferReader(const uint8_t data[], size_t size)
		: BufferReader(data, size, false) {}

	void scanTo(const void* data, size_t dataSize)
	{
		for (; _idx + dataSize <= _size; ++_idx)
		{
			if (memcmp(_data + _idx, data, dataSize) == 0)
				return;
		}
		throw Exception("data not found");
	}

	const uint8_t* getPointer() const { return _data + _idx; }

	uint32_t readUIntVar()
	{
		uint32_t val = 0;
		uint8_t b;
		for (int i = 0; i < 4; ++i)
		{
			read(b);
			val = (val << 7) | uint32_t(b & 0x7F);
			if ((b & 0x80) == 0)
				break;
		}
		return val;
	}
	uint32_t readBigEndianUInt24()
	{
		uint8_t b1, b2, b3;
		read(b1); read(b2); read(b3);
		return (((b1 << 8) | b2) << 8) | b3;
	}
};

class NewBufferWriter : public BufferWriter
{
public:
	void writeBigEndianUInt16(uint16_t iValue) { write(_byteSwap(iValue)); }
	void writeBigEndianUInt32(uint32_t iValue) { write(_byteSwap(iValue)); }
	void writeUIntVar(uint32_t iValue)
	{
		int bytesCount = 1;
		uint32_t buffer = iValue & 0x7F;
		for (; iValue >>= 7; ++bytesCount, buffer = (buffer << 8) | 0x80 | (iValue & 0x7F));
		for (; bytesCount > 0; --bytesCount)
		{
			write(uint8_t(buffer & 0xFF));
			buffer >>= 8;
		}
	}

private:
	template <class T>
	static T _byteSwap(T value)
	{
		uint8_t* ptr = (uint8_t*)(&value);
		reverse(ptr, ptr + sizeof(T));
		return value;
	}
};


vector<uint8_t> xmiToMidi(vector<uint8_t> xmiFileData)
{
	NewBufferReader input(xmiFileData.data(), xmiFileData.size());
	NewBufferWriter output;

	MidiTokensList tokensList;
	MidiToken* pToken;
	int tempo = 500000, tokenTime = 0;
	bool isTempoSet = false, endLoop = false;
	uint8_t tokenType, extendedType;

	input.scanTo("EVNT", 4);
	input.skip(8);

	while (!input.isEOF() && !endLoop)
	{
		while (true)
		{
			input.read(tokenType);

			if (tokenType & 0x80)
				break;
			else
				tokenTime += (int)tokenType * 3;
		}

		pToken = tokensList.append(tokenTime, tokenType);
		pToken->buf = input.getPointer() + 1;

		switch (tokenType & 0xF0)
		{
		case 0xC0:
		case 0xD0:
			input.read(pToken->data);
			pToken->buf = nullptr;
			break;

		case 0x80:
		case 0xA0:
		case 0xB0:
		case 0xE0:
			input.read(pToken->data);
			input.skip(1);
			break;

		case 0x90:
			input.read(extendedType);
			pToken->data = extendedType;
			input.skip(1);
			pToken = tokensList.append(tokenTime + input.readUIntVar() * 3, tokenType);
			pToken->data = extendedType;
			pToken->buf = (const uint8_t*)"\0";
			break;

		case 0xF0:
			extendedType = 0;
			if (tokenType == 0xFF)
			{
				input.read(extendedType);

				if (extendedType == 0x2F)
					endLoop = true;
				else if (extendedType == 0x51)
				{
					if (!isTempoSet)
					{
						input.skip(1);
						tempo = input.readBigEndianUInt24() * 3;
						isTempoSet = true;
						input.skip(-4);
					}
					else
					{
						tokensList.pop_back();
						input.skip(input.readUIntVar());
						break;
					}
				}
			}

			pToken->data = extendedType;
			pToken->len = input.readUIntVar();
			pToken->buf = input.getPointer();
			input.skip(pToken->len);
			break;
		}
	}

	if (tokensList.empty())
		return {};

	sort(tokensList.begin(), tokensList.end());

	output.write("MThd\0\0\0\x06\0\0\0\x01", 12);
	output.writeBigEndianUInt16(tempo * 3 / 25000);
	output.write("MTrk\xBA\xAD\xF0\x0D", 8);

	tokenTime = 0;
	tokenType = 0;
	endLoop = false;

	for (MidiToken& tok : tokensList)
	{
		if (endLoop) break;

		output.writeUIntVar(tok.time - tokenTime);
		tokenTime = tok.time;
		if (tok.type >= 0xF0)
		{
			output.write(tokenType = tok.type);
			if (tokenType == 0xFF)
			{
				output.write(tok.data);
				endLoop = (tok.data == 0x2F);
			}
			output.writeUIntVar(tok.len);
			output.write(tok.buf, tok.len);
		}
		else
		{
			if (tok.type != tokenType)
			{
				output.write(tokenType = tok.type);
			}
			output.write(tok.data);
			if (tok.buf)
			{
				output.write(tok.buf, 1);
			}
		}
	}

	uint32_t len = uint32_t(output.getIndex() - 22);
	output.setIndex(18);
	output.writeBigEndianUInt32(len);

	return output.getData();
}
