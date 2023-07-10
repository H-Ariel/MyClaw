// based on miniz at https://github.com/pjasicek/OpenClaw/tree/master/libwap
// I delete some unused code and add some code to make it work with my project

#include "framework.h"


#if defined(_M_IX86) || defined(_M_X64) || defined(__i386__) || defined(__i386) || defined(__i486__) || defined(__i486) || defined(i386) || defined(__ia64__) || defined(__x86_64__)
// MINIZ_X86_OR_X64_CPU is only used to help set the below macros.
#define MINIZ_X86_OR_X64_CPU 1
// Set MINIZ_USE_UNALIGNED_LOADS_AND_STORES to 1 on CPU's that permit efficient integer loads and stores from unaligned addresses.
#define MINIZ_USE_UNALIGNED_LOADS_AND_STORES 1
#endif

#if (__BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__) || MINIZ_X86_OR_X64_CPU
#define MINIZ_LITTLE_ENDIAN 1
#endif

#if defined(_M_X64) || defined(_WIN64) || defined(__MINGW64__) || defined(_LP64) || defined(__LP64__) || defined(__ia64__) || defined(__x86_64__)
#define TINFL_USE_64BIT_BITBUF 1
#define TINFL_BITBUF_SIZE (64)
typedef uint64_t tinfl_bit_buf_t;
#else
#define TINFL_USE_64BIT_BITBUF 0
#define TINFL_BITBUF_SIZE (32)
typedef uint32_t tinfl_bit_buf_t;
#endif

#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES && MINIZ_LITTLE_ENDIAN
#define MZ_READ_LE16(p) *((const uint16_t*)(p))
#define MZ_READ_LE32(p) *((const uint32_t*)(p))
#else
#define MZ_READ_LE16(p) ((uint32_t)(((const uint8_t*)(p))[0]) | ((uint32_t)(((const uint8_t *)(p))[1]) << 8U))
#define MZ_READ_LE32(p) ((uint32_t)(((const uint8_t*)(p))[0]) | ((uint32_t)(((const uint8_t *)(p))[1]) << 8U) | ((uint32_t)(((const uint8_t *)(p))[2]) << 16U) | ((uint32_t)(((const uint8_t *)(p))[3]) << 24U))
#endif

#define TINFL_LZ_DICT_SIZE 0x8000 // max size of LZ dictionary



// I add this to make it easier to use the Miniz in my project
#define MZ_EXCEPTION(msg) throw Exception("MiniZip: " msg)


// Return status codes. MZ_PARAM_ERROR is non-standard.
// TODO: use my MZ_EXCEPTION macro to throw exceptions instead of returning error codes.
enum {
	MZ_OK = 0,
	MZ_STREAM_END = 1,
	MZ_DATA_ERROR = -3,
	MZ_BUF_ERROR = -5
};

// Internal/private bits follow.
enum {
	TINFL_MAX_HUFF_TABLES = 3,
	TINFL_MAX_HUFF_SYMBOLS_0 = 288,
	TINFL_MAX_HUFF_SYMBOLS_1 = 32,
	TINFL_FAST_LOOKUP_BITS = 10,
	TINFL_FAST_LOOKUP_SIZE = 1 << TINFL_FAST_LOOKUP_BITS
};

// Return status.
enum tinfl_status {
	TINFL_STATUS_BAD_PARAM = -3,
	TINFL_STATUS_ADLER32_MISMATCH = -2,
	TINFL_STATUS_FAILED = -1,
	TINFL_STATUS_DONE = 0,
	TINFL_STATUS_HAS_MORE_OUTPUT = 2
};


struct tinfl_huff_table
{
	tinfl_huff_table() {
		lookUp = DBG_NEW int16_t[TINFL_FAST_LOOKUP_SIZE];
		tree = DBG_NEW int16_t[TINFL_MAX_HUFF_SYMBOLS_0 * 2];
		codeSize = DBG_NEW uint8_t[TINFL_MAX_HUFF_SYMBOLS_0];
	}
	~tinfl_huff_table() {
		delete[] lookUp;
		delete[] tree;
		delete[] codeSize;
	}

