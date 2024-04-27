#include "ActionPlane.h"
#include "PhysicsManager.h"
#include "GameEngine/WindowManager.h"
#include "Objects/ActionPlaneMessage.h"
#include "Objects/Checkpoint.h"
#include "Objects/Elevator.h"
#include "Objects/Crate.h"
#include "Objects/Projectile.h"
#include "Objects/SoundTrigger.h"
#include "Objects/AmbientSound.h"
#include "Objects/Pegs.h"
#include "Objects/DoNothing.h"
#include "Objects/Rope.h"
#include "Objects/SpringBoard.h"
#include "Objects/Statue.h"
#include "Objects/GroundBlower.h"
#include "Objects/ConveyorBelt.h"
#include "Objects/Cannon.h"
#include "Objects/GooVent.h"
#include "Objects/CrabNest.h"
#include "Objects/TProjectilesShooter.h"
#include "Objects/FloorSpike.h"
#include "Objects/GooVent.h"
#include "Objects/Laser.h"
#include "Objects/Stalactite.h"
#include "Objects/LavaMouth.h"
#include "Objects/LavaHand.h"
#include "Enemies/Officer.h"
#include "Enemies/Soldier.h"
#include "Enemies/Rat.h"
#include "Enemies/RobberThief.h"
#include "Enemies/CutThroat.h"
#include "Enemies/Seagull.h"
#include "Enemies/LeRauxe.h"
#include "Enemies/Katherine.h"
#include "Enemies/TownGuard.h"
#include "Enemies/Wolvington.h"
#include "Enemies/RedTailPirate.h"
#include "Enemies/BearSailor.h"
#include "Enemies/CrazyHook.h"
#include "Enemies/HermitCrab.h"
#include "Enemies/PegLeg.h"
#include "Enemies/Mercat.h"
#include "Enemies/Siren.h"
#include "Enemies/Fish.h"
#include "Enemies/Gabriel.h"
#include "Enemies/Marrow.h"
#include "Enemies/Chameleon.h"
#include "Enemies/Aquatis.h"
#include "Enemies/RedTail.h"
#include "Enemies/TigerGuard.h"
#include "Enemies/LordOmar.h"


#define SHAKE_TIME 3000 // time of shaking screen (ms)

#define player	BasePlaneObject::player
#define physics	BasePlaneObject::physics

#define eraseByValue(vec, val) vec.erase(find(vec.begin(), vec.end(), val))

#define ADD_ENEMY(p) { BaseEnemy* enemy = DBG_NEW p; _objects.push_back(enemy); _enemies.push_back(enemy); }
#define ADD_DAMAGE_OBJECT(p) { BaseDamageObject* dObj = DBG_NEW p; _objects.push_back(dObj); _damageObjects.push_back(dObj); }
#define ADD_BOSS_OBJECT(p) { _bossObjects.push_back(DBG_NEW p); }
#define ADD_BOSS(p) { _boss = DBG_NEW p; _bossObjects.push_back(_boss); }

#ifdef _DEBUG
#define NO_ENEMIES
#define NO_OBSTACLES
#endif



ActionPlane* ActionPlane::_instance = nullptr;
shared_ptr<SavedGameManager::GameData> ActionPlane::_loadGameData;


ActionPlane::ActionPlane(WapWwd* wwd, WwdPlane* wwdPlane)
	: LevelPlane(wwd, wwdPlane), _planeSize({}), _boss(nullptr), _shakeTime(0),
	_BossStagerDelay(0), _isInBoss(false), _levelState(LevelState::Playing)
{
	if (_instance)
		DBG_PRINT("Warning: ActionPlane already exists (instance of level %d)", _instance->_wwd->levelNumber);
	_instance = this;
}
ActionPlane::~ActionPlane()
{
	eraseByValue(_objects, player.get()); // we don't want to delete the player because we save him for the next level

	for (BasePlaneObject* i : _objects)
		delete i;
	for (BasePlaneObject* i : _bossObjects)
		delete i;

	// because it static member and we don't want recycle objects...
	_instance = nullptr;
	_loadGameData = nullptr;
}

