#include "lodepng.h"

#include <limits.h> /* LONG_MAX */
#include <stdio.h> /* file handling */

#ifdef LODEPNG_COMPILE_ALLOCATORS
#include <stdlib.h> /* allocations */
#endif /* LODEPNG_COMPILE_ALLOCATORS */

#if defined(_MSC_VER) && (_MSC_VER >= 1310) /*Visual Studio: A few warning types are not desired here.*/
#pragma warning( disable : 4244 ) /*implicit conversions: not warned by gcc -Wall -Wextra and requires too much casts*/
#pragma warning( disable : 4996 ) /*VS does not like fopen, but fopen_s is not standard C so unusable here*/
#endif /*_MSC_VER */

const char* LODEPNG_VERSION_STRING = "20220717";

/*
This source file is divided into the following large parts. The code sections
with the "LODEPNG_COMPILE_" #defines divide this up further in an intermixed way.
-Tools for C and common code for PNG and Zlib
-C Code for Zlib (huffman, deflate, ...)
-C Code for PNG (file format chunks, adam7, PNG filters, color conversions, ...)
-The C++ wrapper around all of the above
*/

/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // Tools for C, and common code for PNG and Zlib.                       // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

/*The malloc, realloc and free functions defined here with "lodepng_" in front
of the name, so that you can easily change them to others related to your
platform if needed. Everything else in the code calls these. Pass
-DLODEPNG_NO_COMPILE_ALLOCATORS to the compiler, or comment out
#define LODEPNG_COMPILE_ALLOCATORS in the header, to disable the ones here and
define them in your own project's source files without needing to change
lodepng source code. Don't forget to remove "static" if you copypaste them
from here.*/

#ifdef LODEPNG_COMPILE_ALLOCATORS
static void* lodepng_malloc(size_t size) {
#ifdef LODEPNG_MAX_ALLOC
	if (size > LODEPNG_MAX_ALLOC) return 0;
#endif
	return malloc(size);
}

/* NOTE: when realloc returns NULL, it leaves the original memory untouched */
static void* lodepng_realloc(void* ptr, size_t new_size) {
#ifdef LODEPNG_MAX_ALLOC
	if (new_size > LODEPNG_MAX_ALLOC) return 0;
#endif
	return realloc(ptr, new_size);
}

static void lodepng_free(void* ptr) {
	free(ptr);
}
#else /*LODEPNG_COMPILE_ALLOCATORS*/
/* TODO: support giving additional void* payload to the custom allocators */
void* lodepng_malloc(size_t size);
void* lodepng_realloc(void* ptr, size_t new_size);
void lodepng_free(void* ptr);
#endif /*LODEPNG_COMPILE_ALLOCATORS*/

/* convince the compiler to inline a function, for use when this measurably improves performance */
/* inline is not available in C90, but use it when supported by the compiler */
#if (defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || (defined(__cplusplus) && (__cplusplus >= 199711L))
#define LODEPNG_INLINE inline
#else
#define LODEPNG_INLINE /* not available */
#endif

/* restrict is not available in C90, but use it when supported by the compiler */
#if (defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))) ||\
	(defined(_MSC_VER) && (_MSC_VER >= 1400)) || \
	(defined(__WATCOMC__) && (__WATCOMC__ >= 1250) && !defined(__cplusplus))
#define LODEPNG_RESTRICT __restrict
#else
#define LODEPNG_RESTRICT /* not available */
#endif

/* Replacements for C library functions such as memcpy and strlen, to support platforms
where a full C library is not available. The compiler can recognize them and compile
to something as fast. */

static void lodepng_memcpy(void* LODEPNG_RESTRICT dst,
	const void* LODEPNG_RESTRICT src, size_t size) {
	size_t i;
	for (i = 0; i < size; i++) ((char*)dst)[i] = ((const char*)src)[i];
}

static void lodepng_memset(void* LODEPNG_RESTRICT dst,
	int value, size_t num) {
	size_t i;
	for (i = 0; i < num; i++) ((char*)dst)[i] = (char)value;
}

/* does not check memory out of bounds, do not use on untrusted data */
static size_t lodepng_strlen(const char* a) {
	const char* orig = a;
	/* avoid warning about unused function in case of disabled COMPILE... macros */
	(void)(&lodepng_strlen);
	while (*a) a++;
	return (size_t)(a - orig);
}

#define LODEPNG_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define LODEPNG_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define LODEPNG_ABS(x) ((x) < 0 ? -(x) : (x))

#if defined(LODEPNG_COMPILE_PNG)
/* Safely check if adding two integers will overflow (no undefined
behavior, compiler removing the code, etc...) and output result. */
static int lodepng_addofl(size_t a, size_t b, size_t* result) {
	*result = a + b; /* Unsigned addition is well defined and safe in C90 */
	return *result < a;
}
#endif /*defined(LODEPNG_COMPILE_PNG) */

/*
Often in case of an error a value is assigned to a variable and then it breaks
out of a loop (to go to the cleanup phase of a function). This macro does that.
It makes the error handling code shorter and more readable.

Example: if(!uivector_resize(&lz77_encoded, datasize)) ERROR_BREAK(83);
*/
#define CERROR_BREAK(errorvar, code){\
  errorvar = code;\
  break;\
}

/*version of CERROR_BREAK that assumes the common case where the error variable is named "error"*/
#define ERROR_BREAK(code) CERROR_BREAK(error, code)

/*Set error var to the error code, and return it.*/
#define CERROR_RETURN_ERROR(errorvar, code){\
  errorvar = code;\
  return code;\
}

/*Try the code, if it returns error, also return the error.*/
#define CERROR_TRY_RETURN(call){\
  unsigned error = call;\
  if(error) return error;\
}

/*Set error var to the error code, and return from the void function.*/
#define CERROR_RETURN(errorvar, code){\
  errorvar = code;\
  return;\
}

/*
About uivector, ucvector and string:
-All of them wrap dynamic arrays or text strings in a similar way.
-LodePNG was originally written in C++. The vectors replace the std::vectors that were used in the C++ version.
-The string tools are made to avoid problems with compilers that declare things like strncat as deprecated.
-They're not used in the interface, only internally in this file as static functions.
-As with many other structs in this file, the init and cleanup functions serve as ctor and dtor.
*/

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_ENCODER
/*dynamic vector of unsigned ints*/
typedef struct uivector {
	unsigned* data;
	size_t size; /*size in number of unsigned longs*/
	size_t allocsize; /*allocated size in bytes*/
} uivector;

static void uivector_cleanup(void* p) {
	((uivector*)p)->size = ((uivector*)p)->allocsize = 0;
	lodepng_free(((uivector*)p)->data);
	((uivector*)p)->data = NULL;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_resize(uivector* p, size_t size) {
	size_t allocsize = size * sizeof(unsigned);
	if (allocsize > p->allocsize) {
		size_t newsize = allocsize + (p->allocsize >> 1u);
		void* data = lodepng_realloc(p->data, newsize);
		if (data) {
			p->allocsize = newsize;
			p->data = (unsigned*)data;
		}
		else return 0; /*error: not enough memory*/
	}
	p->size = size;
	return 1; /*success*/
}

static void uivector_init(uivector* p) {
	p->data = NULL;
	p->size = p->allocsize = 0;
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned uivector_push_back(uivector* p, unsigned c) {
	if (!uivector_resize(p, p->size + 1)) return 0;
	p->data[p->size - 1] = c;
	return 1;
}
#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_ZLIB*/

/* /////////////////////////////////////////////////////////////////////////// */

/*dynamic vector of unsigned chars*/
typedef struct ucvector {
	unsigned char* data;
	size_t size; /*used size*/
	size_t allocsize; /*allocated size*/
} ucvector;

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_reserve(ucvector* p, size_t size) {
	if (size > p->allocsize) {
		size_t newsize = size + (p->allocsize >> 1u);
		void* data = lodepng_realloc(p->data, newsize);
		if (data) {
			p->allocsize = newsize;
			p->data = (unsigned char*)data;
		}
		else return 0; /*error: not enough memory*/
	}
	return 1; /*success*/
}

/*returns 1 if success, 0 if failure ==> nothing done*/
static unsigned ucvector_resize(ucvector* p, size_t size) {
	p->size = size;
	return ucvector_reserve(p, size);
}

static ucvector ucvector_init(unsigned char* buffer, size_t size) {
	ucvector v;
	v.data = buffer;
	v.allocsize = v.size = size;
	return v;
}

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

/*free string pointer and set it to NULL*/
static void string_cleanup(char** out) {
	lodepng_free(*out);
	*out = NULL;
}

/*also appends null termination character*/
static char* alloc_string_sized(const char* in, size_t insize) {
	char* out = (char*)lodepng_malloc(insize + 1);
	if (out) {
		lodepng_memcpy(out, in, insize);
		out[insize] = 0;
	}
	return out;
}

/* dynamically allocates a new string with a copy of the null terminated input text */
static char* alloc_string(const char* in) {
	return alloc_string_sized(in, lodepng_strlen(in));
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
#endif /*LODEPNG_COMPILE_PNG*/

/* ////////////////////////////////////////////////////////////////////////// */

#if defined(LODEPNG_COMPILE_PNG)
static unsigned lodepng_read32bitInt(const unsigned char* buffer) {
	return (((unsigned)buffer[0] << 24u) | ((unsigned)buffer[1] << 16u) |
		((unsigned)buffer[2] << 8u) | (unsigned)buffer[3]);
}
#endif /*defined(LODEPNG_COMPILE_PNG)*/

#if defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)
/*buffer must have at least 4 allocated bytes available*/
static void lodepng_set32bitInt(unsigned char* buffer, unsigned value) {
	buffer[0] = (unsigned char)((value >> 24) & 0xff);
	buffer[1] = (unsigned char)((value >> 16) & 0xff);
	buffer[2] = (unsigned char)((value >> 8) & 0xff);
	buffer[3] = (unsigned char)((value) & 0xff);
}
#endif /*defined(LODEPNG_COMPILE_PNG) || defined(LODEPNG_COMPILE_ENCODER)*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / File IO                                                                / */
/* ////////////////////////////////////////////////////////////////////////// */


/*write given buffer to the file, overwriting the file, it doesn't append to it.*/
unsigned lodepng_save_file(const unsigned char* buffer, size_t buffersize, const char* filename) {
	FILE* file;
	file = fopen(filename, "wb");
	if (!file) return 79;
	fwrite(buffer, 1, buffersize, file);
	fclose(file);
	return 0;
}


/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of common code and tools. Begin of Zlib related code.            // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ZLIB
#ifdef LODEPNG_COMPILE_ENCODER

typedef struct {
	ucvector* data;
	unsigned char bp; /*ok to overflow, indicates bit pos inside byte*/
} LodePNGBitWriter;

static void LodePNGBitWriter_init(LodePNGBitWriter* writer, ucvector* data) {
	writer->data = data;
	writer->bp = 0;
}

/*TODO: this ignores potential out of memory errors*/
#define WRITEBIT(writer, bit){\
  /* append new byte */\
  if(((writer->bp) & 7u) == 0) {\
	if(!ucvector_resize(writer->data, writer->data->size + 1)) return;\
	writer->data->data[writer->data->size - 1] = 0;\
  }\
  (writer->data->data[writer->data->size - 1]) |= (bit << ((writer->bp) & 7u));\
  ++writer->bp;\
}

/* LSB of value is written first, and LSB of bytes is used first */
static void writeBits(LodePNGBitWriter* writer, unsigned value, size_t nbits) {
	if (nbits == 1) { /* compiler should statically compile this case if nbits == 1 */
		WRITEBIT(writer, value);
	}
	else {
		/* TODO: increase output size only once here rather than in each WRITEBIT */
		size_t i;
		for (i = 0; i != nbits; ++i) {
			WRITEBIT(writer, (unsigned char)((value >> i) & 1));
		}
	}
}

/* This one is to use for adding huffman symbol, the value bits are written MSB first */
static void writeBitsReversed(LodePNGBitWriter* writer, unsigned value, size_t nbits) {
	size_t i;
	for (i = 0; i != nbits; ++i) {
		/* TODO: increase output size only once here rather than in each WRITEBIT */
		WRITEBIT(writer, (unsigned char)((value >> (nbits - 1u - i)) & 1u));
	}
}
#endif /*LODEPNG_COMPILE_ENCODER*/

static unsigned reverseBits(unsigned bits, unsigned num) {
	/*TODO: implement faster lookup table based version when needed*/
	unsigned i, result = 0;
	for (i = 0; i < num; i++) result |= ((bits >> (num - i - 1u)) & 1u) << i;
	return result;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflate - Huffman                                                      / */
/* ////////////////////////////////////////////////////////////////////////// */

#define FIRST_LENGTH_CODE_INDEX 257
#define LAST_LENGTH_CODE_INDEX 285
/*256 literals, the end code, some length codes, and 2 unused codes*/
#define NUM_DEFLATE_CODE_SYMBOLS 288
/*the distance codes have their own symbols, 30 used, 2 unused*/
#define NUM_DISTANCE_SYMBOLS 32
/*the code length codes. 0-15: code lengths, 16: copy previous 3-6 times, 17: 3-10 zeros, 18: 11-138 zeros*/
#define NUM_CODE_LENGTH_CODES 19

/*the base lengths represented by codes 257-285*/
static const unsigned LENGTHBASE[29]
= { 3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
   67, 83, 99, 115, 131, 163, 195, 227, 258 };

/*the extra bits used by codes 257-285 (added to base length)*/
static const unsigned LENGTHEXTRA[29]
= { 0, 0, 0, 0, 0, 0, 0,  0,  1,  1,  1,  1,  2,  2,  2,  2,  3,  3,  3,  3,
	4,  4,  4,   4,   5,   5,   5,   5,   0 };

/*the base backwards distances (the bits of distance codes appear after length codes and use their own huffman tree)*/
static const unsigned DISTANCEBASE[30]
= { 1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
   769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577 };

/*the extra bits of backwards distances (added to base)*/
static const unsigned DISTANCEEXTRA[30]
= { 0, 0, 0, 0, 1, 1, 2,  2,  3,  3,  4,  4,  5,  5,   6,   6,   7,   7,   8,
	 8,    9,    9,   10,   10,   11,   11,   12,    12,    13,    13 };

/*the order in which "code length alphabet code lengths" are stored as specified by deflate, out of this the huffman
tree of the dynamic huffman tree lengths is generated*/
static const unsigned CLCL_ORDER[NUM_CODE_LENGTH_CODES]
= { 16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };

/* ////////////////////////////////////////////////////////////////////////// */

/*
Huffman tree struct, containing multiple representations of the tree
*/
typedef struct HuffmanTree {
	unsigned* codes; /*the huffman codes (bit patterns representing the symbols)*/
	unsigned* lengths; /*the lengths of the huffman codes*/
	unsigned maxbitlen; /*maximum number of bits a single code can get*/
	unsigned numcodes; /*number of symbols in the alphabet = number of codes*/
	/* for reading only */
	unsigned char* table_len; /*length of symbol from lookup table, or max length if secondary lookup needed*/
	unsigned short* table_value; /*value of symbol from lookup table, or pointer to secondary table if needed*/
} HuffmanTree;

static void HuffmanTree_init(HuffmanTree* tree) {
	tree->codes = 0;
	tree->lengths = 0;
	tree->table_len = 0;
	tree->table_value = 0;
}

static void HuffmanTree_cleanup(HuffmanTree* tree) {
	lodepng_free(tree->codes);
	lodepng_free(tree->lengths);
	lodepng_free(tree->table_len);
	lodepng_free(tree->table_value);
}

/* amount of bits for first huffman table lookup (aka root bits), see HuffmanTree_makeTable and huffmanDecodeSymbol.*/
/* values 8u and 9u work the fastest */
#define FIRSTBITS 9u

/* a symbol value too big to represent any valid symbol, to indicate reading disallowed huffman bits combination,
which is possible in case of only 0 or 1 present symbols. */
#define INVALIDSYMBOL 65535u

/* make table for huffman decoding */
static unsigned HuffmanTree_makeTable(HuffmanTree* tree) {
	static const unsigned headsize = 1u << FIRSTBITS; /*size of the first table*/
	static const unsigned mask = (1u << FIRSTBITS) /*headsize*/ - 1u;
	size_t i, numpresent, pointer, size; /*total table size*/
	unsigned* maxlens = (unsigned*)lodepng_malloc(headsize * sizeof(unsigned));
	if (!maxlens) return 83; /*alloc fail*/

	/* compute maxlens: max total bit length of symbols sharing prefix in the first table*/
	lodepng_memset(maxlens, 0, headsize * sizeof(*maxlens));
	for (i = 0; i < tree->numcodes; i++) {
		unsigned symbol = tree->codes[i];
		unsigned l = tree->lengths[i];
		unsigned index;
		if (l <= FIRSTBITS) continue; /*symbols that fit in first table don't increase secondary table size*/
		/*get the FIRSTBITS MSBs, the MSBs of the symbol are encoded first. See later comment about the reversing*/
		index = reverseBits(symbol >> (l - FIRSTBITS), FIRSTBITS);
		maxlens[index] = LODEPNG_MAX(maxlens[index], l);
	}
	/* compute total table size: size of first table plus all secondary tables for symbols longer than FIRSTBITS */
	size = headsize;
	for (i = 0; i < headsize; ++i) {
		unsigned l = maxlens[i];
		if (l > FIRSTBITS) size += (1u << (l - FIRSTBITS));
	}
	tree->table_len = (unsigned char*)lodepng_malloc(size * sizeof(*tree->table_len));
	tree->table_value = (unsigned short*)lodepng_malloc(size * sizeof(*tree->table_value));
	if (!tree->table_len || !tree->table_value) {
		lodepng_free(maxlens);
		/* freeing tree->table values is done at a higher scope */
		return 83; /*alloc fail*/
	}
	/*initialize with an invalid length to indicate unused entries*/
	for (i = 0; i < size; ++i) tree->table_len[i] = 16;

	/*fill in the first table for long symbols: max prefix size and pointer to secondary tables*/
	pointer = headsize;
	for (i = 0; i < headsize; ++i) {
		unsigned l = maxlens[i];
		if (l <= FIRSTBITS) continue;
		tree->table_len[i] = l;
		tree->table_value[i] = pointer;
		pointer += (1u << (l - FIRSTBITS));
	}
	lodepng_free(maxlens);

	/*fill in the first table for short symbols, or secondary table for long symbols*/
	numpresent = 0;
	for (i = 0; i < tree->numcodes; ++i) {
		unsigned l = tree->lengths[i];
		unsigned symbol, reverse;
		if (l == 0) continue;
		symbol = tree->codes[i]; /*the huffman bit pattern. i itself is the value.*/
		/*reverse bits, because the huffman bits are given in MSB first order but the bit reader reads LSB first*/
		reverse = reverseBits(symbol, l);
		numpresent++;

		if (l <= FIRSTBITS) {
			/*short symbol, fully in first table, replicated num times if l < FIRSTBITS*/
			unsigned num = 1u << (FIRSTBITS - l);
			unsigned j;
			for (j = 0; j < num; ++j) {
				/*bit reader will read the l bits of symbol first, the remaining FIRSTBITS - l bits go to the MSB's*/
				unsigned index = reverse | (j << l);
				if (tree->table_len[index] != 16) return 55; /*invalid tree: long symbol shares prefix with short symbol*/
				tree->table_len[index] = l;
				tree->table_value[index] = i;
			}
		}
		else {
			/*long symbol, shares prefix with other long symbols in first lookup table, needs second lookup*/
			/*the FIRSTBITS MSBs of the symbol are the first table index*/
			unsigned index = reverse & mask;
			unsigned maxlen = tree->table_len[index];
			/*log2 of secondary table length, should be >= l - FIRSTBITS*/
			unsigned tablelen = maxlen - FIRSTBITS;
			unsigned start = tree->table_value[index]; /*starting index in secondary table*/
			unsigned num = 1u << (tablelen - (l - FIRSTBITS)); /*amount of entries of this symbol in secondary table*/
			unsigned j;
			if (maxlen < l) return 55; /*invalid tree: long symbol shares prefix with short symbol*/
			for (j = 0; j < num; ++j) {
				unsigned reverse2 = reverse >> FIRSTBITS; /* l - FIRSTBITS bits */
				unsigned index2 = start + (reverse2 | (j << (l - FIRSTBITS)));
				tree->table_len[index2] = l;
				tree->table_value[index2] = i;
			}
		}
	}

	if (numpresent < 2) {
		/* In case of exactly 1 symbol, in theory the huffman symbol needs 0 bits,
		but deflate uses 1 bit instead. In case of 0 symbols, no symbols can
		appear at all, but such huffman tree could still exist (e.g. if distance
		codes are never used). In both cases, not all symbols of the table will be
		filled in. Fill them in with an invalid symbol value so returning them from
		huffmanDecodeSymbol will cause error. */
		for (i = 0; i < size; ++i) {
			if (tree->table_len[i] == 16) {
				/* As length, use a value smaller than FIRSTBITS for the head table,
				and a value larger than FIRSTBITS for the secondary table, to ensure
				valid behavior for advanceBits when reading this symbol. */
				tree->table_len[i] = (i < headsize) ? 1 : (FIRSTBITS + 1);
				tree->table_value[i] = INVALIDSYMBOL;
			}
		}
	}
	else {
		/* A good huffman tree has N * 2 - 1 nodes, of which N - 1 are internal nodes.
		If that is not the case (due to too long length codes), the table will not
		have been fully used, and this is an error (not all bit combinations can be
		decoded): an oversubscribed huffman tree, indicated by error 55. */
		for (i = 0; i < size; ++i) {
			if (tree->table_len[i] == 16) return 55;
		}
	}

	return 0;
}

/*
Second step for the ...makeFromLengths and ...makeFromFrequencies functions.
numcodes, lengths and maxbitlen must already be filled in correctly. return
value is error.
*/
static unsigned HuffmanTree_makeFromLengths2(HuffmanTree* tree) {
	unsigned* blcount;
	unsigned* nextcode;
	unsigned error = 0;
	unsigned bits, n;

	tree->codes = (unsigned*)lodepng_malloc(tree->numcodes * sizeof(unsigned));
	blcount = (unsigned*)lodepng_malloc((tree->maxbitlen + 1) * sizeof(unsigned));
	nextcode = (unsigned*)lodepng_malloc((tree->maxbitlen + 1) * sizeof(unsigned));
	if (!tree->codes || !blcount || !nextcode) error = 83; /*alloc fail*/

	if (!error) {
		for (n = 0; n != tree->maxbitlen + 1; n++) blcount[n] = nextcode[n] = 0;
		/*step 1: count number of instances of each code length*/
		for (bits = 0; bits != tree->numcodes; ++bits) ++blcount[tree->lengths[bits]];
		/*step 2: generate the nextcode values*/
		for (bits = 1; bits <= tree->maxbitlen; ++bits) {
			nextcode[bits] = (nextcode[bits - 1] + blcount[bits - 1]) << 1u;
		}
		/*step 3: generate all the codes*/
		for (n = 0; n != tree->numcodes; ++n) {
			if (tree->lengths[n] != 0) {
				tree->codes[n] = nextcode[tree->lengths[n]]++;
				/*remove superfluous bits from the code*/
				tree->codes[n] &= ((1u << tree->lengths[n]) - 1u);
			}
		}
	}

	lodepng_free(blcount);
	lodepng_free(nextcode);

	if (!error) error = HuffmanTree_makeTable(tree);
	return error;
}

/*
given the code lengths (as stored in the PNG file), generate the tree as defined
by Deflate. maxbitlen is the maximum bits that a code in the tree can have.
return value is error.
*/
static unsigned HuffmanTree_makeFromLengths(HuffmanTree* tree, const unsigned* bitlen,
	size_t numcodes, unsigned maxbitlen) {
	unsigned i;
	tree->lengths = (unsigned*)lodepng_malloc(numcodes * sizeof(unsigned));
	if (!tree->lengths) return 83; /*alloc fail*/
	for (i = 0; i != numcodes; ++i) tree->lengths[i] = bitlen[i];
	tree->numcodes = (unsigned)numcodes; /*number of symbols*/
	tree->maxbitlen = maxbitlen;
	return HuffmanTree_makeFromLengths2(tree);
}

#ifdef LODEPNG_COMPILE_ENCODER

/*BPM: Boundary Package Merge, see "A Fast and Space-Economical Algorithm for Length-Limited Coding",
Jyrki Katajainen, Alistair Moffat, Andrew Turpin, 1995.*/

/*chain node for boundary package merge*/
typedef struct BPMNode {
	int weight; /*the sum of all weights in this chain*/
	unsigned index; /*index of this leaf node (called "count" in the paper)*/
	struct BPMNode* tail; /*the next nodes in this chain (null if last)*/
	int in_use;
} BPMNode;

/*lists of chains*/
typedef struct BPMLists {
	/*memory pool*/
	unsigned memsize;
	BPMNode* memory;
	unsigned numfree;
	unsigned nextfree;
	BPMNode** freelist;
	/*two heads of lookahead chains per list*/
	unsigned listsize;
	BPMNode** chains0;
	BPMNode** chains1;
} BPMLists;

/*creates a new chain node with the given parameters, from the memory in the lists */
static BPMNode* bpmnode_create(BPMLists* lists, int weight, unsigned index, BPMNode* tail) {
	unsigned i;
	BPMNode* result;

	/*memory full, so garbage collect*/
	if (lists->nextfree >= lists->numfree) {
		/*mark only those that are in use*/
		for (i = 0; i != lists->memsize; ++i) lists->memory[i].in_use = 0;
		for (i = 0; i != lists->listsize; ++i) {
			BPMNode* node;
			for (node = lists->chains0[i]; node != 0; node = node->tail) node->in_use = 1;
			for (node = lists->chains1[i]; node != 0; node = node->tail) node->in_use = 1;
		}
		/*collect those that are free*/
		lists->numfree = 0;
		for (i = 0; i != lists->memsize; ++i) {
			if (!lists->memory[i].in_use) lists->freelist[lists->numfree++] = &lists->memory[i];
		}
		lists->nextfree = 0;
	}

	result = lists->freelist[lists->nextfree++];
	result->weight = weight;
	result->index = index;
	result->tail = tail;
	return result;
}

/*sort the leaves with stable mergesort*/
static void bpmnode_sort(BPMNode* leaves, size_t num) {
	BPMNode* mem = (BPMNode*)lodepng_malloc(sizeof(*leaves) * num);
	size_t width, counter = 0;
	for (width = 1; width < num; width *= 2) {
		BPMNode* a = (counter & 1) ? mem : leaves;
		BPMNode* b = (counter & 1) ? leaves : mem;
		size_t p;
		for (p = 0; p < num; p += 2 * width) {
			size_t q = (p + width > num) ? num : (p + width);
			size_t r = (p + 2 * width > num) ? num : (p + 2 * width);
			size_t i = p, j = q, k;
			for (k = p; k < r; k++) {
				if (i < q && (j >= r || a[i].weight <= a[j].weight)) b[k] = a[i++];
				else b[k] = a[j++];
			}
		}
		counter++;
	}
	if (counter & 1) lodepng_memcpy(leaves, mem, sizeof(*leaves) * num);
	lodepng_free(mem);
}

/*Boundary Package Merge step, numpresent is the amount of leaves, and c is the current chain.*/
static void boundaryPM(BPMLists* lists, BPMNode* leaves, size_t numpresent, int c, int num) {
	unsigned lastindex = lists->chains1[c]->index;

	if (c == 0) {
		if (lastindex >= numpresent) return;
		lists->chains0[c] = lists->chains1[c];
		lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, 0);
	}
	else {
		/*sum of the weights of the head nodes of the previous lookahead chains.*/
		int sum = lists->chains0[c - 1]->weight + lists->chains1[c - 1]->weight;
		lists->chains0[c] = lists->chains1[c];
		if (lastindex < numpresent && sum > leaves[lastindex].weight) {
			lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, lists->chains1[c]->tail);
			return;
		}
		lists->chains1[c] = bpmnode_create(lists, sum, lastindex, lists->chains1[c - 1]);
		/*in the end we are only interested in the chain of the last list, so no
		need to recurse if we're at the last one (this gives measurable speedup)*/
		if (num + 1 < (int)(2 * numpresent - 2)) {
			boundaryPM(lists, leaves, numpresent, c - 1, num);
			boundaryPM(lists, leaves, numpresent, c - 1, num);
		}
	}
}

unsigned lodepng_huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
	size_t numcodes, unsigned maxbitlen) {
	unsigned error = 0;
	unsigned i;
	size_t numpresent = 0; /*number of symbols with non-zero frequency*/
	BPMNode* leaves; /*the symbols, only those with > 0 frequency*/

	if (numcodes == 0) return 80; /*error: a tree of 0 symbols is not supposed to be made*/
	if ((1u << maxbitlen) < (unsigned)numcodes) return 80; /*error: represent all symbols*/

	leaves = (BPMNode*)lodepng_malloc(numcodes * sizeof(*leaves));
	if (!leaves) return 83; /*alloc fail*/

	for (i = 0; i != numcodes; ++i) {
		if (frequencies[i] > 0) {
			leaves[numpresent].weight = (int)frequencies[i];
			leaves[numpresent].index = i;
			++numpresent;
		}
	}

	lodepng_memset(lengths, 0, numcodes * sizeof(*lengths));

	/*ensure at least two present symbols. There should be at least one symbol
	according to RFC 1951 section 3.2.7. Some decoders incorrectly require two. To
	make these work as well ensure there are at least two symbols. The
	Package-Merge code below also doesn't work correctly if there's only one
	symbol, it'd give it the theoretical 0 bits but in practice zlib wants 1 bit*/
	if (numpresent == 0) {
		lengths[0] = lengths[1] = 1; /*note that for RFC 1951 section 3.2.7, only lengths[0] = 1 is needed*/
	}
	else if (numpresent == 1) {
		lengths[leaves[0].index] = 1;
		lengths[leaves[0].index == 0 ? 1 : 0] = 1;
	}
	else {
		BPMLists lists;
		BPMNode* node;

		bpmnode_sort(leaves, numpresent);

		lists.listsize = maxbitlen;
		lists.memsize = 2 * maxbitlen * (maxbitlen + 1);
		lists.nextfree = 0;
		lists.numfree = lists.memsize;
		lists.memory = (BPMNode*)lodepng_malloc(lists.memsize * sizeof(*lists.memory));
		lists.freelist = (BPMNode**)lodepng_malloc(lists.memsize * sizeof(BPMNode*));
		lists.chains0 = (BPMNode**)lodepng_malloc(lists.listsize * sizeof(BPMNode*));
		lists.chains1 = (BPMNode**)lodepng_malloc(lists.listsize * sizeof(BPMNode*));
		if (!lists.memory || !lists.freelist || !lists.chains0 || !lists.chains1) error = 83; /*alloc fail*/

		if (!error) {
			for (i = 0; i != lists.memsize; ++i) lists.freelist[i] = &lists.memory[i];

			bpmnode_create(&lists, leaves[0].weight, 1, 0);
			bpmnode_create(&lists, leaves[1].weight, 2, 0);

			for (i = 0; i != lists.listsize; ++i) {
				lists.chains0[i] = &lists.memory[0];
				lists.chains1[i] = &lists.memory[1];
			}

			/*each boundaryPM call adds one chain to the last list, and we need 2 * numpresent - 2 chains.*/
			for (i = 2; i != 2 * numpresent - 2; ++i) boundaryPM(&lists, leaves, numpresent, (int)maxbitlen - 1, (int)i);

			for (node = lists.chains1[maxbitlen - 1]; node; node = node->tail) {
				for (i = 0; i != node->index; ++i) ++lengths[leaves[i].index];
			}
		}

		lodepng_free(lists.memory);
		lodepng_free(lists.freelist);
		lodepng_free(lists.chains0);
		lodepng_free(lists.chains1);
	}

	lodepng_free(leaves);
	return error;
}

