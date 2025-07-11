#include "Player.h"
#include "../../GlobalObjects.h"
#include "../../CheatsManager.h"
#include "../Projectiles/SirenScream.h"
#include "../LavaMouth.h"
#include "../Rope.h"
#include "../ConveyorBelt.h"
#include "../Stalactite.h"
#include "../PowderKeg.h"
#include "../OneTimeAnimation.h"
#include "../../Enemies/BaseEnemy.h"


constexpr auto& cheats = GO::cheats;


// TODO: find perfect values (if they are still not perfect :D )

constexpr float SpeedX_SuperSpeed	= 0.416f;
constexpr float SpeedX_LiftPowderKeg= 0.144f;
constexpr float SpeedY_SuperClimb	= 0.412f;
constexpr float SpeedY_RegularJump	= 0.800f;
constexpr float SpeedY_SuperJump	= 0.920f;
constexpr float SpeedY_MAX = MAX_Y_SPEED;

#ifdef _DEBUG // move fast in debug mode
constexpr float SpeedX_Normal = SpeedX_SuperSpeed;
constexpr float SpeedY_Climb = SpeedY_SuperClimb;
#else
constexpr float SpeedX_Normal = 0.288f;
constexpr float SpeedY_Climb = 0.288f;
#endif

constexpr int MAX_HEALTH_AMOUNT = 100;


#define Powerup_Catnip Powerup_Catnip_White // used for catnip powerup (for uniform use)


constexpr int MAX_DYNAMITE_SPEED_X = DEFAULT_PROJECTILE_SPEED * 8 / 7;
constexpr int MAX_DYNAMITE_SPEED_Y = DEFAULT_PROJECTILE_SPEED * 5 / 3;


Player::Player()
	: BaseCharacter({}), _finishLevel(false), _powerupSparkles(&_saveCurrRect), startPosition({})
	, _invincibilityComponent(this), _powerupTimer(bind(&Player::stopPowerup, this))
{
	_animations = AssetsManager::loadAnimationsFromDirectory("CLAW/ANIS", "", _invincibilityComponent.getColorsList());

	backToLife();

	// change keys for easy codeing
	renameKey(_animations, string("JUMPDYNAMITE"), string("JUMPPOSTDYNAMITE"));
	renameKey(_animations, string("DUCKEMPTYDYNAMITE"), string("DUCKEMPTYPOSTDYNAMITE"));
	renameKey(_animations, string("EMPTYDYNAMITE"), string("EMPTYPOSTDYNAMITE"));
	renameKey(_animations, string("EMPTYJUMPDYNAMITE"), string("JUMPEMPTYPOSTDYNAMITE"));
	renameKey(_animations, string("EMPTYJUMPPISTOL"), string("JUMPEMPTYPISTOL"));
	renameKey(_animations, string("EMPTYJUMPMAGIC"), string("JUMPEMPTYMAGIC"));

	AttackAnimations = { "SWIPE", "KICK", "UPPERCUT", "PUNCH", "DUCKSWIPE", "JUMPSWIPE" };
	NoLoopAnimations = { "LOOKUP", "SPIKEDEATH", "LIFT" };
	UninterruptibleAnimations = { "LIFT" ,"THROW" ,"FALLDEATH" ,"EMPTYPOSTDYNAMITE" ,"DUCKEMPTYPOSTDYNAMITE" ,"SPIKEDEATH" };

	_exclamationMark = AssetsManager::getCopyAnimationFromDirectory("GAME/IMAGES/EXCLAMATION");
	_animations["SIREN-FREEZE"] = AssetsManager::getAnimationFromPidImage("CLAW/IMAGES/100.PID");
	_animations["FLYING-CHEAT"] = AssetsManager::getAnimationFromPidImage("CLAW/IMAGES/401.PID");
	_animations["FLYING-CHEAT"]->upsideDown = true;

	_animations["SQUEEZED"] = make_shared<UIAnimation>(vector<UIAnimation::FrameData*>({
		DBG_NEW UIAnimation::FrameData(AssetsManager::loadImage("CLAW/IMAGES/450.PID")),
		DBG_NEW UIAnimation::FrameData(AssetsManager::loadImage("CLAW/IMAGES/451.PID")),
		DBG_NEW UIAnimation::FrameData(AssetsManager::loadImage("CLAW/IMAGES/452.PID"))
	}));
}
Player::~Player() {}

