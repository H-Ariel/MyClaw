#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "Objects/PowderKeg.h"


class BaseEnemy;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(const WwdPlane& plane, shared_ptr<WapWorld> wwd);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	Player* getPlayer() const;

	static void addPlaneObject(BasePlaneObject* obj); // TODO: make non-static
	static const vector<PowderKeg*>& getPowderKegs();
	static const vector<BaseEnemy*>& getEnemies();
	static const vector<Projectile*>& getProjectiles();

private:
	void checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath);
	void addObject(const WwdObject& obj);
	
	static vector<BasePlaneObject*> _objects; // TODO: make non-static
	static vector<PowderKeg*> _powderKegs; // TODO: make non-static
	static vector<BaseEnemy*> _enemies; // TODO: make non-static
	static vector<Projectile*> _projectiles; // TODO: make non-static
	static bool _needSort; // TODO: make non-static
	shared_ptr<WapWorld> _wwd;
	Player* _player;
	const D2D1_SIZE_F _planeSize;
};
