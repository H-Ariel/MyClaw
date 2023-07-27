#include "ActionPlane.h"
#include "WindowManager.h"
#include "AssetsManager.h"
#include "Player.h"
#include "Objects/Checkpoint.h"
#include "Objects/Elevator.h"
#include "Objects/Crate.h"
#include "Objects/Projectile.h"
#include "Objects/Officer.h"
#include "Objects/Soldier.h"
#include "Objects/Rat.h"
#include "Objects/RobberThief.h"
#include "Objects/CutThroat.h"
#include "Objects/Seagull.h"
#include "Objects/LeRauxe.h"
#include "Objects/Katherine.h"
#include "Objects/TownGuard.h"
#include "Objects/SoundTrigger.h"
#include "Objects/Pegs.h"
#include "Objects/DoNothing.h"
#include "Objects/Cannon.h"
#include "Objects/Rope.h"
#include "Objects/SteppingStone.h"
#include "Objects/SpringBoard.h"
#include "Objects/Statue.h"
#include "Objects/GroundBlower.h"
#include "Objects/GooVent.h"
#include "Objects/Wolvington.h"
#include "Objects/ConveyorBelt.h"
#include "Objects/RedTailPirate.h"
#include "Objects/BearSailor.h"
#include "Objects/CrazyHook.h"
#include "Objects/HermitCrab.h"
#include "Objects/CrabNest.h"
#include "Objects/TProjectilesShooter.h"
#include "Objects/PegLeg.h"
#include "Objects/Mercat.h"
#include "Objects/Siren.h"
#include "Objects/Fish.h"
#include "Objects/Gabriel.h"
#include "Objects/Marrow.h"
#include "Objects/Chameleon.h"
#include "Objects/FloorSpike.h"
#include "Objects/GooVent.h"
#include "Objects/Laser.h"


#define EMPTY_TILE -1

#define RECT_SPEED			0.5f // speed of the rect that shows when CC is died
#define CC_FALLDEATH_SPEED	0.7f // speed of CC when he falls out the window

#define MIN_OFFSET_X 0
#define MIN_OFFSET_Y 0
#define MAX_OFFSET_X (_planeSize.width - wndSize.width)
#define MAX_OFFSET_Y (_planeSize.height - wndSize.height)

#define eraseByValue(vec, val) vec.erase(find(vec.begin(), vec.end(), val))
#define player BasePlaneObject::player

#define ADD_ENEMY(p) { BaseEnemy* enemy = DBG_NEW p; _objects.push_back(enemy); _enemies.push_back(enemy); }
#define ADD_DAMAGE_OBJECT(p) { BaseDamageObject* dObj = DBG_NEW p; _objects.push_back(dObj); _damageObjects.push_back(dObj); }
#define ADD_BOSS_OBJECT(p) { _bossObjects.push_back(DBG_NEW p); }


// TODO: make sure we impleted all the logics
//#define SAVE_LOGICS "c:/users/ariel/desktop/remain- level7 logics.txt"
//#ifndef _DEBUG
//#undef LOW_DETAILS
//#define USE_ENEMIES
//#endif

/*
class SimpleObject : public BasePlaneObject
{
public:
	SimpleObject(const WwdObject& obj) : BasePlaneObject(obj) {}
	void Logic(uint32_t elapsedTime) override {}
	void Draw() override { WindowManager::drawRect(Rectangle2D(position.x - 32,
		position.y - 32, position.x + 32, position.y + 32), ColorF::Green); }
	Rectangle2D GetRect() override { return Rectangle2D(); }
};
*/

ActionPlane* ActionPlane::_instance = nullptr;

