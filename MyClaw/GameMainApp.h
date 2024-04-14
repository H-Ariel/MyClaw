#pragma once

#include "GameEngine/BaseApp.h"


class GameMainApp : public BaseApp
{
public:
	GameMainApp();
	~GameMainApp();

	void init() override;

private:
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
};
