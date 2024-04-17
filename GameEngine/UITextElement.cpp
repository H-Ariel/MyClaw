#include "WindowManager.h"


FontData::FontData()
	: size(30), family(L"Consolas"), hAlignment(HorizontalAlignment::Center), vAlignment(VerticalAlignment::Center)
{
}


UITextElement::UITextElement()
	: text(L""), color(ColorF::Black), size({})
{
	_textFormat = WindowManager::createTextFormat(_font);
}
UITextElement::~UITextElement()
{
	_textFormat->Release();
}

void UITextElement::Draw()
{
	WindowManager::drawText(text, _textFormat, color, GetRect());
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
		_textFormat->Release();
	_textFormat = WindowManager::createTextFormat(_font);
}
FontData UITextElement::getFont() const
{
	return _font;
}