ActionPlane::ActionPlane(WapWorld* wwd, int levelNumber)
	: _state(States::Play), _deathAniWait(false), _wwd(wwd),
	_needSort(true), _holeRadius(0), _physicsManager(nullptr)
{
	if (_instance != nullptr)
		throw Exception("ActionPlane already exists");
	else
		_instance = this;

	WwdObject playerData;
	playerData.x = _wwd->startX;
	playerData.y = _wwd->startY;
	playerData.z = 4000;
	_objects.push_back(player = DBG_NEW Player(playerData, _planeSize));

}
ActionPlane::~ActionPlane()
{
	for (BasePlaneObject* i : _objects)
		delete i;
	for (BasePlaneObject* i : _bossObjects)
		delete i;

	SafeDelete(_physicsManager);

	// because it static member and we don't want recycle objects...
	_instance = nullptr;
}

void ActionPlane::init()
{
	_planeSize.width = (float)TILE_SIZE * tilesOnAxisX;
	_planeSize.height = (float)TILE_SIZE * tilesOnAxisY;


#ifdef SAVE_LOGICS
	set<string> allLogics;
#endif

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level);
	_physicsManager = DBG_NEW PhysicsManager(_wwd); // must be after WWD map loaded and before objects added

	for (const WwdObject& obj : objects)
	{
#ifdef SAVE_LOGICS
		allLogics.insert(obj.logic);
#endif

		try
		{
			addObject(obj);
		}
		catch (const Exception& ex)
		{
			// an exception may be thrown if the path is invalid or if we don't impleted the logic of object
#ifdef _DEBUG
			cout << "Error while adding object \"" << obj.logic << "\": message: " << ex.what() << endl;
#endif
		}
}
#ifdef SAVE_LOGICS
	ofstream of(SAVE_LOGICS);
	for (auto& i : allLogics) of << i << endl;
#endif
}

