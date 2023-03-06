#pragma once

#include "framework.h"


// Single-call decompression.
// Returns MZ_OK on success, or one of the error codes from mz_inflate() on failure.
int mz_uncompress(uint8_t* pDest, uint32_t pDest_len, const uint8_t* const pSource, uint32_t source_len);
