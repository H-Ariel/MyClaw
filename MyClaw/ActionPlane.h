#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "PhysicsManager.h"
#include "Objects/PowderKeg.h"
#include "Objects/FloorSpike.h"
#include "Objects/GooVent.h"
#include "Objects/Laser.h"


class BaseEnemy;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(const WwdPlane& plane, shared_ptr<WapWorld> wwd, int levelNumber);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	Player* getPlayer() const { return _player; }

	// TODO: make non-static
	static void addPlaneObject(BasePlaneObject* obj);
	static const vector<PowderKeg*>& getPowderKegs() { return _powderKegs; }
	static const vector<BaseEnemy*>& getEnemies() { return _enemies; }
	static const vector<Projectile*>& getProjectiles() { return _projectiles; }
	static const vector<FloorSpike*>& getFloorSpikes() { return _floorSpikes; }
	static const vector<GooVent*>& getGooVents() { return _gooVents; }
	static const vector<Laser*>& getLasers() { return _lasers; }

private:
	void addObject(const WwdObject& obj, int levelNumber);

	// TODO: make non-static
	static vector<BasePlaneObject*> _objects;
	static vector<PowderKeg*> _powderKegs;
	static vector<BaseEnemy*> _enemies;
	static vector<Projectile*> _projectiles;
	static vector<FloorSpike*> _floorSpikes;
	static vector<GooVent*> _gooVents;
	static vector<Laser*> _lasers;
	static bool _needSort;

	enum class States : int8_t {
		Play,
		Fall, // CC falls out the window
		Close, // close the screen
		Open // open the screen
	};

	PhysicsManager _physicsManager;
	shared_ptr<WapWorld> _wwd;
	Player* _player;
	const D2D1_SIZE_F _planeSize;
	
	// the next variables used when CC died
	float _holeRadius; // the radius of the hole that remains until closed
	bool _deathAniWait; // waiting for disqualification animation to finish
	States _state;
};
