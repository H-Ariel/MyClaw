#include "UIAnimation.h"
#include "AudioManager.h"


UIAnimation::FrameData::FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, const string& soundKey)
	: image(image), duration(duration), soundKey(soundKey), elapsedTime(0), soundPlayed(false)
{
}


UIAnimation::UIAnimation(const vector<FrameData*>& images)
	: _images(images)
{
	reset();
}
UIAnimation::~UIAnimation()
{
	for (FrameData* i : _images)
	{
		delete i;
	}
}

void UIAnimation::Logic(uint32_t elapsedTime)
{
	if (updateFrames)
	{
		_images[_currImgIdx]->elapsedTime += elapsedTime;

#ifndef _DEBUG // in debug mode we don't play sound
		if (!_images[_currImgIdx]->soundKey.empty())
		{
			if (!_images[_currImgIdx]->soundPlayed && !_isFinishAnimation)
			{
				AudioManager::playWav(_images[_currImgIdx]->soundKey, false);
				_images[_currImgIdx]->soundPlayed = true;
			}
		}
#endif

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
void UIAnimation::Draw()
{
	_images[_currImgIdx]->image->Draw();
}
Rectangle2D UIAnimation::GetRect()
{
	return _images[_currImgIdx]->image->GetRect();
}

void UIAnimation::updateImageData() const
{
	_images[_currImgIdx]->image->position = position;
	_images[_currImgIdx]->image->mirrored = mirrored;
	_images[_currImgIdx]->image->opacity = opacity;
}
void UIAnimation::reset()
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

vector<UIAnimation::FrameData*> UIAnimation::getImagesList() const
{
	vector<FrameData*> newImages;
	for (FrameData* i : _images)
		newImages.push_back(DBG_NEW FrameData(i->image->getCopy(), i->duration, i->soundKey));
	return newImages;
}
size_t UIAnimation::getTotalDuration() const
{
	size_t totalDuration = 0;
	for (FrameData* i : _images)
		totalDuration += i->duration;
	return totalDuration;
}
