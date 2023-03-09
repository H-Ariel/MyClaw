#include "Animation.h"
#include "AssetsManager.h"
#include "PidPalette.h"


// TODO: save `WavPlayer` instead `soundFilePath` ?
Animation::FrameData::FrameData(string pidPath, uint32_t duration, string soundFilePath)
	: image(AssetsManager::loadImage(pidPath)), duration(duration), soundFilePath(soundFilePath), elapsedTime(0), soundPlayed(false)
{
}
Animation::FrameData::FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, string soundFilePath)
	: image(image), duration(duration), soundFilePath(soundFilePath), elapsedTime(0), soundPlayed(false)
{
}


Animation::Animation(RezArchive* rezArchive, string aniPath, string _imageSetPath)
{
	shared_ptr<BufferReader> aniFileReader = rezArchive->getFileBufferReader(aniPath);
	string imageSetPath;
	string soundFilePath;
	char imgName[32];
	uint32_t imageSetPathLength, framesCount;
	uint16_t triggeredEventFlag, imageFileId, duration;
	bool useSoundFile;

	// no sound for enemies animations
	const bool UseSoundForThisAnim =
		!contains(aniPath, "RAT") &&
		!contains(aniPath, "SOLDIER") &&
		!contains(aniPath, "OFFICER");

	/********************** ANI HEADER/PROPERTIES **********************/

	if (aniFileReader->readByte() != 0x20) // file signature
	{
		throw Exception("invalid ANI file. path=" + aniPath);
	}
	aniFileReader->skip(11);
	aniFileReader->read(framesCount);
	aniFileReader->read(imageSetPathLength);
	aniFileReader->skip(12);


	// Check if loaded file at least resembles ANI file format since there is no checksum
	if (framesCount == 0 || (size_t)framesCount * 20 + imageSetPathLength > aniFileReader->getSize())
	{
		throw Exception("I'm here!");
		// todo: delete this condition
	}

	imageSetPath = aniFileReader->ReadString(imageSetPathLength);
	if (!_imageSetPath.empty())
	{
		imageSetPath = _imageSetPath;
	}
	string original = imageSetPath;
	imageSetPath = PathManager::getImageSetPath(imageSetPath);

	if (imageSetPath.empty())
	{
		throw Exception(__FUNCTION__ ": imageSetPath is empty. aniPath=" + aniPath);
	}

	for (uint32_t i = 0; i < framesCount; i++, soundFilePath = "")
	{
		aniFileReader->read(triggeredEventFlag);
		useSoundFile = aniFileReader->readByte();// != 1; todo: unremark
		aniFileReader->skip(5);
		aniFileReader->read(imageFileId);
		aniFileReader->read(duration);
		aniFileReader->skip(8);

		if (triggeredEventFlag == 2)
		{
			soundFilePath = aniFileReader->ReadNullTerminatedString();

			if (!soundFilePath.empty() && useSoundFile)
			{
				soundFilePath = replaceString(soundFilePath, '_', '/');
				soundFilePath = PathManager::getSoundFilePath(soundFilePath);
			}
		}

		// TODO: delete this condition and it's block
		if (!UseSoundForThisAnim)
		{
			soundFilePath = "";
		}

		sprintf(imgName, "/%03d.PID", imageFileId); // according to `fixFileName` at `RezArchive.cpp`

		// TODO: hack - something else
		if (contains(aniPath, "/ANIS/RAT/THROWSOUTH.ANI") && imageFileId == 0) continue;

		_images.push_back(DBG_NEW FrameData(
			imageSetPath + imgName,
			duration,
			soundFilePath
		));
	}

	reset();
}
Animation::Animation(vector<FrameData*> images)
	: _images(images)
{
	reset();
}
Animation::~Animation()
{
	for (FrameData* i : _images)
	{
		delete i;
	}
}

void Animation::Logic(uint32_t elapsedTime)
{
	if (updateFrames)
	{
		_images[_currImgIdx]->elapsedTime += elapsedTime;

		if (!_images[_currImgIdx]->soundFilePath.empty())
		{
			if (!_images[_currImgIdx]->soundPlayed && !_isFinishAnimation)
			{
				AssetsManager::playWavFile(_images[_currImgIdx]->soundFilePath);
				_images[_currImgIdx]->soundPlayed = true;
			}
		}

		if (_images[_currImgIdx]->elapsedTime >= _images[_currImgIdx]->duration)
		{
			_images[_currImgIdx]->soundPlayed = false;
			_images[_currImgIdx]->elapsedTime = 0;
			_isFinishAnimation = (_currImgIdx == _images.size() - 1);

			if (_isFinishAnimation)
			{
				if (loopAni)
				{
					_currImgIdx = 0;
				}
			}
			else
			{
				_currImgIdx += 1;
			}
		}
	}

	updateImageData();
}
void Animation::Draw()
{
	_images[_currImgIdx]->image->Draw();
}
D2D1_RECT_F Animation::GetRect()
{
	return _images[_currImgIdx]->image->GetRect();
}

size_t Animation::getFrameNumber() const
{
	return _currImgIdx;
}

void Animation::updateImageData() const
{
	_images[_currImgIdx]->image->position = position;
	_images[_currImgIdx]->image->mirrored = mirrored;
}

void Animation::reset()
{
	_currImgIdx = 0;
	_isFinishAnimation = false;
	mirrored = false;
	updateFrames = true;
	loopAni = true;
	for (FrameData* i : _images)
	{
		i->elapsedTime = 0;
		i->soundPlayed = false;
	}
}
bool Animation::isFinishAnimation() const
{
	return _isFinishAnimation;
}
bool Animation::isPassedHalf() const
{
	return _currImgIdx >= _images.size() / 2;
}

shared_ptr<Animation> Animation::getCopy()
{
	vector<FrameData*> newImages;
	for (FrameData* i : _images)
	{
		newImages.push_back(DBG_NEW FrameData(i->image->getCopy(), i->duration, i->soundFilePath));
	}
	return allocNewSharedPtr<Animation>(newImages);
}
