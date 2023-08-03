#include "UITextElement.h"
#include "WindowManager.h"


FontData::FontData()
	: size(30), family(L"Consolas"), horizontalAlignment(HorizontalAlignment::Center), verticalAlignment(VerticalAlignment::Center)
{
}
FontData::FontData(float size)
	: size(size), family(L"Consolas"), horizontalAlignment(HorizontalAlignment::Center), verticalAlignment(VerticalAlignment::Center)
{
}
FontData::FontData(float size, wstring family)
	: size(size), family(family), horizontalAlignment(HorizontalAlignment::Center), verticalAlignment(VerticalAlignment::Center)
{
}
FontData::FontData(float size, wstring family, HorizontalAlignment horizontalAlignment, VerticalAlignment verticalAlignment)
	: size(size), family(family), horizontalAlignment(horizontalAlignment), verticalAlignment(verticalAlignment)
{
}


UITextElement::UITextElement()
	: text(L""), size({ 0,0 }), fgcolor(ColorF::Black), bgcolor(ColorF::White)
{
}
UITextElement::UITextElement(wstring text, FontData font, D2D1_SIZE_F size, D2D1_POINT_2F position, ColorF forgroundColor, ColorF backgroundColor)
	: UIBaseElement(position),
	text(text), font(font), size(size), fgcolor(forgroundColor), bgcolor(backgroundColor)
{
}
void UITextElement::Draw()
{
	Rectangle2D dst(
		position.x - size.width / 2,
		position.y - size.height / 2,
		position.x + size.width / 2,
		position.y + size.height / 2
	);
	WindowManager::fillRect(dst, bgcolor);
	WindowManager::drawText(text, font, dst, fgcolor);
}
