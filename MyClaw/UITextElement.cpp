#include "WindowManager.h"


FontData::FontData()
	: size(30), family(L"Consolas"), hAlignment(HorizontalAlignment::Center), vAlignment(VerticalAlignment::Center)
{
}


UITextElement::UITextElement()
	: text(L""), size({})
{
	_brush = WindowManager::createSolidBrush(ColorF::Black);
}
UITextElement::~UITextElement()
{
	SafeRelease(_brush);
}

void UITextElement::Draw()
{
	IDWriteTextFormat* textFormat = WindowManager::createTextFormat(font);
	WindowManager::drawText(text, textFormat, _brush, GetRect());
	SafeRelease(textFormat);
}
Rectangle2D UITextElement::GetRect()
{
	return Rectangle2D(
		position.x - size.width / 2, position.y - size.height / 2,
		position.x + size.width / 2, position.y + size.height / 2
	);
}

void UITextElement::setColor(ColorF color)
{
	_brush->SetColor(color);
}
ColorF UITextElement::getColor() const 
{
	D2D1_COLOR_F color = _brush->GetColor();
	return ColorF(color.r, color.g, color.b, color.a);
}
