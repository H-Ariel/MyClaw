#include "HelpScreenEngine.h"
#include "../ClawLevelEngine.h"


HelpScreenEngine::HelpScreenEngine() 
	: ScreenEngine("STATES/HELP/SCREENS/HELP.PCX"), _isInGame(false)
{
}
HelpScreenEngine::HelpScreenEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields)
	: ScreenEngine(clawLevelEngineFields, "STATES/HELP/SCREENS/HELP.PCX"), _isInGame(true)
{
}
void HelpScreenEngine::OnKeyUp(int key)
{
	backToMenu(); 
}
void HelpScreenEngine::OnMouseButtonUp(MouseButtons btn) 
{ 
	backToMenu(); 
}
void HelpScreenEngine::backToMenu()
{
	if (_isInGame)
		changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
	else
		ScreenEngine::backToMenu();
}