/*Create the Huffman tree given the symbol frequencies*/
static unsigned HuffmanTree_makeFromFrequencies(HuffmanTree* tree, const unsigned* frequencies,
	size_t mincodes, size_t numcodes, unsigned maxbitlen) {
	unsigned error = 0;
	while (!frequencies[numcodes - 1] && numcodes > mincodes) --numcodes; /*trim zeroes*/
	tree->lengths = (unsigned*)lodepng_malloc(numcodes * sizeof(unsigned));
	if (!tree->lengths) return 83; /*alloc fail*/
	tree->maxbitlen = maxbitlen;
	tree->numcodes = (unsigned)numcodes; /*number of symbols*/

	error = lodepng_huffman_code_lengths(tree->lengths, frequencies, numcodes, maxbitlen);
	if (!error) error = HuffmanTree_makeFromLengths2(tree);
	return error;
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*get the literal and length code tree of a deflated block with fixed tree, as per the deflate specification*/
static unsigned generateFixedLitLenTree(HuffmanTree* tree) {
	unsigned i, error = 0;
	unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DEFLATE_CODE_SYMBOLS * sizeof(unsigned));
	if (!bitlen) return 83; /*alloc fail*/

	/*288 possible codes: 0-255=literals, 256=endcode, 257-285=lengthcodes, 286-287=unused*/
	for (i = 0; i <= 143; ++i) bitlen[i] = 8;
	for (i = 144; i <= 255; ++i) bitlen[i] = 9;
	for (i = 256; i <= 279; ++i) bitlen[i] = 7;
	for (i = 280; i <= 287; ++i) bitlen[i] = 8;

	error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DEFLATE_CODE_SYMBOLS, 15);

	lodepng_free(bitlen);
	return error;
}

/*get the distance code tree of a deflated block with fixed tree, as specified in the deflate specification*/
static unsigned generateFixedDistanceTree(HuffmanTree* tree) {
	unsigned i, error = 0;
	unsigned* bitlen = (unsigned*)lodepng_malloc(NUM_DISTANCE_SYMBOLS * sizeof(unsigned));
	if (!bitlen) return 83; /*alloc fail*/

	/*there are 32 distance codes, but 30-31 are unused*/
	for (i = 0; i != NUM_DISTANCE_SYMBOLS; ++i) bitlen[i] = 5;
	error = HuffmanTree_makeFromLengths(tree, bitlen, NUM_DISTANCE_SYMBOLS, 15);

	lodepng_free(bitlen);
	return error;
}


#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / Deflator (Compressor)                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */

static const size_t MAX_SUPPORTED_DEFLATE_LENGTH = 258;

/*search the index in the array, that has the largest value smaller than or equal to the given value,
given array must be sorted (if no value is smaller, it returns the size of the given array)*/
static size_t searchCodeIndex(const unsigned* array, size_t array_size, size_t value) {
	/*binary search (only small gain over linear). TODO: use CPU log2 instruction for getting symbols instead*/
	size_t left = 1;
	size_t right = array_size - 1;

	while (left <= right) {
		size_t mid = (left + right) >> 1;
		if (array[mid] >= value) right = mid - 1;
		else left = mid + 1;
	}
	if (left >= array_size || array[left] > value) left--;
	return left;
}

static void addLengthDistance(uivector* values, size_t length, size_t distance) {
	/*values in encoded vector are those used by deflate:
	0-255: literal bytes
	256: end
	257-285: length/distance pair (length code, followed by extra length bits, distance code, extra distance bits)
	286-287: invalid*/

	unsigned length_code = (unsigned)searchCodeIndex(LENGTHBASE, 29, length);
	unsigned extra_length = (unsigned)(length - LENGTHBASE[length_code]);
	unsigned dist_code = (unsigned)searchCodeIndex(DISTANCEBASE, 30, distance);
	unsigned extra_distance = (unsigned)(distance - DISTANCEBASE[dist_code]);

	size_t pos = values->size;
	/*TODO: return error when this fails (out of memory)*/
	unsigned ok = uivector_resize(values, values->size + 4);
	if (ok) {
		values->data[pos + 0] = length_code + FIRST_LENGTH_CODE_INDEX;
		values->data[pos + 1] = extra_length;
		values->data[pos + 2] = dist_code;
		values->data[pos + 3] = extra_distance;
	}
}

/*3 bytes of data get encoded into two bytes. The hash cannot use more than 3
bytes as input because 3 is the minimum match length for deflate*/
static const unsigned HASH_NUM_VALUES = 65536;
static const unsigned HASH_BIT_MASK = 65535; /*HASH_NUM_VALUES - 1, but C90 does not like that as initializer*/

typedef struct Hash {
	int* head; /*hash value to head circular pos - can be outdated if went around window*/
	/*circular pos to prev circular pos*/
	unsigned short* chain;
	int* val; /*circular pos to hash value*/

	/*TODO: do this not only for zeros but for any repeated byte. However for PNG
	it's always going to be the zeros that dominate, so not important for PNG*/
	int* headz; /*similar to head, but for chainz*/
	unsigned short* chainz; /*those with same amount of zeros*/
	unsigned short* zeros; /*length of zeros streak, used as a second hash chain*/
} Hash;

static unsigned hash_init(Hash* hash, unsigned windowsize) {
	unsigned i;
	hash->head = (int*)lodepng_malloc(sizeof(int) * HASH_NUM_VALUES);
	hash->val = (int*)lodepng_malloc(sizeof(int) * windowsize);
	hash->chain = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

	hash->zeros = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);
	hash->headz = (int*)lodepng_malloc(sizeof(int) * (MAX_SUPPORTED_DEFLATE_LENGTH + 1));
	hash->chainz = (unsigned short*)lodepng_malloc(sizeof(unsigned short) * windowsize);

	if (!hash->head || !hash->chain || !hash->val || !hash->headz || !hash->chainz || !hash->zeros) {
		return 83; /*alloc fail*/
	}

	/*initialize hash table*/
	for (i = 0; i != HASH_NUM_VALUES; ++i) hash->head[i] = -1;
	for (i = 0; i != windowsize; ++i) hash->val[i] = -1;
	for (i = 0; i != windowsize; ++i) hash->chain[i] = i; /*same value as index indicates uninitialized*/

	for (i = 0; i <= MAX_SUPPORTED_DEFLATE_LENGTH; ++i) hash->headz[i] = -1;
	for (i = 0; i != windowsize; ++i) hash->chainz[i] = i; /*same value as index indicates uninitialized*/

	return 0;
}

static void hash_cleanup(Hash* hash) {
	lodepng_free(hash->head);
	lodepng_free(hash->val);
	lodepng_free(hash->chain);

	lodepng_free(hash->zeros);
	lodepng_free(hash->headz);
	lodepng_free(hash->chainz);
}



static unsigned getHash(const unsigned char* data, size_t size, size_t pos) {
	unsigned result = 0;
	if (pos + 2 < size) {
		/*A simple shift and xor hash is used. Since the data of PNGs is dominated
		by zeroes due to the filters, a better hash does not have a significant
		effect on speed in traversing the chain, and causes more time spend on
		calculating the hash.*/
		result ^= ((unsigned)data[pos + 0] << 0u);
		result ^= ((unsigned)data[pos + 1] << 4u);
		result ^= ((unsigned)data[pos + 2] << 8u);
	}
	else {
		size_t amount, i;
		if (pos >= size) return 0;
		amount = size - pos;
		for (i = 0; i != amount; ++i) result ^= ((unsigned)data[pos + i] << (i * 8u));
	}
	return result & HASH_BIT_MASK;
}

static unsigned countZeros(const unsigned char* data, size_t size, size_t pos) {
	const unsigned char* start = data + pos;
	const unsigned char* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
	if (end > data + size) end = data + size;
	data = start;
	while (data != end && *data == 0) ++data;
	/*subtracting two addresses returned as 32-bit number (max value is MAX_SUPPORTED_DEFLATE_LENGTH)*/
	return (unsigned)(data - start);
}

/*wpos = pos & (windowsize - 1)*/
static void updateHashChain(Hash* hash, size_t wpos, unsigned hashval, unsigned short numzeros) {
	hash->val[wpos] = (int)hashval;
	if (hash->head[hashval] != -1) hash->chain[wpos] = hash->head[hashval];
	hash->head[hashval] = (int)wpos;

	hash->zeros[wpos] = numzeros;
	if (hash->headz[numzeros] != -1) hash->chainz[wpos] = hash->headz[numzeros];
	hash->headz[numzeros] = (int)wpos;
}

