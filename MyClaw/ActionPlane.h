#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "BaseEnemy.h"
#include "SavedDataManager.h"
#include "Objects/PowderKeg.h"


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(WapWwd* wwd, WwdPlane* wwdPlane);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void init() override;
	void addObject(const WwdObject& obj) override;

	static void loadGame(int level, int checkpoint);
	static void addPlaneObject(BasePlaneObject* obj);
	static void resetObjects(); // reset objects after CC die
	static void playerEnterToBoss(float bossWarpX);

	static const vector<PowderKeg*>& getPowderKegs() { return _instance->_powderKegs; }
	static const vector<BaseEnemy*>& getEnemies() { return _instance->_enemies; }
	static const vector<Projectile*>& getProjectiles() { return _instance->_projectiles; }
	static const vector<BaseDamageObject*>& getDamageObjects() { return _instance->_damageObjects; }
	static bool isInBoss() { return _instance->_isInBoss; }
	static int getBossHealth() { return _instance->_boss ? _instance->_boss->getHealth() : 0; }
	
	static void writeMessage(const string& message, int timeout = 2000);

	static void updatePosition();

private:
	void bossStagerLogic(uint32_t elapsedTime);

	enum class LevelState
	{
		Playing,
		BossStager_Start,
		BossStager_BossTalk,
		BossStager_ClawTalk,
		BossStager_BossName,
		BossStager_Waiting
	};

	vector<BasePlaneObject*> _bossObjects; // Keep the boss objects separately so as not to call their Logic method (saves time)
	
	// save objects for easy access
	vector<PowderKeg*> _powderKegs;
	vector<BaseEnemy*> _enemies;
	vector<Projectile*> _projectiles;
	vector<BaseDamageObject*> _damageObjects;

	vector<Rectangle2D> _shakeRects;
	D2D1_SIZE_F _planeSize;
	BaseBoss* _boss;
	int _shakeTime, _BossStagerDelay;
	bool _isInBoss;
	LevelState _levelState;

	static ActionPlane* _instance;
	static shared_ptr<SavedDataManager::GameData> _loadGameData;
};
