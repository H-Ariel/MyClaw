#pragma once

#include "Framework/Buffer.h"


struct WapAni
{
	struct Frame
	{
		uint16_t useSoundFile; // 0 - no, other - yes
		uint16_t imageFileId;
		uint16_t duration;
		string eventFilePath; // sound file path
	};

	WapAni(shared_ptr<Buffer> aniFileStream);

	string imageSetPath;
	DynamicArray<Frame> animationFrames;
};
