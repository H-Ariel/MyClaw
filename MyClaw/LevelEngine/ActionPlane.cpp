#include "ActionPlane.h"
#include "GlobalObjects.h"
#include "CheatsManager.h"
#include "LevelObjectsFactory.h"
#include "ClawLevelEngine.h"
#include "Physics/PhysicsManager.h"
#include "GameEngine/WindowManager.h"
#include "Objects/ActionPlaneMessage.h"
#include "Objects/ConveyorBelt.h"
#include "Objects/Checkpoint.h"
#include "Objects/Stalactite.h"
#include "Objects/Crate.h"
#include "Objects/OneTimeAnimation.h"
#include "Enemies/Bosses/Gabriel/GabrielRedTailPirate.h"


constexpr int SHAKE_TIME = 3000; // time of shaking screen (ms)
constexpr auto& player = GO::player;
constexpr auto& physics = GO::physics;


// TODO think about "State" design pattern...


ActionPlane::ActionPlane(WapWwd* wwd, WwdPlane* wwdPlane, ClawLevelEngine* cEngine)
	: LevelPlane(wwd, wwdPlane), _planeSize({}), _boss(nullptr), _shakeTime(0),
	_BossStagerDelay(0), _isInBoss(false), _levelState(LevelState::Playing),
	cEngine(cEngine), _levelObjectsFactory(DBG_NEW LevelObjectsFactory(wwd))
{
	//if (BasePlaneObject::actionPlane) LOG("[Warning] ActionPlane already exists\n"); // should never happen
	GO::actionPlane = this;
}

ActionPlane::~ActionPlane()
{
	std::erase(_objects, player.get()); // we don't want to delete the player because we save him for the next level

	for (BasePlaneObject* i : _objects)
		delete i;
	for (BasePlaneObject* i : _bossObjects)
		delete i;

	delete _levelObjectsFactory;

	// because it static member and we don't want recycle objects...
	GO::actionPlane = nullptr;
}

void ActionPlane::init()
{
	// initialize global fields

	_planeSize.width = (float)TILE_SIZE * _wwdPlane->tilesOnAxisX;
	_planeSize.height = (float)TILE_SIZE * _wwdPlane->tilesOnAxisY;

	AssetsManager::startBackgroundMusic(AssetsManager::BackgroundMusicType::Level);
	physics = make_shared<PhysicsManager>(_wwdPlane->tiles, _wwd->tileDescriptions); // must be after WWD map loaded and before objects added

	if (!GO::cheats)
	{
		// init cheats-manager here because it is for all objects, but mainly for player
		GO::cheats = make_shared<CheatsManager>(this);
	}

	// player's initializtion must be before LevelPlane::readPlaneObjects() because some of objects need player
	if (player && player->hasLives()) // if we have player from previous level, we don't need to create new one
	{
		player->startPosition.x = (float)_wwd->startX;
		player->startPosition.y = (float)_wwd->startY;
		player->nextLevel();
	}
	else
	{
		player = make_shared<Player>();
		player->position.x = player->startPosition.x = (float)_wwd->startX;
		player->position.y = player->startPosition.y = (float)_wwd->startY;
	}

	LevelPlane::init();
	joinConveyorBelts(); // must be after LevelPlane::init()

	if (_loadGameData)
	{
		player->setGameData(*_loadGameData);
		_loadGameData = nullptr;
	}
	_objects.push_back(player.get()); // must be after LevelPlane::init() because we reset the objects vector there

	_levelState = LevelState::Playing;
	_BossStagerDelay = 0;
}
// This function joins belts into a long belt (saves memory and calculations).
void ActionPlane::joinConveyorBelts() {
	vector<ConveyorBelt*> pConveyorBelts;

	// insert belts in sorted order
	for (BasePlaneObject* obj : _objects) {
		if (ConveyorBelt* belt = dynamic_cast<ConveyorBelt*>(obj)) {
			pConveyorBelts.insert(lower_bound(pConveyorBelts.begin(), pConveyorBelts.end(), belt,
				[](ConveyorBelt* a, ConveyorBelt* b) {
					// ensure belts are sorted first by Y (top to bottom) and then by X (left to right) for accurate adjacency checks
					return a->position.y < b->position.y ||
						(a->position.y == b->position.y && a->position.x < b->position.x);
				}), belt);
		}
	}

	if (pConveyorBelts.empty())
		return;

	// join belts that are next to each other into one
	auto it = pConveyorBelts.begin();
	ConveyorBelt* prevBelt = *it;
	++it;

	while (it != pConveyorBelts.end()) {
		ConveyorBelt* curr = *it;

		while (curr->position.y == prevBelt->position.y &&
			prevBelt->GetRect().right + 1 == curr->GetRect().left)
		{
			if (prevBelt->hasSameMovement(curr)) {
				prevBelt->extend(curr);
				curr->removeObject = true; // we do not need it anymore
			}

			++it;
			curr = *it;
		}
		prevBelt = *it;
		++it;
	}
}