void Player::Logic(uint32_t elapsedTime)
{
	if (cheats->isFlying())
	{
		handleFlyingCheat(elapsedTime);
		return;
	}

	updateTimers(elapsedTime);

	if (isSqueezed())
	{
		_ani->mirrored = _isMirrored;
		callAnimationLogic(elapsedTime);
		return;
	}

	if (FindInArray(UninterruptibleAnimations, _aniName))
	{
		if (!_ani->isFinishAnimation() || _aniName == "FALLDEATH" || _aniName == "SPIKEDEATH")
		{
			callAnimationLogic(elapsedTime);
			calcRect();
			updateInvincibilityColorEffect();
			return;
		}
	}

	if (_freezeTime > 0)
	{
		_freezeTime -= elapsedTime;
		resetKeys();
	}

	float speedX = SpeedX_Normal, speedYClimb = SpeedY_Climb;

	if (_currPowerup == Item::Powerup_Catnip)
	{
		_powerupSparkles.Logic(elapsedTime);
		speedX = SpeedX_SuperSpeed;
		speedYClimb = SpeedY_SuperClimb;
	}

	if (handleDamage(elapsedTime))
		return;

	if (_raisedPowderKeg)
		speedX = SpeedX_LiftPowderKeg;


	const D2D1_POINT_2F prevPosition = position;

	const bool inAir = isFalling() || isJumping();
	const bool lookup = _upPressed && (isStanding() || _aniName == "LOOKUP");
	bool duck = _downPressed && (isStanding() || isDuck());

	if (lookup)
	{
		_isAttack = false;
		_useWeapon = false;
	}

	const bool canWalk = (!_useWeapon || (_useWeapon && inAir)) && (!_isAttack || (_isAttack && inAir)) && !duck && !lookup && !_altPressed && !_isOnLadder;
	const bool goLeft = _leftPressed && canWalk;
	const bool goRight = _rightPressed && canWalk;
	bool climbUp = false, climbDown = false;


	if (_leftPressed) _isMirrored = true;
	else if (_rightPressed || isClimbing()) _isMirrored = false;

	if (goLeft) speed.x = -speedX;
	else if (goRight) speed.x = speedX;
	else speed.x = 0;

	if (conveyorBelt)
		speed.x += conveyorBelt->getSpeed();

	if (inAir && _raisedPowderKeg)
	{
		_raisedPowderKeg->fall();
		_raisedPowderKeg = nullptr;
	}

	if (_isCollideWithLadder)
	{
		handleLadderCollision(elapsedTime);
		_isCollideWithLadder = false;
		climbUp = _upPressed && !_isOnLadderTop;
		climbDown = _downPressed;

		if (climbUp)
		{
			speed.y = -speedYClimb;
			_isOnLadder = true;
			elevator = nullptr;
			conveyorBelt = nullptr;
		}
		else if (climbDown)
		{
			if (isStanding()) // stand at ladder bottom
			{
				duck = true;
				_isOnLadder = false;
				climbDown = false;
			}
			else
			{
				speed.y = speedYClimb;
				_isOnLadder = true;
				elevator = nullptr;
				conveyorBelt = nullptr;
			}
		}
		else if (_isOnLadder)
		{
			speed.y = 0; // idle on ladder
		}
		else
		{
			if (!elevator)
			{
				speed.y += GRAVITY * elapsedTime;
			}
		}
	}
	else
	{
		_isOnLadder = false;
		if (!elevator && !rope)
		{
			speed.y += GRAVITY * elapsedTime;
		}
	}

	if (speed.y > SpeedY_MAX)
		speed.y = SpeedY_MAX;

	if (!isInDeathAnimation())
	{
		if (!lookup)
		{
			// update position based on speed, but make sure we don't go outside the level
			position.x += speed.x * elapsedTime;
			position.y += speed.y * elapsedTime;
		}

		setAnimation(elapsedTime, lookup, climbUp, climbDown,
			duck, inAir, goLeft, goRight, position.y - prevPosition.y > 5);

		if (_raisedPowderKeg)
		{
			_raisedPowderKeg->position.x = position.x;
			_raisedPowderKeg->position.y = position.y - 104;
		}
	}
	else
	{
		_ani->loopAni = false;
	}

	_ani->opacity = isGhost() || isInvincibility() ? 0.5f : 1.f;
	_ani->mirrored = _isMirrored && !_isOnLadder;
	callAnimationLogic(elapsedTime);

	calcRect();
	calcAttackRect();
}

void Player::callAnimationLogic(uint32_t elapsedTime)
{
	_ani->position = position;
	_ani->Logic(elapsedTime);
}
void Player::changeAnimation(const string& newAniName)
{
	_aniName = newAniName;
	_ani = _animations[_aniName];
	_ani->reset();
	_ani->mirrored = _isMirrored && !_isOnLadder;
	_ani->position = position;
}

