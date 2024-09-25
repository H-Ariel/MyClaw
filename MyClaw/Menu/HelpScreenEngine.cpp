#include "HelpScreenEngine.h"
#include "../ClawLevelEngine.h"


HelpScreenEngine::HelpScreenEngine(shared_ptr<ClawLevelEngineFields> clawLevelEngineFields)
	: ScreenEngine(clawLevelEngineFields, "STATES/HELP/SCREENS/HELP.PCX")
{
}
void HelpScreenEngine::OnKeyUp(int key)
{
	changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
}
void HelpScreenEngine::OnMouseButtonUp(MouseButtons btn) 
{ 
	changeEngine<ClawLevelEngine>(_clawLevelEngineFields);
}
