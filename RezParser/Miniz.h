#pragma once

#include "Framework/Framework.h"

// impleted in Miniz.cpp
void mz_uncompress(uint8_t pDest[], uint32_t pDestLen, const uint8_t pSource[], uint32_t sourceLen);
