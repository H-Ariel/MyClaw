#pragma once

#include "BaseEngine.h"


class GameMainApp
{
public:
	GameMainApp();
	~GameMainApp();

	void run();


private:
	void runEngine();

	static void registerMyWindowClass();
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


	shared_ptr<BaseEngine> _pEngine;
	bool runApp;
};
