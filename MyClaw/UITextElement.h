#pragma once

#include "UIBaseElement.h"


class FontData
{
public:
	enum class HorizontalAlignment { Right, Center, Left };
	enum class VerticalAlignment { Top, Center, Bottom };

	FontData();

	float size; // font size
	wstring family; // font family
	HorizontalAlignment hAlignment;
	VerticalAlignment vAlignment;
};


class UITextElement : public UIBaseElement
{
public:
	UITextElement();
	~UITextElement();
	
	void Draw() override;
	Rectangle2D GetRect() override;

	void setColor(ColorF color);
	ColorF getColor() const;

	FontData font;
	wstring text;
	D2D1_SIZE_F size; // destination rectange size
	
private:
	ID2D1SolidColorBrush* _brush;
};
