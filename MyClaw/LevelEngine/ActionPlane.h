#pragma once

#include "LevelPlane.h"
#include "Enemies/BaseEnemy.h"
#include "Objects/Player/Player.h"
#include "Objects/PowderKeg.h"


class ClawLevelEngine;
class LevelObjectsFactory;
class BaseBoss;


class ActionPlane : public LevelPlane
{
public:
	ActionPlane(WapWwd* wwd, WwdPlane* wwdPlane, ClawLevelEngine* cEngine);
	~ActionPlane();

	void init() override;
	void loadGame(int level, int checkpoint);
	void addObject(const WwdObject& obj) override;
	void addPlaneObject(BasePlaneObject* obj);

	void Logic(uint32_t elapsedTime) override;
	void updatePosition();

	void resetObjects(); // reset objects after CC die
	void enterEasyMode(); // when easymode chaet called
	void exitEasyMode();
	void playerEnterToBoss(float bossWarpX);
	
	const vector<PowderKeg*>& getPowderKegs() const { return _powderKegs; }
	const vector<BaseEnemy*>& getEnemies() const{ return _enemies; }
	const vector<Projectile*>& getProjectiles() const { return _projectiles; }
	const vector<BaseDamageObject*>& getDamageObjects() const { return _damageObjects; }
	bool isInBoss() const { return _isInBoss; }
	int getBossHealth() const;

private:
	void joinConveyorBelts();
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
	LevelObjectsFactory* _levelObjectsFactory;
};
