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


#define EMPTY_TILE -1

#define RECT_SPEED			0.5f // speed of the rect that shows when player is died
#define CC_FALLDEATH_SPEED	0.7f // speed of CC when he falls out the window

#define MIN_OFFSET_X 0
#define MIN_OFFSET_Y 0
#define MAX_OFFSET_X (_planeSize.width - wndSize.width)
#define MAX_OFFSET_Y (_planeSize.height - wndSize.height)

#define eraseByValue(vec, val) vec.erase(find(vec.begin(), vec.end(), val))

// TODO: make sure we impleted all the logics
//#define SAVE_LOGICS "c:/users/ariel/desktop/remain- level7 logics.txt"
//#ifndef _DEBUG
//#undef LOW_DETAILS
#define USE_ENEMIES
//#endif

class SimpleObject : public BasePlaneObject
{
public:
	SimpleObject(const WwdObject& obj) : BasePlaneObject(obj) {}
	void Logic(uint32_t elapsedTime) override {}
	void Draw() override {
		WindowManager::drawRect(Rectangle2D(position.x - 32, position.y - 32, position.x + 32, position.y + 32), ColorF::Green);
	}
	Rectangle2D GetRect() override { return Rectangle2D(); }
};


PhysicsManager* ActionPlane::_physicsManager = nullptr;
vector<BasePlaneObject*> ActionPlane::_objects;
vector<PowderKeg*> ActionPlane::_powderKegs;
vector<BaseEnemy*> ActionPlane::_enemies;
vector<Projectile*> ActionPlane::_projectiles;
vector<FloorSpike*> ActionPlane::_floorSpikes;
vector<GooVent*> ActionPlane::_gooVents;
vector<Laser*> ActionPlane::_lasers;
bool ActionPlane::_needSort = true;


