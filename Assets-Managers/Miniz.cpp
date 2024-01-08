#include "GameEngine/framework.h"


#ifdef _M_X64
typedef uint64_t tinfl_bit_buf_t;
#else
typedef uint32_t tinfl_bit_buf_t;
#endif


// Internal/private bits follow
enum {
	TINFL_MAX_HUFF_TABLES = 3,
	TINFL_MAX_HUFF_SYMBOLS_0 = 288,
	TINFL_MAX_HUFF_SYMBOLS_1 = 32,
	TINFL_FAST_LOOKUP_BITS = 10,
	TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
};

enum {
	TINFL_STATUS_ADLER32_MISMATCH = -2,
	TINFL_STATUS_FAILED = -1,
	TINFL_STATUS_DONE = 0,
	TINFL_STATUS_HAS_MORE_OUTPUT = 2
};


struct HuffTable
{
	int16_t lookUp[TINFL_FAST_LOOKUP_SIZE];
	int16_t tree[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
	uint8_t codeSize[TINFL_MAX_HUFF_SYMBOLS_0];
};


// ------------------- Low-level Decompression (completely independent from all compression API's)

#define TINFL_GET_BYTE do { i = (pInBufCur >= pInBufEnd) ? 0 : (*pInBufCur++); } while (0)
#define TINFL_NEED_BITS(n) do { TINFL_GET_BYTE; bitBuf |= (((tinfl_bit_buf_t)i) << numBits); numBits += 8; } while (numBits < (uint32_t)(n))
#define TINFL_GET_BITS(b, n) do { if (numBits < (uint32_t)(n)) { TINFL_NEED_BITS(n); } b = bitBuf & ((1 << (n)) - 1); bitBuf >>= (n); numBits -= (n); } while (0)

// TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define TINFL_HUFF_DECODE(sym, pHuff) do { \
	if (numBits < 15) { \
		if ((pInBufEnd - pInBufCur) < 2) { \
			do { \
			temp = (pHuff).lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
			if (temp >= 0) { \
				codeLen = temp >> 9; \
				if ((codeLen) && (numBits >= codeLen)) \
					break; \
			} else if (numBits > TINFL_FAST_LOOKUP_BITS) { \
				codeLen = TINFL_FAST_LOOKUP_BITS; \
				do { \
					temp = (pHuff).tree[~temp + ((bitBuf >> codeLen++) & 1)]; \
				} while ((temp < 0) && (numBits >= (codeLen + 1))); if (temp >= 0) break; \
			} TINFL_GET_BYTE; bitBuf |= (((tinfl_bit_buf_t)i) << numBits); numBits += 8; \
			} while (numBits < 15);\
		} else { \
			bitBuf |= (((tinfl_bit_buf_t)pInBufCur[0]) << numBits) | (((tinfl_bit_buf_t)pInBufCur[1]) << (numBits + 8)); pInBufCur += 2; numBits += 16; \
		} \
	} \
	if ((temp = (pHuff).lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) { \
		codeLen = temp >> 9, temp &= 511; \
	} else { \
		codeLen = TINFL_FAST_LOOKUP_BITS; do { temp = (pHuff).tree[~temp + ((bitBuf >> codeLen++) & 1)]; } while (temp < 0); \
	} sym = temp; bitBuf >>= codeLen; numBits -= codeLen; \
} while (0)


// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
int mz_uncompress_main(uint8_t pDest[], uint32_t destLen, const uint8_t pSource[], uint32_t sourceLen, uint32_t& zAdler32)
{
	static const uint16_t lengthBase[31] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0 };
	static const uint16_t distBase[32] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0 };
	static const uint16_t minTableSizes[3] = { 257, 1, 4 };
	static const uint8_t lengthExtra[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0 };
	static const uint8_t distExtra[32] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
	static const uint8_t lengthDezigzag[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

	HuffTable tables[TINFL_MAX_HUFF_TABLES] = {}, * pTable;
	tinfl_bit_buf_t bitBuf = 0;
	size_t outBufSizeMask = (size_t)-1, distFromOutBufStart = 0;
	const uint8_t* pInBufCur = pSource + 2, * const pInBufEnd = pSource + sourceLen, * ptr;
	uint8_t* pOutBufCur = pDest, * const pOutBufEnd = pDest + destLen, * pSrc;
	uint32_t tableSizes[TINFL_MAX_HUFF_TABLES] = {}, nextCode[17]{}, totalSyms[16]{};
	uint32_t numBits = 0, dist = 0, counter = 0, numExtra = 0, final = 0, type = 0, extraBits;
	uint32_t codeLen, usedSyms, total, symIndex, revCode, curCode, codeSize, i;
	int temp, sym2;
	int16_t treeCur, treeNext;
	uint8_t lenCodes[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137] = {};

	do {
		TINFL_GET_BITS(final, 3);
		type = final >> 1;

		for (counter = 0; counter < 3; counter++) {
			TINFL_GET_BITS(tableSizes[counter], "\05\05\04"[counter]);
			tableSizes[counter] += minTableSizes[counter];
		}
		memset(tables[2].codeSize, 0, sizeof(tables[2].codeSize));
		for (counter = 0; counter < tableSizes[2]; counter++) {
			TINFL_GET_BITS(tables[2].codeSize[lengthDezigzag[counter]], 3);
		}
		tableSizes[2] = 19;

		for (; (int)type >= 0; type--)
		{
			pTable = tables + type;
			memset(totalSyms, 0, sizeof(totalSyms));
			memset(pTable->lookUp, 0, sizeof(pTable->lookUp));
			memset(pTable->tree, 0, sizeof(pTable->tree));

			for (i = 0; i < tableSizes[type]; totalSyms[pTable->codeSize[i++]]++);
			usedSyms = 0, total = 0;
			nextCode[0] = nextCode[1] = 0;
			for (i = 1; i <= 15; ++i) {
				usedSyms += totalSyms[i];
				nextCode[i + 1] = (total = ((total + totalSyms[i]) << 1));
			}
			if (65536 != total && usedSyms > 1)
				return TINFL_STATUS_FAILED;

			for (treeNext = -1, symIndex = 0; symIndex < tableSizes[type]; ++symIndex) {
				revCode = 0;
				codeSize = pTable->codeSize[symIndex];
				if (!codeSize) continue;
				curCode = nextCode[codeSize]++;
				for (i = codeSize; i > 0; i--, curCode >>= 1)
					revCode = (revCode << 1) | (curCode & 1);
				if (codeSize <= TINFL_FAST_LOOKUP_BITS) {
					i = ((codeSize << 9) | symIndex);
					while (revCode < TINFL_FAST_LOOKUP_SIZE) {
						pTable->lookUp[revCode] = (int16_t)i;
						revCode += (1 << codeSize);
					}
					continue;
				}
				if (0 == (treeCur = pTable->lookUp[revCode & (TINFL_FAST_LOOKUP_SIZE - 1)])) {
					pTable->lookUp[revCode & (TINFL_FAST_LOOKUP_SIZE - 1)] = treeNext;
					treeCur = treeNext;
					treeNext -= 2;
				}
				revCode >>= (TINFL_FAST_LOOKUP_BITS - 1);
				for (i = codeSize; i > (TINFL_FAST_LOOKUP_BITS + 1); i--) {
					treeCur -= ((revCode >>= 1) & 1);
					if (!pTable->tree[-treeCur - 1]) {
						pTable->tree[-treeCur - 1] = treeNext;
						treeCur = treeNext;
						treeNext -= 2;
					}
					else
						treeCur = pTable->tree[-treeCur - 1];
				}
				treeCur -= ((revCode >>= 1) & 1);
				pTable->tree[-treeCur - 1] = (int16_t)symIndex;
			}
			if (type == 2)
			{
				for (counter = 0; counter < tableSizes[0] + tableSizes[1];)
				{
					TINFL_HUFF_DECODE(dist, tables[2]);
					if (dist < 16) {
						lenCodes[counter++] = (uint8_t)dist;
						continue;
					}
					if (dist == 16 && !counter)
						return TINFL_STATUS_FAILED;
					numExtra = "\02\03\07"[dist - 16];
					TINFL_GET_BITS(i, numExtra);
					i += "\03\03\013"[dist - 16];
					memset(lenCodes + counter, (dist == 16) ? lenCodes[counter - 1] : 0, i);
					counter += i;
				}
				if (tableSizes[0] + tableSizes[1] != counter)
					return TINFL_STATUS_FAILED;

				memcpy(tables[0].codeSize, lenCodes, tableSizes[0]);
				memcpy(tables[1].codeSize, lenCodes + tableSizes[0], tableSizes[1]);
			}
		}

		for (;;)
		{
			for (;;)
			{
				if (pInBufEnd - pInBufCur < 4 || pOutBufEnd - pOutBufCur < 2)
				{
					TINFL_HUFF_DECODE(counter, tables[0]);
					if (counter >= 256)
						break;
					if (pOutBufCur >= pOutBufEnd)
						return TINFL_STATUS_HAS_MORE_OUTPUT;
					*pOutBufCur++ = (uint8_t)counter;
				}
				else
				{
#if _M_X64
					if (numBits < 30) { bitBuf |= ((tinfl_bit_buf_t(*((const uint32_t*)(pInBufCur)))) << numBits); pInBufCur += 4; numBits += 32; }
#else
					if (numBits < 15) { bitBuf |= ((tinfl_bit_buf_t(*((const uint16_t*)(pInBufCur)))) << numBits); pInBufCur += 2; numBits += 16; }
#endif
					if ((sym2 = tables[0].lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
						codeLen = sym2 >> 9;
					else
						for (codeLen = TINFL_FAST_LOOKUP_BITS; sym2 < 0; sym2 = tables[0].tree[~sym2 + ((bitBuf >> codeLen++) & 1)]);

					counter = sym2;
					bitBuf >>= codeLen;
					numBits -= codeLen;
					if (counter & 256)
						break;

#if !_M_X64
					if (numBits < 15) { bitBuf |= (((tinfl_bit_buf_t) * ((const uint16_t*)(pInBufCur))) << numBits); pInBufCur += 2; numBits += 16; }
#endif
					if ((sym2 = tables[0].lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
						codeLen = sym2 >> 9;
					else
						for (codeLen = TINFL_FAST_LOOKUP_BITS; sym2 < 0; sym2 = tables[0].tree[~sym2 + ((bitBuf >> codeLen++) & 1)]);

					bitBuf >>= codeLen;
					numBits -= codeLen;

					pOutBufCur[0] = (uint8_t)counter;
					if (sym2 & 256)
					{
						pOutBufCur++;
						counter = sym2;
						break;
					}
					pOutBufCur[1] = (uint8_t)sym2;
					pOutBufCur += 2;
				}
			}
			if ((counter &= 511) == 256)
				break;

			numExtra = lengthExtra[counter - 257];
			counter = lengthBase[counter - 257];
			if (numExtra)
			{
				TINFL_GET_BITS(extraBits, numExtra);
				counter += extraBits;
			}

			TINFL_HUFF_DECODE(dist, tables[1]);
			numExtra = distExtra[dist];
			dist = distBase[dist];
			if (numExtra)
			{
				TINFL_GET_BITS(extraBits, numExtra);
				dist += extraBits;
			}

			distFromOutBufStart = pOutBufCur - pDest;
			if (dist > distFromOutBufStart)
				return TINFL_STATUS_FAILED;

			pSrc = pDest + ((distFromOutBufStart - dist) & outBufSizeMask);

			if (max(pOutBufCur, pSrc) + counter > pOutBufEnd)
			{
				while (counter--)
				{
					if (pOutBufCur >= pOutBufEnd)
						return TINFL_STATUS_HAS_MORE_OUTPUT;
					*pOutBufCur++ = pDest[(distFromOutBufStart++ - dist) & outBufSizeMask];
				}
				continue;
			}
			else if (counter >= 9 && counter <= dist)
			{
				for (ptr = pSrc + (counter & ~7); pSrc < ptr; pOutBufCur += 8, pSrc += 8)
					memcpy(pOutBufCur, pSrc, 8);

				if ((counter &= 7) < 3)
				{
					if (counter)
					{
						pOutBufCur[0] = pSrc[0];
						if (counter > 1)
							pOutBufCur[1] = pSrc[1];
						pOutBufCur += counter;
					}
					continue;
				}
			}

			do {
				pOutBufCur[0] = pSrc[0];
				pOutBufCur[1] = pSrc[1];
				pOutBufCur[2] = pSrc[2];
				pOutBufCur += 3; pSrc += 3;
			} while ((int)(counter -= 3) > 2);

			if ((int)counter > 0)
			{
				pOutBufCur[0] = pSrc[0];
				if ((int)counter > 1)
					pOutBufCur[1] = pSrc[1];
				pOutBufCur += counter;
			}
		}
	} while (!(final & 1));

	TINFL_NEED_BITS(numBits & 7);
	bitBuf >>= (numBits & 7);
	numBits -= (numBits & 7);

	for (zAdler32 = 1, counter = 0; counter < 4; ++counter) {
		if (numBits)
			TINFL_GET_BITS(i, 8);
		else
			TINFL_GET_BYTE;
		zAdler32 = (zAdler32 << 8) | i;
	}

//	sourceLen = uint32_t(pInBufCur - pSource);
//	destLen = uint32_t(pOutBufCur - pDest);

	return TINFL_STATUS_DONE;
}

void mz_uncompress(uint8_t pDest[], uint32_t destLen, const uint8_t pSource[], uint32_t sourceLen)
{
	uint32_t zAdler32;
	int status = mz_uncompress_main(pDest, destLen, pSource, sourceLen, zAdler32);

	if (status >= 0)
	{
		const uint8_t* ptr = pDest;
		uint32_t bufLen = destLen, blockLen = bufLen % 5552, c, i, j, k;

		for (c = 0, k = 1; bufLen; bufLen -= blockLen, blockLen = 5552)
		{
			for (i = 7; i < blockLen; i += 8, ptr += 8)
				for (j = 0; j < 8; c += (k += ptr[j++]));

			for (i -= 7; i < blockLen; i++, k += *ptr++, c += k);

			k %= 65521U;
			c %= 65521U;
		}

		if (status == TINFL_STATUS_DONE && (c << 16) + k != zAdler32)
			status = TINFL_STATUS_ADLER32_MISMATCH;
	}

	if (status != 0)
		throw Exception("MiniZip: invalid data");
}
