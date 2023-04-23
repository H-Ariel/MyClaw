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
	ActionPlane(const WwdPlaneData& planeData, WapWorld* wwd, int levelNumber);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	Player* getPlayer() const { return _player; }

	// TODO: make non-static
	static void addPlaneObject(BasePlaneObject* obj);
	static const PhysicsManager& getPhysicsManager() { return *_physicsManager; }
	static const vector<PowderKeg*>& getPowderKegs() { return _powderKegs; }
	static const vector<BaseEnemy*>& getEnemies() { return _enemies; }
	static const vector<Projectile*>& getProjectiles() { return _projectiles; }
	static const vector<FloorSpike*>& getFloorSpikes() { return _floorSpikes; }
	static const vector<GooVent*>& getGooVents() { return _gooVents; }
	static const vector<Laser*>& getLasers() { return _lasers; }

private:
	void addObject(const WwdObject& obj, int levelNumber, WapWorld* wwd);

	// TODO: make non-static
	static PhysicsManager* _physicsManager;
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

	Player* _player;
	const D2D1_SIZE_F _planeSize;
	
	// the next variables used when CC died and the screen is closing
	float _holeRadius; // the radius of the hole that remains until closed
	bool _deathAniWait; // waiting for disqualification animation to finish
	States _state;
};
