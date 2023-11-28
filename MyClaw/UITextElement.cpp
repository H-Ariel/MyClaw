#include "WindowManager.h"


FontData::FontData()
	: size(30), family(L"Consolas"), hAlignment(HorizontalAlignment::Center), vAlignment(VerticalAlignment::Center)
{
}


UITextElement::UITextElement()
	: text(L""), size({})
{
	_brush = WindowManager::createSolidBrush(ColorF::Black);
	_textFormat = WindowManager::createTextFormat(_font);
}
UITextElement::~UITextElement()
{
	SafeRelease(_brush);
	SafeRelease(_textFormat);
}

void UITextElement::Draw()
{
	WindowManager::drawText(text, _textFormat, _brush, GetRect());
}
Rectangle2D UITextElement::GetRect()
{
	return Rectangle2D(
		position.x - size.width / 2, position.y - size.height / 2,
		position.x + size.width / 2, position.y + size.height / 2
	);
}

void UITextElement::setFont(const FontData& font)
{
	_font = font;
	if (_textFormat)
		SafeRelease(_textFormat);
	_textFormat = WindowManager::createTextFormat(_font);
}
FontData UITextElement::getFont() const
{
	return _font;
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
