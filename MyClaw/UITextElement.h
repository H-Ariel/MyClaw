#pragma once

#include "UIBaseElement.h"


enum class HorizontalAlignment { Right, Center, Left };
enum class VerticalAlignment { Top, Center, Bottom };

class FontData
{
public:
	FontData();
	FontData(float size);
	FontData(float size, wstring family);
	FontData(float size, wstring family, HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment);

	float size; // font size
	wstring family; // font family
	HorizontalAlignment horizontalAlignment;
	VerticalAlignment verticalAlignment;
};

class UITextElement : public UIBaseElement
{
public:
	UITextElement();
	UITextElement(wstring text, FontData font, D2D1_SIZE_F size, D2D1_POINT_2F position, ColorF forgroundColor, ColorF backgroundColor);

	void Draw() override;


	wstring text;
	D2D1_SIZE_F size;
	FontData font;
	ColorF fgcolor; // forground color
	ColorF bgcolor; // background color
};