	void clearLookUp() {
		memset(lookUp, 0, sizeof(int16_t) * TINFL_FAST_LOOKUP_SIZE);
	}
	void clearTree() {
		memset(tree, 0, sizeof(int16_t) * TINFL_MAX_HUFF_SYMBOLS_0 * 2);
	}
	void clearCodeSize() {
		memset(codeSize, 0, sizeof(uint8_t) * TINFL_MAX_HUFF_SYMBOLS_0);
	}

	int16_t* lookUp;
	int16_t* tree;
	uint8_t* codeSize;
};

struct tinfl_decompressor
{
	tinfl_decompressor() {
		state = 0;
		lenCodes = DBG_NEW uint8_t[TINFL_MAX_HUFF_SYMBOLS_0 + TINFL_MAX_HUFF_SYMBOLS_1 + 137];
	}
	~tinfl_decompressor() {
		delete[] lenCodes;
	}

	tinfl_huff_table tables[TINFL_MAX_HUFF_TABLES];
	uint8_t* lenCodes;
	uint32_t state, zhdr0, zhdr1, zAdler32, final, type, checkAdler32, tableSizes[TINFL_MAX_HUFF_TABLES];
	uint8_t rawHeader[4];
};


// Main low-level decompressor coroutine function. This is the only function actually needed for decompression. All the other functions are just high-level helpers for improved usability.
// This is a universal API, i.e. it can be used as a building block to build any desired higher level decompression API. In the limit case, it can be called once per every byte input or output.
int tinfl_decompress(const uint8_t* pInBufNext, uint32_t* pInBufSize, uint8_t* pOutBufStart, uint32_t* pOutBufSize);


int mz_uncompress(uint8_t* pDest, uint32_t* pDestLen, const uint8_t* pSource, uint32_t sourceLen)
{
	uint32_t inBytes = sourceLen, outBytes = *pDestLen;
	int status = tinfl_decompress(pSource, &inBytes, pDest, &outBytes);
	
	if (status == MZ_STREAM_END)
	{
		*pDestLen = outBytes;
		status = MZ_OK;
	}
	else if (status == MZ_BUF_ERROR && sourceLen == inBytes)
	{
		status = MZ_DATA_ERROR;
	}

	return status;
}


#define TINFL_CR_RETURN(stateIndex, result) do { status = result; decomp.state = stateIndex; goto common_exit; /*case stateIndex:;*/ } while (0)
#define TINFL_CR_RETURN_FOREVER(stateIndex, result) do { for (;;) TINFL_CR_RETURN(stateIndex, result); } while (0)

// TODO: If the caller has indicated that there's no more input, and we attempt to read beyond the input buf, then something is wrong with the input because the inflator never
// reads ahead more than it needs to. Currently TINFL_GET_BYTE() pads the end of the stream with 0's in this scenario.
#define TINFL_GET_BYTE(stateIndex, c) do { \
	if (pInBufCur >= pInBufEnd) c = 0; \
	else c = *pInBufCur++; \
} while (0)

#define TINFL_NEED_BITS(stateIndex, n) do { uint32_t c; TINFL_GET_BYTE(stateIndex, c); bitBuf |= (((tinfl_bit_buf_t)c) << numBits); numBits += 8; } while (numBits < (uint32_t)(n))
#define TINFL_SKIP_BITS(stateIndex, n) do { if (numBits < (uint32_t)(n)) { TINFL_NEED_BITS(stateIndex, n); } bitBuf >>= (n); numBits -= (n); } while (0)
#define TINFL_GET_BITS(stateIndex, b, n) do { if (numBits < (uint32_t)(n)) { TINFL_NEED_BITS(stateIndex, n); } b = bitBuf & ((1 << (n)) - 1); bitBuf >>= (n); numBits -= (n); } while (0)

