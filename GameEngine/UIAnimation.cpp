#include "UIAnimation.h"
#include "AudioManager.h"


UIAnimation::FrameData::FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, const string& soundKey)
	: image(image), duration(duration), soundKey(soundKey), elapsedTime(0), soundPlayed(false)
{
	if (image == nullptr)
		throw Exception("image can't be null"); // should never happen
}


UIAnimation::UIAnimation(const vector<FrameData*>& frames)
	: _frames(frames)
{
	reset();
}
UIAnimation::~UIAnimation()
{
	for (FrameData* i : _frames)
	{
		delete i;
	}
}

void UIAnimation::Logic(uint32_t elapsedTime)
{
	if (updateFrames)
	{
		_frames[_currFrameIdx]->elapsedTime += elapsedTime;

		if (!_frames[_currFrameIdx]->soundKey.empty())
		{
			if (!_frames[_currFrameIdx]->soundPlayed && !_isFinishAnimation)
			{
#ifndef _DEBUG // cancel audio in debug mode
				AudioManager::playWav(_frames[_currFrameIdx]->soundKey, false);
#endif
				_frames[_currFrameIdx]->soundPlayed = true;
			}
		}

		if (_frames[_currFrameIdx]->elapsedTime >= _frames[_currFrameIdx]->duration)
		{
			advanceFrame();
		}
	}
}
void UIAnimation::advanceFrame()
{
	_frames[_currFrameIdx]->soundPlayed = false;
	_frames[_currFrameIdx]->elapsedTime = 0;
	_isFinishAnimation = (_currFrameIdx == _frames.size() - 1);

	if (_isFinishAnimation)
	{
		if (loopAni)
		{
			_currFrameIdx = 0;
		}
	}
	else
	{
		_currFrameIdx += 1; // we do not need use modulo because the check of `_isFinishAnimation`
	}
}
void UIAnimation::Draw()
{
	updateImageData();
	_frames[_currFrameIdx]->image->Draw();
}
Rectangle2D UIAnimation::GetRect()
{
	updateImageData();
	return _frames[_currFrameIdx]->image->GetRect();
}

void UIAnimation::updateImageData() const
{
	_frames[_currFrameIdx]->image->position = position;
	_frames[_currFrameIdx]->image->mirrored = mirrored;
	_frames[_currFrameIdx]->image->upsideDown = upsideDown;
	_frames[_currFrameIdx]->image->opacity = opacity;
}
void UIAnimation::reset()
{
	_currFrameIdx = 0;
	_isFinishAnimation = false;
	mirrored = false;
	upsideDown = false;
	updateFrames = true;
	loopAni = true;
	opacity = 1;
	for (FrameData* i : _frames)
	{
		i->elapsedTime = 0;
		i->soundPlayed = false;
	}
}

vector<UIAnimation::FrameData*> UIAnimation::getFramesList() const
{
	vector<FrameData*> newFrames;
	for (FrameData* i : _frames)
		newFrames.push_back(DBG_NEW FrameData(i->image->getCopy(), i->duration, i->soundKey));
	return newFrames;
}
size_t UIAnimation::getTotalDuration() const
{
	size_t totalDuration = 0;
	for (FrameData* i : _frames)
		totalDuration += i->duration;
	return totalDuration;
}
