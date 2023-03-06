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
#include "Objects/Raux.h"
#include "Objects/SoundTrigger.h"
#include "Objects/Pegs.h"
#include "Objects/DoNothing.h"
#include "Objects/Cannon.h"


#define EMPTY_TILE -1

#define _checkCollides_define1(x) if (cumulatedCollision.x == 0) cumulatedCollision.x += collisions[collisionsNumber].x;
#define _checkCollides_define2(x) if (cumulatedCollision.x != 0) for (cumulatedCollision.x = 0, i = 0; i < collisionsNumber; cumulatedCollision.x = fmax(cumulatedCollision.x, collisions[i++].x));

#define MIN_OFFSET_X 0
#define MIN_OFFSET_Y 0
#define MAX_OFFSET_X (_planeSize.width - wndSize.width)
#define MAX_OFFSET_Y (_planeSize.height - wndSize.height)


#define eraseByValue(vec, val) vec.erase(find(vec.begin(), vec.end(), val))


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


/*inline bool isEnemy(BasePlaneObject* obj)
{
	return isbaseinstance<BaseEnemy>(obj);
}*/


vector<BasePlaneObject*> ActionPlane::_objects;
vector<PowderKeg*> ActionPlane::_powderKegs;
vector<BaseEnemy*> ActionPlane::_enemies;
vector<Projectile*> ActionPlane::_projectiles;
bool ActionPlane::_needSort;


