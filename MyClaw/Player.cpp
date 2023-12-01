#include "Player.h"
#include "Assets-Managers/AssetsManager.h"
#include "ActionPlane.h"
#include "Objects/Rope.h"
#include "Objects/Stalactite.h"
#include "Objects/LavaMouth.h"
#include "BaseEnemy.h"
#include "Objects/EnemyProjectile.h"
#include "Enemies/Aquatis.h"


// TODO: find perfect values (if they are still not perfect :D )

#define SpeedX_Normal			0.288f
#define SpeedX_SuperSpeed		0.416f
#define SpeedX_LiftPowderKeg	0.144f
#define SpeedY_Climb			0.288f
#define SpeedY_SuperClimb		0.412f
#define SpeedY_RegularJump		0.800f
#define SpeedY_SuperJump		0.920f
#define SpeedY_MAX				1.500f

#ifdef _DEBUG // move fast in debug mode
#undef SpeedX_Normal
#undef SpeedY_Climb
#undef SpeedY_RegularJump
#define SpeedX_Normal		SpeedX_SuperSpeed
#define SpeedY_Climb		SpeedY_SuperClimb
#define SpeedY_RegularJump	SpeedY_SuperJump
#endif

#define EXCLAMATION_MARK	_animations["exclamation-mark"] // it used when claw is speaking

// code blocks for `collectItem`
#define ADD_WEAPON(t, n) { _weaponsAmount[ClawProjectile::Types:: t] += n; return true; }
#define ADD_HEALTH(n) \
	if (_health < 100) { \
		_health += n; \
		if (_health > 100) _health = 100; \
		return true; \
	} break;
#define SET_POWERUP(t) \
	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Powerup); \
	if (_currPowerup != Item:: t) _powerupLeftTime = 0; \
	_powerupLeftTime += item->getDuration(); \
	_currPowerup = Item:: t; \
	return true;

#define renameKey(oldKey, newKey) { _animations[newKey] = _animations[oldKey]; _animations.erase(oldKey); }

#define Powerup_Catnip Powerup_Catnip_White // used for catnip powerup


static int32_t MAX_DYNAMITE_SPEED_X = DEFAULT_PROJECTILE_SPEED * 8 / 7;
static int32_t MAX_DYNAMITE_SPEED_Y = DEFAULT_PROJECTILE_SPEED * 5 / 3;


Player::PowerupSparkles::PowerupSparkles(Rectangle2D* playerRc)
	: _playerRc(playerRc)
{
	for (auto& s : _sparkles)
	{
		s = AssetsManager::loadCopyAnimation("GAME/ANIS/GLITTER1.ANI");
		init(s);
	}
}
void Player::PowerupSparkles::init(shared_ptr<Animation> sparkle)
{
	const float a = (_playerRc->right - _playerRc->left) / 2; // vertical radius
	const float b = (_playerRc->bottom - _playerRc->top) / 2; // horizontal radius
	float x, y;

	do {
		x = getRandomFloat(-a, a);
		y = getRandomFloat(-b, b);
		// check if (x,y) is in the player ellipse
	} while (pow(x / a, 2) + pow(y / b, 2) > 1);

	sparkle->position.x = x + (_playerRc->right + _playerRc->left) / 2;
	sparkle->position.y = y + (_playerRc->bottom + _playerRc->top) / 2;

	for (int i = 0, n = getRandomInt(0, 3); i < n; i++) sparkle->Logic(50);
}
void Player::PowerupSparkles::Logic(uint32_t elapsedTime)
{
	for (auto& s : _sparkles)
	{
		if (s->isFinishAnimation())
			init(s);
		s->Logic(elapsedTime);
	}
}
void Player::PowerupSparkles::Draw() {
	for (auto& s : _sparkles) s->Draw();
}