void Player::setAnimation(uint32_t elapsedTime, bool lookup, bool climbUp, bool climbDown,
	bool duck, bool inAir,bool goLeft, bool goRight, bool isFall)
{
	// select animation

	const string prevAniName = _aniName;

	if (_freezeTime > 0)
	{
		_aniName = "SIREN-FREEZE";
	}
	else if (climbDown)
	{
		_aniName = "CLIMBDOWN";
	}
	else if (climbUp)
	{
		_aniName = "CLIMB";
	}
	else if (_isOnLadder)
	{
		_aniName = "CLIMBIDLE";
	}
	else if (lookup)
	{
		_aniName = "LOOKUP";
	}
	else if (rope)
	{
		_aniName = "SWING";
		_isMirrored = rope->isPassedHalf();
		speed = {};
	}
	else if (_isAttack)
	{
		if (!FindInArray(AttackAnimations, _aniName))
		{
			_aniName = "SWIPE";

			if (duck) _aniName = "DUCK" + _aniName;
			else if (inAir) _aniName = "JUMP" + _aniName;
			else
			{
				if (_currPowerup != Item::Powerup_FireSword &&
					_currPowerup != Item::Powerup_IceSword &&
					_currPowerup != Item::Powerup_LightningSword)
				{
					// get random attack, 40%-swipe, 20%-[kick|uppercut|punch]
					switch (getRandomInt(0, 5))
					{
					case 0: _aniName = "KICK"; break;
					case 1: _aniName = "UPPERCUT"; break;
					case 2: _aniName = "PUNCH"; break;
					default: break;
					}
				}
			}
		}
		else
		{
			_isAttack = !_ani->isFinishAnimation();
		}
	}
	else if (_altPressed && _inventory.hasDynamiteEquipped() && !inAir)
	{
		_aniName = "PREDYNAMITE";
		if (duck) _aniName = "DUCK" + _aniName;
	}
	else if (_useWeapon)
	{
		if (isWeaponAnimation() && !endsWith(_aniName, "PREDYNAMITE"))
		{
			_useWeapon = !_ani->isFinishAnimation();
		}
		else
		{
			switch (_inventory.getCurrentWeaponType())
			{
			case ClawProjectile::Types::Pistol: _aniName = "PISTOL"; break;
			case ClawProjectile::Types::Magic: _aniName = "MAGIC"; break;
			case ClawProjectile::Types::Dynamite: _aniName = "POSTDYNAMITE"; break;
			}

			if (_inventory.getCurrentWeaponAmount() > 0)
			{
				useWeapon(duck, inAir);
			}
			else
			{
				_aniName = "EMPTY" + _aniName;
			}

			if (duck) _aniName = "DUCK" + _aniName;
			else if (inAir) _aniName = "JUMP" + _aniName;
		}
	}
	else if (duck)
	{
		_aniName = "DUCK";
	}
	else if (isFall)
	{
		_aniName = "FALL";
	}
	else if (isJumping())
	{
		_aniName = "JUMP";
	}
	else if (goLeft || goRight)
	{
		_aniName = _raisedPowderKeg ? "LIFTWALK" : "WALK";
	}
	else if (_aniName != "JUMP")
	{
		// TODO better condition
		if ((_ani != _animations["THROW"] && _ani != _animations["LIFT"]) || _ani->isFinishAnimation())
		{
			if (_ani == _animations["THROW"] && _ani->isFinishAnimation())
				_aniName = "STAND";
			else
				_aniName = _raisedPowderKeg ? "LIFT2" : "STAND";
		}
	}
	

	if (prevAniName != _aniName)
	{
		_ani = _animations[_aniName];
		_ani->reset();
		_ani->updateFrames = true;

		_ani->loopAni = !FindInArray(NoLoopAnimations, _aniName)
			&& !FindInArray(AttackAnimations, _aniName) && !isWeaponAnimation() && !isDuck();

		if (endsWith(_aniName, "SWIPE") && _currPowerup != Item::None)
			shootSwordProjectile();
		else // invincibility mode does not involved projectiles
			updateInvincibilityColorEffect();
	}
}
void Player::updateTimers(uint32_t elapsedTime)
{
	if (_holdAltTime < 1000) _holdAltTime += elapsedTime; // the max time for holding is 1000 milliseconds
}
void Player::handleFlyingCheat(uint32_t elapsedTime)
{
	if (_upPressed) position.y -= elapsedTime;
	if (_downPressed) position.y += elapsedTime;
	if (_leftPressed) position.x -= elapsedTime;
	if (_rightPressed) position.x += elapsedTime;

	callAnimationLogic(elapsedTime);
}
bool Player::handleDamage(uint32_t elapsedTime)
{
	if (checkForHurts() || _health <= 0)
	{
		if (_health <= 0)
		{
			_aniName = "FALLDEATH";
		}
		else if (!isFreeze()) // when CC is freeze, he can't show take damage
		{
			_aniName = "DAMAGE" + to_string(getRandomInt(1, 2));
		}

		_ani = _animations[_aniName];
		_ani->reset();
		_ani->loopAni = false;
		_ani->mirrored = _isMirrored;
		_ani->position = position;
		_isOnLadder = false;
	}
	if (isTakeDamage() || _aniName == "FALLDEATH")
	{
		if (_raisedPowderKeg)
		{
			_raisedPowderKeg->fall();
			_raisedPowderKeg = nullptr;
		}

		_lastAttackRect = {};
		callAnimationLogic(elapsedTime);
		if (_ani->isFinishAnimation())
			_damageRestTimer.reset(250);
		calcRect();
		calcAttackRect();
		return true;
	}
	return false;
}
void Player::handleLadderCollision(uint32_t elapsedTime)
{
	// TODO implement
}

