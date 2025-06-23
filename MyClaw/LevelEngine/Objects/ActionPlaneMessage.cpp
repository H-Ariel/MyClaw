#include "ActionPlaneMessage.h"
#include "GameEngine/WindowManager.h"


ActionPlaneMessage::ActionPlaneMessage(const string& message, int timeout)
	: BasePlaneObject({}), _timer(timeout, [this] { removeObject = true; })
{
	text.text = wstring(message.begin(), message.end());
	text.color = ColorF::White;
	FontData font; font.size = 20;
	text.setFont(font);
	drawZ = DefaultZCoord::Interface;
	addTimer(&_timer);
}

void ActionPlaneMessage::Logic(uint32_t elapsedTime)
{
	text.size = WindowManager::getRealSize();
	text.position.x = text.size.width / 2;
	text.position.y = text.size.height / 3;
}

void ActionPlaneMessage::Draw() { text.Draw(); }
Rectangle2D ActionPlaneMessage::GetRect() { return text.GetRect(); }
