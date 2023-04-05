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


#define EMPTY_TILE -1

#define RECT_SPEED			0.5f
#define CC_FALLDEATH_SPEED	0.7f

#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define MIN_OFFSET_X 0
#define MIN_OFFSET_Y 0
#define MAX_OFFSET_X (_planeSize.width - wndSize.width)
#define MAX_OFFSET_Y (_planeSize.height - wndSize.height)


#define eraseByValue(vec, val) vec.erase(find(vec.begin(), vec.end(), val))

// TODO: make sure we impleted all the logics
//#define SAVE_LOGICS "c:/users/ariel/desktop/remain- level7 logics.txt"
//#define DRAW_RECTANGLES
//#define USE_ENEMIES
//#undef LOW_DETAILS


class SimpleObject : public BasePlaneObject
{
public:
	SimpleObject(WwdObject obj)
		: BasePlaneObject(obj)
	{
	}
	void Logic(uint32_t elapsedTime) override {}
	void Draw() override {
		WindowManager::drawRect(RectF(position.x - 32, position.y - 32, position.x + 32, position.y + 32), ColorF::Green);
	}
	D2D1_RECT_F GetRect() override { return RectF(); }
};


vector<BasePlaneObject*> ActionPlane::_objects;
vector<PowderKeg*> ActionPlane::_powderKegs;
vector<BaseEnemy*> ActionPlane::_enemies;
vector<Projectile*> ActionPlane::_projectiles;
vector<FloorSpike*> ActionPlane::_floorSpikes;
vector<GooVent*> ActionPlane::_gooVents;
vector<Laser*> ActionPlane::_lasers;
bool ActionPlane::_needSort;


ActionPlane::ActionPlane(const WwdPlane& plane, shared_ptr<WapWorld> wwd, int8_t levelNumber)
	: LevelPlane(plane), _wwd(wwd), _state(States::Play), _deathAniWait(false),
	_planeSize({ (float)plane.tilePixelWidth * plane.tilesOnAxisX, (float)plane.tilePixelHeight * plane.tilesOnAxisY })
{
	_objects.clear(); // because it static member and we don't want recycle objects...
	_powderKegs.clear();
	_enemies.clear();
	_projectiles.clear();
	_floorSpikes.clear();
	_gooVents.clear();
	_lasers.clear();


	WwdObject playerData;
	playerData.x = _wwd->startX;
	playerData.y = _wwd->startY;
	playerData.z = 4000;
	_player = DBG_NEW Player(playerData, _planeSize);

#ifdef SAVE_LOGICS
	set<string> allLogics;
#endif

	for (const WwdObject& obj : plane.objects)
	{
#ifdef SAVE_LOGICS
		allLogics.insert(obj.logic);
#endif

		try
		{
			addObject(obj, levelNumber);
		}
		catch (Exception& ex)
		{
			// an exception may be thrown if the path is invalid or if we don't impleted the logic of object
#ifdef _DEBUG
			//OutputDebugStringA((__FUNCTION__ ": " + ex.what() + '\n').c_str());
			cout << __FUNCTION__ ": " << ex.what() << endl;
#endif
		}
	}
#ifdef SAVE_LOGICS
	ofstream of(SAVE_LOGICS);
	for (auto& i : allLogics) of << i << endl;
#endif

	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level, false);

	_objects.push_back(_player);
	_needSort = true;
}
ActionPlane::~ActionPlane()
{
	for (BasePlaneObject* i : _objects)
	{
		delete i;
	}
}