// TINFL_HUFF_DECODE() decodes the next Huffman coded symbol. It's more complex than you would initially expect because the zlib API expects the decompressor to never read
// beyond the final byte of the deflate stream. (In other words, when this macro wants to read another byte from the input, it REALLY needs another byte in order to fully
// decode the next Huffman code.) Handling this properly is particularly important on raw deflate (non-zlib) streams, which aren't followed by a byte aligned adler-32.
// The slow path is only executed at the very end of the input buffer.
#define TINFL_HUFF_DECODE(stateIndex, sym, pHuff) do { \
	int temp; uint32_t codeLen, c; \
	if (numBits < 15) { \
		if ((pInBufEnd - pInBufCur) < 2) { \
			do { \
				temp = (pHuff)->lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]; \
				if (temp >= 0) { \
					codeLen = temp >> 9; \
					if (codeLen && numBits >= codeLen) break; \
				} else if (numBits > TINFL_FAST_LOOKUP_BITS) { \
					codeLen = TINFL_FAST_LOOKUP_BITS; \
					do { \
						temp = (pHuff)->tree[~temp + ((bitBuf >> codeLen++) & 1)]; \
					} while (temp < 0 && numBits >= codeLen + 1); \
					if (temp >= 0) break; \
				} TINFL_GET_BYTE(stateIndex, c); bitBuf |= (((tinfl_bit_buf_t)c) << numBits); numBits += 8; \
			} while (numBits < 15);\
		} else { \
			bitBuf |= (((tinfl_bit_buf_t)pInBufCur[0]) << numBits) | (((tinfl_bit_buf_t)pInBufCur[1]) << (numBits + 8)); pInBufCur += 2; numBits += 16; \
		} \
	} \
	if ((temp = (pHuff)->lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0) \
		codeLen = temp >> 9, temp &= 511; \
	else { \
		codeLen = TINFL_FAST_LOOKUP_BITS; \
		do { \
			temp = (pHuff)->tree[~temp + ((bitBuf >> codeLen++) & 1)]; \
		} while (temp < 0); \
	} sym = temp; bitBuf >>= codeLen; numBits -= codeLen; \
} while (0)

