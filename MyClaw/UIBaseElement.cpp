#include "UIBaseElement.h"


UIBaseElement::UIBaseElement(D2D1_POINT_2F position) : position(position) {}
UIBaseElement::~UIBaseElement() {}

void UIBaseElement::Logic(uint32_t elapsedTime) {}
void UIBaseElement::Draw() {}

D2D1_RECT_F UIBaseElement::GetRect() { return RectF(); }
