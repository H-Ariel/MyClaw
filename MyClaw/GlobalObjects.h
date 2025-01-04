#pragma once

#include "GameEngine/GameEngineFramework.hpp"


class Player;
class PhysicsManager;
class CheatsManager;
class ActionPlane;

class BasePlaneObject;
class PowderKeg;
class BaseEnemy;
class Projectile;
class BaseDamageObject;


class GlobalObjects
{
public:
	static shared_ptr<Player> player;
	static shared_ptr<PhysicsManager> physics;
	static shared_ptr<CheatsManager> cheats;
	static ActionPlane* actionPlane;

	static D2D1_POINT_2F& getPlayerPosition();
	static Rectangle2D getPlayerRect();
	static pair<Rectangle2D, int> getPlayerAttackRect();
	static bool isPlayerTakeDamage();
	static bool isPlayerDuck();
	static bool isPlayerAttack();
	static bool isPlayerGhost();
	static bool isPlayerSqueezed();
	static bool isPlayerFreeze();

	static void squeezePlayer(D2D1_POINT_2F pos, bool mirror = false);
	static void unsqueezePlayer();


	static void addObjectToActionPlane(BasePlaneObject* obj);
	static const vector<PowderKeg*>& getActionPlanePowderKegs();
	static const vector<BaseEnemy*>& getActionPlaneEnemies();
	static const vector<Projectile*>& getActionPlaneProjectiles();
	static const vector<BaseDamageObject*>& getActionPlaneDamageObjects();

};

#ifndef GO
#define GO GlobalObjects // easy access in code
#endif