void ActionPlane::loadGame(int level, int checkpoint)
{
	SavedDataManager::GameData data = SavedDataManager::loadGame(level, checkpoint);
	if (data.level == level && data.savePoint == checkpoint)
	{
		// success to load checkpoint
		_loadGameData = make_shared<SavedDataManager::GameData>(data);
	}
}

void ActionPlane::addObject(const WwdObject& obj)
{
	if (obj.logic == "Shake") // this logic does not create an object
	{
		_shakeRects.push_back(Rectangle2D((float)obj.attackRect.left, (float)obj.attackRect.top,
			(float)obj.attackRect.right, (float)obj.attackRect.bottom));
	}

	BasePlaneObject* bpObj = _levelObjectsFactory->createObject(obj);
	if (!bpObj)
		return; // nothingto do with empty object

	/* Objects */
	if (obj.logic == "FrontCandy" || obj.logic == "FrontAniCandy"
		|| obj.logic == "BehindCandy" || obj.logic == "BehindAniCandy"
		|| obj.logic == "DoNothing" || obj.logic == "DoNothingNormal"
		|| obj.logic == "AniCycle" || obj.logic == "AniCycleNormal"
		|| obj.logic == "Sign" || obj.logic == "GooCoverup"
		|| obj.logic == "GlobalAmbientSound"
		|| obj.logic == "AmbientSound" || obj.logic == "SpotAmbientSound"
		|| contains(obj.logic, "SoundTrigger")
		|| obj.logic == "BehindCrate" || obj.logic == "FrontCrate"
		|| obj.logic == "FrontStackedCrates" || obj.logic == "BackStackedCrates"
		|| obj.logic == "FrontStatue" || obj.logic == "BehindStatue"
		|| endsWith(obj.logic, "Elevator")
		|| obj.logic == "StartSteppingStone"
		|| startsWith(obj.logic, "TogglePeg") || startsWith(obj.logic, "SteppingStone")
		|| obj.logic == "CrumblingPeg"
		|| obj.logic == "BreakPlank"
		|| obj.logic == "TreasurePowerup" || obj.logic == "GlitterlessPowerup"
		|| obj.logic == "SpecialPowerup" || obj.logic == "AmmoPowerup"
		|| obj.logic == "BossWarp" || obj.logic == "HealthPowerup"
		|| obj.logic == "EndOfLevelPowerup" || obj.logic == "MagicPowerup"
		// || obj.logic == "CursePowerup"
		|| obj.logic == "AniRope"
		|| obj.logic == "SpringBoard" || obj.logic == "WaterRock"
		|| obj.logic == "GroundBlower"
		|| obj.logic == "ConveyorBelt"
		|| obj.logic == "CrabNest"
		|| obj.logic == "TowerCannonLeft" || obj.logic == "TowerCannonRight"
		|| obj.logic == "SkullCannon"
		|| obj.logic == "TProjectile"
		|| obj.logic == "LavaHand")
	{
		_objects.push_back(bpObj);
	}
	else if (obj.logic == "PowderKeg")
	{
		_objects.push_back(bpObj);
		_powderKegs.push_back(static_cast<PowderKeg*>(bpObj));
	}
	else if (endsWith(obj.logic, "Checkpoint"))
	{
		_objects.push_back(bpObj);

		if (_loadGameData && (
			(_loadGameData->savePoint == 1 && obj.logic == "FirstSuperCheckpoint") ||
			(_loadGameData->savePoint == 2 && obj.logic == "SecondSuperCheckpoint")))
		{
			player->position = player->startPosition = bpObj->position;
		}
	}

	/* Enemies */

	else if (obj.logic == "Officer"
		|| obj.logic == "Soldier"
		|| obj.logic == "Rat"
		|| obj.logic == "PunkRat"
		|| obj.logic == "RobberThief"
		|| obj.logic == "CutThroat"
		|| obj.logic == "Seagull"
		|| obj.logic == "TownGuard1"
		|| obj.logic == "TownGuard2"
		|| obj.logic == "RedTailPirate"
		|| obj.logic == "BearSailor"
		|| obj.logic == "CrazyHook"
		|| obj.logic == "HermitCrab"
		|| obj.logic == "PegLeg"
		|| obj.logic == "Mercat"
		|| obj.logic == "Siren"
		|| obj.logic == "Fish"
		|| obj.logic == "Chameleon"
		|| obj.logic == "TigerGuard")
	{
		_objects.push_back(bpObj);
		_enemies.push_back(static_cast<BaseEnemy*>(bpObj));
	}

	/* Obstacles */
	else if (obj.logic == "GooVent"
		|| startsWith(obj.logic, "FloorSpike")
		|| startsWith(obj.logic, "SawBlade")
		|| obj.logic == "LavaGeyser"
		|| obj.logic == "Laser"
		|| obj.logic == "LavaMouth")
	{
		_objects.push_back(bpObj);
		_damageObjects.push_back(static_cast<BaseDamageObject*>(bpObj));
	}
	else if (obj.logic == "Stalactite")
	{
		_objects.push_back(bpObj);
		_projectiles.push_back(static_cast<Stalactite*>(bpObj));
	}

	/* Bosses and their objects */
	else if (obj.logic == "Raux"
		|| obj.logic == "Katherine"
		|| obj.logic == "Wolvington"
		|| obj.logic == "Gabriel"
		|| obj.logic == "Marrow"
		|| obj.logic == "Aquatis"
		|| obj.logic == "RedTail"
		|| obj.logic == "Omar")
	{
		_boss = static_cast<BaseBoss*>(bpObj);
		_bossObjects.push_back(bpObj);
	}
	else if (obj.logic == "GabrielCannon"
		|| obj.logic == "CannonSwitch"
		|| obj.logic == "CannonButton"
		|| obj.logic == "Parrot"
		|| obj.logic == "MarrowFloor"
		|| obj.logic == "Tentacle"
		|| obj.logic == "AquatisCrack"
		|| obj.logic == "AquatisDynamite"
		|| obj.logic == "AquatisStalactite"
		|| obj.logic == "RedTailSpikes"
		|| obj.logic == "RedTailWind")
	{
		_bossObjects.push_back(bpObj);
	}
}

