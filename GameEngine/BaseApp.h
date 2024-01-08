#pragma once

#include "BaseEngine.h"


class BaseApp
{
public:
	BaseApp();
	~BaseApp();

	virtual void run();

protected:
	void runEngine();

	shared_ptr<BaseEngine> _pEngine;
	bool runApp;

private:
	static void registerMyWindowClass();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};
