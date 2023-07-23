#pragma once

#include "BaseEngine.h"


class UIBaseButton : public UIBaseElement
{
public:
	UIBaseButton();
	UIBaseButton(function<void(MouseButtons)> onClick, BaseEngine* parent);
	~UIBaseButton();

	void Logic(uint32_t elapsedTime) override;
	Rectangle2D GetRect() override;

	function<void(MouseButtons)> onClick;
	D2D1_SIZE_F size;

private:
	static void RegisterMyButtonClass();
	static LRESULT CALLBACK UIButtonBase_WndProc(HWND, UINT, WPARAM, LPARAM);

	static map<HWND, UIBaseButton*> _allButtons;

	HWND _btnWnd;
	BaseEngine* _parent;
	void CreateWnd();
};