void ActionPlane::Logic(uint32_t elapsedTime)
{
	if (_player->isFinishLevel()) return;

	auto _update_position = [&] {
		// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
		const D2D1_SIZE_F wndSize = WindowManager::getSize();
		const D2D1_RECT_F playerRc = _player->GetRect();
		position.x = _player->position.x - wndSize.width / 2.0f;
		position.y = _player->position.y - wndSize.height / 2.0f;

		if (position.x < MIN_OFFSET_X) position.x = MIN_OFFSET_X;
		if (position.x > MAX_OFFSET_X) position.x = MAX_OFFSET_X;
		if (position.y < MIN_OFFSET_Y) position.y = MIN_OFFSET_Y;
		if (position.y > MAX_OFFSET_Y) position.y = MAX_OFFSET_Y;
	};

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
				_update_position();

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
		checkCollides(_player, [&] { _player->loseLife(); });
	}
	
	_update_position();

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

		if (isbaseinstance<BaseEnemy>(obj) || isProjectile(obj)
			|| isinstance<ClawDynamite>(obj) || isinstance<PowderKeg>(obj)
			|| (isinstance<Item>(obj) && ((BaseDynamicPlaneObject*)obj)->getSpeedY() != 0))
		{
			checkCollides((BaseDynamicPlaneObject*)obj, [obj] { obj->removeObject = true; });
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
			{
				vector<Item*> items = ((BaseEnemy*)obj)->getItems();
				_objects.insert(_objects.end(), items.begin(), items.end());
				eraseByValue(_enemies, obj);
			}
			else if (isProjectile(obj))
			{
				eraseByValue(_projectiles, obj);
			}
			else if (isinstance<PowderKeg>(obj))
			{
				eraseByValue(_powderKegs, obj);
			}

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

	for (BasePlaneObject* i : _objects)
	{
		i->Draw();
	}

	if (_state == States::Close || _state == States::Open)
	{
		const D2D1_SIZE_F wndSz = WindowManager::getSize();
		const float wp = wndSz.width + position.x, hp = wndSz.height + position.y;
		const D2D1_RECT_F rc1{ position.x,position.y,_player->position.x - _holeRadius,hp };
		const D2D1_RECT_F rc2{ rc1.right,position.y,wp,_player->position.y - _holeRadius };
		const D2D1_RECT_F rc3{ rc1.right,_player->position.y + _holeRadius,wp,hp };
		const D2D1_RECT_F rc4{ _player->position.x + _holeRadius,rc2.bottom,wp,rc3.top };

		WindowManager::fillRect(rc1, ColorF::Black);
		WindowManager::fillRect(rc2, ColorF::Black);
		WindowManager::fillRect(rc3, ColorF::Black);
		WindowManager::fillRect(rc4, ColorF::Black);

		// TODO: draw circle, not 4 rectangles
		//WindowManager::drawCircle(_player->position, _holeRadius, ColorF::Black, 20);
	}
	
#ifdef DRAW_RECTANGLES
	// draw rectangles around tiles limits

	WwdTileDescription tileDesc;
	ColorF color(0);
	D2D1_RECT_F tileRc = {};
	const D2D1_SIZE_F wndSz = WindowManager::getSize();
	const uint32_t MinXPos = (uint32_t)(position.x / _plane.tilePixelWidth);
	const uint32_t MinYPos = (uint32_t)(position.y / _plane.tilePixelHeight);
	const uint32_t MaxXPos = (uint32_t)(MinXPos + wndSz.width / _plane.tilePixelWidth + 2);
	const uint32_t MaxYPos = (uint32_t)(MinYPos + wndSz.height / _plane.tilePixelHeight + 2);
	uint32_t i, j;
	int32_t tileId;

	for (i = MinYPos; i < MaxYPos && i < _plane.tilesOnAxisY; i++)
	{
		for (j = MinXPos; j < MaxXPos && j < _plane.tilesOnAxisX; j++)
		{
			tileRc.left = (float)(j * _plane.tilePixelWidth);
			tileRc.top = (float)(i * _plane.tilePixelHeight);
			tileRc.right = tileRc.left + _plane.tilePixelWidth;
			tileRc.bottom = tileRc.top + _plane.tilePixelHeight;

			WindowManager::drawRect(tileRc, ColorF::Red);

			tileId = _plane.tiles[i][j];

			if (tileId != EMPTY_TILE)
			{
				tileDesc = _wwd->tilesDescription[tileId];

				if (tileDesc.type == WwdTileDescription::TileType_Double)
				{
					tileRc.left += tileDesc.rect.left;
					tileRc.top += tileDesc.rect.top;
					tileRc.right = tileRc.right + tileDesc.rect.right - tileDesc.width;
					tileRc.bottom = tileRc.bottom + tileDesc.rect.bottom - tileDesc.height;

					if (tileDesc.insideAttrib == WwdTileDescription::TileAttribute_Solid)
					{
						color = ColorF::Green;
					}
					else if (tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Solid)
					{
						color = ColorF::Yellow;
					}
					else if (tileDesc.insideAttrib == WwdTileDescription::TileAttribute_Ground)
					{
						color = ColorF::Magenta;
					}
					else if (tileDesc.insideAttrib == WwdTileDescription::TileAttribute_Death)
					{
						color = ColorF::Purple;
					}
					else
					{
						color.a = 0;
						tileRc = {};
					}
				}
				else if (tileDesc.type == WwdTileDescription::TileType_Single)
				{
					if (tileDesc.insideAttrib == WwdTileDescription::TileAttribute_Death)
					{
						color = ColorF::Purple;
					}
					else
					{
						color.a = 0;
						tileRc = {};
					}
				}
				
				WindowManager::drawRect(tileRc, color, 3);
			}
		}
	}
#endif
}