Player::Player(const WwdObject& obj)
	: BaseCharacter(obj), _currWeapon(ClawProjectile::Types::Pistol), 
	_finishLevel(false), _powerupSparkles(&_saveCurrRect)
{
	_animations = AssetsManager::loadAnimationsFromDirectory("CLAW/ANIS");
	_weaponsAmount[ClawProjectile::Types::Pistol] = 10;
	_weaponsAmount[ClawProjectile::Types::Magic] = 5;
	_weaponsAmount[ClawProjectile::Types::Dynamite] = 3;
	startPosition = position;
	_lives = 6;
	_score = 0;

	backToLife();

	// change keys for easy codeing
	renameKey("JUMPDYNAMITE", "JUMPPOSTDYNAMITE");
	renameKey("DUCKEMPTYDYNAMITE", "DUCKEMPTYPOSTDYNAMITE");
	renameKey("EMPTYDYNAMITE", "EMPTYPOSTDYNAMITE");
	renameKey("EMPTYJUMPDYNAMITE", "JUMPEMPTYPOSTDYNAMITE");
	renameKey("EMPTYJUMPPISTOL", "JUMPEMPTYPISTOL");
	renameKey("EMPTYJUMPMAGIC", "JUMPEMPTYMAGIC");

	AttackAnimations = { "SWIPE", "KICK", "UPPERCUT", "PUNCH", "DUCKSWIPE", "JUMPSWIPE" };
	NoLoopAnimations = { "LOOKUP", "SPIKEDEATH", "LIFT" };

	EXCLAMATION_MARK = AssetsManager::createCopyAnimationFromDirectory("GAME/IMAGES/EXCLAMATION");
	_animations["SIREN-FREEZE"] = AssetsManager::createAnimationFromPidImage("CLAW/IMAGES/100.PID");

	_animations["SQUEEZED"] = allocNewSharedPtr<Animation>(vector<Animation::FrameData*>({
		DBG_NEW Animation::FrameData("CLAW/IMAGES/450.PID"),
		DBG_NEW Animation::FrameData("CLAW/IMAGES/451.PID"),
		DBG_NEW Animation::FrameData("CLAW/IMAGES/452.PID")
	}));
}

