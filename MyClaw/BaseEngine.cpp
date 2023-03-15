#include "BaseEngine.h"
#include "WindowManager.h"


BaseEngine::BaseEngine() : mousePosition({}), StopEngine(false) {}

void BaseEngine::OnKeyUp(int key) {}
void BaseEngine::OnKeyDown(int key) {}
void BaseEngine::OnMouseButtonUp(MouseButtons btn) {}
void BaseEngine::OnMouseButtonDown(MouseButtons btn) {}
void BaseEngine::OnMouseWheel(int delta) {}
void BaseEngine::OnResize() {}

void BaseEngine::Logic(uint32_t elapsedTime)
{
	for (UIBaseElement* e : _elementsList) e->Logic(elapsedTime);
}
void BaseEngine::Draw()
{
	WindowManager::BeginDraw();
	for (UIBaseElement* e : _elementsList) e->Draw();
	WindowManager::EndDraw();
}

shared_ptr<BaseEngine> BaseEngine::getNextEngine() { StopEngine = false; return _nextEngine; }
