#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "PhysicsManager.h"
#include "Objects/PowderKeg.h"


class BaseEnemy;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(WapWorld* wwd, int levelNumber);
	~ActionPlane();

	void init() override;

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	// TODO: make non-static
	static void addPlaneObject(BasePlaneObject* obj);
	static const PhysicsManager& getPhysicsManager() { return *(_instance->_physicsManager); }
	static const vector<PowderKeg*>& getPowderKegs() { return _instance->_powderKegs; }
	static const vector<BaseEnemy*>& getEnemies() { return _instance->_enemies; }
	static const vector<Projectile*>& getProjectiles() { return _instance->_projectiles; }
	static const vector<BaseDamageObject*>& getDamageObjects() { return _instance->_damageObjects; }
	static void playerEnterToBoss();

private:
	void updatePosition();
	void addObject(const WwdObject& obj);
	
	enum class States : int8_t {
		Play, // normal gameplay
		Fall, // CC falls out the window
		Close, // close the screen
		Open // open the screen
	};

	PhysicsManager* _physicsManager;
	vector<BasePlaneObject*> _objects, _bossObjects;
	vector<PowderKeg*> _powderKegs;
	vector<BaseEnemy*> _enemies;
	vector<Projectile*> _projectiles;
	vector<BaseDamageObject*> _damageObjects;
	D2D1_SIZE_F _planeSize;
	WapWorld* _wwd;
	float _holeRadius; // the radius of the hole that remains until closed
	bool _deathAniWait; // waiting for disqualification animation to finish
	bool _needSort;
	States _state;


	static ActionPlane* _instance;
};