void Player::updateInvincibilityColorEffect()
{
	if (isInvincibility()) {
		_ani->setCurrentColor(_invincibilityComponent.getColor());
		_ani->opacity = 0.5;
	}
}
void Player::cancelInvincibilityEffect()
{
	// back to regular images
	for (auto& [k, a] : _animations) {
		a->setCurrentColor(nullptr);
		a->opacity = 1;
	}
}

void Player::onSpacePressed() {
	if (cheats->isFlying() || isSqueezed() || FindInArray(UninterruptibleAnimations, _aniName))
		return;

	if (speed.y <= 0.1f || _isOnLadder) // if player is standing (ignore very small speed changes) or climbing
	{
		jump(); // TODO: add case of small jump
		_isOnLadder = false;
	}
}
void Player::onZPressed() {
	if (_raisedPowderKeg)
	{
		// throw the powder keg
		_raisedPowderKeg->thrown(!_isMirrored);
		_raisedPowderKeg = nullptr;
		changeAnimation("THROW");
	}
	else
	{
		if (_ani == _animations["THROW"] && !_ani->isFinishAnimation())
			return;

		// try lift powder keg
		const vector<PowderKeg*>& powderKegs = GO::getActionPlanePowderKegs();
		auto keg = find_if(powderKegs.begin(), powderKegs.end(),
			[&](PowderKeg* p) {
				if (_saveCurrRect.intersects(p->GetRect()))
					return p->raise();
				return false;
			});
		if (keg != powderKegs.end()) {
			_raisedPowderKeg = *keg;
			changeAnimation("LIFT");
		}
	}

	// TODO: throw enemies
}

void Player::Draw()
{
	BaseCharacter::Draw();

	if (!_dialogTimer.isFinished())
	{
		_exclamationMark->position = { position.x, position.y - 64 };
		_exclamationMark->Draw();
	}

	if (_currPowerup == Item::Powerup_Catnip)
		_powerupSparkles.Draw();
}
Rectangle2D Player::GetRect() { return _saveCurrRect; }
pair<Rectangle2D, int> Player::GetAttackRect() { return _saveCurrAttackRect; }
void Player::calcRect()
{
	if (_isOnLadder)
		_saveCurrRect.left = 0;
	else
		_saveCurrRect.left = -7.f + 15 * _isMirrored;

	_saveCurrRect.right = _saveCurrRect.left + 44;

	if (isDuck())
	{
		if (_aniName == "DUCKEMPTYPOSTDYNAMITE")
		{
			Rectangle2D aniRc = _ani->GetRect();
			_saveCurrRect.top = position.y - aniRc.top;
			_saveCurrRect.bottom = aniRc.bottom - position.y;
		}
		else
		{
			_saveCurrRect.top = 30;
			_saveCurrRect.bottom = 90;
		}
	}
	else if (_aniName == "LIFT" || _aniName == "THROW" || _aniName == "EMPTYPOSTDYNAMITE" || startsWith(_aniName, "DAMAGE"))
	{
		// this cause that CC doesn't hover when he lifts or throws
		Rectangle2D aniRc = _ani->GetRect();
		_saveCurrRect.top = position.y - aniRc.top;
		_saveCurrRect.bottom = aniRc.bottom - position.y;
	}
	else
	{
		_saveCurrRect.top = 5;
		_saveCurrRect.bottom = 115;
	}

	_saveCurrRect = setRectByCenter(_saveCurrRect);
}
void Player::calcAttackRect()
{
	Rectangle2D rc;
	int damage = 0;

	if (_aniName == "SWIPE")
	{
		rc.top = 60;
		rc.bottom = 80;

		if (_isMirrored)
		{
			rc.left = -80;
			rc.right = 0;
		}
		else
		{
			rc.left = 50;
			rc.right = 130;
		}

		damage = 5;
	}
	else if (_aniName == "JUMPSWIPE")
	{
		rc.top = 35;
		rc.bottom = 55;

		if (_isMirrored)
		{
			rc.left = -70;
			rc.right = 0;
		}
		else
		{
			rc.left = 50;
			rc.right = 120;
		}

		damage = 5;
	}
	else if (_aniName == "KICK")
	{
		rc.top = 25;
		rc.bottom = 85;

		if (_isMirrored)
		{
			rc.left = -25;
			rc.right = 15;
		}
		else
		{
			rc.left = 35;
			rc.right = 75;
		}

		damage = 5;
	}
	else if (_aniName == "UPPERCUT")
	{
		rc.top = 5;
		rc.bottom = 65;

		if (_isMirrored)
		{
			rc.left = -30;
			rc.right = 10;
		}
		else
		{
			rc.left = 35;
			rc.right = 75;
		}

		damage = 5;
	}
	else if (_aniName == "PUNCH")
	{
		rc.top = 30;
		rc.bottom = 65;

		if (_isMirrored)
		{
			rc.left = -30;
			rc.right = 10;
		}
		else
		{
			rc.left = 35;
			rc.right = 75;
		}

		damage = 5;
	}
	else if (_aniName == "DUCKSWIPE")
	{
		rc.top = 45;
		rc.bottom = 60;

		if (_isMirrored)
		{
			rc.left = -65;
			rc.right = 0;
		}
		else
		{
			rc.left = 50;
			rc.right = 115;
		}

		damage = 2;
	}
	else { _saveCurrAttackRect = {}; return; }

	rc = setRectByCenter(rc, _saveCurrRect);

	_saveCurrAttackRect = { rc, (_currPowerup == Item::Powerup_Catnip
		|| _currPowerup == Item::Powerup_FireSword
		|| _currPowerup == Item::Powerup_IceSword
		|| _currPowerup == Item::Powerup_LightningSword
		|| cheats->isSuperStrong()) ? 1000 : damage }; // 1000 so CC defeats even the strongest bosses
}