void ActionPlane::Logic(uint32_t elapsedTime)
{
	if (player->isFinishLevel()) return;

	if (_deathAniWait)
	{
		switch (_state)
		{
		case States::Close:
			_holeRadius -= RECT_SPEED * elapsedTime;
			if (_holeRadius <= 0)
			{
				_state = States::Open;
				player->backToLife();
				updatePosition();

				for (BasePlaneObject* obj : _objects)
				{
					obj->Reset();
				}
			}
			break;

		case States::Open:
			_holeRadius += RECT_SPEED * elapsedTime;
			if (player->position.x - position.x < _holeRadius)
			{
				_deathAniWait = false;
				_state = States::Play;
			}
			break;

		case States::Fall:
			player->position.y += CC_FALLDEATH_SPEED * elapsedTime;
			player->Logic(0); // update position of animation
			if (player->position.y - position.y > WindowManager::getSize().height)
			{
				player->loseLife();
				_state = States::Close;
				_deathAniWait = true;
				auto wndSz = WindowManager::getSize();
				_holeRadius = max(wndSz.width, wndSz.height) / 2;
			}
			break;
		}
		return;
	}

	if (player->isFinishDeathAnimation() && player->hasLives() && _state == States::Play)
	{
		if (player->isSpikeDeath())
		{
			_state = States::Close;
			auto wndSz = WindowManager::getSize();
			_holeRadius = max(wndSz.width, wndSz.height) / 2;
		}
		else //if (player->isFallDeath())
		{
			_state = States::Fall;
		}

		_deathAniWait = true;
		return;
	}

	if (!player->isInDeathAnimation())
	{
		_physicsManager->checkCollides(player, [&] { player->loseLife(); });
	}

	updatePosition();

	if (_needSort)
	{
		sort(_objects.begin(), _objects.end(), [](BasePlaneObject* a, BasePlaneObject* b) { return a->ZCoord < b->ZCoord; });
	}

	BasePlaneObject* obj;
	
	for (size_t i = 0; i < _objects.size(); i++)
	{
		obj = _objects[i];
		obj->Logic(elapsedTime);

		if (isbaseinstance<BaseEnemy>(obj) || isProjectile(obj) || isinstance<PowderKeg>(obj)
			|| (isinstance<Item>(obj) && ((Item*)obj)->getSpeedY() != 0)
			|| isinstance<GabrielRedTailPirate>(obj))
		{
			_physicsManager->checkCollides((BaseDynamicPlaneObject*)obj, [obj] { obj->removeObject = true; });
		}
		else if (isinstance<StackedCrates>(obj))
		{
			vector<Item*> items = ((StackedCrates*)obj)->getItems();
			_objects.insert(_objects.end(), items.begin(), items.end());
		}
		else if (isbaseinstance<Crate>(obj))
		{
			Crate* crate = (Crate*)obj;
			if (crate->isBreaking())
			{
				vector<Item*> items = crate->getItems();
				_objects.insert(_objects.end(), items.begin(), items.end());
			}
		}

		if (obj->removeObject)
		{
			if (isbaseinstance<BaseEnemy>(obj))
				eraseByValue(_enemies, obj);
			else if (isProjectile(obj))
				eraseByValue(_projectiles, obj);
			else if (isinstance<PowderKeg>(obj))
				eraseByValue(_powderKegs, obj);

			delete obj;
			_objects.erase(_objects.begin() + i);
			i--; // cancel `i++`
		}
	}

	AssetsManager::callLogics(elapsedTime);
}
void ActionPlane::Draw()
{
	LevelPlane::Draw();

	for (BasePlaneObject* obj : _objects)
		obj->Draw();

	_physicsManager->Draw();

	if (_state == States::Close || _state == States::Open)
	{
		const D2D1_SIZE_F wndSz = WindowManager::getSize();
		const float wp = wndSz.width + position.x, hp = wndSz.height + position.y;
		const Rectangle2D rc1(position.x, position.y, player->position.x - _holeRadius, hp);
		const Rectangle2D rc2(rc1.right, position.y, wp, player->position.y - _holeRadius);
		const Rectangle2D rc3(rc1.right, player->position.y + _holeRadius, wp, hp);
		const Rectangle2D rc4(player->position.x + _holeRadius, rc2.bottom, wp, rc3.top);

		WindowManager::fillRect(rc1, ColorF::Black);
		WindowManager::fillRect(rc2, ColorF::Black);
		WindowManager::fillRect(rc3, ColorF::Black);
		WindowManager::fillRect(rc4, ColorF::Black);

		// TODO: draw circle, not 4 rectangles
		//WindowManager::drawCircle(player->position, _holeRadius, ColorF::Black, 20);
	}
}

void ActionPlane::playerEnterToBoss()
{
	// clear all objects that we don't need in boss
	for (auto& i : _instance->_powderKegs) i->removeObject = true;
	for (auto& i : _instance->_enemies) i->removeObject = true;
	for (auto& i : _instance->_projectiles) i->removeObject = true;
	for (auto& i : _instance->_damageObjects) i->removeObject = true;

	// move all objects that we need in boss to the objects' list
	for (auto& i : _instance->_bossObjects) {
		_instance->_objects.push_back(i);

		if (isbaseinstance<BaseEnemy>(i))
			_instance->_enemies.push_back((BaseEnemy*)i);
		else if (isProjectile(i))
			_instance->_projectiles.push_back((Projectile*)i);

	}
	_instance->_bossObjects.clear();
	_instance->_needSort = true;

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Boss);
}

void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	// TODO? if (obj == nullptr) return; 
	_instance->_objects.push_back(obj);
	_instance->_needSort = true;
	if (isProjectile(obj)) _instance->_projectiles.push_back((Projectile*)obj);
	else if (isbaseinstance<BaseEnemy>(obj)) _instance->_enemies.push_back((BaseEnemy*)obj); // TODO: make sure we need this
}

