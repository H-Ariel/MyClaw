#pragma once

#include "BaseEngine.h"


class BaseApp
{
public:
	BaseApp(WNDPROC wndproc = nullptr, const TCHAR title[] = L"");
	virtual ~BaseApp();

	// initialize the application.
	// you should override this function and create the engine (or use
	// class ctor, but I think it's better to create the engine here)
	virtual void init();

	// run the application.
	void run();

protected:
	// The main window procedure.
	// you can override this function in your derived class, but don't
	// forget to call this function as default in your derived class.
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	shared_ptr<BaseEngine> _pEngine;
	bool _runApp;


private:
	static void registerMyWindowClass(WNDPROC wndproc);
};