void Player::useWeapon(bool duck, bool inAir)
{
	WwdObject obj;
	obj.x = (int32_t)(position.x + (_isMirrored ? _saveCurrRect.left - _saveCurrRect.right : _saveCurrRect.right - _saveCurrRect.left));
	obj.speedX = (_isMirrored ? -DEFAULT_PROJECTILE_SPEED : DEFAULT_PROJECTILE_SPEED);

	switch (_inventory.getCurrentWeaponType())
	{
	case ClawProjectile::Types::Pistol:
		obj.y = (int32_t)(position.y - (duck ? -12 : 16));
		obj.damage = 8;
		break;

	case ClawProjectile::Types::Magic:
		obj.y = (int32_t)(position.y + (duck ? 18 : -6));
		obj.damage = 25;
		break;

	case ClawProjectile::Types::Dynamite:
		// find and calculate perfect speed (don't forget `if (_holdAltTime < 1000) ...`

		obj.x = (int32_t)position.x;
		obj.y = (int32_t)position.y;
		obj.damage = 15;

		if (inAir)
		{
			obj.speedX = _isMirrored ? -MAX_DYNAMITE_SPEED_X : MAX_DYNAMITE_SPEED_X;
			obj.speedY = -MAX_DYNAMITE_SPEED_Y / 2;
		}
		else
		{
			int f = obj.speedX * _holdAltTime / 1000;
			if (_isMirrored)
				obj.speedX = max(f, -MAX_DYNAMITE_SPEED_X);
			else
				obj.speedX = min(f, MAX_DYNAMITE_SPEED_X);
			obj.speedY = min(obj.speedY, -MAX_DYNAMITE_SPEED_Y) * _holdAltTime / 1000;
		}

		break;

	default: throw Exception("no more weapons..."); // should never happen
	}

	_inventory.useWeapon();
	if (inAir) obj.y += 10;
	GO::addObjectToActionPlane(ClawProjectile::createNew(_inventory.getCurrentWeaponType(), obj));
}

void Player::stopFalling(float collisionSize)
{
	if (cheats->isFlying()) return;

	BaseCharacter::stopFalling(collisionSize);
	if (speed.x == 0 && !isDuck() && !isStanding() && !_isAttack && !isWeaponAnimation())
	{
		changeAnimation("STAND");
	}
	_isOnLadder = false;
}
void Player::stopMovingLeft(float collisionSize)
{
	if (isClimbing() || cheats->isFlying()) return;

	if (speed.x != 0)
	{
		speed.x = 0;
		position.x += collisionSize;
	}
	elevator = nullptr;
	conveyorBelt = nullptr;
}
void Player::stopMovingRight(float collisionSize)
{
	if (isClimbing() || cheats->isFlying()) return;

	if (speed.x != 0)
	{
		speed.x = 0;
		position.x -= collisionSize;
	}
	elevator = nullptr;
	conveyorBelt = nullptr;
}
void Player::whenTouchDeath()
{
	if (!cheats->isGodMode() && !cheats->isFlying())
		loseLife();
}
void Player::jump(float force)
{
	if (_aniName == "LOOKUP" || isDuck() || _raisedPowderKeg) return;

	elevator = nullptr;
	rope = nullptr;
	conveyorBelt = nullptr;

	speed.y = -force;
}
void Player::jump()
{
	if (_currPowerup == Item::Powerup_Catnip || cheats->isSuperJump())
		jump(SpeedY_SuperJump);
	else
		jump(SpeedY_RegularJump);
}

