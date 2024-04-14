#include "UIBaseButton.h"
#include "WindowManager.h"


#define MY_BUTTON_CLASS_NAME L"my_button"


map<HWND, UIBaseButton*> UIBaseButton::_allButtons = { { nullptr, nullptr } };


UIBaseButton::UIBaseButton()
	: size({}), _parent(nullptr)
{
	CreateWnd();
}
UIBaseButton::UIBaseButton(function<void(MouseButtons)> onClick, BaseEngine* parent)
	: onClick(onClick), size({}), _parent(parent)
{
	CreateWnd();
}
UIBaseButton::~UIBaseButton()
{
	if (_btnWnd && _allButtons.count(_btnWnd))
		_allButtons.erase(_btnWnd);
}

void UIBaseButton::Logic(uint32_t)
{
	// ensure that the drawn button is also the virtual button (set up button position by its center)
	Rectangle2D rc = GetRect();
	SetWindowPos(_btnWnd, 0, (int)rc.left, (int)rc.top, (int)(rc.right - rc.left), (int)(rc.bottom - rc.top), SWP_NOOWNERZORDER);
}
Rectangle2D UIBaseButton::GetRect()
{
	return Rectangle2D(
		position.x - size.width / 2,
		position.y - size.height / 2,
		position.x + size.width / 2,
		position.y + size.height / 2
	);
}

void UIBaseButton::RegisterMyButtonClass()
{
	static bool doesClassRegistered = false;
	if (doesClassRegistered) return;
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = UIBaseButton::UIButtonBase_WndProc;
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(nullptr, IDI_APPLICATION);
	wcex.lpszClassName = MY_BUTTON_CLASS_NAME;
	RegisterClassEx(&wcex);
	doesClassRegistered = true;
}

LRESULT CALLBACK UIBaseButton::UIButtonBase_WndProc(HWND hwnd, UINT msg, WPARAM wPrm, LPARAM lPrm)
{
	//UIBaseButton* btn = _allButtons.count(hwnd) ? _allButtons[hwnd] : nullptr;
	UIBaseButton* btn = nullptr;
	auto it = _allButtons.find(hwnd);
	if (it != _allButtons.end())
		btn = it->second;

	switch (msg)
	{
	case WM_MOUSEMOVE:	if (btn && btn->_parent)
		btn->_parent->mousePosition = Point2U(
			(UINT32)(LOWORD(lPrm) + btn->position.x - btn->size.width / 2),
			(UINT32)(HIWORD(lPrm) + btn->position.y - btn->size.height / 2)
		);
		break;

	case WM_LBUTTONUP: if (btn && btn->onClick) btn->onClick(MouseButtons::Left); break;
	case WM_MBUTTONUP: if (btn && btn->onClick) btn->onClick(MouseButtons::Middle); break;
	case WM_RBUTTONUP: if (btn && btn->onClick) btn->onClick(MouseButtons::Right); break;
	default: return DefWindowProc(hwnd, msg, wPrm, lPrm);
	}

	return 0;
}

void UIBaseButton::CreateWnd()
{
	RegisterMyButtonClass();
	_btnWnd = CreateWindow(MY_BUTTON_CLASS_NAME, L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, WindowManager::getHwnd(), nullptr, HINST_THISCOMPONENT, nullptr);
	if (!_btnWnd) throw Exception("Failed to create button");
	ShowWindow(_btnWnd, SW_NORMAL);
	_allButtons[_btnWnd] = this;
	Logic(0); // set up window's position and size
}
