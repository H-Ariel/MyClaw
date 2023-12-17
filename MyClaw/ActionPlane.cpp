#include "ActionPlane.h"
#include "LevelHUD.h"
#include "PhysicsManager.h"
#include "GUI/WindowManager.h"
#include "GUI/ActionPlaneMessage.h"
#include "Assets-Managers/AssetsManager.h"
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

#ifdef _DEBUG
//#undef LOW_DETAILS
#define NO_ENEMIES
#define NO_OBSTACLES
#endif


bool cmpDrawZ(BasePlaneObject* a, BasePlaneObject* b) { return a->drawZ < b->drawZ; }
bool cmpLogicZ(BasePlaneObject* a, BasePlaneObject* b) { return a->logicZ < b->logicZ; }


ActionPlane* ActionPlane::_instance = nullptr;
shared_ptr<SavedGameManager::GameData> ActionPlane::_loadGameData;


ActionPlane::ActionPlane(WapWorld* wwd)
	: LevelPlane(wwd), _planeSize({}), _boss(nullptr), _shakeTime(0)
{
	if (_instance != nullptr)
		DBG_PRINT("Warning: ActionPlane already exists (level %d)", _instance->_wwd->levelNumber);
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

	if (_shakeTime > 0)
		_shakeTime -= elapsedTime;
	updatePosition();

	// TODO: do not sort twice ?
	sort(_objects.begin(), _objects.end(), cmpLogicZ); // for this method (`Logic`)

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
		else if (isbaseinstance<BaseEnemy>(obj) || isbaseinstance<Projectile>(obj) || isinstance<PowderKeg>(obj)
			|| (isinstance<Item>(obj) && ((Item*)obj)->speed.y != 0)
			|| isinstance<GabrielRedTailPirate>(obj))
		{
			// TODO: delete this `else if` statement and call `physics->checkCollides` inside `obj->Logic`
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
				eraseByValue(_enemies, obj);
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

			delete obj;
			_objects.erase(_objects.begin() + i);
			i--; // cancel `i++`
		}
	}

	sort(_objects.begin(), _objects.end(), cmpDrawZ); // for `Draw` method

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

void ActionPlane::readPlaneObjects(BufferReader& reader, int numOfObjects)
{
	// initialize global fields and then read objects: (we init here because now we have all data)

	_planeSize.width = (float)TILE_SIZE * tilesOnAxisX;
	_planeSize.height = (float)TILE_SIZE * tilesOnAxisY;

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level);
	physics = allocNewSharedPtr<PhysicsManager>(_wwd, this); // must be after WWD map loaded and before objects added

	// player's initializtion must be before LevelPlane::readPlaneObjects() because some of objects need player
	if (player)
	{
		player->startPosition.x = (float)_wwd->startX;
		player->startPosition.y = (float)_wwd->startY;
		player->nextLevel();
	}
	else
	{
		player = allocNewSharedPtr<Player>();
		player->position.x = player->startPosition.x = (float)_wwd->startX;
		player->position.y = player->startPosition.y = (float)_wwd->startY;
	}

	LevelPlane::readPlaneObjects(reader, numOfObjects);
	ConveyorBelt::GlobalInit(); // must be after LevelPlane::readPlaneObjects()

	if (_loadGameData)
	{
		player->setGameData(*_loadGameData.get());
		_loadGameData = nullptr;
	}
	_objects.push_back(player.get()); // must be after LevelPlane::readPlaneObjects() because we reset the objects vector there

	sort(_objects.begin(), _objects.end(), cmpLogicZ);
}
void ActionPlane::addObject(const WwdObject& obj)
{
#ifndef LOW_DETAILS
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
		_objects.push_back(DBG_NEW GlobalAmbientSound(obj));
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
		_objects.push_back(p); _powderKegs.push_back(p);
	}
	else
