#include "AssetsManager.h"


Animation::FrameData::FrameData(const string& pidPath, uint32_t duration, const string& soundFilePath)
	: image(AssetsManager::loadImage(pidPath)), duration(duration), soundFilePath(soundFilePath), elapsedTime(0), soundPlayed(false)
{
}
Animation::FrameData::FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, const string& soundFilePath)
	: image(image), duration(duration), soundFilePath(soundFilePath), elapsedTime(0), soundPlayed(false)
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

