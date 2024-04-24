#include "AniFile.h"


WapAni::WapAni(shared_ptr<BufferReader> aniFileStream)
{
	size_t size = aniFileStream->getSize();
	uint32_t i;
	uint32_t signature;
	uint32_t animationFramesCount;
	uint32_t imageSetPathLength;

	// Read first 32 bytes
	aniFileStream->read(signature);
	aniFileStream->skip(8);
	aniFileStream->read(animationFramesCount);
	aniFileStream->read(imageSetPathLength);
	aniFileStream->skip(12); // unknown

	// Check if loaded file at leasr resembles ANI file format since there is no checksum
	if (animationFramesCount == 0 || (animationFramesCount * 20 + imageSetPathLength) > size)
		throw Exception("Invalid ANI file");

	imageSetPath = aniFileStream->ReadString(imageSetPathLength);

	/********************** ANI ANIMATION FRAMES **********************/

	uint16_t triggeredEventFlag;

	// Load all animation frames, one by one
	for (i = 0; i < animationFramesCount; i++)
	{
		Frame animFrame = {};

		aniFileStream->read(triggeredEventFlag);
		aniFileStream->read(animFrame.useSoundFile);
		aniFileStream->skip(4); // unknown
		aniFileStream->read(animFrame.imageFileId);
		aniFileStream->read(animFrame.duration);
		aniFileStream->skip(8); // unknown

		if (triggeredEventFlag == 2)
		{
			animFrame.eventFilePath = aniFileStream->ReadNullTerminatedString();
		}

		animationFrames.push_back(animFrame);
	}
}