#endif
	if (endsWith(obj.logic, "Elevator"))
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
		
		WwdTileDescription& tileDesc = _wwd->tilesDescription[idx];

		float topOffset = (float)tileDesc.rect.top;

		for (int32_t i = 0; i < obj.width; i++)
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
#endif
	else if (obj.logic == "LavaHand")
	{
		_objects.push_back(DBG_NEW LavaHand(obj));
	}
	else if (obj.logic == "Raux")
	{
		ADD_BOSS_OBJECT(LeRauxe(obj));
	}
	else if (obj.logic == "Katherine")
	{
		ADD_BOSS_OBJECT(Katherine(obj));
	}
	else if (obj.logic == "Wolvington")
	{
		ADD_BOSS_OBJECT(Wolvington(obj));
	}
	else if (obj.logic == "Gabriel")
	{
		ADD_BOSS_OBJECT(Gabriel(obj));
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
		ADD_BOSS_OBJECT(Marrow(obj));
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
		ADD_BOSS_OBJECT(Aquatis(obj));
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
		ADD_BOSS_OBJECT(RedTail(obj));
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
		ADD_BOSS_OBJECT(LordOmar(obj));
	}

//	throw Exception("TODO: logic=" + obj.logic);
}

void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	//if (obj == nullptr) return;
	_instance->_objects.push_back(obj); // don't insert in middle because some of the object add new objects in their destructor
	if (isbaseinstance<Projectile>(obj)) _instance->_projectiles.push_back((Projectile*)obj);
	else if (isbaseinstance<BaseEnemy>(obj)) _instance->_enemies.push_back((BaseEnemy*)obj);
}
void ActionPlane::loadGame(int level, int checkpoint)
{
	SavedGameManager::GameData data = SavedGameManager::load(level, checkpoint);
	if (data.level == level && data.savePoint == checkpoint)
	{
		// success to load checkpoint
		_loadGameData = allocNewSharedPtr<SavedGameManager::GameData>(data);
	}
}
void ActionPlane::resetObjects()
{
	for (BasePlaneObject* obj : _instance->_objects)
		obj->Reset();
}
void ActionPlane::playerEnterToBoss(float bossWarpX)
{
	// clear all objects that we don't need in boss
	for (auto& i : _instance->_powderKegs) i->removeObject = true;
	for (auto& i : _instance->_enemies) i->removeObject = true;
	for (auto& i : _instance->_projectiles) i->removeObject = true;
	for (auto& i : _instance->_damageObjects) i->removeObject = true;
	
	// find all objects that we don't need in boss and remove them (the boss-warp 
	// and boss area are in the right side of the screen so it easy to find them)
	for (auto& obj : _instance->_objects)
	{
		if (obj->position.x < bossWarpX)
			obj->removeObject = true;
	}

	// move all objects that we need in boss to the objects' list
	for (auto& i : _instance->_bossObjects)
	{
		_instance->_objects.push_back(i);
		if (isbaseinstance<BaseEnemy>(i))
		{
			if (isbaseinstance<BaseBoss>(i)) // TODO: i think that this condition is not needed...
				_instance->_boss = (BaseBoss*)i;
			_instance->_enemies.push_back((BaseEnemy*)i);
		}
		else if (isbaseinstance<Projectile>(i))
			_instance->_projectiles.push_back((Projectile*)i);
		else if (isbaseinstance<BaseDamageObject>(i))
			_instance->_damageObjects.push_back((BaseDamageObject*)i);
	}
	_instance->_bossObjects.clear();

	LevelHUD::setBossInitialHealth(_instance->_boss->getHealth());

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Boss);
}

void ActionPlane::updatePosition()
{
	// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
	const D2D1_SIZE_F wndSize = WindowManager::getSize();
	position.x = player->position.x - wndSize.width / 2.0f;
	position.y = player->position.y - wndSize.height / 2.0f;

	float maxOffsetX = _planeSize.width - wndSize.width;
	float maxOffsetY = _planeSize.height - wndSize.height;

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

void ActionPlane::writeMessage(const string& message, int timeout)
{
	addPlaneObject(DBG_NEW ActionPlaneMessage(message, timeout));
}