bool Player::collectItem(Item* item)
{
	Item::Type type = item->getType();

	switch (type)
	{
	case Item::Default:
	case Item::Treasure_Coins:
	case Item::Treasure_Goldbars:
	case Item::Treasure_Rings_Red:
	case Item::Treasure_Rings_Green:
	case Item::Treasure_Rings_Blue:
	case Item::Treasure_Rings_Purple:
	case Item::Treasure_Necklace:
	case Item::Treasure_Chalices_Red:
	case Item::Treasure_Chalices_Green:
	case Item::Treasure_Chalices_Blue:
	case Item::Treasure_Chalices_Purple:
	case Item::Treasure_Crosses_Red:
	case Item::Treasure_Crosses_Green:
	case Item::Treasure_Crosses_Blue:
	case Item::Treasure_Crosses_Purple:
	case Item::Treasure_Scepters_Red:
	case Item::Treasure_Scepters_Green:
	case Item::Treasure_Scepters_Blue:
	case Item::Treasure_Scepters_Purple:
	case Item::Treasure_Geckos_Red:
	case Item::Treasure_Geckos_Green:
	case Item::Treasure_Geckos_Blue:
	case Item::Treasure_Geckos_Purple:
	case Item::Treasure_Crowns_Red:
	case Item::Treasure_Crowns_Green:
	case Item::Treasure_Crowns_Blue:
	case Item::Treasure_Crowns_Purple:
	case Item::Treasure_Skull_Red:
	case Item::Treasure_Skull_Green:
	case Item::Treasure_Skull_Blue:
	case Item::Treasure_Skull_Purple:	_inventory.collectTreasure(type); return true;

	case Item::Ammo_Deathbag:	if (_inventory.addWeapon(ClawProjectile::Types::Pistol,  25)) return true; break;
	case Item::Ammo_Shot:		if (_inventory.addWeapon(ClawProjectile::Types::Pistol,   5)) return true; break;
	case Item::Ammo_Shotbag:	if (_inventory.addWeapon(ClawProjectile::Types::Pistol,  10)) return true; break;
	case Item::Ammo_Magic_5:	if (_inventory.addWeapon(ClawProjectile::Types::Magic,    5)) return true; break;
	case Item::Ammo_Magic_10:	if (_inventory.addWeapon(ClawProjectile::Types::Magic,   10)) return true; break;
	case Item::Ammo_Magic_25:	if (_inventory.addWeapon(ClawProjectile::Types::Magic,   25)) return true; break;
	case Item::Ammo_Dynamite:	if (_inventory.addWeapon(ClawProjectile::Types::Dynamite, 5)) return true; break;

	case Item::Health_Level:	if (addHealth( 5)) return true; break;
	case Item::Health_10:		if (addHealth(10)) return true; break;
	case Item::Health_15:		if (addHealth(15)) return true; break;
	case Item::Health_25:		if (addHealth(25)) return true; break;

	case Item::MapPiece:
	case Item::NineLivesGem:	_finishLevel = true; return true;

	case Item::Powerup_Invincibility:
		_invincibilityComponent.init();
		updateInvincibilityColorEffect();
		[[fallthrough]]; // special init for this type
	case Item::Powerup_Catnip_White:
	case Item::Powerup_Catnip_Red:
	case Item::Powerup_Invisibility:
	case Item::Powerup_FireSword:
	case Item::Powerup_LightningSword:
	case Item::Powerup_IceSword:
		startPowerup(item);
		return true;

	case Item::Powerup_ExtraLife:	if (_inventory.addLives(1)) return true; else break;

	case Item::Warp:
	case Item::BossWarp:
		// impleted as `class Warp`
		break;

	// these items used in multiplayer mode, so I don't need to implement them now
	case Item::Curse_Ammo:		break;
	case Item::Curse_Magic:		break;
	case Item::Curse_Health:	break;
	case Item::Curse_Life:		break;
	case Item::Curse_Treasure:	break;
	case Item::Curse_Freeze:	break;
	}

	return false;
}

bool Player::isFalling() const
{
	// `20` is MAX_ITER_TIME at `GameMainApp`. `20*GRAVITY` is the speed when CC starts falling
	return speed.y > (20 * GRAVITY) && !_isOnLadder;
}
bool Player::isStanding() const
{
	return _aniName == "STAND" || _aniName == "IDLE"
		|| _aniName == "LIFT" || _aniName == "LIFT2"
		|| _aniName == "THROW" || _aniName == "SIREN-FREEZE";
}
bool Player::isDuck() const
{
	return startsWith(_aniName, "DUCK");
}
bool Player::isTakeDamage() const
{
	return startsWith(_aniName, "DAMAGE") && !_ani->isFinishAnimation();
}
bool Player::isWeaponAnimation() const
{
	return endsWith(_aniName, "PISTOL") || endsWith(_aniName, "MAGIC") || endsWith(_aniName, "DYNAMITE");
}

