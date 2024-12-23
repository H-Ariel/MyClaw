#include "UIAnimation.h"
#include "AudioManager.h"


UIAnimation::FrameData::FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, const string& soundKey)
	: image(image), duration(duration), soundKey(soundKey), elapsedTime(0), soundPlayed(false)
{
	if (image == nullptr)
		throw Exception("image can't be null"); // should never happen
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

		if (!_images[_currImgIdx]->soundKey.empty())
		{
			if (!_images[_currImgIdx]->soundPlayed && !_isFinishAnimation)
			{
#ifndef _DEBUG // cancel audio in debug mode
				AudioManager::playWav(_images[_currImgIdx]->soundKey, false);
#endif
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
}
void UIAnimation::Draw()
{
	updateImageData();
	_images[_currImgIdx]->image->Draw();
}
Rectangle2D UIAnimation::GetRect()
{
	updateImageData();
	return _images[_currImgIdx]->image->GetRect();
}

void UIAnimation::updateImageData() const
{
	_images[_currImgIdx]->image->position = position;
	_images[_currImgIdx]->image->mirrored = mirrored;
	_images[_currImgIdx]->image->upsideDown = upsideDown;
	_images[_currImgIdx]->image->opacity = opacity;
}
void UIAnimation::reset()
{
	_currImgIdx = 0;
	_isFinishAnimation = false;
	mirrored = false;
	upsideDown = false;
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
