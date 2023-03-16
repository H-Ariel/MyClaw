#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "Objects/PowderKeg.h"
#include "Objects/FloorSpike.h"
#include "Objects/GooVent.h"


class BaseEnemy;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(const WwdPlane& plane, shared_ptr<WapWorld> wwd);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	Player* getPlayer() const {
		return _player;
	}

	// TODO: make non-static
	static void addPlaneObject(BasePlaneObject* obj);
	static const vector<PowderKeg*>& getPowderKegs() { return _powderKegs; }
	static const vector<BaseEnemy*>& getEnemies() { return _enemies; }
	static const vector<Projectile*>& getProjectiles() { return _projectiles; }
	static const vector<FloorSpike*>& getFloorSpikes() { return _floorSpikes; }
	static const vector<GooVent*>& getGooVents() { return _gooVents; }

private:
	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath);
	void addObject(const WwdObject& obj);

	// TODO: make non-static
	static vector<BasePlaneObject*> _objects;
	static vector<PowderKeg*> _powderKegs;
	static vector<BaseEnemy*> _enemies;
	static vector<Projectile*> _projectiles;
	static vector<FloorSpike*> _floorSpikes;
	static vector<GooVent*> _gooVents;
	static bool _needSort;

	enum class States : int8_t {
		Play,
		// the black screen
		Rect_Close,
		Rect_Open
	};

	shared_ptr<WapWorld> _wwd;
	Player* _player;
	const D2D1_SIZE_F _planeSize;
	
	// the next variables used when `_state` is `CC_Dead`
	States _state;
	float _CCDead_NoBlackScreen_Radius;
	bool _CCDead_shouldWait;
};
