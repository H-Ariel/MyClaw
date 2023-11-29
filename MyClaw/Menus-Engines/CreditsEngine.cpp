#include "CreditsEngine.h"
#include "../Assets-Managers/AssetsManager.h"


CreditsEngine::CreditsEngine()
	: MenuEngine("STATES/CREDITS/SCREENS/CREDITS.PCX"),
	_startY(9800 / 480.f * _bgImg->size.height), // `480` is the original image height
	_endY(-9600 / 480.f * _bgImg->size.height)
{
	_bgImg->Logic(0); // update _bgImg position

	string creditsText = "Claw - Rewritten by Ariel Halili\n\n" + AssetsManager::getCreditsText();
	FontData font;
	font.size = _bgImg->size.height / 30;

	_creditsTextElement = DBG_NEW UITextElement;
	_creditsTextElement->text = wstring(creditsText.begin(), creditsText.end());
	_creditsTextElement->setColor(ColorF::White);
	_creditsTextElement->size = _bgImg->size;
	_creditsTextElement->position.x = _bgImg->position.x;
	_creditsTextElement->position.y = _bgImg->size.height / 2 + _startY;
	_creditsTextElement->setFont(font);
	_elementsList.push_back(_creditsTextElement);

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Credits);
}
CreditsEngine::~CreditsEngine()
{
	AssetsManager::stopBackgroundMusic();
}

void CreditsEngine::Logic(uint32_t elapsedTime)
{
	MenuEngine::Logic(elapsedTime);

	_creditsTextElement->position.y -= 0.07f * elapsedTime;

	if (_creditsTextElement->position.y < _endY)
		backToMenu();
}
void CreditsEngine::OnKeyUp(int key) 
{
	backToMenu(); 
}
void CreditsEngine::OnMouseButtonUp(MouseButtons btn) 
{
	backToMenu();
}
void CreditsEngine::OnResize() 
{
	backToMenu(); 
}
