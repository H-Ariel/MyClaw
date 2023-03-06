#pragma once

#include "BaseEngine.h"
#include "Animation.h"


struct HierarchicalMenu;


// an image whose size is determined by window's size
class MenuBackgroundImage : public UIBaseImage
{
public:
	MenuBackgroundImage(string pcxPath);
	void Logic(uint32_t elapsedTime) override;

private:
	const float _imgRatio; // `width / height`
};


class MenuEngine : public BaseEngine
{
public:
	MenuEngine(bool allocChildren = true, string bgPcxPath = "");
	MenuEngine(D2D1_POINT_2U mousePosition, shared_ptr<Animation> cursor, bool allocChildren = true, string bgPcxPath = "");
	~MenuEngine();

	void Logic(uint32_t elapsedTime) override;

protected:
	static stack<const HierarchicalMenu*> _menusStack;
	static const HierarchicalMenu* _currMenu;

	MenuBackgroundImage* _bgImg;
	shared_ptr<Animation> _cursor;
};

class LevelLoadingEngine : public MenuEngine
{
public:
	LevelLoadingEngine(int lvlNo);
	void Logic(uint32_t elapsedTime);

private:
	uint32_t _totalTime;
	const int8_t _lvlNo;
	bool _isDrawn;
};

class LevelEndEngine : public MenuEngine
{
public:
	LevelEndEngine(int lvlNo);

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void playNextLevel();

	enum State : int8_t {
		Start,
		InsertMap, DrawDots,
		DrawGem,
		DrawScore,
		Wait,
		End
	};

	const int8_t _lvlNo;
	int8_t _state;
};