void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	//if (obj == nullptr) return;
	_objects.push_back(obj); // don't insert in middle because some of the object add new objects in their destructor
	if (isinstance<Projectile>(obj)) _projectiles.push_back((Projectile*)obj);
	else if (isinstance<BaseEnemy>(obj)) _enemies.push_back((BaseEnemy*)obj);
}

void ActionPlane::writeMessage(const string& message, int timeout)
{
	addPlaneObject(DBG_NEW ActionPlaneMessage(message, timeout));
}

void ActionPlane::Logic(uint32_t elapsedTime)
{
	if (player->isFinishLevel()) return; // TODO: check if we use this condition

	if (_levelState != LevelState::Playing)
	{
		bossStagerLogic(elapsedTime);
		return;
	}

	if (_shakeTime > 0)
		_shakeTime -= elapsedTime;
	updatePosition();

	sort(_objects.begin(), _objects.end(),
		[](BasePlaneObject* a, BasePlaneObject* b) { return a->logicZ < b->logicZ; }); // for this method (`Logic`)

	BasePlaneObject* obj;
	bool exploseShake = false; // shake screen after explodes of Claw's dynamit and powder-kegs

	for (size_t i = 0; i < _objects.size(); i++)
	{
		obj = _objects[i];

		if (obj->isDelayed()) {
			obj->decreaseTimeDelay(elapsedTime);
			continue;
		}

		obj->Logic(elapsedTime);

		if (obj == player.get())
		{
			if (!player->isInDeathAnimation())
			{
				physics->checkCollides(player.get());
			}
		}
		else if (isinstance<BaseEnemy>(obj) || isinstance<Projectile>(obj)
			|| (isinstance<PowderKeg>(obj) && !((PowderKeg*)obj)->isExplode())
			|| (isinstance<Item>(obj) && ((Item*)obj)->speed.y != 0)
			|| isinstance<GabrielRedTailPirate>(obj))
		{
			physics->checkCollides((BaseDynamicPlaneObject*)obj);
		}
		else if (isinstance<StackedCrates>(obj))
		{
			_objects += ((StackedCrates*)obj)->getItems();
		}
		else if (isinstance<Crate>(obj))
		{
			Crate* crate = (Crate*)obj;
			if (crate->isBreaking())
				_objects += crate->getItems();
		}
		else if (isinstance<Warp>(obj) && ((Warp*)obj)->isActivated())
		{
			Warp* pWarp = (Warp*)obj;
			cEngine->playerEnterWrap(pWarp);
			pWarp->deactivateWarp();
		}

		if (_shakeTime <= 0 &&
			(isinstance<PowderKeg>(obj) && ((PowderKeg*)obj)->isStartExplode()) ||
			(isinstance<ClawDynamite>(obj) && ((ClawDynamite*)obj)->isStartExplode()))
			exploseShake = true;

		if (obj->removeObject)
		{
			if (isinstance<BaseEnemy>(obj))
			{
				std::erase(_enemies, obj);
				if (isinstance<BaseBoss>(obj))
				{
					_boss = nullptr;
					_isInBoss = false;
				}
			}
			else if (isinstance<Projectile>(obj))
			{
				if (isinstance<Stalactite>(obj))
				{
					obj->removeObject = false; // we don't want to delete this object. it inherit from Projectile but we can restore it.
					continue;
				}
				else
					std::erase(_projectiles, obj);
			}
			else if (isinstance<PowderKeg>(obj))
				std::erase(_powderKegs, obj);
			else if (isinstance<BaseDamageObject>(obj))
				std::erase(_damageObjects, obj);

			delete obj;
			_objects.erase(_objects.begin() + i);
			i--; // cancel `i++`
		}
	}

	sort(_objects.begin(), _objects.end(),
		[](BasePlaneObject* a, BasePlaneObject* b) { return a->drawZ < b->drawZ; }); // for `Draw` method

	AssetsManager::callLogics(elapsedTime);

#ifndef _DEBUG // when I'm debugging, I don't want to shake the screen (it's annoying)
	const Rectangle2D playerRect = player->GetRect();
	auto it = find_if(_shakeRects.begin(), _shakeRects.end(), [&](const Rectangle2D& r) { return playerRect.intersects(r); });
	if (it != _shakeRects.end()) {
		_shakeTime = SHAKE_TIME;
		_shakeRects.erase(it);
	}
	else if (exploseShake)
		_shakeTime = 500;
#endif
}