void Player::backToLife()
{
	_isCollideWithLadder = false;
	_isOnLadderTop = false;
	_isOnLadder = false;
	resetKeys();
	_useWeapon = false;
	_isAttack = false;
	elevator = nullptr;
	rope = nullptr;
	conveyorBelt = nullptr;
	_health = 100;
	if (!cheats->isFlying()) {
		_aniName = "STAND";
		_ani = _animations[_aniName];
	}
	position = startPosition;
	speed = {};
	_saveCurrRect = GetRect();
	_powerupTimer.stop();
	stopPowerup();
	_dialogTimer.stop();
	_holdAltTime = 0;
	_raisedPowderKeg = nullptr;
	_lastAttackRect = {};
	_saveCurrAttackRect = {};
	_damageRestTimer.stop();
	_isMirrored = false;
	_freezeTime = 0;

	calcRect();
	Logic(0); // update position and animation
}
void Player::loseLife()
{
	if (!isInDeathAnimation() || isFallDeath())
	{
		_inventory.loseLife();
		_health = 0;
		_damageRestTimer.stop();
		changeAnimation("SPIKEDEATH");
		_ani->loopAni = false;
		_powerupTimer.stop();
		stopPowerup();
	}
}
void Player::nextLevel()
{
	_finishLevel = false;
	_inventory.resetTreasures();
	backToLife();
}
void Player::endLife()
{
	_health = 0;
}

void Player::shootSwordProjectile()
{
	ClawProjectile::Types type;
	switch (_currPowerup)
	{
	case Item::Powerup_FireSword: type = ClawProjectile::Types::FireSword; break;
	case Item::Powerup_IceSword: type = ClawProjectile::Types::IceSword; break;
	case Item::Powerup_LightningSword: type = ClawProjectile::Types::LightningSword; break;
	default: return;
	}

	WwdObject obj;
	calcAttackRect();
	Rectangle2D atkRc = GetAttackRect().first;
	obj.x = (int32_t)position.x;
	obj.y = int32_t(atkRc.top + atkRc.bottom) / 2;
	obj.speedX = (_isMirrored ? -DEFAULT_PROJECTILE_SPEED : DEFAULT_PROJECTILE_SPEED);
	obj.damage = 25;
	GO::addObjectToActionPlane(ClawProjectile::createNew(type, obj));
}

SavedDataManager::GameData Player::getGameData() const
{
	SavedDataManager::GameData data = {};

	data.lives = _inventory.getLives();
	data.health = _health;
	data.score = _inventory.getScore();
	data.pistolAmount = _inventory.getWeaponAmount(ClawProjectile::Types::Pistol);
	data.magicAmount = _inventory.getWeaponAmount(ClawProjectile::Types::Magic);
	data.dynamiteAmount = _inventory.getWeaponAmount(ClawProjectile::Types::Dynamite);

	return data;
}
void Player::setGameData(const SavedDataManager::GameData& data)
{
	// TODO method in Inventory
	_inventory.setLives(data.lives);
	_health = data.health;
	_inventory.setScore(data.score);
	_inventory.setWeaponAmount(ClawProjectile::Types::Pistol, data.pistolAmount);
	_inventory.setWeaponAmount(ClawProjectile::Types::Magic, data.magicAmount);
	_inventory.setWeaponAmount(ClawProjectile::Types::Dynamite, data.dynamiteAmount);
}

void Player::loseHealth(int damage) {
	if (!isInvincibility() && !cheats->isGodMode())
		_health -= damage;
}
bool Player::checkForHurts()
{
	if (_isAttack || isTakeDamage() || !_damageRestTimer.isFinished() || isInvincibility() || cheats->isGodMode())
		return false;

	pair<Rectangle2D, int> atkRc;
	Rectangle2D damageRc;
	int damage;

	for (BaseEnemy* enemy : GO::getActionPlaneEnemies())
	{
		if (enemy->isAttack())
		{
			atkRc = enemy->GetAttackRect();
			if (_lastAttackRect != atkRc.first)
			{
				damageRc = _saveCurrRect.getCollision(atkRc.first);
				if (!damageRc.isEmpty())
				{
					_lastAttackRect = atkRc.first;
					loseHealth(atkRc.second);

					// draw damage animation
					OneTimeAnimation* ani = DBG_NEW OneTimeAnimation({
							position.x + (damageRc.left - damageRc.right) / 2,
							position.y + (damageRc.top - damageRc.bottom) / 2
						}, AssetsManager::getCopyAnimationFromDirectory("GAME/IMAGES/CLAWHIT", false, 50));
					myMemCpy(ani->drawZ, drawZ + 1);
					GO::addObjectToActionPlane(ani);

					return true;
				}
			}
		}
	}

	for (Projectile* p : GO::getActionPlaneProjectiles())
	{
		if (isinstance<SirenScream>(p))
		{
			_freezeTime = 3000; // freeze CC for 3 seconds
			return false;
		}
		else if (isinstance<EnemyProjectile>(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				loseHealth(p->getDamage());
				p->removeObject = true;
				return true;
			}
		}
		else if (isinstance<Stalactite>(p) || isinstance<LavaMouth>(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				loseHealth(p->getDamage());
				return true;
			}
		}
		else if (isinstance<ClawDynamite>(p)) // CC can hurt himself with dynamite
		{
			if ((damage = p->getDamage()) > 0)
			{
				if ((damageRc = p->GetRect()) != _lastAttackRect)
				{
					if (_saveCurrRect.intersects(damageRc))
					{
						loseHealth(damage);
						_lastAttackRect = damageRc;
						return true;
					}
				}
			}
		}
	}

	for (PowderKeg* p : GO::getActionPlanePowderKegs())
	{
		damageRc = p->GetRect();
		if (damageRc != _lastAttackRect && (damage = p->getDamage()) > 0)
		{
			if (_saveCurrRect.intersects(damageRc))
			{
				loseHealth(damage);
				_lastAttackRect = damageRc;
				return true;
			}
		}
	}

	for (BaseDamageObject* obj : GO::getActionPlaneDamageObjects())
	{
		if ((damage = obj->getDamage()) > 0)
			if (_saveCurrRect.intersects(obj->GetRect()))
			{
				loseHealth(damage);
				return true;
			}
	}

	_lastAttackRect = {};

	return false;
}