/*
LZ77-encode the data. Return value is error code. The input are raw bytes, the output
is in the form of unsigned integers with codes representing for example literal bytes, or
length/distance pairs.
It uses a hash table technique to let it encode faster. When doing LZ77 encoding, a
sliding window (of windowsize) is used, and all past bytes in that window can be used as
the "dictionary". A brute force search through all possible distances would be slow, and
this hash technique is one out of several ways to speed this up.
*/
static unsigned encodeLZ77(uivector* out, Hash* hash,
	const unsigned char* in, size_t inpos, size_t insize, unsigned windowsize,
	unsigned minmatch, unsigned nicematch, unsigned lazymatching) {
	size_t pos;
	unsigned i, error = 0;
	/*for large window lengths, assume the user wants no compression loss. Otherwise, max hash chain length speedup.*/
	unsigned maxchainlength = windowsize >= 8192 ? windowsize : windowsize / 8u;
	unsigned maxlazymatch = windowsize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

	unsigned usezeros = 1; /*not sure if setting it to false for windowsize < 8192 is better or worse*/
	unsigned numzeros = 0;

	unsigned offset; /*the offset represents the distance in LZ77 terminology*/
	unsigned length;
	unsigned lazy = 0;
	unsigned lazylength = 0, lazyoffset = 0;
	unsigned hashval;
	unsigned current_offset, current_length;
	unsigned prev_offset;
	const unsigned char* lastptr, * foreptr, * backptr;
	unsigned hashpos;

	if (windowsize == 0 || windowsize > 32768) return 60; /*error: windowsize smaller/larger than allowed*/
	if ((windowsize & (windowsize - 1)) != 0) return 90; /*error: must be power of two*/

	if (nicematch > MAX_SUPPORTED_DEFLATE_LENGTH) nicematch = MAX_SUPPORTED_DEFLATE_LENGTH;

	for (pos = inpos; pos < insize; ++pos) {
		size_t wpos = pos & (windowsize - 1); /*position for in 'circular' hash buffers*/
		unsigned chainlength = 0;

		hashval = getHash(in, insize, pos);

		if (usezeros && hashval == 0) {
			if (numzeros == 0) numzeros = countZeros(in, insize, pos);
			else if (pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
		}
		else {
			numzeros = 0;
		}

		updateHashChain(hash, wpos, hashval, numzeros);

		/*the length and offset found for the current position*/
		length = 0;
		offset = 0;

		hashpos = hash->chain[wpos];

		lastptr = &in[insize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? insize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

		/*search for the longest string*/
		prev_offset = 0;
		for (;;) {
			if (chainlength++ >= maxchainlength) break;
			current_offset = (unsigned)(hashpos <= wpos ? wpos - hashpos : wpos - hashpos + windowsize);

			if (current_offset < prev_offset) break; /*stop when went completely around the circular buffer*/
			prev_offset = current_offset;
			if (current_offset > 0) {
				/*test the next characters*/
				foreptr = &in[pos];
				backptr = &in[pos - current_offset];

				/*common case in PNGs is lots of zeros. Quickly skip over them as a speedup*/
				if (numzeros >= 3) {
					unsigned skip = hash->zeros[hashpos];
					if (skip > numzeros) skip = numzeros;
					backptr += skip;
					foreptr += skip;
				}

				while (foreptr != lastptr && *backptr == *foreptr) /*maximum supported length by deflate is max length*/ {
					++backptr;
					++foreptr;
				}
				current_length = (unsigned)(foreptr - &in[pos]);

				if (current_length > length) {
					length = current_length; /*the longest length*/
					offset = current_offset; /*the offset that is related to this longest length*/
					/*jump out once a length of max length is found (speed gain). This also jumps
					out if length is MAX_SUPPORTED_DEFLATE_LENGTH*/
					if (current_length >= nicematch) break;
				}
			}

			if (hashpos == hash->chain[hashpos]) break;

			if (numzeros >= 3 && length > numzeros) {
				hashpos = hash->chainz[hashpos];
				if (hash->zeros[hashpos] != numzeros) break;
			}
			else {
				hashpos = hash->chain[hashpos];
				/*outdated hash value, happens if particular value was not encountered in whole last window*/
				if (hash->val[hashpos] != (int)hashval) break;
			}
		}

		if (lazymatching) {
			if (!lazy && length >= 3 && length <= maxlazymatch && length < MAX_SUPPORTED_DEFLATE_LENGTH) {
				lazy = 1;
				lazylength = length;
				lazyoffset = offset;
				continue; /*try the next byte*/
			}
			if (lazy) {
				lazy = 0;
				if (pos == 0) ERROR_BREAK(81);
				if (length > lazylength + 1) {
					/*push the previous character as literal*/
					if (!uivector_push_back(out, in[pos - 1])) ERROR_BREAK(83 /*alloc fail*/);
				}
				else {
					length = lazylength;
					offset = lazyoffset;
					hash->head[hashval] = -1; /*the same hashchain update will be done, this ensures no wrong alteration*/
					hash->headz[numzeros] = -1; /*idem*/
					--pos;
				}
			}
		}
		if (length >= 3 && offset > windowsize) ERROR_BREAK(86 /*too big (or overflown negative) offset*/);

		/*encode it as length/distance pair or literal value*/
		if (length < 3) /*only lengths of 3 or higher are supported as length/distance pair*/ {
			if (!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
		}
		else if (length < minmatch || (length == 3 && offset > 4096)) {
			/*compensate for the fact that longer offsets have more extra bits, a
			length of only 3 may be not worth it then*/
			if (!uivector_push_back(out, in[pos])) ERROR_BREAK(83 /*alloc fail*/);
		}
		else {
			addLengthDistance(out, length, offset);
			for (i = 1; i < length; ++i) {
				++pos;
				wpos = pos & (windowsize - 1);
				hashval = getHash(in, insize, pos);
				if (usezeros && hashval == 0) {
					if (numzeros == 0) numzeros = countZeros(in, insize, pos);
					else if (pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
				}
				else {
					numzeros = 0;
				}
				updateHashChain(hash, wpos, hashval, numzeros);
			}
		}
	} /*end of the loop through each character of input*/

	return error;
}

/* /////////////////////////////////////////////////////////////////////////// */

static unsigned deflateNoCompression(ucvector* out, const unsigned char* data, size_t datasize) {
	/*non compressed deflate block data: 1 bit BFINAL,2 bits BTYPE,(5 bits): it jumps to start of next byte,
	2 bytes LEN, 2 bytes NLEN, LEN bytes literal DATA*/

	size_t i, numdeflateblocks = (datasize + 65534u) / 65535u;
	unsigned datapos = 0;
	for (i = 0; i != numdeflateblocks; ++i) {
		unsigned BFINAL, BTYPE, LEN, NLEN;
		unsigned char firstbyte;
		size_t pos = out->size;

		BFINAL = (i == numdeflateblocks - 1);
		BTYPE = 0;

		LEN = 65535;
		if (datasize - datapos < 65535u) LEN = (unsigned)datasize - datapos;
		NLEN = 65535 - LEN;

		if (!ucvector_resize(out, out->size + LEN + 5)) return 83; /*alloc fail*/

		firstbyte = (unsigned char)(BFINAL + ((BTYPE & 1u) << 1u) + ((BTYPE & 2u) << 1u));
		out->data[pos + 0] = firstbyte;
		out->data[pos + 1] = (unsigned char)(LEN & 255);
		out->data[pos + 2] = (unsigned char)(LEN >> 8u);
		out->data[pos + 3] = (unsigned char)(NLEN & 255);
		out->data[pos + 4] = (unsigned char)(NLEN >> 8u);
		lodepng_memcpy(out->data + pos + 5, data + datapos, LEN);
		datapos += LEN;
	}

	return 0;
}

/*
write the lz77-encoded data, which has lit, len and dist codes, to compressed stream using huffman trees.
tree_ll: the tree for lit and len codes.
tree_d: the tree for distance codes.
*/
static void writeLZ77data(LodePNGBitWriter* writer, const uivector* lz77_encoded,
	const HuffmanTree* tree_ll, const HuffmanTree* tree_d) {
	size_t i = 0;
	for (i = 0; i != lz77_encoded->size; ++i) {
		unsigned val = lz77_encoded->data[i];
		writeBitsReversed(writer, tree_ll->codes[val], tree_ll->lengths[val]);
		if (val > 256) /*for a length code, 3 more things have to be added*/ {
			unsigned length_index = val - FIRST_LENGTH_CODE_INDEX;
			unsigned n_length_extra_bits = LENGTHEXTRA[length_index];
			unsigned length_extra_bits = lz77_encoded->data[++i];

			unsigned distance_code = lz77_encoded->data[++i];

			unsigned distance_index = distance_code;
			unsigned n_distance_extra_bits = DISTANCEEXTRA[distance_index];
			unsigned distance_extra_bits = lz77_encoded->data[++i];

			writeBits(writer, length_extra_bits, n_length_extra_bits);
			writeBitsReversed(writer, tree_d->codes[distance_code], tree_d->lengths[distance_code]);
			writeBits(writer, distance_extra_bits, n_distance_extra_bits);
		}
	}
}

/*Deflate for a block of type "dynamic", that is, with freely, optimally, created huffman trees*/
static unsigned deflateDynamic(LodePNGBitWriter* writer, Hash* hash,
	const unsigned char* data, size_t datapos, size_t dataend,
	const LodePNGCompressSettings* settings, unsigned final) {
	unsigned error = 0;

	/*
	A block is compressed as follows: The PNG data is lz77 encoded, resulting in
	literal bytes and length/distance pairs. This is then huffman compressed with
	two huffman trees. One huffman tree is used for the lit and len values ("ll"),
	another huffman tree is used for the dist values ("d"). These two trees are
	stored using their code lengths, and to compress even more these code lengths
	are also run-length encoded and huffman compressed. This gives a huffman tree
	of code lengths "cl". The code lengths used to describe this third tree are
	the code length code lengths ("clcl").
	*/

	/*The lz77 encoded data, represented with integers since there will also be length and distance codes in it*/
	uivector lz77_encoded;
	HuffmanTree tree_ll; /*tree for lit,len values*/
	HuffmanTree tree_d; /*tree for distance codes*/
	HuffmanTree tree_cl; /*tree for encoding the code lengths representing tree_ll and tree_d*/
	unsigned* frequencies_ll = 0; /*frequency of lit,len codes*/
	unsigned* frequencies_d = 0; /*frequency of dist codes*/
	unsigned* frequencies_cl = 0; /*frequency of code length codes*/
	unsigned* bitlen_lld = 0; /*lit,len,dist code lengths (int bits), literally (without repeat codes).*/
	unsigned* bitlen_lld_e = 0; /*bitlen_lld encoded with repeat codes (this is a rudimentary run length compression)*/
	size_t datasize = dataend - datapos;

	/*
	If we could call "bitlen_cl" the the code length code lengths ("clcl"), that is the bit lengths of codes to represent
	tree_cl in CLCL_ORDER, then due to the huffman compression of huffman tree representations ("two levels"), there are
	some analogies:
	bitlen_lld is to tree_cl what data is to tree_ll and tree_d.
	bitlen_lld_e is to bitlen_lld what lz77_encoded is to data.
	bitlen_cl is to bitlen_lld_e what bitlen_lld is to lz77_encoded.
	*/

	unsigned BFINAL = final;
	size_t i;
	size_t numcodes_ll, numcodes_d, numcodes_lld, numcodes_lld_e, numcodes_cl;
	unsigned HLIT, HDIST, HCLEN;

	uivector_init(&lz77_encoded);
	HuffmanTree_init(&tree_ll);
	HuffmanTree_init(&tree_d);
	HuffmanTree_init(&tree_cl);
	/* could fit on stack, but >1KB is on the larger side so allocate instead */
	frequencies_ll = (unsigned*)lodepng_malloc(286 * sizeof(*frequencies_ll));
	frequencies_d = (unsigned*)lodepng_malloc(30 * sizeof(*frequencies_d));
	frequencies_cl = (unsigned*)lodepng_malloc(NUM_CODE_LENGTH_CODES * sizeof(*frequencies_cl));

	if (!frequencies_ll || !frequencies_d || !frequencies_cl) error = 83; /*alloc fail*/

	/*This while loop never loops due to a break at the end, it is here to
	allow breaking out of it to the cleanup phase on error conditions.*/
	while (!error) {
		lodepng_memset(frequencies_ll, 0, 286 * sizeof(*frequencies_ll));
		lodepng_memset(frequencies_d, 0, 30 * sizeof(*frequencies_d));
		lodepng_memset(frequencies_cl, 0, NUM_CODE_LENGTH_CODES * sizeof(*frequencies_cl));

		if (settings->use_lz77) {
			error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
				settings->minmatch, settings->nicematch, settings->lazymatching);
			if (error) break;
		}
		else {
			if (!uivector_resize(&lz77_encoded, datasize)) ERROR_BREAK(83 /*alloc fail*/);
			for (i = datapos; i < dataend; ++i) lz77_encoded.data[i - datapos] = data[i]; /*no LZ77, but still will be Huffman compressed*/
		}

		/*Count the frequencies of lit, len and dist codes*/
		for (i = 0; i != lz77_encoded.size; ++i) {
			unsigned symbol = lz77_encoded.data[i];
			++frequencies_ll[symbol];
			if (symbol > 256) {
				unsigned dist = lz77_encoded.data[i + 2];
				++frequencies_d[dist];
				i += 3;
			}
		}
		frequencies_ll[256] = 1; /*there will be exactly 1 end code, at the end of the block*/

		/*Make both huffman trees, one for the lit and len codes, one for the dist codes*/
		error = HuffmanTree_makeFromFrequencies(&tree_ll, frequencies_ll, 257, 286, 15);
		if (error) break;
		/*2, not 1, is chosen for mincodes: some buggy PNG decoders require at least 2 symbols in the dist tree*/
		error = HuffmanTree_makeFromFrequencies(&tree_d, frequencies_d, 2, 30, 15);
		if (error) break;

		numcodes_ll = LODEPNG_MIN(tree_ll.numcodes, 286);
		numcodes_d = LODEPNG_MIN(tree_d.numcodes, 30);
		/*store the code lengths of both generated trees in bitlen_lld*/
		numcodes_lld = numcodes_ll + numcodes_d;
		bitlen_lld = (unsigned*)lodepng_malloc(numcodes_lld * sizeof(*bitlen_lld));
		/*numcodes_lld_e never needs more size than bitlen_lld*/
		bitlen_lld_e = (unsigned*)lodepng_malloc(numcodes_lld * sizeof(*bitlen_lld_e));
		if (!bitlen_lld || !bitlen_lld_e) ERROR_BREAK(83); /*alloc fail*/
		numcodes_lld_e = 0;

		for (i = 0; i != numcodes_ll; ++i) bitlen_lld[i] = tree_ll.lengths[i];
		for (i = 0; i != numcodes_d; ++i) bitlen_lld[numcodes_ll + i] = tree_d.lengths[i];

		/*run-length compress bitlen_ldd into bitlen_lld_e by using repeat codes 16 (copy length 3-6 times),
		17 (3-10 zeroes), 18 (11-138 zeroes)*/
		for (i = 0; i != numcodes_lld; ++i) {
			unsigned j = 0; /*amount of repetitions*/
			while (i + j + 1 < numcodes_lld && bitlen_lld[i + j + 1] == bitlen_lld[i]) ++j;

			if (bitlen_lld[i] == 0 && j >= 2) /*repeat code for zeroes*/ {
				++j; /*include the first zero*/
				if (j <= 10) /*repeat code 17 supports max 10 zeroes*/ {
					bitlen_lld_e[numcodes_lld_e++] = 17;
					bitlen_lld_e[numcodes_lld_e++] = j - 3;
				}
				else /*repeat code 18 supports max 138 zeroes*/ {
					if (j > 138) j = 138;
					bitlen_lld_e[numcodes_lld_e++] = 18;
					bitlen_lld_e[numcodes_lld_e++] = j - 11;
				}
				i += (j - 1);
			}
			else if (j >= 3) /*repeat code for value other than zero*/ {
				size_t k;
				unsigned num = j / 6u, rest = j % 6u;
				bitlen_lld_e[numcodes_lld_e++] = bitlen_lld[i];
				for (k = 0; k < num; ++k) {
					bitlen_lld_e[numcodes_lld_e++] = 16;
					bitlen_lld_e[numcodes_lld_e++] = 6 - 3;
				}
				if (rest >= 3) {
					bitlen_lld_e[numcodes_lld_e++] = 16;
					bitlen_lld_e[numcodes_lld_e++] = rest - 3;
				}
				else j -= rest;
				i += j;
			}
			else /*too short to benefit from repeat code*/ {
				bitlen_lld_e[numcodes_lld_e++] = bitlen_lld[i];
			}
		}

		/*generate tree_cl, the huffmantree of huffmantrees*/
		for (i = 0; i != numcodes_lld_e; ++i) {
			++frequencies_cl[bitlen_lld_e[i]];
			/*after a repeat code come the bits that specify the number of repetitions,
			those don't need to be in the frequencies_cl calculation*/
			if (bitlen_lld_e[i] >= 16) ++i;
		}

		error = HuffmanTree_makeFromFrequencies(&tree_cl, frequencies_cl,
			NUM_CODE_LENGTH_CODES, NUM_CODE_LENGTH_CODES, 7);
		if (error) break;

		/*compute amount of code-length-code-lengths to output*/
		numcodes_cl = NUM_CODE_LENGTH_CODES;
		/*trim zeros at the end (using CLCL_ORDER), but minimum size must be 4 (see HCLEN below)*/
		while (numcodes_cl > 4u && tree_cl.lengths[CLCL_ORDER[numcodes_cl - 1u]] == 0) {
			numcodes_cl--;
		}

		/*
		Write everything into the output

		After the BFINAL and BTYPE, the dynamic block consists out of the following:
		- 5 bits HLIT, 5 bits HDIST, 4 bits HCLEN
		- (HCLEN+4)*3 bits code lengths of code length alphabet
		- HLIT + 257 code lengths of lit/length alphabet (encoded using the code length
		  alphabet, + possible repetition codes 16, 17, 18)
		- HDIST + 1 code lengths of distance alphabet (encoded using the code length
		  alphabet, + possible repetition codes 16, 17, 18)
		- compressed data
		- 256 (end code)
		*/

		/*Write block type*/
		writeBits(writer, BFINAL, 1);
		writeBits(writer, 0, 1); /*first bit of BTYPE "dynamic"*/
		writeBits(writer, 1, 1); /*second bit of BTYPE "dynamic"*/

		/*write the HLIT, HDIST and HCLEN values*/
		/*all three sizes take trimmed ending zeroes into account, done either by HuffmanTree_makeFromFrequencies
		or in the loop for numcodes_cl above, which saves space. */
		HLIT = (unsigned)(numcodes_ll - 257);
		HDIST = (unsigned)(numcodes_d - 1);
		HCLEN = (unsigned)(numcodes_cl - 4);
		writeBits(writer, HLIT, 5);
		writeBits(writer, HDIST, 5);
		writeBits(writer, HCLEN, 4);

		/*write the code lengths of the code length alphabet ("bitlen_cl")*/
		for (i = 0; i != numcodes_cl; ++i) writeBits(writer, tree_cl.lengths[CLCL_ORDER[i]], 3);

		/*write the lengths of the lit/len AND the dist alphabet*/
		for (i = 0; i != numcodes_lld_e; ++i) {
			writeBitsReversed(writer, tree_cl.codes[bitlen_lld_e[i]], tree_cl.lengths[bitlen_lld_e[i]]);
			/*extra bits of repeat codes*/
			if (bitlen_lld_e[i] == 16) writeBits(writer, bitlen_lld_e[++i], 2);
			else if (bitlen_lld_e[i] == 17) writeBits(writer, bitlen_lld_e[++i], 3);
			else if (bitlen_lld_e[i] == 18) writeBits(writer, bitlen_lld_e[++i], 7);
		}

		/*write the compressed data symbols*/
		writeLZ77data(writer, &lz77_encoded, &tree_ll, &tree_d);
		/*error: the length of the end code 256 must be larger than 0*/
		if (tree_ll.lengths[256] == 0) ERROR_BREAK(64);

		/*write the end code*/
		writeBitsReversed(writer, tree_ll.codes[256], tree_ll.lengths[256]);

		break; /*end of error-while*/
	}

	/*cleanup*/
	uivector_cleanup(&lz77_encoded);
	HuffmanTree_cleanup(&tree_ll);
	HuffmanTree_cleanup(&tree_d);
	HuffmanTree_cleanup(&tree_cl);
	lodepng_free(frequencies_ll);
	lodepng_free(frequencies_d);
	lodepng_free(frequencies_cl);
	lodepng_free(bitlen_lld);
	lodepng_free(bitlen_lld_e);

	return error;
}

static unsigned deflateFixed(LodePNGBitWriter* writer, Hash* hash,
	const unsigned char* data,
	size_t datapos, size_t dataend,
	const LodePNGCompressSettings* settings, unsigned final) {
	HuffmanTree tree_ll; /*tree for literal values and length codes*/
	HuffmanTree tree_d; /*tree for distance codes*/

	unsigned BFINAL = final;
	unsigned error = 0;
	size_t i;

	HuffmanTree_init(&tree_ll);
	HuffmanTree_init(&tree_d);

	error = generateFixedLitLenTree(&tree_ll);
	if (!error) error = generateFixedDistanceTree(&tree_d);

	if (!error) {
		writeBits(writer, BFINAL, 1);
		writeBits(writer, 1, 1); /*first bit of BTYPE*/
		writeBits(writer, 0, 1); /*second bit of BTYPE*/

		if (settings->use_lz77) /*LZ77 encoded*/ {
			uivector lz77_encoded;
			uivector_init(&lz77_encoded);
			error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
				settings->minmatch, settings->nicematch, settings->lazymatching);
			if (!error) writeLZ77data(writer, &lz77_encoded, &tree_ll, &tree_d);
			uivector_cleanup(&lz77_encoded);
		}
		else /*no LZ77, but still will be Huffman compressed*/ {
			for (i = datapos; i < dataend; ++i) {
				writeBitsReversed(writer, tree_ll.codes[data[i]], tree_ll.lengths[data[i]]);
			}
		}
		/*add END code*/
		if (!error) writeBitsReversed(writer, tree_ll.codes[256], tree_ll.lengths[256]);
	}

	/*cleanup*/
	HuffmanTree_cleanup(&tree_ll);
	HuffmanTree_cleanup(&tree_d);

	return error;
}

static unsigned lodepng_deflatev(ucvector* out, const unsigned char* in, size_t insize,
	const LodePNGCompressSettings* settings) {
	unsigned error = 0;
	size_t i, blocksize, numdeflateblocks;
	Hash hash;
	LodePNGBitWriter writer;

	LodePNGBitWriter_init(&writer, out);

	if (settings->btype > 2) return 61;
	else if (settings->btype == 0) return deflateNoCompression(out, in, insize);
	else if (settings->btype == 1) blocksize = insize;
	else /*if(settings->btype == 2)*/ {
		/*on PNGs, deflate blocks of 65-262k seem to give most dense encoding*/
		blocksize = insize / 8u + 8;
		if (blocksize < 65536) blocksize = 65536;
		if (blocksize > 262144) blocksize = 262144;
	}

	numdeflateblocks = (insize + blocksize - 1) / blocksize;
	if (numdeflateblocks == 0) numdeflateblocks = 1;

	error = hash_init(&hash, settings->windowsize);

	if (!error) {
		for (i = 0; i != numdeflateblocks && !error; ++i) {
			unsigned final = (i == numdeflateblocks - 1);
			size_t start = i * blocksize;
			size_t end = start + blocksize;
			if (end > insize) end = insize;

			if (settings->btype == 1) error = deflateFixed(&writer, &hash, in, start, end, settings, final);
			else if (settings->btype == 2) error = deflateDynamic(&writer, &hash, in, start, end, settings, final);
		}
	}

	hash_cleanup(&hash);

	return error;
}

unsigned lodepng_deflate(unsigned char** out, size_t* outsize,
	const unsigned char* in, size_t insize,
	const LodePNGCompressSettings* settings) {
	ucvector v = ucvector_init(*out, *outsize);
	unsigned error = lodepng_deflatev(&v, in, insize, settings);
	*out = v.data;
	*outsize = v.size;
	return error;
}

static unsigned deflate(unsigned char** out, size_t* outsize,
	const unsigned char* in, size_t insize,
	const LodePNGCompressSettings* settings) {
	if (settings->custom_deflate) {
		unsigned error = settings->custom_deflate(out, outsize, in, insize, settings);
		/*the custom deflate is allowed to have its own error codes, however, we translate it to code 111*/
		return error ? 111 : 0;
	}
	else {
		return lodepng_deflate(out, outsize, in, insize, settings);
	}
}

#endif /*LODEPNG_COMPILE_ENCODER*/

/* ////////////////////////////////////////////////////////////////////////// */
/* / Adler32                                                                / */
/* ////////////////////////////////////////////////////////////////////////// */

static unsigned update_adler32(unsigned adler, const unsigned char* data, unsigned len) {
	unsigned s1 = adler & 0xffffu;
	unsigned s2 = (adler >> 16u) & 0xffffu;

	while (len != 0u) {
		unsigned i;
		/*at least 5552 sums can be done before the sums overflow, saving a lot of module divisions*/
		unsigned amount = len > 5552u ? 5552u : len;
		len -= amount;
		for (i = 0; i != amount; ++i) {
			s1 += (*data++);
			s2 += s1;
		}
		s1 %= 65521u;
		s2 %= 65521u;
	}

	return (s2 << 16u) | s1;
}

/*Return the adler32 of the bytes data[0..len-1]*/
static unsigned adler32(const unsigned char* data, unsigned len) {
	return update_adler32(1u, data, len);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Zlib                                                                   / */
/* ////////////////////////////////////////////////////////////////////////// */


#ifdef LODEPNG_COMPILE_ENCODER

unsigned lodepng_zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGCompressSettings* settings) {
	size_t i;
	unsigned error;
	unsigned char* deflatedata = 0;
	size_t deflatesize = 0;

	error = deflate(&deflatedata, &deflatesize, in, insize, settings);

	*out = NULL;
	*outsize = 0;
	if (!error) {
		*outsize = deflatesize + 6;
		*out = (unsigned char*)lodepng_malloc(*outsize);
		if (!*out) error = 83; /*alloc fail*/
	}

	if (!error) {
		unsigned ADLER32 = adler32(in, (unsigned)insize);
		/*zlib data: 1 byte CMF (CM+CINFO), 1 byte FLG, deflate data, 4 byte ADLER32 checksum of the Decompressed data*/
		unsigned CMF = 120; /*0b01111000: CM 8, CINFO 7. With CINFO 7, any window size up to 32768 can be used.*/
		unsigned FLEVEL = 0;
		unsigned FDICT = 0;
		unsigned CMFFLG = 256 * CMF + FDICT * 32 + FLEVEL * 64;
		unsigned FCHECK = 31 - CMFFLG % 31;
		CMFFLG += FCHECK;

		(*out)[0] = (unsigned char)(CMFFLG >> 8);
		(*out)[1] = (unsigned char)(CMFFLG & 255);
		for (i = 0; i != deflatesize; ++i) (*out)[i + 2] = deflatedata[i];
		lodepng_set32bitInt(&(*out)[*outsize - 4], ADLER32);
	}

	lodepng_free(deflatedata);
	return error;
}

/* compress using the default or custom zlib function */
static unsigned zlib_compress(unsigned char** out, size_t* outsize, const unsigned char* in,
	size_t insize, const LodePNGCompressSettings* settings) {
	if (settings->custom_zlib) {
		unsigned error = settings->custom_zlib(out, outsize, in, insize, settings);
		/*the custom zlib is allowed to have its own error codes, however, we translate it to code 111*/
		return error ? 111 : 0;
	}
	else {
		return lodepng_zlib_compress(out, outsize, in, insize, settings);
	}
}

#endif /*LODEPNG_COMPILE_ENCODER*/

#else /*no LODEPNG_COMPILE_ZLIB*/

#endif /*LODEPNG_COMPILE_ZLIB*/

/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_ENCODER

/*this is a good tradeoff between speed and compression ratio*/
#define DEFAULT_WINDOWSIZE 2048

void lodepng_compress_settings_init(LodePNGCompressSettings* settings) {
	/*compress with dynamic huffman tree (not in the mathematical sense, just not the predefined one)*/
	settings->btype = 2;
	settings->use_lz77 = 1;
	settings->windowsize = DEFAULT_WINDOWSIZE;
	settings->minmatch = 3;
	settings->nicematch = 128;
	settings->lazymatching = 1;

	settings->custom_zlib = 0;
	settings->custom_deflate = 0;
	settings->custom_context = 0;
}

const LodePNGCompressSettings lodepng_default_compress_settings = { 2, 1, DEFAULT_WINDOWSIZE, 3, 128, 1, 0, 0, 0 };


#endif /*LODEPNG_COMPILE_ENCODER*/


/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // End of Zlib related code. Begin of PNG related code.                 // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_PNG

/* ////////////////////////////////////////////////////////////////////////// */
/* / CRC32                                                                  / */
/* ////////////////////////////////////////////////////////////////////////// */


#ifdef LODEPNG_COMPILE_CRC
/* CRC polynomial: 0xedb88320 */
static unsigned lodepng_crc32_table[256] = {
		   0u, 1996959894u, 3993919788u, 2567524794u,  124634137u, 1886057615u, 3915621685u, 2657392035u,
   249268274u, 2044508324u, 3772115230u, 2547177864u,  162941995u, 2125561021u, 3887607047u, 2428444049u,
   498536548u, 1789927666u, 4089016648u, 2227061214u,  450548861u, 1843258603u, 4107580753u, 2211677639u,
   325883990u, 1684777152u, 4251122042u, 2321926636u,  335633487u, 1661365465u, 4195302755u, 2366115317u,
   997073096u, 1281953886u, 3579855332u, 2724688242u, 1006888145u, 1258607687u, 3524101629u, 2768942443u,
   901097722u, 1119000684u, 3686517206u, 2898065728u,  853044451u, 1172266101u, 3705015759u, 2882616665u,
   651767980u, 1373503546u, 3369554304u, 3218104598u,  565507253u, 1454621731u, 3485111705u, 3099436303u,
   671266974u, 1594198024u, 3322730930u, 2970347812u,  795835527u, 1483230225u, 3244367275u, 3060149565u,
  1994146192u,   31158534u, 2563907772u, 4023717930u, 1907459465u,  112637215u, 2680153253u, 3904427059u,
  2013776290u,  251722036u, 2517215374u, 3775830040u, 2137656763u,  141376813u, 2439277719u, 3865271297u,
  1802195444u,  476864866u, 2238001368u, 4066508878u, 1812370925u,  453092731u, 2181625025u, 4111451223u,
  1706088902u,  314042704u, 2344532202u, 4240017532u, 1658658271u,  366619977u, 2362670323u, 4224994405u,
  1303535960u,  984961486u, 2747007092u, 3569037538u, 1256170817u, 1037604311u, 2765210733u, 3554079995u,
  1131014506u,  879679996u, 2909243462u, 3663771856u, 1141124467u,  855842277u, 2852801631u, 3708648649u,
  1342533948u,  654459306u, 3188396048u, 3373015174u, 1466479909u,  544179635u, 3110523913u, 3462522015u,
  1591671054u,  702138776u, 2966460450u, 3352799412u, 1504918807u,  783551873u, 3082640443u, 3233442989u,
  3988292384u, 2596254646u,   62317068u, 1957810842u, 3939845945u, 2647816111u,   81470997u, 1943803523u,
  3814918930u, 2489596804u,  225274430u, 2053790376u, 3826175755u, 2466906013u,  167816743u, 2097651377u,
  4027552580u, 2265490386u,  503444072u, 1762050814u, 4150417245u, 2154129355u,  426522225u, 1852507879u,
  4275313526u, 2312317920u,  282753626u, 1742555852u, 4189708143u, 2394877945u,  397917763u, 1622183637u,
  3604390888u, 2714866558u,  953729732u, 1340076626u, 3518719985u, 2797360999u, 1068828381u, 1219638859u,
  3624741850u, 2936675148u,  906185462u, 1090812512u, 3747672003u, 2825379669u,  829329135u, 1181335161u,
  3412177804u, 3160834842u,  628085408u, 1382605366u, 3423369109u, 3138078467u,  570562233u, 1426400815u,
  3317316542u, 2998733608u,  733239954u, 1555261956u, 3268935591u, 3050360625u,  752459403u, 1541320221u,
  2607071920u, 3965973030u, 1969922972u,   40735498u, 2617837225u, 3943577151u, 1913087877u,   83908371u,
  2512341634u, 3803740692u, 2075208622u,  213261112u, 2463272603u, 3855990285u, 2094854071u,  198958881u,
  2262029012u, 4057260610u, 1759359992u,  534414190u, 2176718541u, 4139329115u, 1873836001u,  414664567u,
  2282248934u, 4279200368u, 1711684554u,  285281116u, 2405801727u, 4167216745u, 1634467795u,  376229701u,
  2685067896u, 3608007406u, 1308918612u,  956543938u, 2808555105u, 3495958263u, 1231636301u, 1047427035u,
  2932959818u, 3654703836u, 1088359270u,  936918000u, 2847714899u, 3736837829u, 1202900863u,  817233897u,
  3183342108u, 3401237130u, 1404277552u,  615818150u, 3134207493u, 3453421203u, 1423857449u,  601450431u,
  3009837614u, 3294710456u, 1567103746u,  711928724u, 3020668471u, 3272380065u, 1510334235u,  755167117u
};

/*Return the CRC of the bytes buf[0..len-1].*/
unsigned lodepng_crc32(const unsigned char* data, size_t length) {
	unsigned r = 0xffffffffu;
	size_t i;
	for (i = 0; i < length; ++i) {
		r = lodepng_crc32_table[(r ^ data[i]) & 0xffu] ^ (r >> 8u);
	}
	return r ^ 0xffffffffu;
}
#else /* LODEPNG_COMPILE_CRC */
/*in this case, the function is only declared here, and must be defined externally
so that it will be linked in*/
unsigned lodepng_crc32(const unsigned char* data, size_t length);
#endif /* LODEPNG_COMPILE_CRC */

/* ////////////////////////////////////////////////////////////////////////// */
/* / Reading and writing PNG color channel bits                             / */
/* ////////////////////////////////////////////////////////////////////////// */

/* The color channel bits of less-than-8-bit pixels are read with the MSB of bytes first,
so LodePNGBitWriter and LodePNGBitReader can't be used for those. */

static unsigned char readBitFromReversedStream(size_t* bitpointer, const unsigned char* bitstream) {
	unsigned char result = (unsigned char)((bitstream[(*bitpointer) >> 3] >> (7 - ((*bitpointer) & 0x7))) & 1);
	++(*bitpointer);
	return result;
}

/* TODO: make this faster */
static unsigned readBitsFromReversedStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits) {
	unsigned result = 0;
	size_t i;
	for (i = 0; i < nbits; ++i) {
		result <<= 1u;
		result |= (unsigned)readBitFromReversedStream(bitpointer, bitstream);
	}
	return result;
}

static void setBitOfReversedStream(size_t* bitpointer, unsigned char* bitstream, unsigned char bit) {
	/*the current bit in bitstream may be 0 or 1 for this to work*/
	if (bit == 0) bitstream[(*bitpointer) >> 3u] &= (unsigned char)(~(1u << (7u - ((*bitpointer) & 7u))));
	else         bitstream[(*bitpointer) >> 3u] |= (1u << (7u - ((*bitpointer) & 7u)));
	++(*bitpointer);
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG chunks                                                             / */
/* ////////////////////////////////////////////////////////////////////////// */

unsigned lodepng_chunk_length(const unsigned char* chunk) {
	return lodepng_read32bitInt(chunk);
}

void lodepng_chunk_type(char type[5], const unsigned char* chunk) {
	unsigned i;
	for (i = 0; i != 4; ++i) type[i] = (char)chunk[4 + i];
	type[4] = 0; /*null termination char*/
}

unsigned char lodepng_chunk_type_equals(const unsigned char* chunk, const char* type) {
	if (lodepng_strlen(type) != 4) return 0;
	return (chunk[4] == type[0] && chunk[5] == type[1] && chunk[6] == type[2] && chunk[7] == type[3]);
}

unsigned char lodepng_chunk_ancillary(const unsigned char* chunk) {
	return((chunk[4] & 32) != 0);
}

unsigned char lodepng_chunk_private(const unsigned char* chunk) {
	return((chunk[6] & 32) != 0);
}

unsigned char lodepng_chunk_safetocopy(const unsigned char* chunk) {
	return((chunk[7] & 32) != 0);
}

unsigned char* lodepng_chunk_data(unsigned char* chunk) {
	return &chunk[8];
}

const unsigned char* lodepng_chunk_data_const(const unsigned char* chunk) {
	return &chunk[8];
}

unsigned lodepng_chunk_check_crc(const unsigned char* chunk) {
	unsigned length = lodepng_chunk_length(chunk);
	unsigned CRC = lodepng_read32bitInt(&chunk[length + 8]);
	/*the CRC is taken of the data and the 4 chunk type letters, not the length*/
	unsigned checksum = lodepng_crc32(&chunk[4], length + 4);
	if (CRC != checksum) return 1;
	else return 0;
}

void lodepng_chunk_generate_crc(unsigned char* chunk) {
	unsigned length = lodepng_chunk_length(chunk);
	unsigned CRC = lodepng_crc32(&chunk[4], length + 4);
	lodepng_set32bitInt(chunk + 8 + length, CRC);
}

unsigned char* lodepng_chunk_next(unsigned char* chunk, unsigned char* end) {
	size_t available_size = (size_t)(end - chunk);
	if (chunk >= end || available_size < 12) return end; /*too small to contain a chunk*/
	if (chunk[0] == 0x89 && chunk[1] == 0x50 && chunk[2] == 0x4e && chunk[3] == 0x47
		&& chunk[4] == 0x0d && chunk[5] == 0x0a && chunk[6] == 0x1a && chunk[7] == 0x0a) {
		/* Is PNG magic header at start of PNG file. Jump to first actual chunk. */
		return chunk + 8;
	}
	else {
		size_t total_chunk_length;
		if (lodepng_addofl(lodepng_chunk_length(chunk), 12, &total_chunk_length)) return end;
		if (total_chunk_length > available_size) return end; /*outside of range*/
		return chunk + total_chunk_length;
	}
}

const unsigned char* lodepng_chunk_next_const(const unsigned char* chunk, const unsigned char* end) {
	size_t available_size = (size_t)(end - chunk);
	if (chunk >= end || available_size < 12) return end; /*too small to contain a chunk*/
	if (chunk[0] == 0x89 && chunk[1] == 0x50 && chunk[2] == 0x4e && chunk[3] == 0x47
		&& chunk[4] == 0x0d && chunk[5] == 0x0a && chunk[6] == 0x1a && chunk[7] == 0x0a) {
		/* Is PNG magic header at start of PNG file. Jump to first actual chunk. */
		return chunk + 8;
	}
	else {
		size_t total_chunk_length;
		if (lodepng_addofl(lodepng_chunk_length(chunk), 12, &total_chunk_length)) return end;
		if (total_chunk_length > available_size) return end; /*outside of range*/
		return chunk + total_chunk_length;
	}
}

unsigned char* lodepng_chunk_find(unsigned char* chunk, unsigned char* end, const char type[5]) {
	for (;;) {
		if (chunk >= end || end - chunk < 12) return 0; /* past file end: chunk + 12 > end */
		if (lodepng_chunk_type_equals(chunk, type)) return chunk;
		chunk = lodepng_chunk_next(chunk, end);
	}
}

const unsigned char* lodepng_chunk_find_const(const unsigned char* chunk, const unsigned char* end, const char type[5]) {
	for (;;) {
		if (chunk >= end || end - chunk < 12) return 0; /* past file end: chunk + 12 > end */
		if (lodepng_chunk_type_equals(chunk, type)) return chunk;
		chunk = lodepng_chunk_next_const(chunk, end);
	}
}

unsigned lodepng_chunk_append(unsigned char** out, size_t* outsize, const unsigned char* chunk) {
	unsigned i;
	size_t total_chunk_length, new_length;
	unsigned char* chunk_start, * new_buffer;

	if (lodepng_addofl(lodepng_chunk_length(chunk), 12, &total_chunk_length)) return 77;
	if (lodepng_addofl(*outsize, total_chunk_length, &new_length)) return 77;

	new_buffer = (unsigned char*)lodepng_realloc(*out, new_length);
	if (!new_buffer) return 83; /*alloc fail*/
	(*out) = new_buffer;
	(*outsize) = new_length;
	chunk_start = &(*out)[new_length - total_chunk_length];

	for (i = 0; i != total_chunk_length; ++i) chunk_start[i] = chunk[i];

	return 0;
}

/*Sets length and name and allocates the space for data and crc but does not
set data or crc yet. Returns the start of the chunk in chunk. The start of
the data is at chunk + 8. To finalize chunk, add the data, then use
lodepng_chunk_generate_crc */
static unsigned lodepng_chunk_init(unsigned char** chunk,
	ucvector* out,
	unsigned length, const char* type) {
	size_t new_length = out->size;
	if (lodepng_addofl(new_length, length, &new_length)) return 77;
	if (lodepng_addofl(new_length, 12, &new_length)) return 77;
	if (!ucvector_resize(out, new_length)) return 83; /*alloc fail*/
	*chunk = out->data + new_length - length - 12u;

	/*1: length*/
	lodepng_set32bitInt(*chunk, length);

	/*2: chunk name (4 letters)*/
	lodepng_memcpy(*chunk + 4, type, 4);

	return 0;
}

/* like lodepng_chunk_create but with custom allocsize */
static unsigned lodepng_chunk_createv(ucvector* out,
	unsigned length, const char* type, const unsigned char* data) {
	unsigned char* chunk;
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, length, type));

	/*3: the data*/
	lodepng_memcpy(chunk + 8, data, length);

	/*4: CRC (of the chunkname characters and the data)*/
	lodepng_chunk_generate_crc(chunk);

	return 0;
}