void ActionPlane::bossStagerLogic(uint32_t elapsedTime) {
	/*
	The BossStager have 4 states:
	1. update boss' and Claw's positions
	2. The boss talks to Captain Claw
	3. Claw talks to the Claw
	4. Show the boss' name and play the defeat sound (boss name)
	5. Captain Claw and Boss are fighting (normal game state)
	*/

	if (_shakeTime > 0)
		_shakeTime -= elapsedTime;
	updatePosition();

	if (_BossStagerDelay > 0)
	{
		_BossStagerDelay -= elapsedTime;
		return;
	}

	char path[64] = {};
	D2D1_SIZE_F camSz = {};

	switch (_levelState)
	{
	case LevelState::BossStager_Start:
		_boss->Logic(0); // set boss' position
		player->Logic(0); // set player's position
#ifdef _DEBUG
		_levelState = LevelState::Playing; // skip the boss stager
#else
		_levelState = LevelState::BossStager_BossTalk;
#endif
		break;

	case LevelState::BossStager_BossTalk:
		sprintf(path, "LEVEL%d/SOUNDS/STAGING/ENEMY.WAV", _wwd->levelNumber);
		AssetsManager::playWavFile(path);
		_BossStagerDelay = AssetsManager::getWavFileDuration(path);
		_levelState = LevelState::BossStager_ClawTalk;
		break;

	case LevelState::BossStager_ClawTalk:
		sprintf(path, "LEVEL%d/SOUNDS/STAGING/CLAW.WAV", _wwd->levelNumber);
		_BossStagerDelay = AssetsManager::getWavFileDuration(path);
		AssetsManager::playWavFile(path);
		_levelState = LevelState::BossStager_BossName;
		break;

	case LevelState::BossStager_BossName:
		camSz = WindowManager::getCameraSize();
		sprintf(path, "LEVEL%d/IMAGES/BOSSNAME/001.PID", _wwd->levelNumber);
		_objects.push_back(DBG_NEW OneTimeAnimation(
			{ camSz.width / 2 + position.x,camSz.height / 2 + position.y },
			AssetsManager::createAnimationFromPidImage(path), false));
		sprintf(path, "LEVEL%d/SOUNDS/DEFEAT.WAV", _wwd->levelNumber);
		_BossStagerDelay = AssetsManager::getWavFileDuration(path);
		_shakeTime = _BossStagerDelay; // shake the screen while the boss name is showing
		AssetsManager::playWavFile(path);
		_levelState = LevelState::BossStager_Waiting;
		break;

	case LevelState::BossStager_Waiting:
		_objects.back()->removeObject = true; // remove the boss name
		_levelState = LevelState::Playing;
		break;
	}
}

