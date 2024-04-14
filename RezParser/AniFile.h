#pragma once

#include "RezArchiveFile.h"


struct AniAnimationFrame
{
	uint16_t useSoundFile; // 0 - no, other - yes
	uint16_t imageFileId;
	uint16_t duration;
	string eventFilePath; // sound file path
};

struct WapAni
{
	WapAni(shared_ptr<BufferReader> aniFileStream);

	string imageSetPath;
	vector<AniAnimationFrame> animationFrames;
};