void Player::activateDialog(int duration)
{
	_dialogTimer.reset(duration);
	addTimer(&_dialogTimer);
}

#ifdef _DEBUG // in debug mode, player can move freely
void Player::squeeze(D2D1_POINT_2F pos, bool mirror) {}
void Player::unsqueeze() {}
#else
void Player::squeeze(D2D1_POINT_2F pos, bool mirror)
{
	if (isInvincibility())
		return;

	_isMirrored = mirror;
	position = pos;
	_ani = _animations[_aniName = "SQUEEZED"];
	speed = {};

	calcRect();
}
void Player::unsqueeze()
{
	_ani = _animations[_aniName = "STAND"];
}
#endif

bool Player::cheat(int cheatType)
{
	switch (cheatType)
	{
	case CheatsManager::FillLife:		_inventory.setMaxLives(); break;
	case CheatsManager::FillHealth:		_health = MAX_HEALTH_AMOUNT; break;
	case CheatsManager::FillPistol:		_inventory.setWeaponMaxAmount(ClawProjectile::Types::Pistol); break;
	case CheatsManager::FillMagic:		_inventory.setWeaponMaxAmount(ClawProjectile::Types::Magic); break;
	case CheatsManager::FillDynamite:	_inventory.setWeaponMaxAmount(ClawProjectile::Types::Dynamite); break;
	case CheatsManager::FinishLevel:	_finishLevel = true; break;
	case CheatsManager::GodMode:
		_inventory.setMaxLives();
		_health = MAX_HEALTH_AMOUNT;
		_inventory.setWeaponMaxAmount(ClawProjectile::Types::Pistol);
		_inventory.setWeaponMaxAmount(ClawProjectile::Types::Magic);
		_inventory.setWeaponMaxAmount(ClawProjectile::Types::Dynamite);
		break;
	case CheatsManager::Flying:
		if (_raisedPowderKeg) return false; // CC can't fly when he lifts a keg
		_aniName = "FLYING-CHEAT";
		_ani = _animations[_aniName];
		// cancel all actions
		_isAttack = false;
		_saveCurrAttackRect = {};
		_lastAttackRect = {};
		_freezeTime = false;
		_useWeapon = false;
		resetKeys();
		_damageRestTimer.stop();
		elevator = nullptr;
		rope = nullptr;
		conveyorBelt = nullptr;
		break;
	}

	return true;
}

void Player::resetKeys()
{
	_upPressed = false;
	_downPressed = false;
	_leftPressed = false;
	_rightPressed = false;
	_altPressed = false;
	_holdAltTime = 0;
}

void Player::stopPowerup()
{
	if (_currPowerup != Item::None) // now powerup finished
	{
		if (isInvincibility())
			cancelInvincibilityEffect();
		_currPowerup = Item::None;
		AssetsManager::startBackgroundMusic(AssetsManager::BackgroundMusicType::Level);
	}
}
void Player::startPowerup(Item* powerup)
{
	AssetsManager::startBackgroundMusic(AssetsManager::BackgroundMusicType::Powerup);
	Item::Type type = powerup->getType();
	if (type == Item::Type::Powerup_Catnip_Red)
		type = Item::Type::Powerup_Catnip;

	if (_currPowerup != type) {
		_powerupTimer.reset(powerup->getDuration());
		cancelInvincibilityEffect();
	}
	else {
		_powerupTimer.reset(_powerupTimer.getTimeLeft() + powerup->getDuration());
	}
	_currPowerup = type;
	addTimer(&_powerupTimer);
}
bool Player::addHealth(int health)
{
	if (_health < MAX_HEALTH_AMOUNT) {
		_health += health;
		if (_health > MAX_HEALTH_AMOUNT)
			_health = MAX_HEALTH_AMOUNT;
		return true;
	}
	return false;
}
