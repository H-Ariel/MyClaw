#pragma once

#include "UIBaseImage.h"


class RezArchive;


// display claw animation
class Animation : public UIBaseElement
{
public:
	class FrameData
	{
	public:
		FrameData(string pidPath, uint32_t duration = 125, string soundFilePath = "");
		FrameData(shared_ptr<UIBaseImage> image, uint32_t duration, string soundFilePath);

		const string soundFilePath;
		const shared_ptr<UIBaseImage> image;
		const uint32_t duration; // in milliseconds
		uint32_t elapsedTime; // in milliseconds
		bool soundPlayed;
	};


	Animation(RezArchive* rezArchive, string aniPath, string imageSetPath);
	Animation(vector<FrameData*> images);
	~Animation() override;

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	D2D1_RECT_F GetRect() override;
	size_t getFrameNumber() const;

	void updateImageData() const; // position, mirrored, etc.
	void reset(); // reset to the first frame
	bool isFinishAnimation() const; // return if we finish the animation loop
	bool isPassedHalf() const;

	shared_ptr<Animation> getCopy();


	bool updateFrames;
	bool mirrored; // flag to set whether to draw normal or inverted
	bool loopAni; // Defines whether at the end of the animation it will return to the beginning or remain in the current state

private:
	vector<FrameData*> _images;
	size_t _currImgIdx;
	bool _isFinishAnimation;
};