void ActionPlane::Logic(uint32_t elapsedTime)
{
	if (player->isFinishLevel()) return;

	if (_levelState != LevelState::Playing)
	{
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
			_instance->_levelState = LevelState::Playing; // skip the boss stager
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
		obj->Logic(elapsedTime);

		if (obj == player.get())
		{
			if (!player->isInDeathAnimation())
			{
				physics->checkCollides(player.get());
			}
		}
		else if (isbaseinstance<BaseEnemy>(obj) || isbaseinstance<Projectile>(obj)
			|| (isinstance<PowderKeg>(obj) && !((PowderKeg*)obj)->isExplode())
			|| (isinstance<Item>(obj) && ((Item*)obj)->speed.y != 0)
			|| isinstance<GabrielRedTailPirate>(obj))
		{
			physics->checkCollides((BaseDynamicPlaneObject*)obj);
		}
		else if (isinstance<StackedCrates>(obj))
		{
			vector<BasePlaneObject*> items = ((StackedCrates*)obj)->getItems();
			_objects += items;
		}
		else if (isbaseinstance<Crate>(obj))
		{
			Crate* crate = (Crate*)obj;
			if (crate->isBreaking())
			{
				vector<BasePlaneObject*> items = crate->getItems();
				_objects += items;
			}
		}

		if (_shakeTime <= 0 &&
			(isinstance<PowderKeg>(obj) && ((PowderKeg*)obj)->isStartExplode()) ||
			(isinstance<ClawDynamite>(obj) && ((ClawDynamite*)obj)->isStartExplode()))
			exploseShake = true;

		if (obj->removeObject)
		{
			if (isbaseinstance<BaseEnemy>(obj))
			{
				eraseByValue(_enemies, obj);
				if (isbaseinstance<BaseBoss>(obj))
				{
					_boss = nullptr;
					_isInBoss = false;
				}
			}
			else if (isbaseinstance<Projectile>(obj))
			{
				if (isinstance<Stalactite>(obj))
				{
					obj->removeObject = false; // we don't want to delete this object
					continue;
				}
				else
					eraseByValue(_projectiles, obj);
			}
			else if (isinstance<PowderKeg>(obj))
				eraseByValue(_powderKegs, obj);
			else if (isbaseinstance<BaseDamageObject>(obj))
				eraseByValue(_damageObjects, obj);
			else if (isinstance<Warp>(obj) && obj == Warp::DestinationWarp)
			{
				_objects.erase(_objects.begin() + i);
				i--; // cancel `i++`
				continue; // we don't delete so ClawLevelEngine can use it
			}

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
	for (auto i = _shakeRects.begin(); i != _shakeRects.end(); i++)
	{
		if (playerRect.intersects(*i))
		{
			_shakeTime = SHAKE_TIME;
			_shakeRects.erase(i);
			exploseShake = false; // we don't want to shake the screen twice
			break;
		}
	}

	if (exploseShake)
		_shakeTime = 500;
#endif
}

void ActionPlane::init()
{
	// initialize global fields

	_planeSize.width = (float)TILE_SIZE * _wwdPlane->tilesOnAxisX;
	_planeSize.height = (float)TILE_SIZE * _wwdPlane->tilesOnAxisY;

	AssetsManager::startBackgroundMusic(AssetsManager::BackgroundMusicType::Level);
	physics = make_shared<PhysicsManager>(_wwd, this); // must be after WWD map loaded and before objects added

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
	ConveyorBelt::GlobalInit(); // must be after LevelPlane::init()

	if (_loadGameData)
	{
		player->setGameData(*_loadGameData.get());
		_loadGameData = nullptr;
	}
	_objects.push_back(player.get()); // must be after LevelPlane::init() because we reset the objects vector there

	_levelState = LevelState::Playing;
	_BossStagerDelay = 0;
}
void ActionPlane::addObject(const WwdObject& obj)
{
	/* Objects */
	if (obj.logic == "FrontCandy" || obj.logic == "FrontAniCandy" ||
		obj.logic == "BehindCandy" || obj.logic == "BehindAniCandy" ||
		obj.logic == "DoNothing" || obj.logic == "DoNothingNormal" ||
		obj.logic == "AniCycle" || obj.logic == "AniCycleNormal" ||
		obj.logic == "Sign" || obj.logic == "GooCoverup")
	{
		_objects.push_back(DBG_NEW DoNothing(obj));
	}
	else if (obj.logic == "GlobalAmbientSound")
	{
		_objects.push_back(DBG_NEW GlobalAmbientSound(obj, _wwd->levelNumber));
	}
	else if (obj.logic == "AmbientSound" || obj.logic == "SpotAmbientSound")
	{
		_objects.push_back(DBG_NEW AmbientSound(obj));
	}
	else if (contains(obj.logic, "SoundTrigger"))
	{
		_objects.push_back(DBG_NEW SoundTrigger(obj));
	}
	else if (obj.logic == "BehindCrate" || obj.logic == "FrontCrate")
	{
		_objects.push_back(DBG_NEW Crate(obj));
	}
	else if (obj.logic == "FrontStackedCrates" || obj.logic == "BackStackedCrates")
	{
		_objects.push_back(DBG_NEW StackedCrates(obj));
	}
	else if (obj.logic == "FrontStatue" || obj.logic == "BehindStatue")
	{
		_objects.push_back(DBG_NEW Statue(obj));
	}
	else if (obj.logic == "PowderKeg")
	{
		PowderKeg* p = DBG_NEW PowderKeg(obj);
		_objects.push_back(p);
		_powderKegs.push_back(p);
	}
	else if (endsWith(obj.logic, "Elevator"))
	{
		_objects.push_back(Elevator::create(obj, _wwd->levelNumber));
	}
	else if (endsWith(obj.logic, "Checkpoint"))
	{
		_objects.push_back(DBG_NEW Checkpoint(obj, _wwd->levelNumber));

		if (_loadGameData)
		{
			if (
				(_loadGameData->savePoint == 1 && obj.logic == "FirstSuperCheckpoint") ||
				(_loadGameData->savePoint == 2 && obj.logic == "SecondSuperCheckpoint")
				)
			{
				player->position = player->startPosition = ((Checkpoint*)_objects.back())->position;
			}
		}
	}
	else if (obj.logic == "StartSteppingStone")
	{
		_objects.push_back(DBG_NEW StartSteppingStone(obj));
	}
	else if (startsWith(obj.logic, "TogglePeg") || startsWith(obj.logic, "SteppingStone"))
	{
		_objects.push_back(DBG_NEW TogglePeg(obj));
	}
	else if (obj.logic == "CrumblingPeg")
	{
		_objects.push_back(DBG_NEW CrumblingPeg(obj));
	}
	else if (obj.logic == "BreakPlank")
	{
		int idx = -1;
		if (_wwd->levelNumber == 5) idx = 509;
		else if (_wwd->levelNumber == 11) idx = 39;
		//else throw Exception("invalid level");

		WwdTileDescription& tileDesc = _wwd->tileDescriptions[idx];

		float topOffset = (float)tileDesc.rect.top;

		for (int i = 0; i < obj.width; i++)
		{
			_objects.push_back(DBG_NEW BreakPlank(obj, topOffset));
			(int32_t&)obj.x += TILE_SIZE;
			//myMemCpy(obj.x, obj.x + TILE_SIZE);
		}
	}
	else if (obj.logic == "TreasurePowerup" || obj.logic == "GlitterlessPowerup"
		|| obj.logic == "SpecialPowerup" || obj.logic == "AmmoPowerup"
		|| obj.logic == "BossWarp" || obj.logic == "HealthPowerup"
		|| obj.logic == "EndOfLevelPowerup" || obj.logic == "MagicPowerup"
		/*|| obj.logic == "CursePowerup"*/)
	{
		_objects.push_back(Item::getItem(obj, true));
	}
	else if (obj.logic == "AniRope")
	{
		_objects.push_back(DBG_NEW Rope(obj));
	}
	else if (obj.logic == "SpringBoard" || obj.logic == "WaterRock")
	{
		_objects.push_back(DBG_NEW SpringBoard(obj, _wwd->levelNumber));
	}
	else if (obj.logic == "GroundBlower")
	{
		_objects.push_back(DBG_NEW GroundBlower(obj));
	}
	else if (obj.logic == "ConveyorBelt")
	{
		_objects.push_back(DBG_NEW ConveyorBelt(obj));
	}

	/* Enemies */

#ifndef NO_ENEMIES
	else if (obj.logic == "CrabNest")
	{
		_objects.push_back(DBG_NEW CrabNest(obj));
	}
	else if (obj.logic == "Officer")
	{
		ADD_ENEMY(Officer(obj));
	}
	else if (obj.logic == "Soldier")
	{
		ADD_ENEMY(Soldier(obj));
	}
	else if (obj.logic == "Rat")
	{
		ADD_ENEMY(Rat(obj));
	}
	else if (obj.logic == "PunkRat")
	{
		ADD_ENEMY(PunkRat(obj));
	}
	else if (obj.logic == "RobberThief")
	{
		ADD_ENEMY(RobberThief(obj));
	}
	else if (obj.logic == "CutThroat")
	{
		ADD_ENEMY(CutThroat(obj));
	}
	else if (obj.logic == "Seagull")
	{
		ADD_ENEMY(Seagull(obj));
	}
	else if (obj.logic == "TownGuard1" || obj.logic == "TownGuard2")
	{
		ADD_ENEMY(TownGuard(obj));
	}
	else if (obj.logic == "RedTailPirate")
	{
		ADD_ENEMY(RedTailPirate(obj));
	}
	else if (obj.logic == "BearSailor")
	{
		ADD_ENEMY(BearSailor(obj));
	}
	else if (obj.logic == "CrazyHook")
	{
		ADD_ENEMY(CrazyHook(obj));
	}
	else if (obj.logic == "HermitCrab")
	{
		ADD_ENEMY(HermitCrab(obj));
	}
	else if (obj.logic == "PegLeg")
	{
		ADD_ENEMY(PegLeg(obj));
	}
	else if (obj.logic == "Mercat")
	{
		ADD_ENEMY(Mercat(obj));
	}
	else if (obj.logic == "Siren")
	{
		ADD_ENEMY(Siren(obj));
	}
	else if (obj.logic == "Fish")
	{
		ADD_ENEMY(Fish(obj));
	}
	else if (obj.logic == "Chameleon")
	{
		ADD_ENEMY(Chameleon(obj));
	}
	else if (obj.logic == "TigerGuard")
	{
		ADD_ENEMY(TigerGuard(obj));
	}
#endif

	/* Obstacles */

#ifndef NO_OBSTACLES
	else if (obj.logic == "TowerCannonLeft" || obj.logic == "TowerCannonRight")
	{
		_objects.push_back(DBG_NEW TowerCannon(obj));
	}
	else if (obj.logic == "SkullCannon")
	{
		_objects.push_back(DBG_NEW SkullCannon(obj));
	}
	else if (obj.logic == "TProjectile")
	{
		_objects.push_back(DBG_NEW TProjectilesShooter(obj, _wwd->levelNumber));
	}
	else if (obj.logic == "GooVent")
	{
		ADD_DAMAGE_OBJECT(GooVent(obj));
	}
	else if (startsWith(obj.logic, "FloorSpike"))
	{
		ADD_DAMAGE_OBJECT(FloorSpike(obj));
	}
	else if (startsWith(obj.logic, "SawBlade"))
	{
		ADD_DAMAGE_OBJECT(SawBlade(obj));
	}
	else if (obj.logic == "LavaGeyser")
	{
		ADD_DAMAGE_OBJECT(LavaGeyser(obj));
	}
	else if (obj.logic == "Laser")
	{
		ADD_DAMAGE_OBJECT(Laser(obj));
	}
	else if (obj.logic == "LavaMouth")
	{
		ADD_DAMAGE_OBJECT(LavaMouth(obj));
	}
	else if (obj.logic == "Stalactite")
	{
		Stalactite* p = DBG_NEW Stalactite(obj);
		_objects.push_back(p);
		_projectiles.push_back(p);
	}
	else if (obj.logic == "Shake")
	{
		_shakeRects.push_back(Rectangle2D((float)obj.attackRect.left, (float)obj.attackRect.top,
			(float)obj.attackRect.right, (float)obj.attackRect.bottom));
	}
	else if (obj.logic == "LavaHand")
	{
		_objects.push_back(DBG_NEW LavaHand(obj));
	}
#endif

	/* Bosses and their objects */
#pragma region Bosses
	else if (obj.logic == "Raux")
	{
		ADD_BOSS(LeRauxe(obj));
	}
	else if (obj.logic == "Katherine")
	{
		ADD_BOSS(Katherine(obj));
	}
	else if (obj.logic == "Wolvington")
	{
		ADD_BOSS(Wolvington(obj));
	}
	else if (obj.logic == "Gabriel")
	{
		ADD_BOSS(Gabriel(obj));
	}
	else if (obj.logic == "GabrielCannon")
	{
		ADD_BOSS_OBJECT(GabrielCannon(obj));
	}
	else if (obj.logic == "CannonSwitch")
	{
		ADD_BOSS_OBJECT(GabrielCannonSwitch(obj));
	}
	else if (obj.logic == "CannonButton")
	{
		ADD_BOSS_OBJECT(GabrielCannonButton(obj));
	}
	else if (obj.logic == "Marrow")
	{
		ADD_BOSS(Marrow(obj));
	}
	else if (obj.logic == "Parrot")
	{
		ADD_BOSS_OBJECT(MarrowParrot(obj));
	}
	else if (obj.logic == "MarrowFloor")
	{
		ADD_BOSS_OBJECT(MarrowFloor(obj));
	}
	else if (obj.logic == "Aquatis")
	{
		ADD_BOSS(Aquatis(obj));
	}
	else if (obj.logic == "Tentacle")
	{
		ADD_BOSS_OBJECT(AquatisTentacle(obj));
	}
	else if (obj.logic == "AquatisCrack")
	{
		ADD_BOSS_OBJECT(AquatisCrack(obj));
	}
	else if (obj.logic == "AquatisDynamite")
	{
		_bossObjects.push_back(Item::getItem(obj, true));
	}
	else if (obj.logic == "AquatisStalactite")
	{
		ADD_BOSS_OBJECT(AquatisStalactite(obj));
	}
	else if (obj.logic == "RedTail")
	{
		ADD_BOSS(RedTail(obj));
	}
	else if (obj.logic == "RedTailSpikes")
	{
		ADD_BOSS_OBJECT(RedTailSpikes(obj));
	}
	else if (obj.logic == "RedTailWind")
	{
		ADD_BOSS_OBJECT(RedTailWind(obj));
	}
	else if (obj.logic == "Omar")
	{
		ADD_BOSS(LordOmar(obj));
	}
#pragma endregion

	//	throw Exception("TODO: logic=" + obj.logic);
}

void ActionPlane::loadGame(int level, int checkpoint)
{
	SavedGameManager::GameData data = SavedGameManager::load(level, checkpoint);
	if (data.level == level && data.savePoint == checkpoint)
	{
		// success to load checkpoint
		_loadGameData = make_shared<SavedGameManager::GameData>(data);
	}
}
void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	//if (obj == nullptr) return;
	_instance->_objects.push_back(obj); // don't insert in middle because some of the object add new objects in their destructor
	if (isbaseinstance<Projectile>(obj)) _instance->_projectiles.push_back((Projectile*)obj);
	else if (isbaseinstance<BaseEnemy>(obj)) _instance->_enemies.push_back((BaseEnemy*)obj);
}
void ActionPlane::resetObjects()
{
	for (BasePlaneObject* obj : _instance->_objects)
		obj->Reset();
}
void ActionPlane::playerEnterToBoss(float bossWarpX)
{
	// clear all objects that we don't need in boss
	for (auto i : _instance->_powderKegs) i->removeObject = true;
	for (auto i : _instance->_enemies) i->removeObject = true;
	for (auto i : _instance->_projectiles) i->removeObject = true;
	for (auto i : _instance->_damageObjects) i->removeObject = true;

	// find all objects that we don't need in boss and remove them (the boss-warp 
	// and boss area are in the right side of the screen so it easy to find them)
	for (BasePlaneObject* obj : _instance->_objects)
	{
		if (obj->position.x < bossWarpX)
			obj->removeObject = true;
	}

	// move all objects that we need in boss to the objects' list
	for (BasePlaneObject* obj : _instance->_bossObjects)
	{
		_instance->_objects.push_back(obj);
		if (isbaseinstance<BaseEnemy>(obj))
			_instance->_enemies.push_back((BaseEnemy*)obj);
		else if (isbaseinstance<Projectile>(obj))
			_instance->_projectiles.push_back((Projectile*)obj);
		else if (isbaseinstance<BaseDamageObject>(obj))
			_instance->_damageObjects.push_back((BaseDamageObject*)obj);
	}
	_instance->_bossObjects.clear();

	_instance->_isInBoss = true;

	AssetsManager::startBackgroundMusic(AssetsManager::BackgroundMusicType::Boss);
	_instance->_BossStagerDelay = 0;
	_instance->_levelState = LevelState::BossStager_Start;
}

void ActionPlane::updatePosition()
{
	const D2D1_SIZE_F camSize = WindowManager::getCameraSize();
	BaseCharacter* character = player.get();

	if (_instance->_levelState == LevelState::BossStager_BossTalk || _instance->_levelState == LevelState::BossStager_ClawTalk)
		character = _instance->_boss; // if we are in boss stager, we need to show the boss while s/he is talking

	// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
	_instance->position.x = character->position.x - camSize.width / 2.0f;
	_instance->position.y = character->position.y - camSize.height / 2.0f;

	float maxOffsetX = _instance->_planeSize.width - camSize.width;
	float maxOffsetY = _instance->_planeSize.height - camSize.height;

	if (_instance->position.x < 0) _instance->position.x = 0;
	if (_instance->position.y < 0) _instance->position.y = 0;
	if (_instance->position.x > maxOffsetX) _instance->position.x = maxOffsetX;
	if (_instance->position.y > maxOffsetY) _instance->position.y = maxOffsetY;

	if (_instance->_shakeTime > 0)
	{
		// to shake the screen we just move it a little bit
		const float shakeDelta = 5;
		if (_instance->_shakeTime / 75 % 2) { _instance->position.x -= shakeDelta; _instance->position.y -= shakeDelta; }
		else { _instance->position.x += shakeDelta; _instance->position.y += shakeDelta; }
	}
}

void ActionPlane::writeMessage(const string& message, int timeout)
{
	addPlaneObject(DBG_NEW ActionPlaneMessage(message, timeout));
}