unsigned lodepng_chunk_create(unsigned char** out, size_t* outsize,
	unsigned length, const char* type, const unsigned char* data) {
	ucvector v = ucvector_init(*out, *outsize);
	unsigned error = lodepng_chunk_createv(&v, length, type, data);
	*out = v.data;
	*outsize = v.size;
	return error;
}

/* ////////////////////////////////////////////////////////////////////////// */
/* / Color types, channels, bits                                            / */
/* ////////////////////////////////////////////////////////////////////////// */

/*checks if the colortype is valid and the bitdepth bd is allowed for this colortype.
Return value is a LodePNG error code.*/
static unsigned checkColorValidity(LodePNGColorType colortype, unsigned bd) {
	switch (colortype) {
	case LCT_GREY:       if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8 || bd == 16)) return 37; break;
	case LCT_RGB:        if (!(bd == 8 || bd == 16)) return 37; break;
	case LCT_PALETTE:    if (!(bd == 1 || bd == 2 || bd == 4 || bd == 8)) return 37; break;
	case LCT_GREY_ALPHA: if (!(bd == 8 || bd == 16)) return 37; break;
	case LCT_RGBA:       if (!(bd == 8 || bd == 16)) return 37; break;
	case LCT_MAX_OCTET_VALUE: return 31; /* invalid color type */
	default: return 31; /* invalid color type */
	}
	return 0; /*allowed color type / bits combination*/
}

static unsigned getNumColorChannels(LodePNGColorType colortype) {
	switch (colortype) {
	case LCT_GREY: return 1;
	case LCT_RGB: return 3;
	case LCT_PALETTE: return 1;
	case LCT_GREY_ALPHA: return 2;
	case LCT_RGBA: return 4;
	case LCT_MAX_OCTET_VALUE: return 0; /* invalid color type */
	default: return 0; /*invalid color type*/
	}
}

static unsigned lodepng_get_bpp_lct(LodePNGColorType colortype, unsigned bitdepth) {
	/*bits per pixel is amount of channels * bits per channel*/
	return getNumColorChannels(colortype) * bitdepth;
}

/* ////////////////////////////////////////////////////////////////////////// */

void lodepng_color_mode_init(LodePNGColorMode* info) {
	info->key_defined = 0;
	info->key_r = info->key_g = info->key_b = 0;
	info->colortype = LCT_RGBA;
	info->bitdepth = 8;
	info->palette = 0;
	info->palettesize = 0;
}

/*allocates palette memory if needed, and initializes all colors to black*/
static void lodepng_color_mode_alloc_palette(LodePNGColorMode* info) {
	size_t i;
	/*if the palette is already allocated, it will have size 1024 so no reallocation needed in that case*/
	/*the palette must have room for up to 256 colors with 4 bytes each.*/
	if (!info->palette) info->palette = (unsigned char*)lodepng_malloc(1024);
	if (!info->palette) return; /*alloc fail*/
	for (i = 0; i != 256; ++i) {
		/*Initialize all unused colors with black, the value used for invalid palette indices.
		This is an error according to the PNG spec, but common PNG decoders make it black instead.
		That makes color conversion slightly faster due to no error handling needed.*/
		info->palette[i * 4 + 0] = 0;
		info->palette[i * 4 + 1] = 0;
		info->palette[i * 4 + 2] = 0;
		info->palette[i * 4 + 3] = 255;
	}
}

void lodepng_color_mode_cleanup(LodePNGColorMode* info) {
	lodepng_palette_clear(info);
}

unsigned lodepng_color_mode_copy(LodePNGColorMode* dest, const LodePNGColorMode* source) {
	lodepng_color_mode_cleanup(dest);
	lodepng_memcpy(dest, source, sizeof(LodePNGColorMode));
	if (source->palette) {
		dest->palette = (unsigned char*)lodepng_malloc(1024);
		if (!dest->palette && source->palettesize) return 83; /*alloc fail*/
		lodepng_memcpy(dest->palette, source->palette, source->palettesize * 4);
	}
	return 0;
}

LodePNGColorMode lodepng_color_mode_make(LodePNGColorType colortype, unsigned bitdepth) {
	LodePNGColorMode result;
	lodepng_color_mode_init(&result);
	result.colortype = colortype;
	result.bitdepth = bitdepth;
	return result;
}

static int lodepng_color_mode_equal(const LodePNGColorMode* a, const LodePNGColorMode* b) {
	size_t i;
	if (a->colortype != b->colortype) return 0;
	if (a->bitdepth != b->bitdepth) return 0;
	if (a->key_defined != b->key_defined) return 0;
	if (a->key_defined) {
		if (a->key_r != b->key_r) return 0;
		if (a->key_g != b->key_g) return 0;
		if (a->key_b != b->key_b) return 0;
	}
	if (a->palettesize != b->palettesize) return 0;
	for (i = 0; i != a->palettesize * 4; ++i) {
		if (a->palette[i] != b->palette[i]) return 0;
	}
	return 1;
}

void lodepng_palette_clear(LodePNGColorMode* info) {
	if (info->palette) lodepng_free(info->palette);
	info->palette = 0;
	info->palettesize = 0;
}

unsigned lodepng_palette_add(LodePNGColorMode* info,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	if (!info->palette) /*allocate palette if empty*/ {
		lodepng_color_mode_alloc_palette(info);
		if (!info->palette) return 83; /*alloc fail*/
	}
	if (info->palettesize >= 256) {
		return 108; /*too many palette values*/
	}
	info->palette[4 * info->palettesize + 0] = r;
	info->palette[4 * info->palettesize + 1] = g;
	info->palette[4 * info->palettesize + 2] = b;
	info->palette[4 * info->palettesize + 3] = a;
	++info->palettesize;
	return 0;
}

/*calculate bits per pixel out of colortype and bitdepth*/
unsigned lodepng_get_bpp(const LodePNGColorMode* info) {
	return lodepng_get_bpp_lct(info->colortype, info->bitdepth);
}

unsigned lodepng_get_channels(const LodePNGColorMode* info) {
	return getNumColorChannels(info->colortype);
}

unsigned lodepng_is_greyscale_type(const LodePNGColorMode* info) {
	return info->colortype == LCT_GREY || info->colortype == LCT_GREY_ALPHA;
}

unsigned lodepng_is_alpha_type(const LodePNGColorMode* info) {
	return (info->colortype & 4) != 0; /*4 or 6*/
}

unsigned lodepng_is_palette_type(const LodePNGColorMode* info) {
	return info->colortype == LCT_PALETTE;
}

unsigned lodepng_has_palette_alpha(const LodePNGColorMode* info) {
	size_t i;
	for (i = 0; i != info->palettesize; ++i) {
		if (info->palette[i * 4 + 3] < 255) return 1;
	}
	return 0;
}

unsigned lodepng_can_have_alpha(const LodePNGColorMode* info) {
	return info->key_defined
		|| lodepng_is_alpha_type(info)
		|| lodepng_has_palette_alpha(info);
}

static size_t lodepng_get_raw_size_lct(unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth) {
	size_t bpp = lodepng_get_bpp_lct(colortype, bitdepth);
	size_t n = (size_t)w * (size_t)h;
	return ((n / 8u) * bpp) + ((n & 7u) * bpp + 7u) / 8u;
}

size_t lodepng_get_raw_size(unsigned w, unsigned h, const LodePNGColorMode* color) {
	return lodepng_get_raw_size_lct(w, h, color->colortype, color->bitdepth);
}


#ifdef LODEPNG_COMPILE_PNG

/*in an idat chunk, each scanline is a multiple of 8 bits, unlike the lodepng output buffer,
and in addition has one extra byte per line: the filter byte. So this gives a larger
result than lodepng_get_raw_size. Set h to 1 to get the size of 1 row including filter byte. */
static size_t lodepng_get_raw_size_idat(unsigned w, unsigned h, unsigned bpp) {
	/* + 1 for the filter byte, and possibly plus padding bits per line. */
	/* Ignoring casts, the expression is equal to (w * bpp + 7) / 8 + 1, but avoids overflow of w * bpp */
	size_t line = ((size_t)(w / 8u) * bpp) + 1u + ((w & 7u) * bpp + 7u) / 8u;
	return (size_t)h * line;
}

#endif /*LODEPNG_COMPILE_PNG*/

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static void LodePNGUnknownChunks_init(LodePNGInfo* info) {
	unsigned i;
	for (i = 0; i != 3; ++i) info->unknown_chunks_data[i] = 0;
	for (i = 0; i != 3; ++i) info->unknown_chunks_size[i] = 0;
}

static void LodePNGUnknownChunks_cleanup(LodePNGInfo* info) {
	unsigned i;
	for (i = 0; i != 3; ++i) lodepng_free(info->unknown_chunks_data[i]);
}

static unsigned LodePNGUnknownChunks_copy(LodePNGInfo* dest, const LodePNGInfo* src) {
	unsigned i;

	LodePNGUnknownChunks_cleanup(dest);

	for (i = 0; i != 3; ++i) {
		size_t j;
		dest->unknown_chunks_size[i] = src->unknown_chunks_size[i];
		dest->unknown_chunks_data[i] = (unsigned char*)lodepng_malloc(src->unknown_chunks_size[i]);
		if (!dest->unknown_chunks_data[i] && dest->unknown_chunks_size[i]) return 83; /*alloc fail*/
		for (j = 0; j < src->unknown_chunks_size[i]; ++j) {
			dest->unknown_chunks_data[i][j] = src->unknown_chunks_data[i][j];
		}
	}

	return 0;
}

/******************************************************************************/

static void LodePNGText_init(LodePNGInfo* info) {
	info->text_num = 0;
	info->text_keys = NULL;
	info->text_strings = NULL;
}

static void LodePNGText_cleanup(LodePNGInfo* info) {
	size_t i;
	for (i = 0; i != info->text_num; ++i) {
		string_cleanup(&info->text_keys[i]);
		string_cleanup(&info->text_strings[i]);
	}
	lodepng_free(info->text_keys);
	lodepng_free(info->text_strings);
}

static unsigned LodePNGText_copy(LodePNGInfo* dest, const LodePNGInfo* source) {
	size_t i = 0;
	dest->text_keys = NULL;
	dest->text_strings = NULL;
	dest->text_num = 0;
	for (i = 0; i != source->text_num; ++i) {
		CERROR_TRY_RETURN(lodepng_add_text(dest, source->text_keys[i], source->text_strings[i]));
	}
	return 0;
}

static unsigned lodepng_add_text_sized(LodePNGInfo* info, const char* key, const char* str, size_t size) {
	char** new_keys = (char**)(lodepng_realloc(info->text_keys, sizeof(char*) * (info->text_num + 1)));
	char** new_strings = (char**)(lodepng_realloc(info->text_strings, sizeof(char*) * (info->text_num + 1)));

	if (new_keys) info->text_keys = new_keys;
	if (new_strings) info->text_strings = new_strings;

	if (!new_keys || !new_strings) return 83; /*alloc fail*/

	++info->text_num;
	info->text_keys[info->text_num - 1] = alloc_string(key);
	info->text_strings[info->text_num - 1] = alloc_string_sized(str, size);
	if (!info->text_keys[info->text_num - 1] || !info->text_strings[info->text_num - 1]) return 83; /*alloc fail*/

	return 0;
}

unsigned lodepng_add_text(LodePNGInfo* info, const char* key, const char* str) {
	return lodepng_add_text_sized(info, key, str, lodepng_strlen(str));
}

void lodepng_clear_text(LodePNGInfo* info) {
	LodePNGText_cleanup(info);
}

/******************************************************************************/

static void LodePNGIText_init(LodePNGInfo* info) {
	info->itext_num = 0;
	info->itext_keys = NULL;
	info->itext_langtags = NULL;
	info->itext_transkeys = NULL;
	info->itext_strings = NULL;
}

static void LodePNGIText_cleanup(LodePNGInfo* info) {
	size_t i;
	for (i = 0; i != info->itext_num; ++i) {
		string_cleanup(&info->itext_keys[i]);
		string_cleanup(&info->itext_langtags[i]);
		string_cleanup(&info->itext_transkeys[i]);
		string_cleanup(&info->itext_strings[i]);
	}
	lodepng_free(info->itext_keys);
	lodepng_free(info->itext_langtags);
	lodepng_free(info->itext_transkeys);
	lodepng_free(info->itext_strings);
}