void ActionPlane::updatePosition()
{
	const D2D1_SIZE_F camSize = WindowManager::getCameraSize();
	BaseCharacter* character = player.get();

	if (_levelState == LevelState::BossStager_BossTalk || _levelState == LevelState::BossStager_ClawTalk)
		character = _boss; // if we are in boss stager, we need to show the boss while s/he is talking

	// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
	position.x = character->position.x - camSize.width / 2.0f;
	position.y = character->position.y - camSize.height / 2.0f;

	float maxOffsetX = _planeSize.width - camSize.width;
	float maxOffsetY = _planeSize.height - camSize.height;

	if (position.x < 0) position.x = 0;
	if (position.y < 0) position.y = 0;
	if (position.x > maxOffsetX) position.x = maxOffsetX;
	if (position.y > maxOffsetY) position.y = maxOffsetY;

	if (_shakeTime > 0)
	{
		// to shake the screen we just move it a little bit
		const float shakeDelta = 5;
		if (_shakeTime / 75 % 2) { position.x -= shakeDelta; position.y -= shakeDelta; }
		else { position.x += shakeDelta; position.y += shakeDelta; }
	}
}

void ActionPlane::resetObjects()
{
	for (BasePlaneObject* obj : _objects)
		obj->Reset();
}

void ActionPlane::enterEasyMode() {
	for (BasePlaneObject* obj : _objects)
		obj->enterEasyMode(); // TODO: make sure i am realy used this...
}

void ActionPlane::exitEasyMode() {
	for (BasePlaneObject* obj : _objects)
		obj->exitEasyMode();
}

void ActionPlane::playerEnterToBoss(float bossWarpX)
{
	// clear all objects that we don't need in boss
	for (auto i : _powderKegs) i->removeObject = true;
	for (auto i : _enemies) i->removeObject = true;
	for (auto i : _projectiles) i->removeObject = true;
	for (auto i : _damageObjects) i->removeObject = true;

	// find all objects that we don't need in boss and remove them (the boss-warp 
	// and boss area are in the right side of the screen so it easy to find them)
	for (BasePlaneObject* obj : _objects)
	{
		if (obj->position.x < bossWarpX)
			obj->removeObject = true;
	}

	// move all objects that we need in boss to the objects' list
	for (BasePlaneObject* obj : _bossObjects)
	{
		_objects.push_back(obj);
		if (isinstance<BaseEnemy>(obj))
			_enemies.push_back((BaseEnemy*)obj);
		else if (isinstance<Projectile>(obj))
			_projectiles.push_back((Projectile*)obj);
		else if (isinstance<BaseDamageObject>(obj))
			_damageObjects.push_back((BaseDamageObject*)obj);
	}
	_bossObjects.clear();

	_isInBoss = true;

	AssetsManager::startBackgroundMusic(AssetsManager::BackgroundMusicType::Boss);
	_BossStagerDelay = 0;
	_levelState = LevelState::BossStager_Start;
}