ActionPlane::ActionPlane(const WwdPlaneData& planeData, WapWorld* wwd, int levelNumber)
	: LevelPlane(planeData), _state(States::Play), _deathAniWait(false),
	_planeSize({ (float)TILE_SIZE * planeData.tilesOnAxisX, (float)TILE_SIZE * planeData.tilesOnAxisY })
{	
	WwdObject playerData;
	playerData.x = wwd->startX;
	playerData.y = wwd->startY;
	playerData.z = 4000;
	_objects.push_back(_player = DBG_NEW Player(playerData, _planeSize));

#ifdef SAVE_LOGICS
	set<string> allLogics;
#endif

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level, false);
	_physicsManager = DBG_NEW PhysicsManager(&_planeData, wwd, _player, levelNumber); // must be after WWD map loaded and before objects added

	for (const WwdObject& obj : planeData.objects)
	{
#ifdef SAVE_LOGICS
		allLogics.insert(obj.logic);
#endif

		try
		{
			addObject(obj, levelNumber, wwd);
		}
		catch (Exception& ex)
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
ActionPlane::~ActionPlane()
{
	for (BasePlaneObject* i : _objects)
	{
		delete i;
	}

	// because it static member and we don't want recycle objects...
	SafeDelete(_physicsManager);
	_objects.clear();
	_powderKegs.clear();
	_enemies.clear();
	_projectiles.clear();
	_floorSpikes.clear();
	_gooVents.clear();
	_lasers.clear();
	_needSort = true;
}

void ActionPlane::Logic(uint32_t elapsedTime)
{
	if (_player->isFinishLevel()) return;

	if (_deathAniWait)
	{
		switch (_state)
		{
		case States::Close:
			_holeRadius -= RECT_SPEED * elapsedTime;
			if (_holeRadius <= 0)
			{
				_state = States::Open;
				_player->backToLife();
				updatePosition();

				for (BasePlaneObject* obj : _objects)
				{
					obj->Reset();
				}
			}
			break;

		case States::Open:
			_holeRadius += RECT_SPEED * elapsedTime;
			if (_player->position.x - position.x < _holeRadius)
			{
				_deathAniWait = false;
				_state = States::Play;
			}
			break;

		case States::Fall:
			_player->position.y += CC_FALLDEATH_SPEED * elapsedTime;
			_player->Logic(0); // update position of animation
			if (_player->position.y - position.y > WindowManager::getSize().height)
			{
				_player->loseLife();
				_state = States::Close;
				_deathAniWait = true;
				auto wndSz = WindowManager::getSize();
				_holeRadius = max(wndSz.width, wndSz.height) / 2;
			}
			break;
		}
		return;
	}

	if (_player->isFinishDeathAnimation() && _player->hasLives() && _state == States::Play)
	{
		if (_player->isSpikeDeath())
		{
			_state = States::Close;
			auto wndSz = WindowManager::getSize();
			_holeRadius = max(wndSz.width, wndSz.height) / 2;
		}
		else //if (_player->isFallDeath())
		{
			_state = States::Fall;
		}

		_deathAniWait = true;
		return;
	}

	if (!_player->isInDeathAnimation())
	{
		_physicsManager->checkCollides(_player, [&] { _player->loseLife(); });
	}

	updatePosition();

	if (_needSort)
	{
		sort(_objects.begin(), _objects.end(), [](BasePlaneObject* a, BasePlaneObject* b) { return a->ZCoord < b->ZCoord; });
	}

	_player->Logic(elapsedTime);

	BasePlaneObject* obj;

	for (size_t i = 0; i < _objects.size(); i++)
	{
		obj = _objects[i];

		if (obj == _player) continue;

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
		const Rectangle2D rc1(position.x, position.y, _player->position.x - _holeRadius, hp);
		const Rectangle2D rc2(rc1.right, position.y, wp, _player->position.y - _holeRadius);
		const Rectangle2D rc3(rc1.right, _player->position.y + _holeRadius, wp, hp);
		const Rectangle2D rc4(_player->position.x + _holeRadius, rc2.bottom, wp, rc3.top);

		WindowManager::fillRect(rc1, ColorF::Black);
		WindowManager::fillRect(rc2, ColorF::Black);
		WindowManager::fillRect(rc3, ColorF::Black);
		WindowManager::fillRect(rc4, ColorF::Black);

		// TODO: draw circle, not 4 rectangles
		//WindowManager::drawCircle(_player->position, _holeRadius, ColorF::Black, 20);
	}
}

void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	// TODO? if (obj == nullptr) return; 
	_objects.push_back(obj);
	_needSort = true;
	if (isProjectile(obj)) _projectiles.push_back((Projectile*)obj);
	else if (isbaseinstance<BaseEnemy>(obj)) _enemies.push_back((BaseEnemy*)obj); // TODO: make sure we need this
}


#define ADD_ENEMY(p) { BaseEnemy* enemy=p; _objects.push_back(enemy); _enemies.push_back(enemy); }

void ActionPlane::addObject(const WwdObject& obj, int levelNumber, WapWorld* wwd)
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
		_objects.push_back(DBG_NEW GlobalAmbientSound(obj, _player));
	}
	else if (obj.logic == "AmbientSound" || obj.logic == "SpotAmbientSound")
	{
		_objects.push_back(DBG_NEW AmbientSound(obj, _player));
	}
	else if (contains(obj.logic, "SoundTrigger"))
	{
		_objects.push_back(DBG_NEW SoundTrigger(obj, _player));
	}
	else if (obj.logic == "BehindCrate" || obj.logic == "FrontCrate")
	{
		_objects.push_back(DBG_NEW Crate(obj, _player));
	}
	else if (obj.logic == "FrontStackedCrates" || obj.logic == "BackStackedCrates")
	{
		_objects.push_back(DBG_NEW StackedCrates(obj, _player));
	}
	else if (obj.logic == "FrontStatue" || obj.logic == "BehindStatue")
	{
		_objects.push_back(DBG_NEW Statue(obj, _player));
	}
	else 
		if (obj.logic == "PowderKeg")
	{
		PowderKeg* p = DBG_NEW PowderKeg(obj, _player);
		_objects.push_back(p); _powderKegs.push_back(p);
	}
	else