void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	// TODO? if (obj == nullptr) return; 
	_objects.push_back(obj);
	_needSort = true;
	if (isProjectile(obj)) _projectiles.push_back((Projectile*)obj);
	else if (isbaseinstance<BaseEnemy>(obj)) _enemies.push_back((BaseEnemy*)obj);
}

void ActionPlane::checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath)
{
	static const float N = 2.5f; // this number indicate how many tile we will check from every side
	const D2D1_RECT_F objRc = obj->GetRect();
	const uint32_t MinXPos = (uint32_t)(obj->position.x / _plane.tilePixelWidth - N);
	const uint32_t MaxXPos = (uint32_t)(obj->position.x / _plane.tilePixelWidth + N);
	const uint32_t MinYPos = (uint32_t)(obj->position.y / _plane.tilePixelHeight - N);
	const uint32_t MaxYPos = (uint32_t)(obj->position.y / _plane.tilePixelHeight + N);

	D2D1_RECT_F collisions[9] = {}, cumulatedCollision = {}, tileRc = {}, collisionRc = {};
	D2D1_RECT_F originalTileRc, rc1, rc2;
	uint32_t i, j, collisionsNumber = 0;

	const bool isPlayer = (obj == _player); //isinstance<Player>(obj);


	auto _addCollision = [&]() { // add `collisionRect` to the `cumulatedCollision`
		if (!CollisionDistances::isEmpty(collisionRc))
		{
			// add this collision to the list
			collisions[collisionsNumber] = collisionRc;
			CollisionDistances::keepSmallest(collisions[collisionsNumber]);
			// add the collision details to the cummulated collision
			_checkCollides_define1(top);
			_checkCollides_define1(bottom);
			_checkCollides_define1(left);
			_checkCollides_define1(right);
			collisionsNumber++;
		}
	};
	auto _onSolid = [&](D2D1_RECT_F tileRect) {
		collisionRc = CollisionDistances::getCollision(objRc, tileRect);
		_addCollision();
	};
	auto _onGround = [&]() { // same to `BasePlaneObject::tryCatchPlayer`
		collisionRc = CollisionDistances::getCollision(objRc, tileRc);
		D2D1_RECT_F smallest = CollisionDistances::getSmallest(collisionRc);
		if (smallest.bottom > 0 && (collisionRc.right > 0 || collisionRc.left > 0) && _player->isFalling())
		{
			_addCollision();
		}
	};
	auto _onLadder = [&]() {
		if (CollisionDistances::isCollision(objRc, tileRc))
		{
			bool isOnLadderTop = false;

			// check if object is at the top of the ladder, so it should stay here
			if (i > 0)
			{
				int32_t aboveTileId = _plane.tiles[size_t(i - 1)][j];
				isOnLadderTop = (aboveTileId == EMPTY_TILE || (
					_wwd->tilesDescription[aboveTileId].insideAttrib != WwdTileDescription::TileAttribute_Climb &&
					_wwd->tilesDescription[aboveTileId].outsideAttrib != WwdTileDescription::TileAttribute_Climb));

				if (isPlayer)
				{
					isOnLadderTop = !_player->isClimbing() && isOnLadderTop;
				}
			}

			if (isOnLadderTop)
			{
				_onGround(); // ladder top behaves like ground
			}

			if (isPlayer) // let Captain Claw climb
			{
				_player->setLadderFlags(isOnLadderTop);
				if (_player->isClimbing())
				{
					// set the player position on the ladder easily for the user
					_player->position.x = (tileRc.left + tileRc.right) / 2;
				}
			}
		}
	};
	auto _onDeath = [&]() {
		if (CollisionDistances::isCollision(objRc, tileRc))
		{
			whenTouchDeath();
		}
	};

	for (i = MinYPos; i < MaxYPos && i < _plane.tilesOnAxisY; i++)
	{
		for (j = MinXPos; j < MaxXPos && j < _plane.tilesOnAxisX; j++)
		{
			const WwdTileDescription& tileDesc = _wwd->tilesDescription[_plane.tiles[i][j]];

			tileRc.left = (float)(j * _plane.tilePixelWidth);
			tileRc.top = (float)(i * _plane.tilePixelHeight);
			tileRc.right = tileRc.left + _plane.tilePixelWidth;
			tileRc.bottom = tileRc.top + _plane.tilePixelHeight;

			switch (tileDesc.type)
			{
			case WwdTileDescription::TileType_Single: {
				switch (tileDesc.insideAttrib)
				{
				case WwdTileDescription::TileAttribute_Solid:
					_onSolid(tileRc);
					break;
					
				case WwdTileDescription::TileAttribute_Ground:
					_onGround();
					break;

				case WwdTileDescription::TileAttribute_Climb:
					_onLadder();
					break;

				case WwdTileDescription::TileAttribute_Death: // this case is the purple rectangles
					_onDeath();
					break;
				}
			}	break;

			case WwdTileDescription::TileType_Double: { // TODO: improve this part
				originalTileRc = tileRc;

				tileRc.left += tileDesc.rect.left;
				tileRc.top += tileDesc.rect.top;
				tileRc.right = tileRc.right + tileDesc.rect.right - tileDesc.width;
				tileRc.bottom = tileRc.bottom + tileDesc.rect.bottom - tileDesc.height;

				switch (tileDesc.insideAttrib)
				{
				case WwdTileDescription::TileAttribute_Solid: // this case is the green rectangles
					_onSolid(tileRc);
					break;
				case WwdTileDescription::TileAttribute_Ground: // this case is the magenta rectangles
					_onGround();
					break;
				case WwdTileDescription::TileAttribute_Climb:
					_onLadder();
					break;
				case WwdTileDescription::TileAttribute_Death: // also this case is the purple rectangles
					_onDeath();
					break;
				}

				if (tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Solid) // this case is the yellow rectangles
				{
					if (tileDesc.rect.right == tileDesc.width - 1 && tileDesc.rect.top == 0)
					{
						rc1 = { originalTileRc.left, originalTileRc.top, tileRc.left, originalTileRc.bottom };
						rc2 = { tileRc.left, tileRc.bottom, tileRc.right, originalTileRc.bottom };
					}
					else if (tileDesc.rect.left == 0 && tileDesc.rect.top == 0)
					{
						rc1 = { tileRc.right, originalTileRc.top, originalTileRc.right, originalTileRc.bottom };
						rc2 = { originalTileRc.left, tileRc.bottom, originalTileRc.right, originalTileRc.bottom };
					}
					else
					{
						continue;
						// TODO: handle the other case
					}

					_onSolid(rc1);
					_onSolid(rc2);
				}
			}	break;

			default: break;
			}
		}
	}

	// whichever side collides the most, that side is taken into consideration
	CollisionDistances::keepLargest(cumulatedCollision);
	_checkCollides_define2(top);
	_checkCollides_define2(bottom);
	_checkCollides_define2(left);
	_checkCollides_define2(right);

	if (cumulatedCollision.top > 0) obj->bounceTop();
	else if (cumulatedCollision.bottom > 0) obj->stopFalling(cumulatedCollision.bottom);
	else if (cumulatedCollision.left > 0) obj->stopMovingLeft(cumulatedCollision.left);
	else if (cumulatedCollision.right > 0) obj->stopMovingRight(cumulatedCollision.right);
}