void Player::Logic(uint32_t elapsedTime)
{
	if (isSqueezed())
	{
		_ani->position = position;
		_ani->Logic(elapsedTime);
		return;
	}

	if (_aniName == "LIFT" || _aniName == "THROW" || _aniName == "FALLDEATH" ||
		_aniName == "EMPTYPOSTDYNAMITE" || _aniName == "DUCKEMPTYPOSTDYNAMITE")
	{
		if (!_ani->isFinishAnimation() || _aniName == "FALLDEATH")
		{
			_ani->position = position;
			_ani->Logic(elapsedTime);
			return;
		}
	}

	if (_spacePressed && (speed.y == 0 || _isOnLadder))
	{
		jump();
		_spacePressed = false;
		_isOnLadder = false;
	}

	if ((isJumping() || isFalling()) && _aniName != "WALK")
	{
		_leftCollision = false;
		_rightCollision = false;
	}

	if (_freezeTime > 0)
	{
		_freezeTime -= elapsedTime;
		_leftPressed = false;
		_rightPressed = false;
		_downPressed = false;
		_upPressed = false;
		_spacePressed = false;
		_altPressed = false;
		_zPressed = false;
	}

	if (_holdAltTime < 1000) _holdAltTime += elapsedTime; // the max time for holding is 1000 milliseconds
	if (_dialogLeftTime > 0) _dialogLeftTime -= elapsedTime;
	if (_powerupLeftTime > 0) _powerupLeftTime -= elapsedTime;
	else if (_currPowerup != Item::None)
	{
		_powerupLeftTime = 0;
		_currPowerup = Item::None;
		AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level);
	}

	float speedX = SpeedX_Normal, speedYClimb = SpeedY_Climb;

	if (_currPowerup == Item::Powerup_Catnip)
	{
		_powerupSparkles.Logic(elapsedTime);
		speedX = SpeedX_SuperSpeed;
		speedYClimb = SpeedY_SuperClimb;
	}

	if (_raisedPowderKeg)
		speedX = SpeedX_LiftPowderKeg;

	_damageRest -= elapsedTime;
	if (checkForHurts())
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

		_ani->Logic(elapsedTime);
		_lastAttackRect = {};

		if (_ani->isFinishAnimation())
			_damageRest = 250;

		calcRect();
		calcAttackRect();
		return;
	}


	const D2D1_POINT_2F prevPosition = position;

	const bool inAir = isFalling() || isJumping();
	const bool lookup = _upPressed && (isStanding() || _aniName == "LOOKUP");
	const bool duck = _downPressed && (isStanding() || isDuck());

	if (lookup)
	{
		_isAttack = false;
		_useWeapon = false;
	}

	const bool canWalk = (!_useWeapon || (_useWeapon && inAir)) && (!_isAttack || (_isAttack && inAir)) && !duck && !lookup && !_altPressed && !_isOnLadder;
	const bool goLeft = _leftPressed && !_leftCollision && canWalk;
	const bool goRight = _rightPressed && !_rightCollision && canWalk;

	bool climbUp = false, climbDown = false;


	if (_leftPressed) _isMirrored = true;
	else if (_rightPressed || isClimbing()) _isMirrored = false;

	if (goLeft) speed.x = -speedX;
	else if (goRight) speed.x = speedX;
	else speed.x = 0;

	if (inAir && _raisedPowderKeg)
	{
		_raisedPowderKeg->fall();
		_raisedPowderKeg = nullptr;
	}

	if (_isCollideWithLadder)
	{
		_isCollideWithLadder = false;
		climbUp = _upPressed && !_isOnLadderTop;
		climbDown = _downPressed;
		// TODO: if `_isCollideWithLadder` but has a "bottom collision" (solid/groud) should duck

		if (climbUp)
		{
			speed.y = -speedYClimb;
			_isOnLadder = true;
			elevator = nullptr;
		}
		else if (climbDown)
		{
			speed.y = speedYClimb;
			_isOnLadder = true;
			elevator = nullptr;
		}
		else if (_isOnLadder)
		{ // idle on ladder
			speed.y = 0;
		}
		else
		{
			if (elevator == nullptr)
			{
				speed.y += GRAVITY * elapsedTime;
			}
		}

		// check if collision with ground so we need let CC stand
		if (_isOnLadder)
		{
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
						// get random attack
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
		else if (_altPressed && _currWeapon == ClawProjectile::Types::Dynamite && !inAir)
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
				switch (_currWeapon)
				{
				case ClawProjectile::Types::Pistol: _aniName = "PISTOL"; break;
				case ClawProjectile::Types::Magic: _aniName = "MAGIC"; break;
				case ClawProjectile::Types::Dynamite: _aniName = "POSTDYNAMITE"; break;
				}

				if (_weaponsAmount[_currWeapon] > 0)
				{
					WwdObject obj;
					obj.x = (int32_t)(position.x + (_isMirrored ? _saveCurrRect.left - _saveCurrRect.right : _saveCurrRect.right - _saveCurrRect.left));
					obj.z = ZCoord;
					obj.speedX = (_isMirrored ? -DEFAULT_PROJECTILE_SPEED : DEFAULT_PROJECTILE_SPEED);

					switch (_currWeapon)
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
							int32_t f = obj.speedX * _holdAltTime / 1000;
							if (_isMirrored)
								obj.speedX = max(f, -MAX_DYNAMITE_SPEED_X);
							else
								obj.speedX = min(f, MAX_DYNAMITE_SPEED_X);
							obj.speedY = min(obj.speedY, -MAX_DYNAMITE_SPEED_Y) * _holdAltTime / 1000;
						}

						break;

					default:
						throw Exception("no more weapons...");
					}

					_weaponsAmount[_currWeapon] -= 1;
					if (inAir) obj.y += 10;
					ActionPlane::addPlaneObject(ClawProjectile::createNew(_currWeapon, obj));
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
		else if (position.y - prevPosition.y > 5)
		{
			_aniName = "FALL";
		}
		else if (isJumping())
		{
			_aniName = "JUMP";
		}
		else if (goLeft || goRight)
		{
			if (_raisedPowderKeg)
			{
				_aniName = "LIFTWALK";
			}
			else
			{
				_aniName = "WALK";
			}
		}
		else if (_aniName != "JUMP")
		{
			if (_zPressed)
			{
				if (_raisedPowderKeg)
				{
					// throw the powder keg
					_raisedPowderKeg->thrown(!_isMirrored);
					_raisedPowderKeg = nullptr;
					_aniName = "THROW";
					_zPressed = false; // if we throw do not try catch other keg
				}
				else
				{
					// try lift powder keg
					for (PowderKeg* p : ActionPlane::getPowderKegs())
					{
						if (_saveCurrRect.intersects(p->GetRect()))
						{
							if (p->raise())
							{
								_raisedPowderKeg = p;
								_aniName = "LIFT";
								_zPressed = false; // if we lift do not try throw other keg
								break;
							}
						}
					}

					// TODO: throw enemies
				}
			}
			else if (_raisedPowderKeg)
			{
				_aniName = "LIFT2";
			}
			else
			{
				_aniName = "STAND";
			}
		}

		if (_raisedPowderKeg)
		{
			//if (isStanding())
			//	speed.y = 0;
			_raisedPowderKeg->position.x = position.x;
			_raisedPowderKeg->position.y = position.y - 104;
		}


		if (prevAniName != _aniName)
		{
			_ani = _animations[_aniName];
			_ani->reset();
			_ani->updateFrames = true;

			_ani->loopAni = !FindInArray(NoLoopAnimations, _aniName) && !FindInArray(AttackAnimations, _aniName)
				&& !isWeaponAnimation() && !isDuck();

			if (endsWith(_aniName, "SWIPE") && _currPowerup != Item::None)
			{
				shootSwordProjectile();
			}
		}
	}
	else
	{
		_ani->loopAni = false;
	}

	_ani->opacity = isGhost() ? 0.5f : 1.f;
	// TODO: when `_currPowerup` is `Invincibility` draw CC colorfuly
	_ani->mirrored = _isMirrored && !_isOnLadder;
	_ani->position = position;
	_ani->Logic(elapsedTime);

	calcRect();
	calcAttackRect();
}
void Player::Draw()
{
	BaseCharacter::Draw();
	if (_dialogLeftTime > 0)
	{
		EXCLAMATION_MARK->position = { position.x, position.y - 64 };
		EXCLAMATION_MARK->updateImageData();
		EXCLAMATION_MARK->Draw();
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
		|| _currPowerup == Item::Powerup_LightningSword) ? 100 : damage };
}