#endif
	if (endsWith(obj.logic, "Elevator"))
	{
		_objects.push_back(Elevator::create(obj, _player, levelNumber));
	}
	else if (endsWith(obj.logic, "Checkpoint"))
	{
		_objects.push_back(DBG_NEW Checkpoint(obj, _player));
	}
	else if (startsWith(obj.logic, "TogglePeg"))
	{
		_objects.push_back(DBG_NEW TogglePeg(obj, _player));
	}
	else if (obj.logic == "StartSteppingStone")
	{
		_objects.push_back(DBG_NEW StartSteppingStone(obj, _player));
	}
	else if (startsWith(obj.logic, "SteppingStone"))
	{
		_objects.push_back(DBG_NEW SteppingStone(obj, _player));
	}
	else if (obj.logic == "CrumblingPeg")
	{
		_objects.push_back(DBG_NEW CrumblingPeg(obj, _player));
	}
	else if (obj.logic == "BreakPlank")
	{
		int32_t topOffset = 0;
		if (levelNumber == 5) topOffset = wwd->tilesDescription[509].rect.top;
		else if (levelNumber == 11) topOffset = wwd->tilesDescription[39].rect.top;

		for (int32_t i = 0; i < obj.width; i++)
		{
			_objects.push_back(DBG_NEW BreakPlank(obj, _player, (float)topOffset));
			//obj.x += TILE_SIZE;
			myMemCpy(obj.x, obj.x + TILE_SIZE);
		}
	}
	else if (obj.logic == "TreasurePowerup" || obj.logic == "GlitterlessPowerup"
		|| obj.logic == "SpecialPowerup" || obj.logic == "AmmoPowerup"
		|| obj.logic == "BossWarp" || obj.logic == "HealthPowerup"
		|| obj.logic == "EndOfLevelPowerup" || obj.logic == "MagicPowerup"
		/*|| obj.logic == "CursePowerup"*/)
	{
		_objects.push_back(Item::getItem(obj, _player));
	}
	else if (obj.logic == "AniRope")
	{
		_objects.push_back(DBG_NEW Rope(obj, _player));
	}
	else if (obj.logic == "SpringBoard" || obj.logic == "WaterRock")
	{
		_objects.push_back(DBG_NEW SpringBoard(obj, _player));
	}
	else if (obj.logic == "GroundBlower")
	{
		_objects.push_back(DBG_NEW GroundBlower(obj, _player));
	}
	else if (obj.logic == "ConveyorBelt")
	{
		_objects.push_back(DBG_NEW ConveyorBelt(obj, _player));
	}
