#pragma once

#include "BaseEngine.h"
#include "Animation.h"
#include "Objects/Item.h"


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
	MenuEngine(bool allocChildren = true, const string& bgPcxPath = "");
	MenuEngine(D2D1_POINT_2U mousePosition, shared_ptr<Animation> cursor, bool allocChildren = true, const string& bgPcxPath = "");
	~MenuEngine();

	void Logic(uint32_t elapsedTime) override;

protected:
	void backToMenu();

	static stack<const HierarchicalMenu*> _menusStack;
	static const HierarchicalMenu* _currMenu;

	shared_ptr<Animation> _cursor;
	MenuBackgroundImage* _bgImg;
};


class LevelLoadingEngine : public MenuEngine
{
public:
	LevelLoadingEngine(int lvlNo);
	void Logic(uint32_t elapsedTime);

private:
	uint32_t _totalTime;
	const int _lvlNo;
	bool _isDrawn;
};


class LevelEndEngine : public MenuEngine
{
public:
	LevelEndEngine(int lvlNum, map<Item::Type, uint32_t> collectedTreasures);

	void Logic(uint32_t elapsedTime) override;

	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void playNextLevel();

	enum State : int8_t {
		Start,
		DrawScore,
		Wait,
		End
	};

	map<Item::Type, uint32_t> _collectedTreasures;
	const int _lvlNum;
	int8_t _state;
};


class OpeningScreenEngine : public MenuEngine
{
public:
	OpeningScreenEngine();
	void Logic(uint32_t elapsedTime) override;
	void OnKeyUp(int key) override;
	void OnMouseButtonUp(MouseButtons btn) override;

private:
	void continueToMenu();

	uint32_t _wavId;
	int _totalTime;
};
