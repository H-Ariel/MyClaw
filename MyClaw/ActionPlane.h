#pragma once

#include "LevelPlane.h"
#include "Player.h"
#include "BaseEnemy.h"
#include "SavedDataManager.h"
#include "Objects/PowderKeg.h"


class ClawLevelEngine;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(WapWwd* wwd, WwdPlane* wwdPlane, ClawLevelEngine* cEngine);
	~ActionPlane();

	void Logic(uint32_t elapsedTime) override;
	void init() override;
	void addObject(const WwdObject& obj) override;

	void addPlaneObject(BasePlaneObject* obj);
	void writeMessage(const string& message, int timeout = 2000);

	void loadGame(int level, int checkpoint);
	void resetObjects(); // reset objects after CC die
	void playerEnterToBoss(float bossWarpX);
	void updatePosition();

	const vector<PowderKeg*>& getPowderKegs() const { return _powderKegs; }
	const vector<BaseEnemy*>& getEnemies() const{ return _enemies; }
	const vector<Projectile*>& getProjectiles() const { return _projectiles; }
	const vector<BaseDamageObject*>& getDamageObjects() const { return _damageObjects; }
	bool isInBoss() const { return _isInBoss; }
	int getBossHealth() const { return _boss ? _boss->getHealth() : 0; }

private:
	void bossStagerLogic(uint32_t elapsedTime);

	enum class LevelState : int8_t
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
	shared_ptr<SavedDataManager::GameData> _loadGameData;
	D2D1_SIZE_F _planeSize;
	BaseBoss* _boss;
	int _shakeTime, _BossStagerDelay;
	bool _isInBoss;
	LevelState _levelState;
	ClawLevelEngine* cEngine;
};