#ifdef USE_ENEMIES
	else if (obj.logic == "Laser")
	{
		Laser* l = DBG_NEW Laser(obj, _player);
		_lasers.push_back(l); _objects.push_back(l);
	}
	else if (obj.logic == "GooVent")
	{
		GooVent* g = DBG_NEW GooVent(obj, _player);
		_objects.push_back(g); _gooVents.push_back(g);
	}
	else if (obj.logic == "TowerCannonLeft" || obj.logic == "TowerCannonRight")
	{
		_objects.push_back(DBG_NEW TowerCannon(obj, _player));
	}
	else if (obj.logic == "Officer")
	{
		ADD_ENEMY(DBG_NEW Officer(obj, _player));
	}
	else if (obj.logic == "Soldier")
	{
		ADD_ENEMY(DBG_NEW Soldier(obj, _player));
	}
	else if (obj.logic == "Rat")
	{
		ADD_ENEMY(DBG_NEW Rat(obj, _player));
	}
	else if (obj.logic == "PunkRat")
	{
		ADD_ENEMY(DBG_NEW PunkRat(obj, _player));
	}
	else if (obj.logic == "RobberThief")
	{
		ADD_ENEMY(DBG_NEW RobberThief(obj, _player));
	}
	else if (obj.logic == "CutThroat")
	{
		ADD_ENEMY(DBG_NEW CutThroat(obj, _player));
	}
	else if (startsWith(obj.logic, "FloorSpike"))
	{
		FloorSpike* s = DBG_NEW FloorSpike(obj, _player);
		_objects.push_back(s); _floorSpikes.push_back(s);
	}
	else if (obj.logic == "Seagull")
	{
		ADD_ENEMY(DBG_NEW Seagull(obj, _player));
	}
	else if (obj.logic == "TownGuard1" || obj.logic == "TownGuard2")
	{
		ADD_ENEMY(DBG_NEW TownGuard(obj, _player));
	}
	else if (obj.logic == "RedTailPirate")
	{
		ADD_ENEMY(DBG_NEW RedTailPirate(obj, _player));
	}
	else if (obj.logic == "BearSailor")
	{
		ADD_ENEMY(DBG_NEW BearSailor(obj, _player));
	}
	else if (obj.logic == "CrazyHook")
	{
		ADD_ENEMY(DBG_NEW CrazyHook(obj, _player));
	}
	else if (obj.logic == "HermitCrab")
	{
		ADD_ENEMY(DBG_NEW HermitCrab(obj, _player));
	}
	else if (obj.logic == "CrabNest")
	{
		_objects.push_back(DBG_NEW CrabNest(obj, _player));
	}
	else if (startsWith(obj.logic, "SawBlade"))
	{
		SawBlade* s = DBG_NEW SawBlade(obj, _player);
		_objects.push_back(s); _floorSpikes.push_back(s);
	}
	else if (obj.logic == "TProjectile")
	{
		_objects.push_back(DBG_NEW TProjectilesShooter(obj, _player));
	}
	else if (obj.logic == "SkullCannon")
	{
		_objects.push_back(DBG_NEW SkullCannon(obj, _player));
	}
	else if (obj.logic == "PegLeg")
	{
		ADD_ENEMY(DBG_NEW PegLeg(obj, _player));
	}
	else if (obj.logic == "Mercat")
	{
		ADD_ENEMY(DBG_NEW Mercat(obj, _player));
	}
	else if (obj.logic == "Siren")
	{
		ADD_ENEMY(DBG_NEW Siren(obj, _player));
	}
	else if (obj.logic == "Fish")
	{
		ADD_ENEMY(DBG_NEW Fish(obj, _player));
	}
#endif
	else if (obj.logic == "Chameleon")
	{
		ADD_ENEMY(DBG_NEW Chameleon(obj, _player));
	}
	else if (obj.logic == "Raux")
	{
		ADD_ENEMY(DBG_NEW LeRauxe(obj, _player));
	}
	else if (obj.logic == "Katherine")
	{
		ADD_ENEMY(DBG_NEW Katherine(obj, _player));
	}
	else if (obj.logic == "Wolvington")
	{
		ADD_ENEMY(DBG_NEW Wolvington(obj, _player));
	}
	else if (obj.logic == "Gabriel")
	{
		ADD_ENEMY(DBG_NEW Gabriel(obj, _player));
	}
	else if (obj.logic == "GabrielCannon")
	{
		_objects.push_back(DBG_NEW GabrielCannon(obj, _player));
	}
	else if (obj.logic == "CannonSwitch")
	{
		_objects.push_back(DBG_NEW GabrielCannonSwitch(obj, _player));
	}
	else if (obj.logic == "CannonButton")
	{
		_objects.push_back(DBG_NEW GabrielCannonButton(obj, _player));
	}
	else if (obj.logic == "Marrow")
	{
		ADD_ENEMY(DBG_NEW Marrow(obj, _player));
	}
	else if (obj.logic == "Parrot")
	{
		ADD_ENEMY(DBG_NEW MarrowParrot(obj, _player));
	}
	else if (obj.logic == "MarrowFloor")
	{
		_objects.push_back(DBG_NEW MarrowFloor(obj, _player));
	}

	//	throw Exception("TODO: logic=" + obj.logic);
}

void ActionPlane::updatePosition()
{
	// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
	const D2D1_SIZE_F wndSize = WindowManager::getSize();
	position.x = _player->position.x - wndSize.width / 2.0f;
	position.y = _player->position.y - wndSize.height / 2.0f;

	if (position.x < MIN_OFFSET_X) position.x = MIN_OFFSET_X;
	if (position.x > MAX_OFFSET_X) position.x = MAX_OFFSET_X;
	if (position.y < MIN_OFFSET_Y) position.y = MIN_OFFSET_Y;
	if (position.y > MAX_OFFSET_Y) position.y = MAX_OFFSET_Y;
}
