#pragma once

#include "GameEngine/GameEngineFramework.hpp"


class Player;
class PhysicsManager;
class CheatsManager;
class ClawLevelEngine;

class BasePlaneObject;
class PowderKeg;
class BaseEnemy;
class Projectile;
class BaseDamageObject;
class Timer;

/// <summary>
/// Static class holding global shared objects used across the game.
/// </summary>
class GlobalObjects
{
public:
	static shared_ptr<Player> player;
	static shared_ptr<PhysicsManager> physics;
	static shared_ptr<CheatsManager> cheats;
	static ClawLevelEngine* clawLevelEngine;

	static D2D1_POINT_2F& getPlayerPosition();
	static Rectangle2D getPlayerRect();
	static pair<Rectangle2D, int> getPlayerAttackRect();
	static bool isPlayerTakeDamage();
	static bool isPlayerDuck();
	static bool isPlayerAttack();
	static bool isPlayerGhost();
	static bool isPlayerSqueezed();
	static bool isPlayerFreeze();


	static void addObjectToActionPlane(BasePlaneObject* obj);
	static const vector<PowderKeg*>& getActionPlanePowderKegs();
	static const vector<BaseEnemy*>& getActionPlaneEnemies();
	static const vector<Projectile*>& getActionPlaneProjectiles();
	static const vector<BaseDamageObject*>& getActionPlaneDamageObjects();


	static void addTimer(Timer* timer);
};

#ifndef GO
#define GO GlobalObjects // easy access in code
#endif
