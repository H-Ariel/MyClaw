#pragma once

#include "GameEngine/UIBaseButton.h"
#include "MenuEngine.h"
#include "../ScreenRelative.h"


class MenuItem : public UIBaseButton, protected ScreenRelative
{
public:
	MenuItem(const string& imagePath, const string& markedImagePath,
		const string& toggleStateImage, const string& markedToggleStateImage,
		float xRatio, float yRatio,
		function<void(MenuItem*)> itemOnClick, BackgroundImage* bgImg, MenuEngine* parent, bool initialState);

	void Logic(uint32_t) override;
	void Draw() override;

	virtual bool isActive() const;


	enum class ImageState { Normal, Toggled };

	void setStates(ImageState s) { state = s; }
	void switchState() { state = state == ImageState::Toggled ? ImageState::Normal : ImageState::Toggled; }

	bool marked;

protected:
	function<void(MenuItem*)> _itemOnClick; // send `this` as parameter
	shared_ptr<UIBaseImage> _image, _markedImage;
	shared_ptr<UIBaseImage> _toggleStateImage, _markedToggleStateImage;

	ImageState state;
};
