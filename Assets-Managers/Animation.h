#pragma once

#include "GameEngine/UIBaseImage.h"
#include "RezArchive.h"


class Animation : public UIBaseElement
{
public:
	class FrameData
	{
	public:
		FrameData(const string& pidPath, uint32_t duration = 100, const string& soundFilePath = "");
		FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, const string& soundFilePath);

		const string soundFilePath;
		const shared_ptr<UIBaseImage> image;
		const uint32_t duration; // in milliseconds
		uint32_t elapsedTime; // in milliseconds
		bool soundPlayed;
	};


	Animation(const vector<FrameData*>& images);
	~Animation() override;

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	Rectangle2D GetRect() override;

	void updateImageData() const; // position, mirrored, etc.
	void reset(); // reset to the first frame

	shared_ptr<Animation> getCopy() const { return allocNewSharedPtr<Animation>(getImagesList()); }
	vector<FrameData*> getImagesList() const;
	size_t getTotalDuration() const;
	size_t getImagesCount() const { return _images.size(); }
	size_t getFrameNumber() const { return _currImgIdx; }
	bool isFinishAnimation() const { return _isFinishAnimation; } // return if we finish the animation loop
	bool isPassedHalf() const { return _currImgIdx >= _images.size() / 2; }


	float opacity; // value between 0 (transparent) and 1 (opaque)
	bool updateFrames;
	bool mirrored; // flag to set whether to draw normal or inverted
	bool loopAni; // Defines whether at the end of the animation it will return to the beginning or remain in the current state

private:
	vector<FrameData*> _images;
	size_t _currImgIdx;
	bool _isFinishAnimation;
};
