#include "ActionPlaneMessage.h"
#include "WindowManager.h"


ActionPlaneMessage::ActionPlaneMessage(const wstring& message, int timeout)
	: BasePlaneObject({}), _timeLeft(timeout)
{
	text.text = message;
	text.setColor(ColorF::White);
	FontData font; font.size = 20;
	text.setFont(font);

	myMemCpy(ZCoord, (int)DefaultZCoord::Interface);
}

void ActionPlaneMessage::Logic(uint32_t elapsedTime)
{
	_timeLeft -= elapsedTime;
	if (_timeLeft <= 0)
		removeObject = true;

	text.size = WindowManager::getRealSize();
	text.position.x = text.size.width / 2;
	text.position.y = text.size.height / 3;
}

void ActionPlaneMessage::Draw() { text.Draw(); }
Rectangle2D ActionPlaneMessage::GetRect() { return text.GetRect(); }