#define ADD_ENEMY(p) { BaseEnemy* enemy=p; _objects.push_back(enemy); _enemies.push_back(enemy); }

void ActionPlane::addObject(const WwdObject& obj, int8_t levelNumber)
{
#ifndef LOW_DETAILS
	if (obj.logic == "FrontCandy" || obj.logic == "BehindCandy" ||
		obj.logic == "BehindAniCandy" || obj.logic == "FrontAniCandy" ||
		obj.logic == "DoNothing" || obj.logic == "DoNothingNormal" ||
		obj.logic == "AniCycle" || obj.logic == "GooCoverup")
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
	else if (obj.logic == "PowderKeg")
	{
		PowderKeg* p = DBG_NEW PowderKeg(obj, _player);
		_objects.push_back(p); _powderKegs.push_back(p);
	}
	else
#endif
	if (obj.logic == "Elevator"
		|| obj.logic == "TriggerElevator" || obj.logic == "OneWayTriggerElevator"
		|| obj.logic == "StartElevator" || obj.logic == "OneWayStartElevator")
	{
		_objects.push_back(DBG_NEW Elevator(obj, _player));
	}
	else if (obj.logic == "PathElevator")
	{
		_objects.push_back(DBG_NEW PathElevator(obj, _player));
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
		WwdRect rc = {};
		if (levelNumber == 5) rc = _wwd->tilesDescription[509].rect;
		else if (levelNumber == 11) rc = _wwd->tilesDescription[39].rect;

		for (int32_t i = 0; i < obj.width; i++)
		{
			_objects.push_back(DBG_NEW BreakPlank(obj, _player, rc));
			//obj.x += 64;
			myMemCpy(obj.x, obj.x + 64);
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
	else if (obj.logic == "SpringBoard")
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

	//	throw Exception("TODO: logic=" + obj.logic);
}
