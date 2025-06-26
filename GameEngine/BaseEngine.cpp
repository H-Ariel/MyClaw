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
	for (auto it = _timers.begin(); it != _timers.end(); )
	{
		(*it)->update(elapsedTime);
		if ((*it)->isFinished()) it = _timers.erase(it);
		else ++it;
	}

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
	return std::exchange(_nextEngine, nullptr); // do not recycle the engine after it used
}

void BaseEngine::changeEngine(shared_ptr<BaseEngine> newEngine)
{
	_nextEngine = newEngine;
	stopEngine = true;
}