void Player::stopFalling(float collisionSize)
{
	BaseCharacter::stopFalling(collisionSize);
	if (speed.x == 0 && !isDuck() && !isStanding() && !_isAttack && !isWeaponAnimation())
	{
		// If CC stopped falling (and he is not walking) he should stand
		_ani = _animations[_aniName = "STAND"];
		_ani->reset();
		_ani->mirrored = _isMirrored && !_isOnLadder;
		_ani->position = position;
		_ani->updateImageData();
	}
	_isOnLadder = false;
}
void Player::stopMovingLeft(float collisionSize)
{
	if (isClimbing()) return;
	if (speed.x != 0)
	{
		speed.x = 0;
		position.x += collisionSize;
		_leftCollision = true;
	}
	elevator = nullptr;
}
void Player::stopMovingRight(float collisionSize)
{
	if (isClimbing()) return;
	if (speed.x != 0)
	{
		speed.x = 0;
		position.x -= collisionSize;
		_rightCollision = true;
	}
	elevator = nullptr;
}
void Player::jump(float force)
{
	if (_aniName == "LOOKUP" || isDuck() || _raisedPowderKeg) return;

	elevator = nullptr;
	rope = nullptr;

	speed.y = -force;
}
void Player::jump()
{
	if (_currPowerup == Item::Powerup_Catnip)
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
	case Item::Treasure_Skull_Purple: {
		_collectedTreasures[type] += 1;
		uint32_t tScore = item->getTreasureScore();
		_score += tScore;

#ifndef LOW_DETAILS
		int i = 0;

		switch (tScore)
		{
		case   100: i = 1; break;
		case   500: i = 2; break;
		case  1500: i = 3; break;
		case  2500: i = 4; break;
		case  5000: i = 5; break;
		case  7500: i = 6; break;
		case 10000: i = 7; break;
		case 15000: i = 8; break;
		case 25000: i = 9; break;
		}

		vector<Animation::FrameData*> images = AssetsManager::createAnimationFromPidImage("GAME/IMAGES/POINTS/00" + to_string(i) + ".PID")->getImagesList();
		myMemCpy(images[0]->duration, 1000U);
		OneTimeAnimation* ani = DBG_NEW OneTimeAnimation(item->position, allocNewSharedPtr<Animation>(images));
		myMemCpy(ani->ZCoord, item->ZCoord);

		ActionPlane::addPlaneObject(ani);
#endif
	}	return true;

	case Item::Ammo_Deathbag:	ADD_WEAPON(Pistol, 25);
	case Item::Ammo_Shot:		ADD_WEAPON(Pistol, 5);
	case Item::Ammo_Shotbag:	ADD_WEAPON(Pistol, 10);
	case Item::Ammo_Magic_5:	ADD_WEAPON(Magic,  5);
	case Item::Ammo_Magic_10:	ADD_WEAPON(Magic, 10);
	case Item::Ammo_Magic_25:	ADD_WEAPON(Magic, 25);
	case Item::Ammo_Dynamite:	AquatisDynamite::playerTookDynamite(); ADD_WEAPON(Dynamite, 5);

	case Item::Health_Level:	ADD_HEALTH(5);
	case Item::Health_10:		ADD_HEALTH(10);
	case Item::Health_15:		ADD_HEALTH(15);
	case Item::Health_25:		ADD_HEALTH(25);

	case Item::MapPiece:
	case Item::NineLivesGem:
		_finishLevel = true;
		break;

	case Item::Powerup_Catnip_White:
	case Item::Powerup_Catnip_Red:		SET_POWERUP(Powerup_Catnip);
	case Item::Powerup_Invisibility:	SET_POWERUP(Powerup_Invisibility);
	case Item::Powerup_Invincibility:	SET_POWERUP(Powerup_Invincibility);
	case Item::Powerup_FireSword:		SET_POWERUP(Powerup_FireSword);
	case Item::Powerup_LightningSword:	SET_POWERUP(Powerup_LightningSword);
	case Item::Powerup_IceSword:		SET_POWERUP(Powerup_IceSword);
	case Item::Powerup_ExtraLife:		_lives += 1; return true;


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
		|| _aniName == "THROW";
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
	_leftCollision = false;
	_rightCollision = false;
	_isCollideWithLadder = false;
	_isOnLadderTop = false;
	_isOnLadder = false;
	_upPressed = false;
	_downPressed = false;
	_leftPressed = false;
	_rightPressed = false;
	_spacePressed = false;
	_altPressed = false;
	_zPressed = false;
	_useWeapon = false;
	_isAttack = false;
	elevator = nullptr;
	rope = nullptr;
	_health = 100;
	_aniName = "STAND";
	_ani = _animations["STAND"];
	position = startPosition;
	speed = {};
	_saveCurrRect = GetRect();
	_powerupLeftTime = 0;
	_dialogLeftTime = 0;
	_holdAltTime = 0;
	_currPowerup = Item::None;
	_lastPowderKegExplos = nullptr;
	_raisedPowderKeg = nullptr;
	_lastAttackRect = {};
	_saveCurrAttackRect = {};
	_damageRest = 0;
	_isMirrored = false;
	_freezeTime = 0;

	calcRect();
	_saveCurrAttackRect = {};

	Logic(0); // update position and animation
}
void Player::loseLife()
{
	if (!isInDeathAnimation() || isFallDeath())
	{
		_lives -= 1;
		_health = 0;
		_damageRest = 0;
		_aniName = "SPIKEDEATH";
		_ani = _animations[_aniName];
		_ani->reset();
		_powerupLeftTime = 0;
		_currPowerup = Item::None;
		AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level);
	}
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
	obj.z = ZCoord;
	obj.speedX = (_isMirrored ? -DEFAULT_PROJECTILE_SPEED : DEFAULT_PROJECTILE_SPEED);
	obj.damage = 25;
	ActionPlane::addPlaneObject(ClawProjectile::createNew(type, obj));
}