static unsigned LodePNGIText_copy(LodePNGInfo* dest, const LodePNGInfo* source) {
	size_t i = 0;
	dest->itext_keys = NULL;
	dest->itext_langtags = NULL;
	dest->itext_transkeys = NULL;
	dest->itext_strings = NULL;
	dest->itext_num = 0;
	for (i = 0; i != source->itext_num; ++i) {
		CERROR_TRY_RETURN(lodepng_add_itext(dest, source->itext_keys[i], source->itext_langtags[i],
			source->itext_transkeys[i], source->itext_strings[i]));
	}
	return 0;
}

void lodepng_clear_itext(LodePNGInfo* info) {
	LodePNGIText_cleanup(info);
}

static unsigned lodepng_add_itext_sized(LodePNGInfo* info, const char* key, const char* langtag,
	const char* transkey, const char* str, size_t size) {
	char** new_keys = (char**)(lodepng_realloc(info->itext_keys, sizeof(char*) * (info->itext_num + 1)));
	char** new_langtags = (char**)(lodepng_realloc(info->itext_langtags, sizeof(char*) * (info->itext_num + 1)));
	char** new_transkeys = (char**)(lodepng_realloc(info->itext_transkeys, sizeof(char*) * (info->itext_num + 1)));
	char** new_strings = (char**)(lodepng_realloc(info->itext_strings, sizeof(char*) * (info->itext_num + 1)));

	if (new_keys) info->itext_keys = new_keys;
	if (new_langtags) info->itext_langtags = new_langtags;
	if (new_transkeys) info->itext_transkeys = new_transkeys;
	if (new_strings) info->itext_strings = new_strings;

	if (!new_keys || !new_langtags || !new_transkeys || !new_strings) return 83; /*alloc fail*/

	++info->itext_num;

	info->itext_keys[info->itext_num - 1] = alloc_string(key);
	info->itext_langtags[info->itext_num - 1] = alloc_string(langtag);
	info->itext_transkeys[info->itext_num - 1] = alloc_string(transkey);
	info->itext_strings[info->itext_num - 1] = alloc_string_sized(str, size);

	return 0;
}

unsigned lodepng_add_itext(LodePNGInfo* info, const char* key, const char* langtag,
	const char* transkey, const char* str) {
	return lodepng_add_itext_sized(info, key, langtag, transkey, str, lodepng_strlen(str));
}

/* same as set but does not delete */
static unsigned lodepng_assign_icc(LodePNGInfo* info, const char* name, const unsigned char* profile, unsigned profile_size) {
	if (profile_size == 0) return 100; /*invalid ICC profile size*/

	info->iccp_name = alloc_string(name);
	info->iccp_profile = (unsigned char*)lodepng_malloc(profile_size);

	if (!info->iccp_name || !info->iccp_profile) return 83; /*alloc fail*/

	lodepng_memcpy(info->iccp_profile, profile, profile_size);
	info->iccp_profile_size = profile_size;

	return 0; /*ok*/
}

unsigned lodepng_set_icc(LodePNGInfo* info, const char* name, const unsigned char* profile, unsigned profile_size) {
	if (info->iccp_name) lodepng_clear_icc(info);
	info->iccp_defined = 1;

	return lodepng_assign_icc(info, name, profile, profile_size);
}

void lodepng_clear_icc(LodePNGInfo* info) {
	string_cleanup(&info->iccp_name);
	lodepng_free(info->iccp_profile);
	info->iccp_profile = NULL;
	info->iccp_profile_size = 0;
	info->iccp_defined = 0;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

void lodepng_info_init(LodePNGInfo* info) {
	lodepng_color_mode_init(&info->color);
	info->interlace_method = 0;
	info->compression_method = 0;
	info->filter_method = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	info->background_defined = 0;
	info->background_r = info->background_g = info->background_b = 0;

	LodePNGText_init(info);
	LodePNGIText_init(info);

	info->time_defined = 0;
	info->phys_defined = 0;

	info->gama_defined = 0;
	info->chrm_defined = 0;
	info->srgb_defined = 0;
	info->iccp_defined = 0;
	info->iccp_name = NULL;
	info->iccp_profile = NULL;

	info->sbit_defined = 0;
	info->sbit_r = info->sbit_g = info->sbit_b = info->sbit_a = 0;

	LodePNGUnknownChunks_init(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

void lodepng_info_cleanup(LodePNGInfo* info) {
	lodepng_color_mode_cleanup(&info->color);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	LodePNGText_cleanup(info);
	LodePNGIText_cleanup(info);

	lodepng_clear_icc(info);

	LodePNGUnknownChunks_cleanup(info);
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

unsigned lodepng_info_copy(LodePNGInfo* dest, const LodePNGInfo* source) {
	lodepng_info_cleanup(dest);
	lodepng_memcpy(dest, source, sizeof(LodePNGInfo));
	lodepng_color_mode_init(&dest->color);
	CERROR_TRY_RETURN(lodepng_color_mode_copy(&dest->color, &source->color));

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	CERROR_TRY_RETURN(LodePNGText_copy(dest, source));
	CERROR_TRY_RETURN(LodePNGIText_copy(dest, source));
	if (source->iccp_defined) {
		CERROR_TRY_RETURN(lodepng_assign_icc(dest, source->iccp_name, source->iccp_profile, source->iccp_profile_size));
	}

	LodePNGUnknownChunks_init(dest);
	CERROR_TRY_RETURN(LodePNGUnknownChunks_copy(dest, source));
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	return 0;
}

/* ////////////////////////////////////////////////////////////////////////// */

/*index: bitgroup index, bits: bitgroup size(1, 2 or 4), in: bitgroup value, out: octet array to add bits to*/
static void addColorBits(unsigned char* out, size_t index, unsigned bits, unsigned in) {
	unsigned m = bits == 1 ? 7 : bits == 2 ? 3 : 1; /*8 / bits - 1*/
	/*p = the partial index in the byte, e.g. with 4 palettebits it is 0 for first half or 1 for second half*/
	unsigned p = index & m;
	in &= (1u << bits) - 1u; /*filter out any other bits of the input value*/
	in = in << (bits * (m - p));
	if (p == 0) out[index * bits / 8u] = in;
	else out[index * bits / 8u] |= in;
}

typedef struct ColorTree ColorTree;

/*
One node of a color tree
This is the data structure used to count the number of unique colors and to get a palette
index for a color. It's like an octree, but because the alpha channel is used too, each
node has 16 instead of 8 children.
*/
struct ColorTree {
	ColorTree* children[16]; /*up to 16 pointers to ColorTree of next level*/
	int index; /*the payload. Only has a meaningful value if this is in the last level*/
};

static void color_tree_init(ColorTree* tree) {
	lodepng_memset(tree->children, 0, 16 * sizeof(*tree->children));
	tree->index = -1;
}

static void color_tree_cleanup(ColorTree* tree) {
	int i;
	for (i = 0; i != 16; ++i) {
		if (tree->children[i]) {
			color_tree_cleanup(tree->children[i]);
			lodepng_free(tree->children[i]);
		}
	}
}

/*returns -1 if color not present, its index otherwise*/
static int color_tree_get(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	int bit = 0;
	for (bit = 0; bit < 8; ++bit) {
		int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
		if (!tree->children[i]) return -1;
		else tree = tree->children[i];
	}
	return tree ? tree->index : -1;
}

#ifdef LODEPNG_COMPILE_ENCODER
static int color_tree_has(ColorTree* tree, unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return color_tree_get(tree, r, g, b, a) >= 0;
}
#endif /*LODEPNG_COMPILE_ENCODER*/

/*color is not allowed to already exist.
Index should be >= 0 (it's signed to be compatible with using -1 for "doesn't exist")
Returns error code, or 0 if ok*/
static unsigned color_tree_add(ColorTree* tree,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned index) {
	int bit;
	for (bit = 0; bit < 8; ++bit) {
		int i = 8 * ((r >> bit) & 1) + 4 * ((g >> bit) & 1) + 2 * ((b >> bit) & 1) + 1 * ((a >> bit) & 1);
		if (!tree->children[i]) {
			tree->children[i] = (ColorTree*)lodepng_malloc(sizeof(ColorTree));
			if (!tree->children[i]) return 83; /*alloc fail*/
			color_tree_init(tree->children[i]);
		}
		tree = tree->children[i];
	}
	tree->index = (int)index;
	return 0;
}

/*put a pixel, given its RGBA color, into image of any color type*/
static unsigned rgba8ToPixel(unsigned char* out, size_t i,
	const LodePNGColorMode* mode, ColorTree* tree /*for palette*/,
	unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	if (mode->colortype == LCT_GREY) {
		unsigned char gray = r; /*((unsigned short)r + g + b) / 3u;*/
		if (mode->bitdepth == 8) out[i] = gray;
		else if (mode->bitdepth == 16) out[i * 2 + 0] = out[i * 2 + 1] = gray;
		else {
			/*take the most significant bits of gray*/
			gray = ((unsigned)gray >> (8u - mode->bitdepth)) & ((1u << mode->bitdepth) - 1u);
			addColorBits(out, i, mode->bitdepth, gray);
		}
	}
	else if (mode->colortype == LCT_RGB) {
		if (mode->bitdepth == 8) {
			out[i * 3 + 0] = r;
			out[i * 3 + 1] = g;
			out[i * 3 + 2] = b;
		}
		else {
			out[i * 6 + 0] = out[i * 6 + 1] = r;
			out[i * 6 + 2] = out[i * 6 + 3] = g;
			out[i * 6 + 4] = out[i * 6 + 5] = b;
		}
	}
	else if (mode->colortype == LCT_PALETTE) {
		int index = color_tree_get(tree, r, g, b, a);
		if (index < 0) return 82; /*color not in palette*/
		if (mode->bitdepth == 8) out[i] = index;
		else addColorBits(out, i, mode->bitdepth, (unsigned)index);
	}
	else if (mode->colortype == LCT_GREY_ALPHA) {
		unsigned char gray = r; /*((unsigned short)r + g + b) / 3u;*/
		if (mode->bitdepth == 8) {
			out[i * 2 + 0] = gray;
			out[i * 2 + 1] = a;
		}
		else if (mode->bitdepth == 16) {
			out[i * 4 + 0] = out[i * 4 + 1] = gray;
			out[i * 4 + 2] = out[i * 4 + 3] = a;
		}
	}
	else if (mode->colortype == LCT_RGBA) {
		if (mode->bitdepth == 8) {
			out[i * 4 + 0] = r;
			out[i * 4 + 1] = g;
			out[i * 4 + 2] = b;
			out[i * 4 + 3] = a;
		}
		else {
			out[i * 8 + 0] = out[i * 8 + 1] = r;
			out[i * 8 + 2] = out[i * 8 + 3] = g;
			out[i * 8 + 4] = out[i * 8 + 5] = b;
			out[i * 8 + 6] = out[i * 8 + 7] = a;
		}
	}

	return 0; /*no error*/
}

/*put a pixel, given its RGBA16 color, into image of any color 16-bitdepth type*/
static void rgba16ToPixel(unsigned char* out, size_t i,
	const LodePNGColorMode* mode,
	unsigned short r, unsigned short g, unsigned short b, unsigned short a) {
	if (mode->colortype == LCT_GREY) {
		unsigned short gray = r; /*((unsigned)r + g + b) / 3u;*/
		out[i * 2 + 0] = (gray >> 8) & 255;
		out[i * 2 + 1] = gray & 255;
	}
	else if (mode->colortype == LCT_RGB) {
		out[i * 6 + 0] = (r >> 8) & 255;
		out[i * 6 + 1] = r & 255;
		out[i * 6 + 2] = (g >> 8) & 255;
		out[i * 6 + 3] = g & 255;
		out[i * 6 + 4] = (b >> 8) & 255;
		out[i * 6 + 5] = b & 255;
	}
	else if (mode->colortype == LCT_GREY_ALPHA) {
		unsigned short gray = r; /*((unsigned)r + g + b) / 3u;*/
		out[i * 4 + 0] = (gray >> 8) & 255;
		out[i * 4 + 1] = gray & 255;
		out[i * 4 + 2] = (a >> 8) & 255;
		out[i * 4 + 3] = a & 255;
	}
	else if (mode->colortype == LCT_RGBA) {
		out[i * 8 + 0] = (r >> 8) & 255;
		out[i * 8 + 1] = r & 255;
		out[i * 8 + 2] = (g >> 8) & 255;
		out[i * 8 + 3] = g & 255;
		out[i * 8 + 4] = (b >> 8) & 255;
		out[i * 8 + 5] = b & 255;
		out[i * 8 + 6] = (a >> 8) & 255;
		out[i * 8 + 7] = a & 255;
	}
}

/*Get RGBA8 color of pixel with index i (y * width + x) from the raw image with given color type.*/
static void getPixelColorRGBA8(unsigned char* r, unsigned char* g,
	unsigned char* b, unsigned char* a,
	const unsigned char* in, size_t i,
	const LodePNGColorMode* mode) {
	if (mode->colortype == LCT_GREY) {
		if (mode->bitdepth == 8) {
			*r = *g = *b = in[i];
			if (mode->key_defined && *r == mode->key_r) *a = 0;
			else *a = 255;
		}
		else if (mode->bitdepth == 16) {
			*r = *g = *b = in[i * 2 + 0];
			if (mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
			else *a = 255;
		}
		else {
			unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
			size_t j = i * mode->bitdepth;
			unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
			*r = *g = *b = (value * 255) / highest;
			if (mode->key_defined && value == mode->key_r) *a = 0;
			else *a = 255;
		}
	}
	else if (mode->colortype == LCT_RGB) {
		if (mode->bitdepth == 8) {
			*r = in[i * 3 + 0]; *g = in[i * 3 + 1]; *b = in[i * 3 + 2];
			if (mode->key_defined && *r == mode->key_r && *g == mode->key_g && *b == mode->key_b) *a = 0;
			else *a = 255;
		}
		else {
			*r = in[i * 6 + 0];
			*g = in[i * 6 + 2];
			*b = in[i * 6 + 4];
			if (mode->key_defined && 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
				&& 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
				&& 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
			else *a = 255;
		}
	}
	else if (mode->colortype == LCT_PALETTE) {
		unsigned index;
		if (mode->bitdepth == 8) index = in[i];
		else {
			size_t j = i * mode->bitdepth;
			index = readBitsFromReversedStream(&j, in, mode->bitdepth);
		}
		/*out of bounds of palette not checked: see lodepng_color_mode_alloc_palette.*/
		*r = mode->palette[index * 4 + 0];
		*g = mode->palette[index * 4 + 1];
		*b = mode->palette[index * 4 + 2];
		*a = mode->palette[index * 4 + 3];
	}
	else if (mode->colortype == LCT_GREY_ALPHA) {
		if (mode->bitdepth == 8) {
			*r = *g = *b = in[i * 2 + 0];
			*a = in[i * 2 + 1];
		}
		else {
			*r = *g = *b = in[i * 4 + 0];
			*a = in[i * 4 + 2];
		}
	}
	else if (mode->colortype == LCT_RGBA) {
		if (mode->bitdepth == 8) {
			*r = in[i * 4 + 0];
			*g = in[i * 4 + 1];
			*b = in[i * 4 + 2];
			*a = in[i * 4 + 3];
		}
		else {
			*r = in[i * 8 + 0];
			*g = in[i * 8 + 2];
			*b = in[i * 8 + 4];
			*a = in[i * 8 + 6];
		}
	}
}

/*Similar to getPixelColorRGBA8, but with all the for loops inside of the color
mode test cases, optimized to convert the colors much faster, when converting
to the common case of RGBA with 8 bit per channel. buffer must be RGBA with
enough memory.*/
static void getPixelColorsRGBA8(unsigned char* LODEPNG_RESTRICT buffer, size_t numpixels,
	const unsigned char* LODEPNG_RESTRICT in,
	const LodePNGColorMode* mode) {
	unsigned num_channels = 4;
	size_t i;
	if (mode->colortype == LCT_GREY) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i];
				buffer[3] = 255;
			}
			if (mode->key_defined) {
				buffer -= numpixels * num_channels;
				for (i = 0; i != numpixels; ++i, buffer += num_channels) {
					if (buffer[0] == mode->key_r) buffer[3] = 0;
				}
			}
		}
		else if (mode->bitdepth == 16) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i * 2];
				buffer[3] = mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r ? 0 : 255;
			}
		}
		else {
			unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
			size_t j = 0;
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
				buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
				buffer[3] = mode->key_defined && value == mode->key_r ? 0 : 255;
			}
		}
	}
	else if (mode->colortype == LCT_RGB) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				lodepng_memcpy(buffer, &in[i * 3], 3);
				buffer[3] = 255;
			}
			if (mode->key_defined) {
				buffer -= numpixels * num_channels;
				for (i = 0; i != numpixels; ++i, buffer += num_channels) {
					if (buffer[0] == mode->key_r && buffer[1] == mode->key_g && buffer[2] == mode->key_b) buffer[3] = 0;
				}
			}
		}
		else {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = in[i * 6 + 0];
				buffer[1] = in[i * 6 + 2];
				buffer[2] = in[i * 6 + 4];
				buffer[3] = mode->key_defined
					&& 256U * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
					&& 256U * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
					&& 256U * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b ? 0 : 255;
			}
		}
	}
	else if (mode->colortype == LCT_PALETTE) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				unsigned index = in[i];
				/*out of bounds of palette not checked: see lodepng_color_mode_alloc_palette.*/
				lodepng_memcpy(buffer, &mode->palette[index * 4], 4);
			}
		}
		else {
			size_t j = 0;
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				unsigned index = readBitsFromReversedStream(&j, in, mode->bitdepth);
				/*out of bounds of palette not checked: see lodepng_color_mode_alloc_palette.*/
				lodepng_memcpy(buffer, &mode->palette[index * 4], 4);
			}
		}
	}
	else if (mode->colortype == LCT_GREY_ALPHA) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
				buffer[3] = in[i * 2 + 1];
			}
		}
		else {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
				buffer[3] = in[i * 4 + 2];
			}
		}
	}
	else if (mode->colortype == LCT_RGBA) {
		if (mode->bitdepth == 8) {
			lodepng_memcpy(buffer, in, numpixels * 4);
		}
		else {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = in[i * 8 + 0];
				buffer[1] = in[i * 8 + 2];
				buffer[2] = in[i * 8 + 4];
				buffer[3] = in[i * 8 + 6];
			}
		}
	}
}

/*Similar to getPixelColorsRGBA8, but with 3-channel RGB output.*/
static void getPixelColorsRGB8(unsigned char* LODEPNG_RESTRICT buffer, size_t numpixels,
	const unsigned char* LODEPNG_RESTRICT in,
	const LodePNGColorMode* mode) {
	const unsigned num_channels = 3;
	size_t i;
	if (mode->colortype == LCT_GREY) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i];
			}
		}
		else if (mode->bitdepth == 16) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i * 2];
			}
		}
		else {
			unsigned highest = ((1U << mode->bitdepth) - 1U); /*highest possible value for this bit depth*/
			size_t j = 0;
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				unsigned value = readBitsFromReversedStream(&j, in, mode->bitdepth);
				buffer[0] = buffer[1] = buffer[2] = (value * 255) / highest;
			}
		}
	}
	else if (mode->colortype == LCT_RGB) {
		if (mode->bitdepth == 8) {
			lodepng_memcpy(buffer, in, numpixels * 3);
		}
		else {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = in[i * 6 + 0];
				buffer[1] = in[i * 6 + 2];
				buffer[2] = in[i * 6 + 4];
			}
		}
	}
	else if (mode->colortype == LCT_PALETTE) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				unsigned index = in[i];
				/*out of bounds of palette not checked: see lodepng_color_mode_alloc_palette.*/
				lodepng_memcpy(buffer, &mode->palette[index * 4], 3);
			}
		}
		else {
			size_t j = 0;
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				unsigned index = readBitsFromReversedStream(&j, in, mode->bitdepth);
				/*out of bounds of palette not checked: see lodepng_color_mode_alloc_palette.*/
				lodepng_memcpy(buffer, &mode->palette[index * 4], 3);
			}
		}
	}
	else if (mode->colortype == LCT_GREY_ALPHA) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i * 2 + 0];
			}
		}
		else {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = buffer[1] = buffer[2] = in[i * 4 + 0];
			}
		}
	}
	else if (mode->colortype == LCT_RGBA) {
		if (mode->bitdepth == 8) {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				lodepng_memcpy(buffer, &in[i * 4], 3);
			}
		}
		else {
			for (i = 0; i != numpixels; ++i, buffer += num_channels) {
				buffer[0] = in[i * 8 + 0];
				buffer[1] = in[i * 8 + 2];
				buffer[2] = in[i * 8 + 4];
			}
		}
	}
}

/*Get RGBA16 color of pixel with index i (y * width + x) from the raw image with
given color type, but the given color type must be 16-bit itself.*/
static void getPixelColorRGBA16(unsigned short* r, unsigned short* g, unsigned short* b, unsigned short* a,
	const unsigned char* in, size_t i, const LodePNGColorMode* mode) {
	if (mode->colortype == LCT_GREY) {
		*r = *g = *b = 256 * in[i * 2 + 0] + in[i * 2 + 1];
		if (mode->key_defined && 256U * in[i * 2 + 0] + in[i * 2 + 1] == mode->key_r) *a = 0;
		else *a = 65535;
	}
	else if (mode->colortype == LCT_RGB) {
		*r = 256u * in[i * 6 + 0] + in[i * 6 + 1];
		*g = 256u * in[i * 6 + 2] + in[i * 6 + 3];
		*b = 256u * in[i * 6 + 4] + in[i * 6 + 5];
		if (mode->key_defined
			&& 256u * in[i * 6 + 0] + in[i * 6 + 1] == mode->key_r
			&& 256u * in[i * 6 + 2] + in[i * 6 + 3] == mode->key_g
			&& 256u * in[i * 6 + 4] + in[i * 6 + 5] == mode->key_b) *a = 0;
		else *a = 65535;
	}
	else if (mode->colortype == LCT_GREY_ALPHA) {
		*r = *g = *b = 256u * in[i * 4 + 0] + in[i * 4 + 1];
		*a = 256u * in[i * 4 + 2] + in[i * 4 + 3];
	}
	else if (mode->colortype == LCT_RGBA) {
		*r = 256u * in[i * 8 + 0] + in[i * 8 + 1];
		*g = 256u * in[i * 8 + 2] + in[i * 8 + 3];
		*b = 256u * in[i * 8 + 4] + in[i * 8 + 5];
		*a = 256u * in[i * 8 + 6] + in[i * 8 + 7];
	}
}

