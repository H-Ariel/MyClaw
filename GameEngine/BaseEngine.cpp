#include "BaseEngine.h"


BaseEngine::BaseEngine() : mousePosition({}), stopEngine(false),
	clearScreen(false), _nextEngine(nullptr) {}
BaseEngine::~BaseEngine() {}

void BaseEngine::OnKeyUp(int key) {}
void BaseEngine::OnKeyDown(int key) {}
void BaseEngine::OnMouseButtonUp(MouseButtons btn) {}
void BaseEngine::OnMouseButtonDown(MouseButtons btn) {}
void BaseEngine::OnResize() {}

void BaseEngine::Logic(uint32_t elapsedTime)
{
	for (UIBaseElement* e : _elementsList)
		e->Logic(elapsedTime);
}
void BaseEngine::Draw()
{
	for (UIBaseElement* e : _elementsList)
		e->Draw();
}

shared_ptr<BaseEngine> BaseEngine::getNextEngine()
{
	stopEngine = false;
	return _nextEngine;
}

void BaseEngine::changeEngine(shared_ptr<BaseEngine> newEngine)
{
	_nextEngine = newEngine;
	stopEngine = true;
}
