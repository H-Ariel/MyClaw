#pragma once

#include "GameEngine/UIBaseButton.h"
#include "MenuEngine.h"


class MenuItem : public UIBaseButton
{
public:
	MenuItem(const string& pcxPath, float xRatio, float yRatio,
		MenuBackgroundImage* bgImg, ScreenEngine* parent);

	MenuItem(const string& pcxPath, const string& markedPcxPath, float xRatio, float yRatio,
		function<void(MenuItem*)> onClick, MenuBackgroundImage* bgImg, ScreenEngine* parent);

	MenuItem(const string& pcxPath, const string& markedPcxPath,
		const string& pcxPath2, const string& markedPcxPath2,
		float xRatio, float yRatio,
		function<void(MenuItem*)> onClick, MenuBackgroundImage* bgImg, ScreenEngine* parent, int initialState);

	void Logic(uint32_t) override;
	void Draw() override;

	void mulImageSizeRatio(float n); // multiply the size ratio by n

	bool isActive() const { return (bool)onClick; }

	bool marked;
	int state; // 0 for _image, 1 for _image2

private:
	function<void(MenuItem*)> _itemOnClick; // send `this` as parameter
	D2D1_POINT_2F _posRatio;
	D2D1_SIZE_F _sizeRatio;
	MenuBackgroundImage* _bgImg; // save it to get the size and position, so we can save item's image ratio
	shared_ptr<UIBaseImage> _image, _markedImage;
	shared_ptr<UIBaseImage> _image2, _markedImage2;
};
