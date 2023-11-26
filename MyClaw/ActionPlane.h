#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "PhysicsManager.h"
#include "Objects/PowderKeg.h"


class BaseEnemy;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(WapWorld* wwd);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;
	void readPlaneObjects(BufferReader& reader) override;
	void addObject(const WwdObject& obj) override;

	static void addPlaneObject(BasePlaneObject* obj);
	static const PhysicsManager& getPhysicsManager() { return *(_instance->_physicsManager); }
	static const vector<PowderKeg*>& getPowderKegs() { return _instance->_powderKegs; }
	static const vector<BaseEnemy*>& getEnemies() { return _instance->_enemies; }
	static const vector<Projectile*>& getProjectiles() { return _instance->_projectiles; }
	static const vector<BaseDamageObject*>& getDamageObjects() { return _instance->_damageObjects; }
	static void playerEnterToBoss();
	static bool isInBoss() { return _instance->_isInBoss; }

private:
	void updatePosition();
	
	enum class States : int8_t {
		Play, // normal gameplay
		Fall, // CC falls out the window
		Close, // close the screen
		Open // open the screen
	};

	vector<BasePlaneObject*> _bossObjects;
	vector<PowderKeg*> _powderKegs;
	vector<BaseEnemy*> _enemies;
	vector<Projectile*> _projectiles;
	vector<BaseDamageObject*> _damageObjects;
	vector<Rectangle2D> _shakeRects;
	D2D1_SIZE_F _planeSize;
	PhysicsManager* _physicsManager;
	int _shakeTime;
	float _holeRadius; // the radius of the hole that remains until closed
	bool _deathAniWait; // waiting for disqualification animation to finish
	bool _needSort, _isInBoss;
	States _state;

	static ActionPlane* _instance;
};