unsigned lodepng_convert(unsigned char* out, const unsigned char* in,
	const LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in,
	unsigned w, unsigned h) {
	size_t i;
	ColorTree tree;
	size_t numpixels = (size_t)w * (size_t)h;
	unsigned error = 0;

	if (mode_in->colortype == LCT_PALETTE && !mode_in->palette) {
		return 107; /* error: must provide palette if input mode is palette */
	}

	if (lodepng_color_mode_equal(mode_out, mode_in)) {
		size_t numbytes = lodepng_get_raw_size(w, h, mode_in);
		lodepng_memcpy(out, in, numbytes);
		return 0;
	}

	if (mode_out->colortype == LCT_PALETTE) {
		size_t palettesize = mode_out->palettesize;
		const unsigned char* palette = mode_out->palette;
		size_t palsize = (size_t)1u << mode_out->bitdepth;
		/*if the user specified output palette but did not give the values, assume
		they want the values of the input color type (assuming that one is palette).
		Note that we never create a new palette ourselves.*/
		if (palettesize == 0) {
			palettesize = mode_in->palettesize;
			palette = mode_in->palette;
			/*if the input was also palette with same bitdepth, then the color types are also
			equal, so copy literally. This to preserve the exact indices that were in the PNG
			even in case there are duplicate colors in the palette.*/
			if (mode_in->colortype == LCT_PALETTE && mode_in->bitdepth == mode_out->bitdepth) {
				size_t numbytes = lodepng_get_raw_size(w, h, mode_in);
				lodepng_memcpy(out, in, numbytes);
				return 0;
			}
		}
		if (palettesize < palsize) palsize = palettesize;
		color_tree_init(&tree);
		for (i = 0; i != palsize; ++i) {
			const unsigned char* p = &palette[i * 4];
			error = color_tree_add(&tree, p[0], p[1], p[2], p[3], (unsigned)i);
			if (error) break;
		}
	}

	if (!error) {
		if (mode_in->bitdepth == 16 && mode_out->bitdepth == 16) {
			for (i = 0; i != numpixels; ++i) {
				unsigned short r = 0, g = 0, b = 0, a = 0;
				getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
				rgba16ToPixel(out, i, mode_out, r, g, b, a);
			}
		}
		else if (mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGBA) {
			getPixelColorsRGBA8(out, numpixels, in, mode_in);
		}
		else if (mode_out->bitdepth == 8 && mode_out->colortype == LCT_RGB) {
			getPixelColorsRGB8(out, numpixels, in, mode_in);
		}
		else {
			unsigned char r = 0, g = 0, b = 0, a = 0;
			for (i = 0; i != numpixels; ++i) {
				getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);
				error = rgba8ToPixel(out, i, mode_out, &tree, r, g, b, a);
				if (error) break;
			}
		}
	}

	if (mode_out->colortype == LCT_PALETTE) {
		color_tree_cleanup(&tree);
	}

	return error;
}


/* Converts a single rgb color without alpha from one type to another, color bits truncated to
their bitdepth. In case of single channel (gray or palette), only the r channel is used. Slow
function, do not use to process all pixels of an image. Alpha channel not supported on purpose:
this is for bKGD, supporting alpha may prevent it from finding a color in the palette, from the
specification it looks like bKGD should ignore the alpha values of the palette since it can use
any palette index but doesn't have an alpha channel. Idem with ignoring color key. */
unsigned lodepng_convert_rgb(
	unsigned* r_out, unsigned* g_out, unsigned* b_out,
	unsigned r_in, unsigned g_in, unsigned b_in,
	const LodePNGColorMode* mode_out, const LodePNGColorMode* mode_in) {
	unsigned r = 0, g = 0, b = 0;
	unsigned mul = 65535 / ((1u << mode_in->bitdepth) - 1u); /*65535, 21845, 4369, 257, 1*/
	unsigned shift = 16 - mode_out->bitdepth;

	if (mode_in->colortype == LCT_GREY || mode_in->colortype == LCT_GREY_ALPHA) {
		r = g = b = r_in * mul;
	}
	else if (mode_in->colortype == LCT_RGB || mode_in->colortype == LCT_RGBA) {
		r = r_in * mul;
		g = g_in * mul;
		b = b_in * mul;
	}
	else if (mode_in->colortype == LCT_PALETTE) {
		if (r_in >= mode_in->palettesize) return 82;
		r = mode_in->palette[r_in * 4 + 0] * 257u;
		g = mode_in->palette[r_in * 4 + 1] * 257u;
		b = mode_in->palette[r_in * 4 + 2] * 257u;
	}
	else {
		return 31;
	}

	/* now convert to output format */
	if (mode_out->colortype == LCT_GREY || mode_out->colortype == LCT_GREY_ALPHA) {
		*r_out = r >> shift;
	}
	else if (mode_out->colortype == LCT_RGB || mode_out->colortype == LCT_RGBA) {
		*r_out = r >> shift;
		*g_out = g >> shift;
		*b_out = b >> shift;
	}
	else if (mode_out->colortype == LCT_PALETTE) {
		unsigned i;
		/* a 16-bit color cannot be in the palette */
		if ((r >> 8) != (r & 255) || (g >> 8) != (g & 255) || (b >> 8) != (b & 255)) return 82;
		for (i = 0; i < mode_out->palettesize; i++) {
			unsigned j = i * 4;
			if ((r >> 8) == mode_out->palette[j + 0] && (g >> 8) == mode_out->palette[j + 1] &&
				(b >> 8) == mode_out->palette[j + 2]) {
				*r_out = i;
				return 0;
			}
		}
		return 82;
	}
	else {
		return 31;
	}

	return 0;
}

#ifdef LODEPNG_COMPILE_ENCODER

void lodepng_color_stats_init(LodePNGColorStats* stats) {
	/*stats*/
	stats->colored = 0;
	stats->key = 0;
	stats->key_r = stats->key_g = stats->key_b = 0;
	stats->alpha = 0;
	stats->numcolors = 0;
	stats->bits = 1;
	stats->numpixels = 0;
	/*settings*/
	stats->allow_palette = 1;
	stats->allow_greyscale = 1;
}

/*function used for debug purposes with C++*/
/*void printColorStats(LodePNGColorStats* p) {
  std::cout << "colored: " << (int)p->colored << ", ";
  std::cout << "key: " << (int)p->key << ", ";
  std::cout << "key_r: " << (int)p->key_r << ", ";
  std::cout << "key_g: " << (int)p->key_g << ", ";
  std::cout << "key_b: " << (int)p->key_b << ", ";
  std::cout << "alpha: " << (int)p->alpha << ", ";
  std::cout << "numcolors: " << (int)p->numcolors << ", ";
  std::cout << "bits: " << (int)p->bits << std::endl;
}*/

/*Returns how many bits needed to represent given value (max 8 bit)*/
static unsigned getValueRequiredBits(unsigned char value) {
	if (value == 0 || value == 255) return 1;
	/*The scaling of 2-bit and 4-bit values uses multiples of 85 and 17*/
	if (value % 17 == 0) return value % 85 == 0 ? 2 : 4;
	return 8;
}

/*stats must already have been inited. */
unsigned lodepng_compute_color_stats(LodePNGColorStats* stats,
	const unsigned char* in, unsigned w, unsigned h,
	const LodePNGColorMode* mode_in) {
	size_t i;
	ColorTree tree;
	size_t numpixels = (size_t)w * (size_t)h;
	unsigned error = 0;

	/* mark things as done already if it would be impossible to have a more expensive case */
	unsigned colored_done = lodepng_is_greyscale_type(mode_in) ? 1 : 0;
	unsigned alpha_done = lodepng_can_have_alpha(mode_in) ? 0 : 1;
	unsigned numcolors_done = 0;
	unsigned bpp = lodepng_get_bpp(mode_in);
	unsigned bits_done = (stats->bits == 1 && bpp == 1) ? 1 : 0;
	unsigned sixteen = 0; /* whether the input image is 16 bit */
	unsigned maxnumcolors = 257;
	if (bpp <= 8) maxnumcolors = LODEPNG_MIN(257, stats->numcolors + (1u << bpp));

	stats->numpixels += numpixels;

	/*if palette not allowed, no need to compute numcolors*/
	if (!stats->allow_palette) numcolors_done = 1;

	color_tree_init(&tree);

	/*If the stats was already filled in from previous data, fill its palette in tree
	and mark things as done already if we know they are the most expensive case already*/
	if (stats->alpha) alpha_done = 1;
	if (stats->colored) colored_done = 1;
	if (stats->bits == 16) numcolors_done = 1;
	if (stats->bits >= bpp) bits_done = 1;
	if (stats->numcolors >= maxnumcolors) numcolors_done = 1;

	if (!numcolors_done) {
		for (i = 0; i < stats->numcolors; i++) {
			const unsigned char* color = &stats->palette[i * 4];
			error = color_tree_add(&tree, color[0], color[1], color[2], color[3], i);
			if (error) goto cleanup;
		}
	}

	/*Check if the 16-bit input is truly 16-bit*/
	if (mode_in->bitdepth == 16 && !sixteen) {
		unsigned short r = 0, g = 0, b = 0, a = 0;
		for (i = 0; i != numpixels; ++i) {
			getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
			if ((r & 255) != ((r >> 8) & 255) || (g & 255) != ((g >> 8) & 255) ||
				(b & 255) != ((b >> 8) & 255) || (a & 255) != ((a >> 8) & 255)) /*first and second byte differ*/ {
				stats->bits = 16;
				sixteen = 1;
				bits_done = 1;
				numcolors_done = 1; /*counting colors no longer useful, palette doesn't support 16-bit*/
				break;
			}
		}
	}

	if (sixteen) {
		unsigned short r = 0, g = 0, b = 0, a = 0;

		for (i = 0; i != numpixels; ++i) {
			getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);

			if (!colored_done && (r != g || r != b)) {
				stats->colored = 1;
				colored_done = 1;
			}

			if (!alpha_done) {
				unsigned matchkey = (r == stats->key_r && g == stats->key_g && b == stats->key_b);
				if (a != 65535 && (a != 0 || (stats->key && !matchkey))) {
					stats->alpha = 1;
					stats->key = 0;
					alpha_done = 1;
				}
				else if (a == 0 && !stats->alpha && !stats->key) {
					stats->key = 1;
					stats->key_r = r;
					stats->key_g = g;
					stats->key_b = b;
				}
				else if (a == 65535 && stats->key && matchkey) {
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					stats->alpha = 1;
					stats->key = 0;
					alpha_done = 1;
				}
			}
			if (alpha_done && numcolors_done && colored_done && bits_done) break;
		}

		if (stats->key && !stats->alpha) {
			for (i = 0; i != numpixels; ++i) {
				getPixelColorRGBA16(&r, &g, &b, &a, in, i, mode_in);
				if (a != 0 && r == stats->key_r && g == stats->key_g && b == stats->key_b) {
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					stats->alpha = 1;
					stats->key = 0;
					alpha_done = 1;
				}
			}
		}
	}
	else /* < 16-bit */ {
		unsigned char r = 0, g = 0, b = 0, a = 0;
		for (i = 0; i != numpixels; ++i) {
			getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);

			if (!bits_done && stats->bits < 8) {
				/*only r is checked, < 8 bits is only relevant for grayscale*/
				unsigned bits = getValueRequiredBits(r);
				if (bits > stats->bits) stats->bits = bits;
			}
			bits_done = (stats->bits >= bpp);

			if (!colored_done && (r != g || r != b)) {
				stats->colored = 1;
				colored_done = 1;
				if (stats->bits < 8) stats->bits = 8; /*PNG has no colored modes with less than 8-bit per channel*/
			}

			if (!alpha_done) {
				unsigned matchkey = (r == stats->key_r && g == stats->key_g && b == stats->key_b);
				if (a != 255 && (a != 0 || (stats->key && !matchkey))) {
					stats->alpha = 1;
					stats->key = 0;
					alpha_done = 1;
					if (stats->bits < 8) stats->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
				}
				else if (a == 0 && !stats->alpha && !stats->key) {
					stats->key = 1;
					stats->key_r = r;
					stats->key_g = g;
					stats->key_b = b;
				}
				else if (a == 255 && stats->key && matchkey) {
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					stats->alpha = 1;
					stats->key = 0;
					alpha_done = 1;
					if (stats->bits < 8) stats->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
				}
			}

			if (!numcolors_done) {
				if (!color_tree_has(&tree, r, g, b, a)) {
					error = color_tree_add(&tree, r, g, b, a, stats->numcolors);
					if (error) goto cleanup;
					if (stats->numcolors < 256) {
						unsigned char* p = stats->palette;
						unsigned n = stats->numcolors;
						p[n * 4 + 0] = r;
						p[n * 4 + 1] = g;
						p[n * 4 + 2] = b;
						p[n * 4 + 3] = a;
					}
					++stats->numcolors;
					numcolors_done = stats->numcolors >= maxnumcolors;
				}
			}

			if (alpha_done && numcolors_done && colored_done && bits_done) break;
		}

		if (stats->key && !stats->alpha) {
			for (i = 0; i != numpixels; ++i) {
				getPixelColorRGBA8(&r, &g, &b, &a, in, i, mode_in);
				if (a != 0 && r == stats->key_r && g == stats->key_g && b == stats->key_b) {
					/* Color key cannot be used if an opaque pixel also has that RGB color. */
					stats->alpha = 1;
					stats->key = 0;
					alpha_done = 1;
					if (stats->bits < 8) stats->bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
				}
			}
		}

		/*make the stats's key always 16-bit for consistency - repeat each byte twice*/
		stats->key_r += (stats->key_r << 8);
		stats->key_g += (stats->key_g << 8);
		stats->key_b += (stats->key_b << 8);
	}

cleanup:
	color_tree_cleanup(&tree);
	return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
/*Adds a single color to the color stats. The stats must already have been inited. The color must be given as 16-bit
(with 2 bytes repeating for 8-bit and 65535 for opaque alpha channel). This function is expensive, do not call it for
all pixels of an image but only for a few additional values. */
static unsigned lodepng_color_stats_add(LodePNGColorStats* stats,
	unsigned r, unsigned g, unsigned b, unsigned a) {
	unsigned error = 0;
	unsigned char image[8];
	LodePNGColorMode mode;
	lodepng_color_mode_init(&mode);
	image[0] = r >> 8; image[1] = r; image[2] = g >> 8; image[3] = g;
	image[4] = b >> 8; image[5] = b; image[6] = a >> 8; image[7] = a;
	mode.bitdepth = 16;
	mode.colortype = LCT_RGBA;
	error = lodepng_compute_color_stats(stats, image, 1, 1, &mode);
	lodepng_color_mode_cleanup(&mode);
	return error;
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

/*Computes a minimal PNG color model that can contain all colors as indicated by the stats.
The stats should be computed with lodepng_compute_color_stats.
mode_in is raw color profile of the image the stats were computed on, to copy palette order from when relevant.
Minimal PNG color model means the color type and bit depth that gives smallest amount of bits in the output image,
e.g. gray if only grayscale pixels, palette if less than 256 colors, color key if only single transparent color, ...
This is used if auto_convert is enabled (it is by default).
*/
static unsigned auto_choose_color(LodePNGColorMode* mode_out,
	const LodePNGColorMode* mode_in,
	const LodePNGColorStats* stats) {
	unsigned error = 0;
	unsigned palettebits;
	size_t i, n;
	size_t numpixels = stats->numpixels;
	unsigned palette_ok, gray_ok;

	unsigned alpha = stats->alpha;
	unsigned key = stats->key;
	unsigned bits = stats->bits;

	mode_out->key_defined = 0;

	if (key && numpixels <= 16) {
		alpha = 1; /*too few pixels to justify tRNS chunk overhead*/
		key = 0;
		if (bits < 8) bits = 8; /*PNG has no alphachannel modes with less than 8-bit per channel*/
	}

	gray_ok = !stats->colored;
	if (!stats->allow_greyscale) gray_ok = 0;
	if (!gray_ok && bits < 8) bits = 8;

	n = stats->numcolors;
	palettebits = n <= 2 ? 1 : (n <= 4 ? 2 : (n <= 16 ? 4 : 8));
	palette_ok = n <= 256 && bits <= 8 && n != 0; /*n==0 means likely numcolors wasn't computed*/
	if (numpixels < n * 2) palette_ok = 0; /*don't add palette overhead if image has only a few pixels*/
	if (gray_ok && !alpha && bits <= palettebits) palette_ok = 0; /*gray is less overhead*/
	if (!stats->allow_palette) palette_ok = 0;

	if (palette_ok) {
		const unsigned char* p = stats->palette;
		lodepng_palette_clear(mode_out); /*remove potential earlier palette*/
		for (i = 0; i != stats->numcolors; ++i) {
			error = lodepng_palette_add(mode_out, p[i * 4 + 0], p[i * 4 + 1], p[i * 4 + 2], p[i * 4 + 3]);
			if (error) break;
		}

		mode_out->colortype = LCT_PALETTE;
		mode_out->bitdepth = palettebits;

		if (mode_in->colortype == LCT_PALETTE && mode_in->palettesize >= mode_out->palettesize
			&& mode_in->bitdepth == mode_out->bitdepth) {
			/*If input should have same palette colors, keep original to preserve its order and prevent conversion*/
			lodepng_color_mode_cleanup(mode_out); /*clears palette, keeps the above set colortype and bitdepth fields as-is*/
			lodepng_color_mode_copy(mode_out, mode_in);
		}
	}
	else /*8-bit or 16-bit per channel*/ {
		mode_out->bitdepth = bits;
		mode_out->colortype = alpha ? (gray_ok ? LCT_GREY_ALPHA : LCT_RGBA)
			: (gray_ok ? LCT_GREY : LCT_RGB);
		if (key) {
			unsigned mask = (1u << mode_out->bitdepth) - 1u; /*stats always uses 16-bit, mask converts it*/
			mode_out->key_r = stats->key_r & mask;
			mode_out->key_g = stats->key_g & mask;
			mode_out->key_b = stats->key_b & mask;
			mode_out->key_defined = 1;
		}
	}

	return error;
}

#endif /* #ifdef LODEPNG_COMPILE_ENCODER */

/*
Paeth predictor, used by PNG filter type 4
The parameters are of type short, but should come from unsigned chars, the shorts
are only needed to make the paeth calculation correct.
*/
static unsigned char paethPredictor(short a, short b, short c) {
	short pa = LODEPNG_ABS(b - c);
	short pb = LODEPNG_ABS(a - c);
	short pc = LODEPNG_ABS(a + b - c - c);
	/* return input value associated with smallest of pa, pb, pc (with certain priority if equal) */
	if (pb < pa) { a = b; pa = pb; }
	return (pc < pa) ? c : a;
}

/*shared values used by multiple Adam7 related functions*/

static const unsigned ADAM7_IX[7] = { 0, 4, 0, 2, 0, 1, 0 }; /*x start values*/
static const unsigned ADAM7_IY[7] = { 0, 0, 4, 0, 2, 0, 1 }; /*y start values*/
static const unsigned ADAM7_DX[7] = { 8, 8, 4, 4, 2, 2, 1 }; /*x delta values*/
static const unsigned ADAM7_DY[7] = { 8, 8, 8, 4, 4, 2, 2 }; /*y delta values*/

/*
Outputs various dimensions and positions in the image related to the Adam7 reduced images.
passw: output containing the width of the 7 passes
passh: output containing the height of the 7 passes
filter_passstart: output containing the index of the start and end of each
 reduced image with filter bytes
padded_passstart output containing the index of the start and end of each
 reduced image when without filter bytes but with padded scanlines
passstart: output containing the index of the start and end of each reduced
 image without padding between scanlines, but still padding between the images
w, h: width and height of non-interlaced image
bpp: bits per pixel
"padded" is only relevant if bpp is less than 8 and a scanline or image does not
 end at a full byte
*/
static void Adam7_getpassvalues(unsigned passw[7], unsigned passh[7], size_t filter_passstart[8],
	size_t padded_passstart[8], size_t passstart[8], unsigned w, unsigned h, unsigned bpp) {
	/*the passstart values have 8 values: the 8th one indicates the byte after the end of the 7th (= last) pass*/
	unsigned i;

	/*calculate width and height in pixels of each pass*/
	for (i = 0; i != 7; ++i) {
		passw[i] = (w + ADAM7_DX[i] - ADAM7_IX[i] - 1) / ADAM7_DX[i];
		passh[i] = (h + ADAM7_DY[i] - ADAM7_IY[i] - 1) / ADAM7_DY[i];
		if (passw[i] == 0) passh[i] = 0;
		if (passh[i] == 0) passw[i] = 0;
	}

	filter_passstart[0] = padded_passstart[0] = passstart[0] = 0;
	for (i = 0; i != 7; ++i) {
		/*if passw[i] is 0, it's 0 bytes, not 1 (no filtertype-byte)*/
		filter_passstart[i + 1] = filter_passstart[i]
			+ ((passw[i] && passh[i]) ? passh[i] * (1u + (passw[i] * bpp + 7u) / 8u) : 0);
		/*bits padded if needed to fill full byte at end of each scanline*/
		padded_passstart[i + 1] = padded_passstart[i] + passh[i] * ((passw[i] * bpp + 7u) / 8u);
		/*only padded at end of reduced image*/
		passstart[i + 1] = passstart[i] + (passh[i] * passw[i] * bpp + 7u) / 8u;
	}
}


#if defined(LODEPNG_COMPILE_ENCODER)

void lodepng_state_init(LodePNGState* state) {

#ifdef LODEPNG_COMPILE_ENCODER
	lodepng_encoder_settings_init(&state->encoder);
#endif /*LODEPNG_COMPILE_ENCODER*/
	lodepng_color_mode_init(&state->info_raw);
	lodepng_info_init(&state->info_png);
	state->error = 1;
}

void lodepng_state_cleanup(LodePNGState* state) {
	lodepng_color_mode_cleanup(&state->info_raw);
	lodepng_info_cleanup(&state->info_png);
}

void lodepng_state_copy(LodePNGState* dest, const LodePNGState* source) {
	lodepng_state_cleanup(dest);
	*dest = *source;
	lodepng_color_mode_init(&dest->info_raw);
	lodepng_info_init(&dest->info_png);
	dest->error = lodepng_color_mode_copy(&dest->info_raw, &source->info_raw); if (dest->error) return;
	dest->error = lodepng_info_copy(&dest->info_png, &source->info_png); if (dest->error) return;
}

#endif /* defined(LODEPNG_COMPILE_ENCODER) */

#ifdef LODEPNG_COMPILE_ENCODER

/* ////////////////////////////////////////////////////////////////////////// */
/* / PNG Encoder                                                            / */
/* ////////////////////////////////////////////////////////////////////////// */


static unsigned writeSignature(ucvector* out) {
	size_t pos = out->size;
	const unsigned char signature[] = { 137, 80, 78, 71, 13, 10, 26, 10 };
	/*8 bytes PNG signature, aka the magic bytes*/
	if (!ucvector_resize(out, out->size + 8)) return 83; /*alloc fail*/
	lodepng_memcpy(out->data + pos, signature, 8);
	return 0;
}

static unsigned addChunk_IHDR(ucvector* out, unsigned w, unsigned h,
	LodePNGColorType colortype, unsigned bitdepth, unsigned interlace_method) {
	unsigned char* chunk, * data;
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 13, "IHDR"));
	data = chunk + 8;

	lodepng_set32bitInt(data + 0, w); /*width*/
	lodepng_set32bitInt(data + 4, h); /*height*/
	data[8] = (unsigned char)bitdepth; /*bit depth*/
	data[9] = (unsigned char)colortype; /*color type*/
	data[10] = 0; /*compression method*/
	data[11] = 0; /*filter method*/
	data[12] = interlace_method; /*interlace method*/

	lodepng_chunk_generate_crc(chunk);
	return 0;
}

