#pragma once

#include "UIBaseImage.h"


class UIAnimation : public UIBaseElement
{
public:
	class FrameData
	{
	public:
		FrameData(shared_ptr<UIBaseImage> image, uint32_t duration = 100, const string& soundKey = "");

		const string soundKey; // key for AudioManager
		const shared_ptr<UIBaseImage> image;
		const uint32_t duration; // in milliseconds
		uint32_t elapsedTime; // in milliseconds
		bool soundPlayed;
	};


	UIAnimation(const vector<FrameData*>& images);
	~UIAnimation() override;

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;

	void reset(); // reset to the first frame

	shared_ptr<UIAnimation> getCopy() const { return make_shared<UIAnimation>(getImagesList()); }
	vector<FrameData*> getImagesList() const; // return a copy of the images list. WARNING: you should release that memory
	size_t getTotalDuration() const;
	size_t getImagesCount() const { return _images.size(); }
	size_t getFrameNumber() const { return _currImgIdx; }
	float getFramesProgress() const { return (float)getFrameNumber() / getImagesCount(); } // returns frames progress ratio
	bool isFinishAnimation() const { return _isFinishAnimation; } // return if we finish the animation loop
	bool isPassedHalf() const { return _currImgIdx >= _images.size() / 2; }


	float opacity; // value between 0 (transparent) and 1 (opaque)
	bool updateFrames;
	bool mirrored; // flag to set whether to draw normal or inverted
	bool upsideDown; // flag to set whether to draw normal or upside-down
	bool loopAni; // Defines whether at the end of the animation it will return to the beginning or remain in the current state

private:
	void updateImageData() const; // position, mirrored, etc.

	vector<FrameData*> _images;
	size_t _currImgIdx;
	bool _isFinishAnimation;
};
