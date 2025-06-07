#include "UIBaseButton.h"
#include "WindowManager.h"


constexpr auto MY_BUTTON_CLASS_NAME = L"my_button";


UIBaseButton::UIBaseButton(function<void(MouseButtons)> onClick, BaseEngine* parent)
	: onClick(onClick), size({}), _parent(parent)
{
	registerButtonClass();
	_btnWnd = CreateWindow(MY_BUTTON_CLASS_NAME, L"", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, WindowManager::getHwnd(), nullptr, HINST_THISCOMPONENT, this);
	if (!_btnWnd) throw Exception("Failed to create button");
	ShowWindow(_btnWnd, SW_NORMAL);
	Logic(0); // set up window's position and size
}
UIBaseButton::~UIBaseButton()
{
	DestroyWindow(_btnWnd);
}

void UIBaseButton::Logic(uint32_t)
{
	// ensure that the drawn button is also the virtual button (set up button position by its center)
	Rectangle2D rc = GetRect();
	SetWindowPos(_btnWnd, NULL, (int)rc.left, (int)rc.top, (int)(rc.right - rc.left), (int)(rc.bottom - rc.top), SWP_NOOWNERZORDER);
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

void UIBaseButton::registerButtonClass()
{
	static bool doesClassRegistered = false;
	if (doesClassRegistered) return;
	WNDCLASSEX wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = UIBaseButton::UIButtonBase_WndProc;
	wcex.hInstance = HINST_THISCOMPONENT;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.lpszClassName = MY_BUTTON_CLASS_NAME;
	RegisterClassEx(&wcex);
	doesClassRegistered = true;
}

LRESULT CALLBACK UIBaseButton::UIButtonBase_WndProc(HWND hwnd, UINT msg, WPARAM wPrm, LPARAM lPrm)
{
	UIBaseButton* btn = (UIBaseButton*)(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_CREATE:
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lPrm)->lpCreateParams));
		break;

	case WM_MOUSEMOVE:	if (btn && btn->_parent) // update mouse position because parent does not receive child's events
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