/* only adds the chunk if needed (there is a key or palette with alpha) */
static unsigned addChunk_PLTE(ucvector* out, const LodePNGColorMode* info) {
	unsigned char* chunk;
	size_t i, j = 8;

	if (info->palettesize == 0 || info->palettesize > 256) {
		return 68; /*invalid palette size, it is only allowed to be 1-256*/
	}

	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, info->palettesize * 3, "PLTE"));

	for (i = 0; i != info->palettesize; ++i) {
		/*add all channels except alpha channel*/
		chunk[j++] = info->palette[i * 4 + 0];
		chunk[j++] = info->palette[i * 4 + 1];
		chunk[j++] = info->palette[i * 4 + 2];
	}

	lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_tRNS(ucvector* out, const LodePNGColorMode* info) {
	unsigned char* chunk = 0;

	if (info->colortype == LCT_PALETTE) {
		size_t i, amount = info->palettesize;
		/*the tail of palette values that all have 255 as alpha, does not have to be encoded*/
		for (i = info->palettesize; i != 0; --i) {
			if (info->palette[4 * (i - 1) + 3] != 255) break;
			--amount;
		}
		if (amount) {
			CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, amount, "tRNS"));
			/*add the alpha channel values from the palette*/
			for (i = 0; i != amount; ++i) chunk[8 + i] = info->palette[4 * i + 3];
		}
	}
	else if (info->colortype == LCT_GREY) {
		if (info->key_defined) {
			CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 2, "tRNS"));
			chunk[8] = (unsigned char)(info->key_r >> 8);
			chunk[9] = (unsigned char)(info->key_r & 255);
		}
	}
	else if (info->colortype == LCT_RGB) {
		if (info->key_defined) {
			CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 6, "tRNS"));
			chunk[8] = (unsigned char)(info->key_r >> 8);
			chunk[9] = (unsigned char)(info->key_r & 255);
			chunk[10] = (unsigned char)(info->key_g >> 8);
			chunk[11] = (unsigned char)(info->key_g & 255);
			chunk[12] = (unsigned char)(info->key_b >> 8);
			chunk[13] = (unsigned char)(info->key_b & 255);
		}
	}

	if (chunk) lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_IDAT(ucvector* out, const unsigned char* data, size_t datasize,
	LodePNGCompressSettings* zlibsettings) {
	unsigned error = 0;
	unsigned char* zlib = 0;
	size_t zlibsize = 0;

	error = zlib_compress(&zlib, &zlibsize, data, datasize, zlibsettings);
	if (!error) {
		error = lodepng_chunk_createv(out, zlibsize, "IDAT", zlib);
	}
	lodepng_free(zlib);
	return error;
}

static unsigned addChunk_IEND(ucvector* out) {
	return lodepng_chunk_createv(out, 0, "IEND", 0);
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS

static unsigned addChunk_tEXt(ucvector* out, const char* keyword, const char* textstring) {
	unsigned char* chunk = 0;
	size_t keysize = lodepng_strlen(keyword), textsize = lodepng_strlen(textstring);
	size_t size = keysize + 1 + textsize;
	if (keysize < 1 || keysize > 79) return 89; /*error: invalid keyword size*/
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, size, "tEXt"));
	lodepng_memcpy(chunk + 8, keyword, keysize);
	chunk[8 + keysize] = 0; /*null termination char*/
	lodepng_memcpy(chunk + 9 + keysize, textstring, textsize);
	lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_zTXt(ucvector* out, const char* keyword, const char* textstring,
	LodePNGCompressSettings* zlibsettings) {
	unsigned error = 0;
	unsigned char* chunk = 0;
	unsigned char* compressed = 0;
	size_t compressedsize = 0;
	size_t textsize = lodepng_strlen(textstring);
	size_t keysize = lodepng_strlen(keyword);
	if (keysize < 1 || keysize > 79) return 89; /*error: invalid keyword size*/

	error = zlib_compress(&compressed, &compressedsize,
		(const unsigned char*)textstring, textsize, zlibsettings);
	if (!error) {
		size_t size = keysize + 2 + compressedsize;
		error = lodepng_chunk_init(&chunk, out, size, "zTXt");
	}
	if (!error) {
		lodepng_memcpy(chunk + 8, keyword, keysize);
		chunk[8 + keysize] = 0; /*null termination char*/
		chunk[9 + keysize] = 0; /*compression method: 0*/
		lodepng_memcpy(chunk + 10 + keysize, compressed, compressedsize);
		lodepng_chunk_generate_crc(chunk);
	}

	lodepng_free(compressed);
	return error;
}

static unsigned addChunk_iTXt(ucvector* out, unsigned compress, const char* keyword, const char* langtag,
	const char* transkey, const char* textstring, LodePNGCompressSettings* zlibsettings) {
	unsigned error = 0;
	unsigned char* chunk = 0;
	unsigned char* compressed = 0;
	size_t compressedsize = 0;
	size_t textsize = lodepng_strlen(textstring);
	size_t keysize = lodepng_strlen(keyword), langsize = lodepng_strlen(langtag), transsize = lodepng_strlen(transkey);

	if (keysize < 1 || keysize > 79) return 89; /*error: invalid keyword size*/

	if (compress) {
		error = zlib_compress(&compressed, &compressedsize,
			(const unsigned char*)textstring, textsize, zlibsettings);
	}
	if (!error) {
		size_t size = keysize + 3 + langsize + 1 + transsize + 1 + (compress ? compressedsize : textsize);
		error = lodepng_chunk_init(&chunk, out, size, "iTXt");
	}
	if (!error) {
		size_t pos = 8;
		lodepng_memcpy(chunk + pos, keyword, keysize);
		pos += keysize;
		chunk[pos++] = 0; /*null termination char*/
		chunk[pos++] = (compress ? 1 : 0); /*compression flag*/
		chunk[pos++] = 0; /*compression method: 0*/
		lodepng_memcpy(chunk + pos, langtag, langsize);
		pos += langsize;
		chunk[pos++] = 0; /*null termination char*/
		lodepng_memcpy(chunk + pos, transkey, transsize);
		pos += transsize;
		chunk[pos++] = 0; /*null termination char*/
		if (compress) {
			lodepng_memcpy(chunk + pos, compressed, compressedsize);
		}
		else {
			lodepng_memcpy(chunk + pos, textstring, textsize);
		}
		lodepng_chunk_generate_crc(chunk);
	}

	lodepng_free(compressed);
	return error;
}

static unsigned addChunk_bKGD(ucvector* out, const LodePNGInfo* info) {
	unsigned char* chunk = 0;
	if (info->color.colortype == LCT_GREY || info->color.colortype == LCT_GREY_ALPHA) {
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 2, "bKGD"));
		chunk[8] = (unsigned char)(info->background_r >> 8);
		chunk[9] = (unsigned char)(info->background_r & 255);
	}
	else if (info->color.colortype == LCT_RGB || info->color.colortype == LCT_RGBA) {
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 6, "bKGD"));
		chunk[8] = (unsigned char)(info->background_r >> 8);
		chunk[9] = (unsigned char)(info->background_r & 255);
		chunk[10] = (unsigned char)(info->background_g >> 8);
		chunk[11] = (unsigned char)(info->background_g & 255);
		chunk[12] = (unsigned char)(info->background_b >> 8);
		chunk[13] = (unsigned char)(info->background_b & 255);
	}
	else if (info->color.colortype == LCT_PALETTE) {
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 1, "bKGD"));
		chunk[8] = (unsigned char)(info->background_r & 255); /*palette index*/
	}
	if (chunk) lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_tIME(ucvector* out, const LodePNGTime* time) {
	unsigned char* chunk;
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 7, "tIME"));
	chunk[8] = (unsigned char)(time->year >> 8);
	chunk[9] = (unsigned char)(time->year & 255);
	chunk[10] = (unsigned char)time->month;
	chunk[11] = (unsigned char)time->day;
	chunk[12] = (unsigned char)time->hour;
	chunk[13] = (unsigned char)time->minute;
	chunk[14] = (unsigned char)time->second;
	lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_pHYs(ucvector* out, const LodePNGInfo* info) {
	unsigned char* chunk;
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 9, "pHYs"));
	lodepng_set32bitInt(chunk + 8, info->phys_x);
	lodepng_set32bitInt(chunk + 12, info->phys_y);
	chunk[16] = info->phys_unit;
	lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_gAMA(ucvector* out, const LodePNGInfo* info) {
	unsigned char* chunk;
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 4, "gAMA"));
	lodepng_set32bitInt(chunk + 8, info->gama_gamma);
	lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_cHRM(ucvector* out, const LodePNGInfo* info) {
	unsigned char* chunk;
	CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 32, "cHRM"));
	lodepng_set32bitInt(chunk + 8, info->chrm_white_x);
	lodepng_set32bitInt(chunk + 12, info->chrm_white_y);
	lodepng_set32bitInt(chunk + 16, info->chrm_red_x);
	lodepng_set32bitInt(chunk + 20, info->chrm_red_y);
	lodepng_set32bitInt(chunk + 24, info->chrm_green_x);
	lodepng_set32bitInt(chunk + 28, info->chrm_green_y);
	lodepng_set32bitInt(chunk + 32, info->chrm_blue_x);
	lodepng_set32bitInt(chunk + 36, info->chrm_blue_y);
	lodepng_chunk_generate_crc(chunk);
	return 0;
}

static unsigned addChunk_sRGB(ucvector* out, const LodePNGInfo* info) {
	unsigned char data = info->srgb_intent;
	return lodepng_chunk_createv(out, 1, "sRGB", &data);
}

static unsigned addChunk_iCCP(ucvector* out, const LodePNGInfo* info, LodePNGCompressSettings* zlibsettings) {
	unsigned error = 0;
	unsigned char* chunk = 0;
	unsigned char* compressed = 0;
	size_t compressedsize = 0;
	size_t keysize = lodepng_strlen(info->iccp_name);

	if (keysize < 1 || keysize > 79) return 89; /*error: invalid keyword size*/
	error = zlib_compress(&compressed, &compressedsize,
		info->iccp_profile, info->iccp_profile_size, zlibsettings);
	if (!error) {
		size_t size = keysize + 2 + compressedsize;
		error = lodepng_chunk_init(&chunk, out, size, "iCCP");
	}
	if (!error) {
		lodepng_memcpy(chunk + 8, info->iccp_name, keysize);
		chunk[8 + keysize] = 0; /*null termination char*/
		chunk[9 + keysize] = 0; /*compression method: 0*/
		lodepng_memcpy(chunk + 10 + keysize, compressed, compressedsize);
		lodepng_chunk_generate_crc(chunk);
	}

	lodepng_free(compressed);
	return error;
}

static unsigned addChunk_sBIT(ucvector* out, const LodePNGInfo* info) {
	unsigned bitdepth = (info->color.colortype == LCT_PALETTE) ? 8 : info->color.bitdepth;
	unsigned char* chunk = 0;
	if (info->color.colortype == LCT_GREY) {
		if (info->sbit_r == 0 || info->sbit_r > bitdepth) return 115;
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 1, "sBIT"));
		chunk[8] = info->sbit_r;
	}
	else if (info->color.colortype == LCT_RGB || info->color.colortype == LCT_PALETTE) {
		if (info->sbit_r == 0 || info->sbit_g == 0 || info->sbit_b == 0) return 115;
		if (info->sbit_r > bitdepth || info->sbit_g > bitdepth || info->sbit_b > bitdepth) return 115;
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 3, "sBIT"));
		chunk[8] = info->sbit_r;
		chunk[9] = info->sbit_g;
		chunk[10] = info->sbit_b;
	}
	else if (info->color.colortype == LCT_GREY_ALPHA) {
		if (info->sbit_r == 0 || info->sbit_a == 0) return 115;
		if (info->sbit_r > bitdepth || info->sbit_a > bitdepth) return 115;
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 2, "sBIT"));
		chunk[8] = info->sbit_r;
		chunk[9] = info->sbit_a;
	}
	else if (info->color.colortype == LCT_RGBA) {
		if (info->sbit_r == 0 || info->sbit_g == 0 || info->sbit_b == 0 || info->sbit_a == 0 ||
			info->sbit_r > bitdepth || info->sbit_g > bitdepth ||
			info->sbit_b > bitdepth || info->sbit_a > bitdepth) {
			return 115;
		}
		CERROR_TRY_RETURN(lodepng_chunk_init(&chunk, out, 4, "sBIT"));
		chunk[8] = info->sbit_r;
		chunk[9] = info->sbit_g;
		chunk[10] = info->sbit_b;
		chunk[11] = info->sbit_a;
	}
	if (chunk) lodepng_chunk_generate_crc(chunk);
	return 0;
}

#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

static void filterScanline(unsigned char* out, const unsigned char* scanline, const unsigned char* prevline,
	size_t length, size_t bytewidth, unsigned char filterType) {
	size_t i;
	switch (filterType) {
	case 0: /*None*/
		for (i = 0; i != length; ++i) out[i] = scanline[i];
		break;
	case 1: /*Sub*/
		for (i = 0; i != bytewidth; ++i) out[i] = scanline[i];
		for (i = bytewidth; i < length; ++i) out[i] = scanline[i] - scanline[i - bytewidth];
		break;
	case 2: /*Up*/
		if (prevline) {
			for (i = 0; i != length; ++i) out[i] = scanline[i] - prevline[i];
		}
		else {
			for (i = 0; i != length; ++i) out[i] = scanline[i];
		}
		break;
	case 3: /*Average*/
		if (prevline) {
			for (i = 0; i != bytewidth; ++i) out[i] = scanline[i] - (prevline[i] >> 1);
			for (i = bytewidth; i < length; ++i) out[i] = scanline[i] - ((scanline[i - bytewidth] + prevline[i]) >> 1);
		}
		else {
			for (i = 0; i != bytewidth; ++i) out[i] = scanline[i];
			for (i = bytewidth; i < length; ++i) out[i] = scanline[i] - (scanline[i - bytewidth] >> 1);
		}
		break;
	case 4: /*Paeth*/
		if (prevline) {
			/*paethPredictor(0, prevline[i], 0) is always prevline[i]*/
			for (i = 0; i != bytewidth; ++i) out[i] = (scanline[i] - prevline[i]);
			for (i = bytewidth; i < length; ++i) {
				out[i] = (scanline[i] - paethPredictor(scanline[i - bytewidth], prevline[i], prevline[i - bytewidth]));
			}
		}
		else {
			for (i = 0; i != bytewidth; ++i) out[i] = scanline[i];
			/*paethPredictor(scanline[i - bytewidth], 0, 0) is always scanline[i - bytewidth]*/
			for (i = bytewidth; i < length; ++i) out[i] = (scanline[i] - scanline[i - bytewidth]);
		}
		break;
	default: return; /*invalid filter type given*/
	}
}

/* integer binary logarithm, max return value is 31 */
static size_t ilog2(size_t i) {
	size_t result = 0;
	if (i >= 65536) { result += 16; i >>= 16; }
	if (i >= 256) { result += 8; i >>= 8; }
	if (i >= 16) { result += 4; i >>= 4; }
	if (i >= 4) { result += 2; i >>= 2; }
	if (i >= 2) { result += 1; /*i >>= 1;*/ }
	return result;
}

/* integer approximation for i * log2(i), helper function for LFS_ENTROPY */
static size_t ilog2i(size_t i) {
	size_t l;
	if (i == 0) return 0;
	l = ilog2(i);
	/* approximate i*log2(i): l is integer logarithm, ((i - (1u << l)) << 1u)
	linearly approximates the missing fractional part multiplied by i */
	return i * l + ((i - (1u << l)) << 1u);
}

static unsigned filter(unsigned char* out, const unsigned char* in, unsigned w, unsigned h,
	const LodePNGColorMode* color, const LodePNGEncoderSettings* settings) {
	/*
	For PNG filter method 0
	out must be a buffer with as size: h + (w * h * bpp + 7u) / 8u, because there are
	the scanlines with 1 extra byte per scanline
	*/

	unsigned bpp = lodepng_get_bpp(color);
	/*the width of a scanline in bytes, not including the filter type*/
	size_t linebytes = lodepng_get_raw_size_idat(w, 1, bpp) - 1u;

	/*bytewidth is used for filtering, is 1 when bpp < 8, number of bytes per pixel otherwise*/
	size_t bytewidth = (bpp + 7u) / 8u;
	const unsigned char* prevline = 0;
	unsigned x, y;
	unsigned error = 0;
	LodePNGFilterStrategy strategy = settings->filter_strategy;

	/*
	There is a heuristic called the minimum sum of absolute differences heuristic, suggested by the PNG standard:
	 *  If the image type is Palette, or the bit depth is smaller than 8, then do not filter the image (i.e.
		use fixed filtering, with the filter None).
	 * (The other case) If the image type is Grayscale or RGB (with or without Alpha), and the bit depth is
	   not smaller than 8, then use adaptive filtering heuristic as follows: independently for each row, apply
	   all five filters and select the filter that produces the smallest sum of absolute values per row.
	This heuristic is used if filter strategy is LFS_MINSUM and filter_palette_zero is true.

	If filter_palette_zero is true and filter_strategy is not LFS_MINSUM, the above heuristic is followed,
	but for "the other case", whatever strategy filter_strategy is set to instead of the minimum sum
	heuristic is used.
	*/
	if (settings->filter_palette_zero &&
		(color->colortype == LCT_PALETTE || color->bitdepth < 8)) strategy = LFS_ZERO;

	if (bpp == 0) return 31; /*error: invalid color type*/

	if (strategy >= LFS_ZERO && strategy <= LFS_FOUR) {
		unsigned char type = (unsigned char)strategy;
		for (y = 0; y != h; ++y) {
			size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
			size_t inindex = linebytes * y;
			out[outindex] = type; /*filter type byte*/
			filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, type);
			prevline = &in[inindex];
		}
	}
	else if (strategy == LFS_MINSUM) {
		/*adaptive filtering*/
		unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
		size_t smallest = 0;
		unsigned char type, bestType = 0;

		for (type = 0; type != 5; ++type) {
			attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
			if (!attempt[type]) error = 83; /*alloc fail*/
		}

		if (!error) {
			for (y = 0; y != h; ++y) {
				/*try the 5 filter types*/
				for (type = 0; type != 5; ++type) {
					size_t sum = 0;
					filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);

					/*calculate the sum of the result*/
					if (type == 0) {
						for (x = 0; x != linebytes; ++x) sum += (unsigned char)(attempt[type][x]);
					}
					else {
						for (x = 0; x != linebytes; ++x) {
							/*For differences, each byte should be treated as signed, values above 127 are negative
							(converted to signed char). Filtertype 0 isn't a difference though, so use unsigned there.
							This means filtertype 0 is almost never chosen, but that is justified.*/
							unsigned char s = attempt[type][x];
							sum += s < 128 ? s : (255U - s);
						}
					}

					/*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
					if (type == 0 || sum < smallest) {
						bestType = type;
						smallest = sum;
					}
				}

				prevline = &in[y * linebytes];

				/*now fill the out values*/
				out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
				for (x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
			}
		}

		for (type = 0; type != 5; ++type) lodepng_free(attempt[type]);
	}
	else if (strategy == LFS_ENTROPY) {
		unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
		size_t bestSum = 0;
		unsigned type, bestType = 0;
		unsigned count[256];

		for (type = 0; type != 5; ++type) {
			attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
			if (!attempt[type]) error = 83; /*alloc fail*/
		}

		if (!error) {
			for (y = 0; y != h; ++y) {
				/*try the 5 filter types*/
				for (type = 0; type != 5; ++type) {
					size_t sum = 0;
					filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);
					lodepng_memset(count, 0, 256 * sizeof(*count));
					for (x = 0; x != linebytes; ++x) ++count[attempt[type][x]];
					++count[type]; /*the filter type itself is part of the scanline*/
					for (x = 0; x != 256; ++x) {
						sum += ilog2i(count[x]);
					}
					/*check if this is smallest sum (or if type == 0 it's the first case so always store the values)*/
					if (type == 0 || sum > bestSum) {
						bestType = type;
						bestSum = sum;
					}
				}

				prevline = &in[y * linebytes];

				/*now fill the out values*/
				out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
				for (x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
			}
		}

		for (type = 0; type != 5; ++type) lodepng_free(attempt[type]);
	}
	else if (strategy == LFS_PREDEFINED) {
		for (y = 0; y != h; ++y) {
			size_t outindex = (1 + linebytes) * y; /*the extra filterbyte added to each row*/
			size_t inindex = linebytes * y;
			unsigned char type = settings->predefined_filters[y];
			out[outindex] = type; /*filter type byte*/
			filterScanline(&out[outindex + 1], &in[inindex], prevline, linebytes, bytewidth, type);
			prevline = &in[inindex];
		}
	}
	else if (strategy == LFS_BRUTE_FORCE) {
		/*brute force filter chooser.
		deflate the scanline after every filter attempt to see which one deflates best.
		This is very slow and gives only slightly smaller, sometimes even larger, result*/
		size_t size[5];
		unsigned char* attempt[5]; /*five filtering attempts, one for each filter type*/
		size_t smallest = 0;
		unsigned type = 0, bestType = 0;
		unsigned char* dummy;
		LodePNGCompressSettings zlibsettings;
		lodepng_memcpy(&zlibsettings, &settings->zlibsettings, sizeof(LodePNGCompressSettings));
		/*use fixed tree on the attempts so that the tree is not adapted to the filtertype on purpose,
		to simulate the true case where the tree is the same for the whole image. Sometimes it gives
		better result with dynamic tree anyway. Using the fixed tree sometimes gives worse, but in rare
		cases better compression. It does make this a bit less slow, so it's worth doing this.*/
		zlibsettings.btype = 1;
		/*a custom encoder likely doesn't read the btype setting and is optimized for complete PNG
		images only, so disable it*/
		zlibsettings.custom_zlib = 0;
		zlibsettings.custom_deflate = 0;
		for (type = 0; type != 5; ++type) {
			attempt[type] = (unsigned char*)lodepng_malloc(linebytes);
			if (!attempt[type]) error = 83; /*alloc fail*/
		}
		if (!error) {
			for (y = 0; y != h; ++y) /*try the 5 filter types*/ {
				for (type = 0; type != 5; ++type) {
					unsigned testsize = (unsigned)linebytes;
					/*if(testsize > 8) testsize /= 8;*/ /*it already works good enough by testing a part of the row*/

					filterScanline(attempt[type], &in[y * linebytes], prevline, linebytes, bytewidth, type);
					size[type] = 0;
					dummy = 0;
					zlib_compress(&dummy, &size[type], attempt[type], testsize, &zlibsettings);
					lodepng_free(dummy);
					/*check if this is smallest size (or if type == 0 it's the first case so always store the values)*/
					if (type == 0 || size[type] < smallest) {
						bestType = type;
						smallest = size[type];
					}
				}
				prevline = &in[y * linebytes];
				out[y * (linebytes + 1)] = bestType; /*the first byte of a scanline will be the filter type*/
				for (x = 0; x != linebytes; ++x) out[y * (linebytes + 1) + 1 + x] = attempt[bestType][x];
			}
		}
		for (type = 0; type != 5; ++type) lodepng_free(attempt[type]);
	}
	else return 88; /* unknown filter strategy */

	return error;
}

static void addPaddingBits(unsigned char* out, const unsigned char* in,
	size_t olinebits, size_t ilinebits, unsigned h) {
	/*The opposite of the removePaddingBits function
	olinebits must be >= ilinebits*/
	unsigned y;
	size_t diff = olinebits - ilinebits;
	size_t obp = 0, ibp = 0; /*bit pointers*/
	for (y = 0; y != h; ++y) {
		size_t x;
		for (x = 0; x < ilinebits; ++x) {
			unsigned char bit = readBitFromReversedStream(&ibp, in);
			setBitOfReversedStream(&obp, out, bit);
		}
		/*obp += diff; --> no, fill in some value in the padding bits too, to avoid
		"Use of uninitialised value of size ###" warning from valgrind*/
		for (x = 0; x != diff; ++x) setBitOfReversedStream(&obp, out, 0);
	}
}

