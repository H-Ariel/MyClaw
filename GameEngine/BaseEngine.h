#pragma once

#include "UIBaseElement.h"


class BaseEngine
{
public:
	BaseEngine();
	virtual ~BaseEngine();

	virtual void OnKeyUp(int key);
	virtual void OnKeyDown(int key);
	virtual void OnMouseButtonUp(MouseButtons btn);
	virtual void OnMouseButtonDown(MouseButtons btn);
	virtual void OnResize(); // This method is called after the window is resized

	virtual void Logic(uint32_t elapsedTime); // in milliseconds
	virtual void Draw();

	shared_ptr<BaseEngine> getNextEngine();


	D2D1_POINT_2U mousePosition;
	bool stopEngine;
	bool clearScreen; // if true, the screen will be cleared before drawing each frame

protected:
	template <class NextEngine, class... Args>
	void changeEngine(Args... args)
	{
		_nextEngine = make_shared<NextEngine>(args...);
		stopEngine = true;
	}

	vector<UIBaseElement*> _elementsList;
	shared_ptr<BaseEngine> _nextEngine;
};