ActionPlane::ActionPlane(const WwdPlane& plane, shared_ptr<WapWorld> wwd)
	: LevelPlane(plane), _wwd(wwd),
	_planeSize({ (float)plane.tilePixelWidth * plane.tilesOnAxisX, (float)plane.tilePixelHeight * plane.tilesOnAxisY })
{
	_objects.clear(); // because it static member and we don't want recycle objects...
	_powderKegs.clear();
	_enemies.clear();
	_projectiles.clear();


	WwdObject playerData;
	playerData.x = _wwd->startX;
	playerData.y = _wwd->startY;
	playerData.z = 4000;
	_player = DBG_NEW Player(playerData, _planeSize);

	for (const WwdObject& obj : plane.objects)
	{
		try
		{
			addObject(obj);
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

	bool callPlayerLogic = true;

	if (_player->isFinishDeathAnimation())
	{
		if (_player->hasLives())
		{
			_player->backToLife();
			callPlayerLogic = false;

			for (BasePlaneObject* obj : _objects)
			{
				obj->Reset();
			}
		}
	}

	if (callPlayerLogic)
	{
		if (!_player->isInDeathAnimation())
		{
			checkCollides(_player, [&] { _player->loseLife(); });
		}
	}

	// change the display offset according to player position, but clamp it to the limits (the player should to be in screen center)
	const D2D1_SIZE_F wndSize = WindowManager::getSize();
	const D2D1_RECT_F playerRc = _player->GetRect();
	position.x = _player->position.x - wndSize.width / 2.0f;
	position.y = _player->position.y - wndSize.height / 2.0f;
	if (position.x < MIN_OFFSET_X) position.x = MIN_OFFSET_X;
	if (position.x > MAX_OFFSET_X) position.x = MAX_OFFSET_X;
	if (position.y < MIN_OFFSET_Y) position.y = MIN_OFFSET_Y;
	if (position.y > MAX_OFFSET_Y) position.y = MAX_OFFSET_Y;

	AssetsManager::callLogics(elapsedTime);

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

		if (isbaseinstance<BaseEnemy>(obj) || isProjectile(obj) || isinstance<ClawDynamite>(obj) || isinstance<PowderKeg>(obj))
		{
			checkCollides((BaseDynamicPlaneObject*)obj, [obj] { obj->removeObject = true; });
		}
		else if (isinstance<Item>(obj))
		{
			Item* item = (Item*)obj;
			if (item->_speed.y != 0)
			{
				checkCollides(item, [obj] { obj->removeObject = true; });
			}
		}
		else if (isinstance<StackedCrates>(obj))
		{
			vector<Item*> items = ((StackedCrates*)obj)->getItems();
			_objects.insert(_objects.end(), items.begin(), items.end());
		}
		else if (isinstance<Crate>(obj))
		{
			Crate* crate = (Crate*)obj;
			if (crate->isCracking())
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
}
void ActionPlane::Draw()
{
	LevelPlane::Draw();

	for (BasePlaneObject* i : _objects)
	{
		i->Draw();
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

Player* ActionPlane::getPlayer() const
{
	return _player;
}

void ActionPlane::addPlaneObject(BasePlaneObject* obj)
{
	// TODO? if (obj == nullptr) return; 
	_objects.push_back(obj);
	_needSort = true;
	if (isProjectile(obj)) _projectiles.push_back((Projectile*)obj);
}
const vector<PowderKeg*>& ActionPlane::getPowderKegs() { return _powderKegs; }
const vector<BaseEnemy*>& ActionPlane::getEnemies() { return _enemies; }
const vector<Projectile*>& ActionPlane::getProjectiles() { return _projectiles; }

void ActionPlane::checkCollides(BaseDynamicPlaneObject* obj, function<void(void)> whenTouchDeath)
{
	static const float N = 2.5f; // this number indicate how many tile we will check from every side
	const D2D1_RECT_F objRc = obj->GetRect();
	const uint32_t MinXPos = (uint32_t)(obj->position.x / _plane.tilePixelWidth - N);
	const uint32_t MaxXPos = (uint32_t)(obj->position.x / _plane.tilePixelWidth + N);
	const uint32_t MinYPos = (uint32_t)(obj->position.y / _plane.tilePixelHeight - N);
	const uint32_t MaxYPos = (uint32_t)(obj->position.y / _plane.tilePixelHeight + N);

	D2D1_RECT_F collisions[9] = {}, cumulatedCollision = {}, tileRc = {}, collisionRc = {};
	uint32_t i, j, collisionsNumber = 0;


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
	auto _onGround = [&]() {
		collisionRc = CollisionDistances::getCollision(objRc, tileRc);
		if (CollisionDistances::isBottomCollision(collisionRc))
		{
			_addCollision();
		}
	};
	auto _onLadder = [&]() {
		if (CollisionDistances::isCollision(objRc, tileRc))
		{
			const bool isPlayer = isinstance<Player>(obj);

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
					isOnLadderTop = !((Player*)obj)->isClimbing() && isOnLadderTop;
				}
			}

			if (isOnLadderTop)
			{
				collisionRc = CollisionDistances::getCollision(objRc, tileRc);
				if (CollisionDistances::isBottomCollision(collisionRc))
				{
					_addCollision();
				}
			}

			if (isPlayer) // let Captain Claw climb
			{
				Player* player = (Player*)obj;
				//TODO: can't climb up at ladder-top
				//TODO: leave ladder if climb down and touch ground/solid

				player->isCollideWithLadder = true;
				if (player->isClimbing())
				{
					// set the player position on the ladder easily for the user
					player->position.x = (tileRc.left + tileRc.right) / 2;
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

				case WwdTileDescription::TileAttribute_Climb:
					_onLadder();
					break;

				case WwdTileDescription::TileAttribute_Death:
					_onDeath();
					break;
					// if its an item or an enemy it will removed, but if its a player he will lose life so we don't need continue check
				}
			}	break;

			case WwdTileDescription::TileType_Double: { // TODO: improve this part
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
				}

				if (tileDesc.outsideAttrib == WwdTileDescription::TileAttribute_Solid) // this case is the yellow rectangles
				{
					D2D1_RECT_F spaceRc = tileRc, rc1 = {}, rc2 = {};

					tileRc.left = (float)(j * _plane.tilePixelWidth);
					tileRc.top = (float)(i * _plane.tilePixelHeight);
					tileRc.right = tileRc.left + _plane.tilePixelWidth;
					tileRc.bottom = tileRc.top + _plane.tilePixelHeight;


					if (tileDesc.rect.right == tileDesc.width - 1 && tileDesc.rect.top == 0)
					{
						rc1 = RectF(tileRc.left, tileRc.top, spaceRc.left, tileRc.bottom);
						rc2 = RectF(spaceRc.left, spaceRc.bottom, spaceRc.right, tileRc.bottom);
					}
					else if (tileDesc.rect.left == 0 && tileDesc.rect.top == 0 && tileDesc.rect.right == tileDesc.width - 1)
					{
						rc1 = RectF(spaceRc.right, tileRc.top, tileRc.right, tileRc.bottom);
						rc2 = RectF(tileRc.left, spaceRc.bottom, tileRc.right, tileRc.bottom);
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

#define ADD_ENEMY(p) BaseEnemy* enemy=p; _objects.push_back(enemy); _enemies.push_back(enemy);

void ActionPlane::addObject(const WwdObject& obj)
{
#ifndef LOW_DETAILS
	if (obj.logic == "FrontCandy" || obj.logic == "BehindCandy" ||
		obj.logic == "BehindAniCandy" || obj.logic == "FrontAniCandy" ||
		obj.logic == "DoNothing" || obj.logic == "AniCycle" || obj.logic == "GooCoverup")
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
	else
#endif
		if (obj.logic == "PowderKeg")
	{
		PowderKeg* p = DBG_NEW PowderKeg(obj, _player);
		_objects.push_back(p); _powderKegs.push_back(p);
	}
	else
	if (obj.logic == "Elevator"
		|| obj.logic == "TriggerElevator"|| obj.logic == "OneWayTriggerElevator"
		|| obj.logic == "StartElevator" || obj.logic == "OneWayStartElevator")
	{
		_objects.push_back(DBG_NEW Elevator(obj, _player));
	}
	else if (obj.logic == "Checkpoint")
	{
		_objects.push_back(DBG_NEW Checkpoint(obj, _player));
	}
	else if (obj.logic == "FirstSuperCheckpoint" || obj.logic == "SecondSuperCheckpoint")
	{
		_objects.push_back(DBG_NEW SuperCheckpoint(obj, _player));
	}
	else if (obj.logic == "TogglePeg" || obj.logic == "TogglePeg2" || obj.logic == "TogglePeg3" || obj.logic == "TogglePeg4")
	{
		_objects.push_back(DBG_NEW TogglePeg(obj, _player));
	}
	else if (obj.logic == "CrumblingPeg")
	{
		_objects.push_back(DBG_NEW CrumblingPeg(obj, _player));
	}
	else if (obj.logic == "TreasurePowerup" || obj.logic == "GlitterlessPowerup"
		|| obj.logic == "SpecialPowerup" || obj.logic == "AmmoPowerup"
		|| obj.logic == "BossWarp" || obj.logic == "HealthPowerup" /*|| obj.logic == "CursePowerup"*/)
	{
		_objects.push_back(Item::getItem(obj, _player));
	}
#if 01
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
#endif
	else if (obj.logic == "Raux")
	{
		ADD_ENEMY(DBG_NEW Raux(obj, _player));
	}

	//	throw Exception("TODO: logic=" + obj.logic);
}