/*
in: non-interlaced image with size w*h
out: the same pixels, but re-ordered according to PNG's Adam7 interlacing, with
 no padding bits between scanlines, but between reduced images so that each
 reduced image starts at a byte.
bpp: bits per pixel
there are no padding bits, not between scanlines, not between reduced images
in has the following size in bits: w * h * bpp.
out is possibly bigger due to padding bits between reduced images
NOTE: comments about padding bits are only relevant if bpp < 8
*/
static void Adam7_interlace(unsigned char* out, const unsigned char* in, unsigned w, unsigned h, unsigned bpp) {
	unsigned passw[7], passh[7];
	size_t filter_passstart[8], padded_passstart[8], passstart[8];
	unsigned i;

	Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

	if (bpp >= 8) {
		for (i = 0; i != 7; ++i) {
			unsigned x, y, b;
			size_t bytewidth = bpp / 8u;
			for (y = 0; y < passh[i]; ++y)
				for (x = 0; x < passw[i]; ++x) {
					size_t pixelinstart = ((ADAM7_IY[i] + y * ADAM7_DY[i]) * w + ADAM7_IX[i] + x * ADAM7_DX[i]) * bytewidth;
					size_t pixeloutstart = passstart[i] + (y * passw[i] + x) * bytewidth;
					for (b = 0; b < bytewidth; ++b) {
						out[pixeloutstart + b] = in[pixelinstart + b];
					}
				}
		}
	}
	else /*bpp < 8: Adam7 with pixels < 8 bit is a bit trickier: with bit pointers*/ {
		for (i = 0; i != 7; ++i) {
			unsigned x, y, b;
			unsigned ilinebits = bpp * passw[i];
			unsigned olinebits = bpp * w;
			size_t obp, ibp; /*bit pointers (for out and in buffer)*/
			for (y = 0; y < passh[i]; ++y)
				for (x = 0; x < passw[i]; ++x) {
					ibp = (ADAM7_IY[i] + y * ADAM7_DY[i]) * olinebits + (ADAM7_IX[i] + x * ADAM7_DX[i]) * bpp;
					obp = (8 * passstart[i]) + (y * ilinebits + x * bpp);
					for (b = 0; b < bpp; ++b) {
						unsigned char bit = readBitFromReversedStream(&ibp, in);
						setBitOfReversedStream(&obp, out, bit);
					}
				}
		}
	}
}

/*out must be buffer big enough to contain uncompressed IDAT chunk data, and in must contain the full image.
return value is error**/
static unsigned preProcessScanlines(unsigned char** out, size_t* outsize, const unsigned char* in,
	unsigned w, unsigned h,
	const LodePNGInfo* info_png, const LodePNGEncoderSettings* settings) {
	/*
	This function converts the pure 2D image with the PNG's colortype, into filtered-padded-interlaced data. Steps:
	*) if no Adam7: 1) add padding bits (= possible extra bits per scanline if bpp < 8) 2) filter
	*) if adam7: 1) Adam7_interlace 2) 7x add padding bits 3) 7x filter
	*/
	unsigned bpp = lodepng_get_bpp(&info_png->color);
	unsigned error = 0;

	if (info_png->interlace_method == 0) {
		*outsize = h + (h * ((w * bpp + 7u) / 8u)); /*image size plus an extra byte per scanline + possible padding bits*/
		*out = (unsigned char*)lodepng_malloc(*outsize);
		if (!(*out) && (*outsize)) error = 83; /*alloc fail*/

		if (!error) {
			/*non multiple of 8 bits per scanline, padding bits needed per scanline*/
			if (bpp < 8 && w * bpp != ((w * bpp + 7u) / 8u) * 8u) {
				unsigned char* padded = (unsigned char*)lodepng_malloc(h * ((w * bpp + 7u) / 8u));
				if (!padded) error = 83; /*alloc fail*/
				if (!error) {
					addPaddingBits(padded, in, ((w * bpp + 7u) / 8u) * 8u, w * bpp, h);
					error = filter(*out, padded, w, h, &info_png->color, settings);
				}
				lodepng_free(padded);
			}
			else {
				/*we can immediately filter into the out buffer, no other steps needed*/
				error = filter(*out, in, w, h, &info_png->color, settings);
			}
		}
	}
	else /*interlace_method is 1 (Adam7)*/ {
		unsigned passw[7], passh[7];
		size_t filter_passstart[8], padded_passstart[8], passstart[8];
		unsigned char* adam7;

		Adam7_getpassvalues(passw, passh, filter_passstart, padded_passstart, passstart, w, h, bpp);

		*outsize = filter_passstart[7]; /*image size plus an extra byte per scanline + possible padding bits*/
		*out = (unsigned char*)lodepng_malloc(*outsize);
		if (!(*out)) error = 83; /*alloc fail*/

		adam7 = (unsigned char*)lodepng_malloc(passstart[7]);
		if (!adam7 && passstart[7]) error = 83; /*alloc fail*/

		if (!error) {
			unsigned i;

			Adam7_interlace(adam7, in, w, h, bpp);
			for (i = 0; i != 7; ++i) {
				if (bpp < 8) {
					unsigned char* padded = (unsigned char*)lodepng_malloc(padded_passstart[i + 1] - padded_passstart[i]);
					if (!padded) ERROR_BREAK(83); /*alloc fail*/
					addPaddingBits(padded, &adam7[passstart[i]],
						((passw[i] * bpp + 7u) / 8u) * 8u, passw[i] * bpp, passh[i]);
					error = filter(&(*out)[filter_passstart[i]], padded,
						passw[i], passh[i], &info_png->color, settings);
					lodepng_free(padded);
				}
				else {
					error = filter(&(*out)[filter_passstart[i]], &adam7[padded_passstart[i]],
						passw[i], passh[i], &info_png->color, settings);
				}

				if (error) break;
			}
		}

		lodepng_free(adam7);
	}

	return error;
}

#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
static unsigned addUnknownChunks(ucvector* out, unsigned char* data, size_t datasize) {
	unsigned char* inchunk = data;
	while ((size_t)(inchunk - data) < datasize) {
		CERROR_TRY_RETURN(lodepng_chunk_append(&out->data, &out->size, inchunk));
		out->allocsize = out->size; /*fix the allocsize again*/
		inchunk = lodepng_chunk_next(inchunk, data + datasize);
	}
	return 0;
}

static unsigned isGrayICCProfile(const unsigned char* profile, unsigned size) {
	/*
	It is a gray profile if bytes 16-19 are "GRAY", rgb profile if bytes 16-19
	are "RGB ". We do not perform any full parsing of the ICC profile here, other
	than check those 4 bytes to grayscale profile. Other than that, validity of
	the profile is not checked. This is needed only because the PNG specification
	requires using a non-gray color model if there is an ICC profile with "RGB "
	(sadly limiting compression opportunities if the input data is grayscale RGB
	data), and requires using a gray color model if it is "GRAY".
	*/
	if (size < 20) return 0;
	return profile[16] == 'G' && profile[17] == 'R' && profile[18] == 'A' && profile[19] == 'Y';
}

static unsigned isRGBICCProfile(const unsigned char* profile, unsigned size) {
	/* See comment in isGrayICCProfile*/
	if (size < 20) return 0;
	return profile[16] == 'R' && profile[17] == 'G' && profile[18] == 'B' && profile[19] == ' ';
}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/

unsigned lodepng_encode(unsigned char** out, size_t* outsize,
	const unsigned char* image, unsigned w, unsigned h,
	LodePNGState* state) {
	unsigned char* data = 0; /*uncompressed version of the IDAT chunk data*/
	size_t datasize = 0;
	ucvector outv = ucvector_init(NULL, 0);
	LodePNGInfo info;
	const LodePNGInfo* info_png = &state->info_png;
	LodePNGColorMode auto_color;

	lodepng_info_init(&info);
	lodepng_color_mode_init(&auto_color);

	/*provide some proper output values if error will happen*/
	*out = 0;
	*outsize = 0;
	state->error = 0;

	/*check input values validity*/
	if ((info_png->color.colortype == LCT_PALETTE || state->encoder.force_palette)
		&& (info_png->color.palettesize == 0 || info_png->color.palettesize > 256)) {
		/*this error is returned even if auto_convert is enabled and thus encoder could
		generate the palette by itself: while allowing this could be possible in theory,
		it may complicate the code or edge cases, and always requiring to give a palette
		when setting this color type is a simpler contract*/
		state->error = 68; /*invalid palette size, it is only allowed to be 1-256*/
		goto cleanup;
	}
	if (state->encoder.zlibsettings.btype > 2) {
		state->error = 61; /*error: invalid btype*/
		goto cleanup;
	}
	if (info_png->interlace_method > 1) {
		state->error = 71; /*error: invalid interlace mode*/
		goto cleanup;
	}
	state->error = checkColorValidity(info_png->color.colortype, info_png->color.bitdepth);
	if (state->error) goto cleanup; /*error: invalid color type given*/
	state->error = checkColorValidity(state->info_raw.colortype, state->info_raw.bitdepth);
	if (state->error) goto cleanup; /*error: invalid color type given*/

	/* color convert and compute scanline filter types */
	lodepng_info_copy(&info, &state->info_png);
	if (state->encoder.auto_convert) {
		LodePNGColorStats stats;
		unsigned allow_convert = 1;
		lodepng_color_stats_init(&stats);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		if (info_png->iccp_defined &&
			isGrayICCProfile(info_png->iccp_profile, info_png->iccp_profile_size)) {
			/*the PNG specification does not allow to use palette with a GRAY ICC profile, even
			if the palette has only gray colors, so disallow it.*/
			stats.allow_palette = 0;
		}
		if (info_png->iccp_defined &&
			isRGBICCProfile(info_png->iccp_profile, info_png->iccp_profile_size)) {
			/*the PNG specification does not allow to use grayscale color with RGB ICC profile, so disallow gray.*/
			stats.allow_greyscale = 0;
		}
#endif /* LODEPNG_COMPILE_ANCILLARY_CHUNKS */
		state->error = lodepng_compute_color_stats(&stats, image, w, h, &state->info_raw);
		if (state->error) goto cleanup;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		if (info_png->background_defined) {
			/*the background chunk's color must be taken into account as well*/
			unsigned r = 0, g = 0, b = 0;
			LodePNGColorMode mode16 = lodepng_color_mode_make(LCT_RGB, 16);
			lodepng_convert_rgb(&r, &g, &b,
				info_png->background_r, info_png->background_g, info_png->background_b, &mode16, &info_png->color);
			state->error = lodepng_color_stats_add(&stats, r, g, b, 65535);
			if (state->error) goto cleanup;
		}
#endif /* LODEPNG_COMPILE_ANCILLARY_CHUNKS */
		state->error = auto_choose_color(&auto_color, &state->info_raw, &stats);
		if (state->error) goto cleanup;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		if (info_png->sbit_defined) {
			/*if sbit is defined, due to strict requirements of which sbit values can be present for which color modes,
			auto_convert can't be done in many cases. However, do support a few cases here.
			TODO: more conversions may be possible, and it may also be possible to get a more appropriate color type out of
				  auto_choose_color if knowledge about sbit is used beforehand
			*/
			unsigned sbit_max = LODEPNG_MAX(LODEPNG_MAX(LODEPNG_MAX(info_png->sbit_r, info_png->sbit_g),
				info_png->sbit_b), info_png->sbit_a);
			unsigned equal = (!info_png->sbit_g || info_png->sbit_g == info_png->sbit_r)
				&& (!info_png->sbit_b || info_png->sbit_b == info_png->sbit_r)
				&& (!info_png->sbit_a || info_png->sbit_a == info_png->sbit_r);
			allow_convert = 0;
			if (info.color.colortype == LCT_PALETTE &&
				auto_color.colortype == LCT_PALETTE) {
				/* input and output are palette, and in this case it may happen that palette data is
				expected to be copied from info_raw into the info_png */
				allow_convert = 1;
			}
			/*going from 8-bit RGB to palette (or 16-bit as long as sbit_max <= 8) is possible
			since both are 8-bit RGB for sBIT's purposes*/
			if (info.color.colortype == LCT_RGB &&
				auto_color.colortype == LCT_PALETTE && sbit_max <= 8) {
				allow_convert = 1;
			}
			/*going from 8-bit RGBA to palette is also ok but only if sbit_a is exactly 8*/
			if (info.color.colortype == LCT_RGBA && auto_color.colortype == LCT_PALETTE &&
				info_png->sbit_a == 8 && sbit_max <= 8) {
				allow_convert = 1;
			}
			/*going from 16-bit RGB(A) to 8-bit RGB(A) is ok if all sbit values are <= 8*/
			if ((info.color.colortype == LCT_RGB || info.color.colortype == LCT_RGBA) && info.color.bitdepth == 16 &&
				auto_color.colortype == info.color.colortype && auto_color.bitdepth == 8 &&
				sbit_max <= 8) {
				allow_convert = 1;
			}
			/*going to less channels is ok if all bit values are equal (all possible values in sbit,
			  as well as the chosen bitdepth of the result). Due to how auto_convert works,
			  we already know that auto_color.colortype has less than or equal amount of channels than
			  info.colortype. Palette is not used here. This conversion is not allowed if
			  info_png->sbit_r < auto_color.bitdepth, because specifically for alpha, non-presence of
			  an sbit value heavily implies that alpha's bit depth is equal to the PNG bit depth (rather
			  than the bit depths set in the r, g and b sbit values, by how the PNG specification describes
			  handling tRNS chunk case with sBIT), so be conservative here about ignoring user input.*/
			if (info.color.colortype != LCT_PALETTE && auto_color.colortype != LCT_PALETTE &&
				equal && info_png->sbit_r == auto_color.bitdepth) {
				allow_convert = 1;
			}
		}
#endif
		if (state->encoder.force_palette) {
			if (info.color.colortype != LCT_GREY && info.color.colortype != LCT_GREY_ALPHA &&
				(auto_color.colortype == LCT_GREY || auto_color.colortype == LCT_GREY_ALPHA)) {
				/*user speficially forced a PLTE palette, so cannot convert to grayscale types because
				the PNG specification only allows writing a suggested palette in PLTE for truecolor types*/
				allow_convert = 0;
			}
		}
		if (allow_convert) {
			lodepng_color_mode_copy(&info.color, &auto_color);
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
			/*also convert the background chunk*/
			if (info_png->background_defined) {
				if (lodepng_convert_rgb(&info.background_r, &info.background_g, &info.background_b,
					info_png->background_r, info_png->background_g, info_png->background_b, &info.color, &info_png->color)) {
					state->error = 104;
					goto cleanup;
				}
			}
#endif /* LODEPNG_COMPILE_ANCILLARY_CHUNKS */
		}
	}
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	if (info_png->iccp_defined) {
		unsigned gray_icc = isGrayICCProfile(info_png->iccp_profile, info_png->iccp_profile_size);
		unsigned rgb_icc = isRGBICCProfile(info_png->iccp_profile, info_png->iccp_profile_size);
		unsigned gray_png = info.color.colortype == LCT_GREY || info.color.colortype == LCT_GREY_ALPHA;
		if (!gray_icc && !rgb_icc) {
			state->error = 100; /* Disallowed profile color type for PNG */
			goto cleanup;
		}
		if (gray_icc != gray_png) {
			/*Not allowed to use RGB/RGBA/palette with GRAY ICC profile or vice versa,
			or in case of auto_convert, it wasn't possible to find appropriate model*/
			state->error = state->encoder.auto_convert ? 102 : 101;
			goto cleanup;
		}
	}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
	if (!lodepng_color_mode_equal(&state->info_raw, &info.color)) {
		unsigned char* converted;
		size_t size = ((size_t)w * (size_t)h * (size_t)lodepng_get_bpp(&info.color) + 7u) / 8u;

		converted = (unsigned char*)lodepng_malloc(size);
		if (!converted && size) state->error = 83; /*alloc fail*/
		if (!state->error) {
			state->error = lodepng_convert(converted, image, &info.color, &state->info_raw, w, h);
		}
		if (!state->error) {
			state->error = preProcessScanlines(&data, &datasize, converted, w, h, &info, &state->encoder);
		}
		lodepng_free(converted);
		if (state->error) goto cleanup;
	}
	else {
		state->error = preProcessScanlines(&data, &datasize, image, w, h, &info, &state->encoder);
		if (state->error) goto cleanup;
	}

	/* output all PNG chunks */ {
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		size_t i;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		/*write signature and chunks*/
		state->error = writeSignature(&outv);
		if (state->error) goto cleanup;
		/*IHDR*/
		state->error = addChunk_IHDR(&outv, w, h, info.color.colortype, info.color.bitdepth, info.interlace_method);
		if (state->error) goto cleanup;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*unknown chunks between IHDR and PLTE*/
		if (info.unknown_chunks_data[0]) {
			state->error = addUnknownChunks(&outv, info.unknown_chunks_data[0], info.unknown_chunks_size[0]);
			if (state->error) goto cleanup;
		}
		/*color profile chunks must come before PLTE */
		if (info.iccp_defined) {
			state->error = addChunk_iCCP(&outv, &info, &state->encoder.zlibsettings);
			if (state->error) goto cleanup;
		}
		if (info.srgb_defined) {
			state->error = addChunk_sRGB(&outv, &info);
			if (state->error) goto cleanup;
		}
		if (info.gama_defined) {
			state->error = addChunk_gAMA(&outv, &info);
			if (state->error) goto cleanup;
		}
		if (info.chrm_defined) {
			state->error = addChunk_cHRM(&outv, &info);
			if (state->error) goto cleanup;
		}
		if (info_png->sbit_defined) {
			state->error = addChunk_sBIT(&outv, &info);
			if (state->error) goto cleanup;
		}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		/*PLTE*/
		if (info.color.colortype == LCT_PALETTE) {
			state->error = addChunk_PLTE(&outv, &info.color);
			if (state->error) goto cleanup;
		}
		if (state->encoder.force_palette && (info.color.colortype == LCT_RGB || info.color.colortype == LCT_RGBA)) {
			/*force_palette means: write suggested palette for truecolor in PLTE chunk*/
			state->error = addChunk_PLTE(&outv, &info.color);
			if (state->error) goto cleanup;
		}
		/*tRNS (this will only add if when necessary) */
		state->error = addChunk_tRNS(&outv, &info.color);
		if (state->error) goto cleanup;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*bKGD (must come between PLTE and the IDAt chunks*/
		if (info.background_defined) {
			state->error = addChunk_bKGD(&outv, &info);
			if (state->error) goto cleanup;
		}
		/*pHYs (must come before the IDAT chunks)*/
		if (info.phys_defined) {
			state->error = addChunk_pHYs(&outv, &info);
			if (state->error) goto cleanup;
		}

		/*unknown chunks between PLTE and IDAT*/
		if (info.unknown_chunks_data[1]) {
			state->error = addUnknownChunks(&outv, info.unknown_chunks_data[1], info.unknown_chunks_size[1]);
			if (state->error) goto cleanup;
		}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		/*IDAT (multiple IDAT chunks must be consecutive)*/
		state->error = addChunk_IDAT(&outv, data, datasize, &state->encoder.zlibsettings);
		if (state->error) goto cleanup;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
		/*tIME*/
		if (info.time_defined) {
			state->error = addChunk_tIME(&outv, &info.time);
			if (state->error) goto cleanup;
		}
		/*tEXt and/or zTXt*/
		for (i = 0; i != info.text_num; ++i) {
			if (lodepng_strlen(info.text_keys[i]) > 79) {
				state->error = 66; /*text chunk too large*/
				goto cleanup;
			}
			if (lodepng_strlen(info.text_keys[i]) < 1) {
				state->error = 67; /*text chunk too small*/
				goto cleanup;
			}
			if (state->encoder.text_compression) {
				state->error = addChunk_zTXt(&outv, info.text_keys[i], info.text_strings[i], &state->encoder.zlibsettings);
				if (state->error) goto cleanup;
			}
			else {
				state->error = addChunk_tEXt(&outv, info.text_keys[i], info.text_strings[i]);
				if (state->error) goto cleanup;
			}
		}
		/*LodePNG version id in text chunk*/
		if (state->encoder.add_id) {
			unsigned already_added_id_text = 0;
			for (i = 0; i != info.text_num; ++i) {
				const char* k = info.text_keys[i];
				/* Could use strcmp, but we're not calling or reimplementing this C library function for this use only */
				if (k[0] == 'L' && k[1] == 'o' && k[2] == 'd' && k[3] == 'e' &&
					k[4] == 'P' && k[5] == 'N' && k[6] == 'G' && k[7] == '\0') {
					already_added_id_text = 1;
					break;
				}
			}
			if (already_added_id_text == 0) {
				state->error = addChunk_tEXt(&outv, "LodePNG", LODEPNG_VERSION_STRING); /*it's shorter as tEXt than as zTXt chunk*/
				if (state->error) goto cleanup;
			}
		}
		/*iTXt*/
		for (i = 0; i != info.itext_num; ++i) {
			if (lodepng_strlen(info.itext_keys[i]) > 79) {
				state->error = 66; /*text chunk too large*/
				goto cleanup;
			}
			if (lodepng_strlen(info.itext_keys[i]) < 1) {
				state->error = 67; /*text chunk too small*/
				goto cleanup;
			}
			state->error = addChunk_iTXt(
				&outv, state->encoder.text_compression,
				info.itext_keys[i], info.itext_langtags[i], info.itext_transkeys[i], info.itext_strings[i],
				&state->encoder.zlibsettings);
			if (state->error) goto cleanup;
		}

		/*unknown chunks between IDAT and IEND*/
		if (info.unknown_chunks_data[2]) {
			state->error = addUnknownChunks(&outv, info.unknown_chunks_data[2], info.unknown_chunks_size[2]);
			if (state->error) goto cleanup;
		}
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
		state->error = addChunk_IEND(&outv);
		if (state->error) goto cleanup;
	}

cleanup:
	lodepng_info_cleanup(&info);
	lodepng_free(data);
	lodepng_color_mode_cleanup(&auto_color);

	/*instead of cleaning the vector up, give it to the output*/
	*out = outv.data;
	*outsize = outv.size;

	return state->error;
}

unsigned lodepng_encode_memory(unsigned char** out, size_t* outsize, const unsigned char* image,
	unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth) {
	unsigned error;
	LodePNGState state;
	lodepng_state_init(&state);
	state.info_raw.colortype = colortype;
	state.info_raw.bitdepth = bitdepth;
	state.info_png.color.colortype = colortype;
	state.info_png.color.bitdepth = bitdepth;
	lodepng_encode(out, outsize, image, w, h, &state);
	error = state.error;
	lodepng_state_cleanup(&state);
	return error;
}



void lodepng_encoder_settings_init(LodePNGEncoderSettings* settings) {
	lodepng_compress_settings_init(&settings->zlibsettings);
	settings->filter_palette_zero = 1;
	settings->filter_strategy = LFS_MINSUM;
	settings->auto_convert = 1;
	settings->force_palette = 0;
	settings->predefined_filters = 0;
#ifdef LODEPNG_COMPILE_ANCILLARY_CHUNKS
	settings->add_id = 0;
	settings->text_compression = 1;
#endif /*LODEPNG_COMPILE_ANCILLARY_CHUNKS*/
}

#endif /*LODEPNG_COMPILE_ENCODER*/
#endif /*LODEPNG_COMPILE_PNG*/


/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */
/* // C++ Wrapper                                                          // */
/* ////////////////////////////////////////////////////////////////////////// */
/* ////////////////////////////////////////////////////////////////////////// */

#ifdef LODEPNG_COMPILE_CPP
namespace lodepng {

	unsigned encode(const std::string& filename, const std::vector<unsigned char>& in, unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth)
	{
		if (lodepng_get_raw_size_lct(w, h, colortype, bitdepth) > in.size()) return 84;
		std::vector<unsigned char> vBuffer;

		unsigned char* buffer;
		size_t buffersize;
		unsigned error = lodepng_encode_memory(&buffer, &buffersize, &in[0], w, h, colortype, bitdepth);
		if (buffer) {
			vBuffer.insert(vBuffer.end(), buffer, &buffer[buffersize]);
			lodepng_free(buffer);
		}

		if (!error) error = lodepng_save_file(&vBuffer[0], vBuffer.size(), filename.c_str());
		
		return error;
	}

} /* namespace lodepng */
#endif /*LODEPNG_COMPILE_CPP*/