void Player::keyUp(int key)
{
	if (isInDeathAnimation() || _freezeTime > 0) return;

	switch (key)
	{
	case VK_UP:		_upPressed = false; break;
	case VK_DOWN:	_downPressed = false; break;
	case VK_LEFT:	_leftPressed = false; _leftCollision = false; _rightCollision = false; break;
	case VK_RIGHT:	_rightPressed = false; _leftCollision = false; _rightCollision = false; break;
	case VK_SPACE:	_spacePressed = false; break;
	case 'Z':		_zPressed = false; break;

	case VK_SHIFT:	if (!_useWeapon) _currWeapon = ClawProjectile::Types(((int)_currWeapon + 1) % 3); break;
	case '1':		if (!_useWeapon) _currWeapon = ClawProjectile::Types::Pistol; break;
	case '2':		if (!_useWeapon) _currWeapon = ClawProjectile::Types::Magic; break;
	case '3':		if (!_useWeapon) _currWeapon = ClawProjectile::Types::Dynamite; break;

	case VK_MENU:		_useWeapon = !_isOnLadder && !rope && !_raisedPowderKeg; _altPressed = false; break;
	case VK_CONTROL:	_isAttack = !_isOnLadder && !_altPressed && !rope && !_raisedPowderKeg; break;
	}
}
void Player::keyDown(int key)
{
	if (isInDeathAnimation() || _useWeapon || _freezeTime > 0) return;

	switch (key)
	{
	case VK_UP:		if (!_useWeapon && !_isAttack && !_altPressed && !rope && !_raisedPowderKeg) _upPressed = true; break;
	case VK_DOWN:	if (!rope && !_raisedPowderKeg) _downPressed = true; break;
	case VK_LEFT:	_leftPressed = true; break;
	case VK_RIGHT:	_rightPressed = true; break;
	case VK_SPACE:	_spacePressed = true; break;
	case 'Z':		if (!rope) _zPressed = true; break;
	case VK_MENU:	if (!rope && !_raisedPowderKeg) {
		if (!_altPressed) _holdAltTime = 0;
		_altPressed = (_currWeapon != ClawProjectile::Types::Dynamite || _weaponsAmount[ClawProjectile::Types::Dynamite] > 0);
	} break;
	}
}