void ActionPlane::addObject(const WwdObject& obj)
{
#ifndef LOW_DETAILS
	if (obj.logic == "FrontCandy" || obj.logic == "BehindCandy" ||
		obj.logic == "BehindAniCandy" || obj.logic == "FrontAniCandy" ||
		obj.logic == "DoNothing" || obj.logic == "DoNothingNormal" ||
		obj.logic == "AniCycle" || obj.logic == "GooCoverup" ||
		obj.logic == "Sign" || obj.logic == "AniCycleNormal")
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
	else 
		if (obj.logic == "PowderKeg")
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
		_objects.push_back(DBG_NEW Checkpoint(obj));
	}
	else if (startsWith(obj.logic, "TogglePeg"))
	{
		_objects.push_back(DBG_NEW TogglePeg(obj));
	}
	else if (obj.logic == "StartSteppingStone")
	{
		_objects.push_back(DBG_NEW StartSteppingStone(obj));
	}
	else if (startsWith(obj.logic, "SteppingStone"))
	{
		_objects.push_back(DBG_NEW SteppingStone(obj));
	}
	else if (obj.logic == "CrumblingPeg")
	{
		_objects.push_back(DBG_NEW CrumblingPeg(obj));
	}
	else if (obj.logic == "BreakPlank")
	{
		int32_t topOffset = 0;
		if (_wwd->levelNumber == 5) topOffset = _wwd->tilesDescription[509].rect.top;
		else if (_wwd->levelNumber == 11) topOffset = _wwd->tilesDescription[39].rect.top;

		for (int32_t i = 0; i < obj.width; i++)
		{
			_objects.push_back(DBG_NEW BreakPlank(obj, (float)topOffset));
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
		_objects.push_back(Item::getItem(obj));
	}
	else if (obj.logic == "AniRope")
	{
		_objects.push_back(DBG_NEW Rope(obj));
	}
	else if (obj.logic == "SpringBoard" || obj.logic == "WaterRock")
	{
		_objects.push_back(DBG_NEW SpringBoard(obj));
	}
	else if (obj.logic == "GroundBlower")
	{
		_objects.push_back(DBG_NEW GroundBlower(obj));
	}
	else if (obj.logic == "ConveyorBelt")
	{
		_objects.push_back(DBG_NEW ConveyorBelt(obj));
	}
#ifdef USE_ENEMIES
	else if (obj.logic == "TowerCannonLeft" || obj.logic == "TowerCannonRight")
	{
		_objects.push_back(DBG_NEW TowerCannon(obj));
	}
	else if (obj.logic == "GooVent")
	{
		ADD_DAMAGE_OBJECT(GooVent(obj));
	}
	else if (startsWith(obj.logic, "FloorSpike"))
	{
		ADD_DAMAGE_OBJECT(FloorSpike(obj));
	}
	else if (obj.logic == "CrabNest")
	{
		_objects.push_back(DBG_NEW CrabNest(obj));
	}
	else if (startsWith(obj.logic, "SawBlade"))
	{
		ADD_DAMAGE_OBJECT(SawBlade(obj));
	}
	else if (obj.logic == "TProjectile")
	{
		_objects.push_back(DBG_NEW TProjectilesShooter(obj));
	}
	else if (obj.logic == "SkullCannon")
	{
		_objects.push_back(DBG_NEW SkullCannon(obj));
	}
	else if (obj.logic == "Laser")
	{
		ADD_DAMAGE_OBJECT(Laser(obj));
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
#endif
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

	//	throw Exception("TODO: logic=" + obj.logic);
}

void ActionPlane::updatePosition()
{
	// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
	const D2D1_SIZE_F wndSize = WindowManager::getSize();
	position.x = player->position.x - wndSize.width / 2.0f;
	position.y = player->position.y - wndSize.height / 2.0f;

	if (position.x < MIN_OFFSET_X) position.x = MIN_OFFSET_X;
	if (position.x > MAX_OFFSET_X) position.x = MAX_OFFSET_X;
	if (position.y < MIN_OFFSET_Y) position.y = MIN_OFFSET_Y;
	if (position.y > MAX_OFFSET_Y) position.y = MAX_OFFSET_Y;
}
