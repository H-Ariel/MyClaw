#pragma once

#include "UIBaseElement.h"

class BaseEngine;


class UIBaseButton : public UIBaseElement
{
public:
	UIBaseButton();
	UIBaseButton(function<void(MouseButtons)> onClick, BaseEngine* parent);
	~UIBaseButton();

	void Logic(uint32_t elapsedTime) override;
	D2D1_RECT_F GetRect() override;

	function<void(MouseButtons)> onClick;
	D2D1_SIZE_F size;

private:
	static void RegisterMyButtonClass();
	static LRESULT UIButtonBase_WndProc(HWND, UINT, WPARAM, LPARAM);

	static map<HWND, UIBaseButton*> _allButtons;

	HWND _btnWnd;
	BaseEngine* _parent;
	void CreateWnd();
};