bool Player::checkForHurts()
{
#ifdef _DEBUG
	return false; // no damage in debug mode
#endif

	if (_isAttack || isTakeDamage() || _damageRest > 0 || _currPowerup == Item::Powerup_Invincibility) return false;

	pair<Rectangle2D, int> atkRc;
	Rectangle2D damageRc;

	for (BaseEnemy* enemy : ActionPlane::getEnemies())
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
					_health -= atkRc.second;

#ifndef LOW_DETAILS
					// draw damage animation
					OneTimeAnimation* ani = DBG_NEW OneTimeAnimation({
							position.x + (damageRc.left - damageRc.right) / 2,
							position.y + (damageRc.top - damageRc.bottom) / 2
						}, AssetsManager::createCopyAnimationFromDirectory("GAME/IMAGES/CLAWHIT", false, 50));
					myMemCpy(ani->ZCoord, ZCoord + 1);
					ActionPlane::addPlaneObject(ani);
#endif
					return true;
				}
			}
		}
	}

	for (Projectile* p : ActionPlane::getProjectiles())
	{
		if (isinstance<SirenProjectile>(p))
		{
			_freezeTime = 3000; // freeze CC for 3 seconds
			return false;
		}
		if (isEnemyProjectile(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_health -= p->getDamage();
				p->removeObject = true;
				return true;
			}
		}
		else if (isinstance<Stalactite>(p) || isinstance<LavaMouth>(p))
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_health -= p->getDamage();
				return true;
			}
		}
	}

	int damage;

	for (PowderKeg* p : ActionPlane::getPowderKegs())
	{
		if (p != _lastPowderKegExplos && (damage = p->getDamage()) > 0)
		{
			if (_saveCurrRect.intersects(p->GetRect()))
			{
				_health -= damage;
				_lastPowderKegExplos = p;
				return true;
			}
		}
	}

	for (BaseDamageObject* obj : ActionPlane::getDamageObjects())
	{
		if ((damage = obj->getDamage()) > 0)
			if (_saveCurrRect.intersects(obj->GetRect()))
			{
				_health -= damage;
				return true;
			}
	}

	_lastAttackRect = {};

	return false;
}

#ifdef _DEBUG // in debug mode, player can move freely
void Player::squeeze(D2D1_POINT_2F pos) {}
void Player::unsqueeze() {}
#else
void Player::squeeze(D2D1_POINT_2F pos)
{
	if (_currPowerup == Item::Powerup_Invincibility)
		return;

	if (pos.x != 0 && pos.y != 0)
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
