#include "../Assets-Managers/AssetsManager.h"


Animation::FrameData::FrameData(const string& pidPath, uint32_t duration, const string& soundFilePath)
	: image(AssetsManager::loadImage(pidPath)), duration(duration), soundFilePath(soundFilePath), elapsedTime(0), soundPlayed(false)
{
}
Animation::FrameData::FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, const string& soundFilePath)
	: image(image), duration(duration), soundFilePath(soundFilePath), elapsedTime(0), soundPlayed(false)
{
}


vector<Animation::FrameData*> getAnimationImages(RezArchive* rezArchive, const string& aniPath, const string& _imageSetPath)
{
	vector<Animation::FrameData*> images;
	shared_ptr<BufferReader> aniFileReader = rezArchive->getFileBufferReader(aniPath);
	string imageSetPath;
	string soundFilePath;
	char imgName[32];
	uint32_t imageSetPathLength, framesCount;
	uint16_t triggeredEventFlag, imageFileId, duration;
	bool useSoundFile;

	if (aniPath[0] == '/') 
		(string&)aniPath = aniPath.substr(1);

	aniFileReader->skip(12);
	aniFileReader->read(framesCount);
	aniFileReader->read(imageSetPathLength);
	aniFileReader->skip(12);

	imageSetPath = aniFileReader->ReadString(imageSetPathLength);
	if (!_imageSetPath.empty())
	{
		imageSetPath = _imageSetPath;
	}

	imageSetPath = PathManager::getImageSetPath(imageSetPath);

	if (imageSetPath.empty())
	{
		throw Exception("empty imageSetPath at " + aniPath);
	}

	for (uint32_t i = 0; i < framesCount; i++, soundFilePath = "")
	{
		aniFileReader->read(triggeredEventFlag);
		useSoundFile = aniFileReader->read<uint8_t>();// != 0; todo: unremark?
		aniFileReader->skip(5);
		aniFileReader->read(imageFileId);
		aniFileReader->read(duration);
		aniFileReader->skip(8);

		if (triggeredEventFlag == 2)
		{
			soundFilePath = aniFileReader->ReadNullTerminatedString();

			if (endsWith(soundFilePath, "MLF") || endsWith(soundFilePath, "MRF"))
			{ // the rat use this sounds, and I hate it
				soundFilePath = "";
			}
			else if (!soundFilePath.empty() && useSoundFile)
			{
				soundFilePath = PathManager::getSoundFilePath(soundFilePath);
			}
		}


		// TODO: hack - something else
		if (startsWith(aniPath, "LEVEL2/ANIS/RAUX/BLOCK")) duration /= 2;
		else if (startsWith(aniPath, "LEVEL6/ANIS/WOLVINGTON/BLOCK")) duration /= 2;
		else if (aniPath == "LEVEL8/ANIS/GABRIELCANNON/HORZFIRE.ANI") duration *= 8;
		else if (aniPath == "LEVEL8/ANIS/GABRIELCANNON/VERTIFIRE.ANI") duration *= 8;
		else if (imageSetPath == "LEVEL8/IMAGES/GABRIELCANNON" && imageFileId == 0) imageFileId = 1;
		else if (aniPath == "LEVEL8/ANIS/CANNONSWITCH/SWITCH.ANI") duration *= 2;
		else if (aniPath == "LEVEL9/ANIS/SAWBLADE/SPIN.ANI") duration *= 4;
		else if (imageSetPath == "LEVEL10/IMAGES/MARROW" && imageFileId == 0) imageFileId = 1;

		sprintf(imgName, "/%03d.PID", imageFileId); // according to `fixFileName` at `RezArchive.cpp`

		images.push_back(DBG_NEW Animation::FrameData(imageSetPath + imgName, duration, soundFilePath));
	}

	return images;
}

Animation::Animation(RezArchive* rezArchive, const string& aniPath, const string& imageSetPath)
	: Animation(getAnimationImages(rezArchive, aniPath, imageSetPath))
{
}
Animation::Animation(const vector<FrameData*>& images)
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
Rectangle2D Animation::GetRect()
{
	return _images[_currImgIdx]->image->GetRect();
}

void Animation::updateImageData() const
{
	_images[_currImgIdx]->image->position = position;
	_images[_currImgIdx]->image->mirrored = mirrored;
	_images[_currImgIdx]->image->opacity = opacity;
}
void Animation::reset()
{
	_currImgIdx = 0;
	_isFinishAnimation = false;
	mirrored = false;
	updateFrames = true;
	loopAni = true;
	opacity = 1;
	for (FrameData* i : _images)
	{
		i->elapsedTime = 0;
		i->soundPlayed = false;
	}
}

vector<Animation::FrameData*> Animation::getImagesList() const
{
	vector<FrameData*> newImages;
	for (FrameData* i : _images)
	{
		newImages.push_back(DBG_NEW FrameData(i->image->getCopy(), i->duration, i->soundFilePath));
	}
	return newImages;
	// WARNING: you should release that memory
}
size_t Animation::getTotalDuration() const
{
	size_t totalDuration = 0;
	for (FrameData* i : _images)
		totalDuration += i->duration;
	return totalDuration;
}