int tinfl_decompress(const uint8_t* pInBufNext, uint32_t* pInBufSize, uint8_t* pOutBufStart, uint32_t* pOutBufSize)
{
	static const int lengthBase[31] = { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258, 0, 0 };
	static const int lengthExtra[31] = { 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0 };
	static const int distBase[32] = { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577, 0, 0 };
	static const int distExtra[32] = { 0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13 };
	static const uint8_t lengthDezigzag[19] = { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
	static const int minTableSizes[3] = { 257, 1, 4 };

	int status = TINFL_STATUS_FAILED;
	uint32_t numBits = 0, dist = 0, counter = 0, numExtra = 0;
	tinfl_bit_buf_t bitBuf = 0;
	tinfl_decompressor decomp;
	uint8_t* pOutBufNext = pOutBufStart;
	const uint8_t* pInBufCur = pInBufNext, * const pInBufEnd = pInBufNext + *pInBufSize;
	uint8_t* pOutBufCur = pOutBufNext, * const pOutBufEnd = pOutBufNext + *pOutBufSize;
	size_t outBufSizeMask = (size_t)-1, distFromOutBufStart = 0;

	switch (decomp.state) {
	case 0:
		bitBuf = numBits = dist = counter = numExtra = decomp.zhdr0 = decomp.zhdr1 = 0; 
		decomp.zAdler32 = decomp.checkAdler32 = 1;

		TINFL_GET_BYTE(1, decomp.zhdr0);
		TINFL_GET_BYTE(2, decomp.zhdr1);
		counter = (((decomp.zhdr0 * 256 + decomp.zhdr1) % 31 != 0) || (decomp.zhdr1 & 32) || ((decomp.zhdr0 & 15) != 8));
		if (counter)
			TINFL_CR_RETURN_FOREVER(36, TINFL_STATUS_FAILED);

		do {
			TINFL_GET_BITS(3, decomp.final, 3); decomp.type = decomp.final >> 1;
			if (decomp.type == 0)
			{
				TINFL_SKIP_BITS(5, numBits & 7);
				for (counter = 0; counter < 4; ++counter) {
					if (numBits)
						TINFL_GET_BITS(6, decomp.rawHeader[counter], 8);
					else
						TINFL_GET_BYTE(7, decomp.rawHeader[counter]);
				}
				if ((counter = (decomp.rawHeader[0] | (decomp.rawHeader[1] << 8))) != (uint32_t)(0xFFFF ^ (decomp.rawHeader[2] | (decomp.rawHeader[3] << 8)))) {
					TINFL_CR_RETURN_FOREVER(39, TINFL_STATUS_FAILED);
				}
				while (counter && numBits)
				{
					TINFL_GET_BITS(51, dist, 8);
					while (pOutBufCur >= pOutBufEnd) {
						TINFL_CR_RETURN(52, TINFL_STATUS_HAS_MORE_OUTPUT);
					}
					*pOutBufCur++ = (uint8_t)dist;
					counter--;
				}
				while (counter)
				{
					size_t n;
					while (pOutBufCur >= pOutBufEnd)
						TINFL_CR_RETURN(9, TINFL_STATUS_HAS_MORE_OUTPUT);

					while (pInBufCur >= pInBufEnd)
						TINFL_CR_RETURN_FOREVER(40, TINFL_STATUS_FAILED);

					n = min((uint32_t)min(pOutBufEnd - pOutBufCur, pInBufEnd - pInBufCur), counter);
					memcpy(pOutBufCur, pInBufCur, n); pInBufCur += n; pOutBufCur += n; counter -= (uint32_t)n;
				}
			}
			else if (decomp.type == 3)
				TINFL_CR_RETURN_FOREVER(10, TINFL_STATUS_FAILED);
			else
			{
				if (decomp.type == 1)
				{
					uint8_t* p = decomp.tables[0].codeSize; uint32_t i;
					decomp.tableSizes[0] = 288;
					decomp.tableSizes[1] = 32;
					memset(decomp.tables[1].codeSize, 5, 32);
					for (i = 0; i <= 143; ++i) *p++ = 8;
					for (; i <= 255; ++i) *p++ = 9;
					for (; i <= 279; ++i) *p++ = 7;
					for (; i <= 287; ++i) *p++ = 8;
				}
				else
				{
					for (counter = 0; counter < 3; counter++) {
						TINFL_GET_BITS(11, decomp.tableSizes[counter], "\05\05\04"[counter]);
						decomp.tableSizes[counter] += minTableSizes[counter];
					}
					decomp.tables[2].clearCodeSize();

					for (counter = 0; counter < decomp.tableSizes[2]; counter++) {
						uint32_t s;
						TINFL_GET_BITS(14, s, 3);
						decomp.tables[2].codeSize[lengthDezigzag[counter]] = (uint8_t)s;
					}
					decomp.tableSizes[2] = 19;
				}
				for (; (int)decomp.type >= 0; decomp.type--)
				{
					int tree_next, tree_cur; tinfl_huff_table* pTable;
					uint32_t i, j, used_syms, total, sym_index, next_code[17], total_syms[16];

					pTable = &decomp.tables[decomp.type];
					memset(total_syms, 0, sizeof(total_syms));
					pTable->clearLookUp();
					pTable->clearTree();

					for (i = 0; i < decomp.tableSizes[decomp.type]; total_syms[pTable->codeSize[i++]]++);
					used_syms = 0, total = 0;
					next_code[0] = next_code[1] = 0;
					for (i = 1; i <= 15; ++i) {
						used_syms += total_syms[i];
						next_code[i + 1] = total = ((total + total_syms[i]) << 1);
					}
					if (65536 != total && used_syms > 1)
					{
						TINFL_CR_RETURN_FOREVER(35, TINFL_STATUS_FAILED);
					}
					for (tree_next = -1, sym_index = 0; sym_index < decomp.tableSizes[decomp.type]; ++sym_index)
					{
						uint32_t rev_code = 0, l, cur_code, code_size = pTable->codeSize[sym_index];
						if (!code_size)
							continue;
						cur_code = next_code[code_size]++;
						for (l = code_size; l > 0; l--, cur_code >>= 1)
							rev_code = (rev_code << 1) | (cur_code & 1);
						if (code_size <= TINFL_FAST_LOOKUP_BITS) {
							int16_t k = (int16_t)((code_size << 9) | sym_index);
							while (rev_code < TINFL_FAST_LOOKUP_SIZE) {
								pTable->lookUp[rev_code] = k;
								rev_code += (1 << code_size);
							}
							continue;
						}
						if (0 == (tree_cur = pTable->lookUp[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)])) {
							pTable->lookUp[rev_code & (TINFL_FAST_LOOKUP_SIZE - 1)] = (int16_t)tree_next;
							tree_cur = tree_next;
							tree_next -= 2;
						}
						rev_code >>= (TINFL_FAST_LOOKUP_BITS - 1);
						for (j = code_size; j > (TINFL_FAST_LOOKUP_BITS + 1); j--)
						{
							tree_cur -= ((rev_code >>= 1) & 1);
							if (!pTable->tree[-tree_cur - 1]) {
								pTable->tree[-tree_cur - 1] = (int16_t)tree_next;
								tree_cur = tree_next;
								tree_next -= 2;
							}
							else
								tree_cur = pTable->tree[-tree_cur - 1];
						}
						tree_cur -= ((rev_code >>= 1) & 1);
						pTable->tree[-tree_cur - 1] = (int16_t)sym_index;
					}
					if (decomp.type == 2)
					{
						for (counter = 0; counter < (decomp.tableSizes[0] + decomp.tableSizes[1]);)
						{
							uint32_t s;
							TINFL_HUFF_DECODE(16, dist, &decomp.tables[2]);
							if (dist < 16) {
								decomp.lenCodes[counter++] = (uint8_t)dist;
								continue;
							}
							if (dist == 16 && !counter)
								TINFL_CR_RETURN_FOREVER(17, TINFL_STATUS_FAILED);
							numExtra = "\02\03\07"[dist - 16];
							TINFL_GET_BITS(18, s, numExtra);
							s += "\03\03\013"[dist - 16];
							memset(decomp.lenCodes + counter, (dist == 16) ? decomp.lenCodes[counter - 1] : 0, s);
							counter += s;
						}
						if (decomp.tableSizes[0] + decomp.tableSizes[1] != counter)
						{
							TINFL_CR_RETURN_FOREVER(21, TINFL_STATUS_FAILED);
						}
						memcpy(decomp.tables[0].codeSize, decomp.lenCodes, decomp.tableSizes[0]);
						memcpy(decomp.tables[1].codeSize, decomp.lenCodes + decomp.tableSizes[0], decomp.tableSizes[1]);
					}
				}
				for (;;)
				{
					uint8_t* pSrc;
					for (;;)
					{
						if (pInBufEnd - pInBufCur < 4 || pOutBufEnd - pOutBufCur < 2)
						{
							TINFL_HUFF_DECODE(23, counter, &decomp.tables[0]);
							if (counter >= 256)
								break;
							while (pOutBufCur >= pOutBufEnd)
								TINFL_CR_RETURN(24, TINFL_STATUS_HAS_MORE_OUTPUT);
							*pOutBufCur++ = (uint8_t)counter;
						}
						else
						{
							int sym2;
							uint32_t codeLen;
#if TINFL_USE_64BIT_BITBUF
							if (numBits < 30) {
								bitBuf |= (((tinfl_bit_buf_t)MZ_READ_LE32(pInBufCur)) << numBits);
								pInBufCur += 4;
								numBits += 32;
							}
#else
							if (numBits < 15) {
								bitBuf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pInBufCur)) << numBits);
								pInBufCur += 2;
								numBits += 16;
							}
#endif
							if ((sym2 = decomp.tables[0].lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
								codeLen = sym2 >> 9;
							else
							{
								codeLen = TINFL_FAST_LOOKUP_BITS;
								do {
									sym2 = decomp.tables[0].tree[~sym2 + ((bitBuf >> codeLen++) & 1)];
								} while (sym2 < 0);
							}
							counter = sym2; bitBuf >>= codeLen; numBits -= codeLen;
							if (counter & 256)
								break;

#if !TINFL_USE_64BIT_BITBUF
							if (numBits < 15) {
								bitBuf |= (((tinfl_bit_buf_t)MZ_READ_LE16(pInBufCur)) << numBits);
								pInBufCur += 2;
								numBits += 16;
							}
#endif
							if ((sym2 = decomp.tables[0].lookUp[bitBuf & (TINFL_FAST_LOOKUP_SIZE - 1)]) >= 0)
								codeLen = sym2 >> 9;
							else
							{
								codeLen = TINFL_FAST_LOOKUP_BITS;
								do {
									sym2 = decomp.tables[0].tree[~sym2 + ((bitBuf >> codeLen++) & 1)];
								} while (sym2 < 0);
							}
							bitBuf >>= codeLen; numBits -= codeLen;

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
						uint32_t extra_bits;
						TINFL_GET_BITS(25, extra_bits, numExtra);
						counter += extra_bits;
					}

					TINFL_HUFF_DECODE(26, dist, &decomp.tables[1]);
					numExtra = distExtra[dist]; dist = distBase[dist];
					if (numExtra)
					{
						uint32_t extra_bits;
						TINFL_GET_BITS(27, extra_bits, numExtra);
						dist += extra_bits;
					}

					distFromOutBufStart = pOutBufCur - pOutBufStart;
					if (dist > distFromOutBufStart)
					{
						TINFL_CR_RETURN_FOREVER(37, TINFL_STATUS_FAILED);
					}

					pSrc = pOutBufStart + ((distFromOutBufStart - dist) & outBufSizeMask);

					if (max(pOutBufCur, pSrc) + counter > pOutBufEnd)
					{
						while (counter--)
						{
							while (pOutBufCur >= pOutBufEnd) {
								TINFL_CR_RETURN(53, TINFL_STATUS_HAS_MORE_OUTPUT);
							}
							*pOutBufCur++ = pOutBufStart[(distFromOutBufStart++ - dist) & outBufSizeMask];
						}
						continue;
					}
#if MINIZ_USE_UNALIGNED_LOADS_AND_STORES
					else if (counter >= 9 && counter <= dist)
					{
						const uint8_t* pSrc_end = pSrc + (counter & ~7);
						do
						{
							((uint32_t*)pOutBufCur)[0] = ((const uint32_t*)pSrc)[0];
							((uint32_t*)pOutBufCur)[1] = ((const uint32_t*)pSrc)[1];
							pOutBufCur += 8;
						} while ((pSrc += 8) < pSrc_end);
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
#endif
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
			}
		} while (!(decomp.final & 1));

		TINFL_SKIP_BITS(32, numBits & 7);
		for (counter = 0; counter < 4; ++counter) {
			uint32_t s;
			if (numBits)
				TINFL_GET_BITS(41, s, 8);
			else
				TINFL_GET_BYTE(42, s);
			decomp.zAdler32 = (decomp.zAdler32 << 8) | s;
		}

		TINFL_CR_RETURN_FOREVER(34, TINFL_STATUS_DONE);
	}

common_exit:
	*pInBufSize = uint32_t(pInBufCur - pInBufNext);
	*pOutBufSize = uint32_t(pOutBufCur - pOutBufNext);

	if (status >= 0)
	{
		const uint8_t* ptr = pOutBufNext;
		size_t buf_len = *pOutBufSize;
		size_t block_len = buf_len % 5552;
		uint32_t i, s1 = decomp.checkAdler32 & 0xffff, s2 = decomp.checkAdler32 >> 16;
		while (buf_len)
		{
			for (i = 0; i + 7 < block_len; i += 8, ptr += 8)
			{
				/*
				s1 += ptr[0]; s2 += s1;
				s1 += ptr[1]; s2 += s1;
				s1 += ptr[2]; s2 += s1;
				s1 += ptr[3]; s2 += s1;
				s1 += ptr[4]; s2 += s1; 
				s1 += ptr[5]; s2 += s1; 
				s1 += ptr[6]; s2 += s1; 
				s1 += ptr[7]; s2 += s1;
				*/
				for (int8_t j = 0; j <= 7; j++)
				{
					s1 += ptr[j]; s2 += s1;
				}
				s1 = 0;
			}
			for (; i < block_len; ++i, s1 += *ptr++, s2 += s1);
			s1 %= 65521U;
			s2 %= 65521U;
			buf_len -= block_len;
			block_len = 5552;
		}
		decomp.checkAdler32 = (s2 << 16) + s1;
		if (status == TINFL_STATUS_DONE && decomp.checkAdler32 != decomp.zAdler32)
			status = TINFL_STATUS_ADLER32_MISMATCH;
	}

	if (status < 0)
		return MZ_DATA_ERROR;
	else if (status != TINFL_STATUS_DONE)
		return MZ_BUF_ERROR;
	return MZ_STREAM_END;
}
