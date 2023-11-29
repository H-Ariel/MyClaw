#include "HelpScreenEngine.h"
#include "../ClawLevelEngine.h"


HelpScreenEngine::HelpScreenEngine() 
	: MenuEngine("STATES/HELP/SCREENS/HELP.PCX"), _isInGame(false) 
{
}
HelpScreenEngine::HelpScreenEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields)
	: MenuEngine(clawLevelEngineFields, false, "STATES/HELP/SCREENS/HELP.PCX"), _isInGame(true)
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
		MenuEngine::backToMenu();
}
