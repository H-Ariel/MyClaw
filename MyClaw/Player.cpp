#include "Player.h"
#include "AssetsManager.h"
#include "ActionPlane.h"
#include "Objects/Rope.h"
#include "Objects/Projectile.h"
#include "Objects/BaseEnemy.h"


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
#define ADD_HEALTH(n) \
	if (_health < 100) { \
		_health += n; \
		if (_health > 100) _health = 100; \
		return true; \
	} break;
#define SET_POWERUP(type) \
	AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Powerup, true); \
	if (_currPowerup != PowerupType:: type) _powerupLeftTime = 0; \
	_powerupLeftTime += item->getDuration(); \
	_currPowerup = PowerupType:: type; \
	return true;

#define renameKey(oldKey, newKey) { _animations[newKey] = _animations[oldKey]; _animations.erase(oldKey); }


static int32_t MAX_DYNAMITE_SPEED = DEFAULT_PROJECTILE_SPEED * 8 / 7;


class PowerupSparkle
{
private:
	shared_ptr<Animation> _ani;
public:
	static Rectangle2D* playerRc;

	PowerupSparkle()
	{
		_ani = AssetsManager::loadCopyAnimation("GAME/ANIS/GLITTER1.ANI");
		init();
	}
	void init()
	{
		const float a = (playerRc->right - playerRc->left) / 2; // vertical radius
		const float b = (playerRc->bottom - playerRc->top) / 2; // horizontal radius
		float x, y;
		int i, n = getRandomInt(0, 3);

		do {
			x = getRandomFloat(-a, a);
			y = getRandomFloat(-b, b);
			// check if (x,y) is in the player ellipse
		} while (pow(x / a, 2) + pow(y / b, 2) > 1);

		for (i = 0; i < n; i++) _ani->Logic(50);

		_ani->position.x = x + (playerRc->right + playerRc->left) / 2;
		_ani->position.y = y + (playerRc->bottom + playerRc->top) / 2;
	}
	void Logic(uint32_t elapsedTime)
	{
		if (_ani->isFinishAnimation())
			init();
		_ani->Logic(elapsedTime);
	}
	void Draw() { _ani->Draw(); }
};
Rectangle2D* PowerupSparkle::playerRc = nullptr;


Player::Player(const WwdObject& obj, const D2D1_SIZE_F& planeSize)
	: BaseCharacter(obj), _planeSize(planeSize), _currWeapon(ClawProjectile::Types::Pistol), _finishLevel(false)
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
	NoLoopAnimations = { "LOOKUP", "SPIKEDEATH", "LIFT"};

	EXCLAMATION_MARK = AssetsManager::createCopyAnimationFromDirectory("GAME/IMAGES/EXCLAMATION", 125, false);
	_animations["SIREN-FREEZE"] = AssetsManager::createAnimationFromFromPidImage("CLAW/IMAGES/100.PID");

	PowerupSparkle::playerRc = &_saveCurrRect;
}

void Player::Logic(uint32_t elapsedTime)
{
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

	if (_spacePressed && (_speed.y == 0 || _isOnLadder))
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

	if (_holdAltTime < 1050) _holdAltTime += elapsedTime; // the max time for holding is 1050 milliseconds
	if (_dialogLeftTime > 0) _dialogLeftTime -= elapsedTime;
	if (_powerupLeftTime > 0) _powerupLeftTime -= elapsedTime;
	else if (_currPowerup != PowerupType::None)
	{
		_powerupLeftTime = 0;
		_currPowerup = PowerupType::None;
		_powerupSparkles.clear();
		AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level, false);
	}

	float speedX = SpeedX_Normal, speedYClimb = SpeedY_Climb;

	if (_currPowerup == PowerupType::Catnip)
	{
		for (size_t i = 0; i < 30; _powerupSparkles[i++].Logic(elapsedTime));
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
		_ani->mirrored = !_forward; ////////
		_ani->position = position; ////////
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
		_lastAttackRect = {}; // TODO: delete this or what in `checkEnemyHits`

		if (_ani->isFinishAnimation())
			_damageRest = 250;

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


	if (_leftPressed) _forward = false;
	else if (_rightPressed || isClimbing()) _forward = true;

	if (goLeft) _speed.x = -speedX;
	else if (goRight) _speed.x = speedX;
	else _speed.x = 0;

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
			_speed.y = -speedYClimb;
			_isOnLadder = true;
			elevator = nullptr;
		}
		else if (climbDown)
		{
			_speed.y = speedYClimb;
			_isOnLadder = true;
			elevator = nullptr;
		}
		else if (_isOnLadder)
		{ // idle on ladder
			_speed.y = 0;
		}
		else
		{
			if (elevator == nullptr)
			{
				_speed.y += GRAVITY * elapsedTime;
			}
		}
	}
	else
	{
		_isOnLadder = false;
		if (!elevator && !rope)
		{
			_speed.y += GRAVITY * elapsedTime;
		}
	}

	if (_speed.y > SpeedY_MAX)
		_speed.y = SpeedY_MAX;

	if (!isInDeathAnimation())
	{
		// update position based on speed, but make sure we don't go outside the level
		position.x += _speed.x * elapsedTime;
		position.y += _speed.y * elapsedTime;
		
		/*
		// TODO: Delete after we are sure we are not using it
		if (position.x < 0) position.x = 0;
		if (position.y < 0) position.y = 0;
		if (position.x > _planeSize.width) position.x = _planeSize.width;
		if (position.y > _planeSize.height) position.y = _planeSize.height;
		*/

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
			_forward = !rope->isPassedHalf();
			_speed = {};
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
					if (_currPowerup != PowerupType::FireSword &&
						_currPowerup != PowerupType::IceSword &&
						_currPowerup != PowerupType::LightningSword)
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
				// TODO: better code here

				switch (_currWeapon)
				{
				case ClawProjectile::Types::Pistol: _aniName = "PISTOL"; break;
				case ClawProjectile::Types::Magic: _aniName = "MAGIC"; break;
				case ClawProjectile::Types::Dynamite: _aniName = "POSTDYNAMITE"; break;
				}

				int& amount = _weaponsAmount[_currWeapon];

				if (amount > 0)
				{
					WwdObject obj;
					obj.x = (int32_t)(position.x + (_forward ? _saveCurrRect.right - _saveCurrRect.left : _saveCurrRect.left - _saveCurrRect.right));
					obj.z = ZCoord;
					obj.speedX = (_forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED);

					if (_currWeapon == ClawProjectile::Types::Pistol)
					{
						obj.y = (int32_t)(position.y - (duck ? -8 : 16));
						obj.damage = 8;
					}
					else if (_currWeapon == ClawProjectile::Types::Magic)
					{
						obj.y = (int32_t)(position.y + (duck ? 18 : -6));
						obj.damage = 25;
					}
					else if (_currWeapon == ClawProjectile::Types::Dynamite)
					{
						// TODO: find and calculate perfect speed
						// (don't forget `if (_holdAltTime < 1050) ...`

						obj.x = (int32_t)position.x;
						obj.y = (int32_t)position.y;
						//obj.speedY = -DEFAULT_PROJECTILE_SPEED;
						obj.damage = 15;

						if (inAir)
						{
							obj.speedX = _forward ? MAX_DYNAMITE_SPEED : -MAX_DYNAMITE_SPEED;
							obj.speedY = -MAX_DYNAMITE_SPEED;
						}
						else
						{
							if (_forward)
								obj.speedX = min(obj.speedX * _holdAltTime / 1000, MAX_DYNAMITE_SPEED);
							else
								obj.speedX = max(obj.speedX * _holdAltTime / 1000, -MAX_DYNAMITE_SPEED);
							obj.speedY = min(obj.speedY * _holdAltTime / 1000, -MAX_DYNAMITE_SPEED);
						}
					}
					else throw Exception("no more weapons...");

					amount -= 1;
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
		else if (duck && !_altPressed)
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
		else if (_aniName != "DUCK" && isDuck() && _ani->isFinishAnimation())
		{
			_aniName = "DUCK";
		}
		else if (_aniName != "JUMP")
		{
			if (_zPressed)
			{
				if (_raisedPowderKeg)
				{
					// throw the powder keg
					_raisedPowderKeg->thrown(_forward);
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
			_speed.y = 0;
			_raisedPowderKeg->position.x = position.x;
			_raisedPowderKeg->position.y = position.y - 104;
		}


		if (prevAniName != _aniName)
		{
			_ani = _animations[_aniName];
			_ani->reset();

			_ani->loopAni = !FindInArray(NoLoopAnimations, _aniName) && !FindInArray(AttackAnimations, _aniName)
				&& !isWeaponAnimation() && !isDuck();

			if (endsWith(_aniName, "SWIPE") && _currPowerup != PowerupType::None)
			{
				auto shootSwordProjectile = [&](ClawProjectile::Types type) {
					WwdObject obj;
					calcAttackRect();
					Rectangle2D atkRc = GetAttackRect().first;
					obj.x = (int32_t)position.x;
					obj.y = int32_t(atkRc.top + atkRc.bottom) / 2;
					obj.z = ZCoord;
					obj.speedX = (_forward ? DEFAULT_PROJECTILE_SPEED : -DEFAULT_PROJECTILE_SPEED);
					obj.damage = 25;
					ActionPlane::addPlaneObject(ClawProjectile::createNew(type, obj));
				};

				switch (_currPowerup)
				{
				case PowerupType::FireSword: shootSwordProjectile(ClawProjectile::Types::FireSword); break;
				case PowerupType::IceSword: shootSwordProjectile(ClawProjectile::Types::IceSword); break;
				case PowerupType::LightningSword: shootSwordProjectile(ClawProjectile::Types::LightningSword); break;
				}
			}
		}
	}
	else
	{
		_ani->loopAni = false;
	}

	_ani->mirrored = !_forward && !_isOnLadder;
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
	for (auto& s : _powerupSparkles)
	{
		s.second.Draw();
	}
}
Rectangle2D Player::GetRect() { return _saveCurrRect; }
pair<Rectangle2D, int> Player::GetAttackRect() { return _saveCurrAttackRect; }
void Player::calcRect()
{
	// calculate _saveCurrRect

	_saveCurrRect.left = -7.f + 15 * (!_forward);
	_saveCurrRect.right = _saveCurrRect.left + 44;

	if (isDuck())
	{
		_saveCurrRect.top = 30;
		_saveCurrRect.bottom = 90;
	}
	/*
	// TODO: improve this so CC doesn't hover when he lifts
	else if (_aniName == "LIFT")
	{
		_saveCurrRect.top = 30;
		_saveCurrRect.bottom = 70;
	}
	*/
	else
	{
		_saveCurrRect.top = 5;
		_saveCurrRect.bottom = 115;
	}

	// set rectangle by center
	float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2;
	float addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	_saveCurrRect.top += addY;
	_saveCurrRect.bottom += addY;
	_saveCurrRect.left += addX;
	_saveCurrRect.right += addX;
}
void Player::calcAttackRect()
{
	// calculate _saveCurrAttackRect

	Rectangle2D rc;
	int damage = 0;

	if (_aniName == "SWIPE")
	{
		rc.top = 60;
		rc.bottom = 80;

		if (_forward)
		{
			rc.left = 50;
			rc.right = 130;
		}
		else
		{
			rc.left = -80;
			rc.right = 0;
		}

		damage = 5;
	}
	else if (_aniName == "JUMPSWIPE")
	{
		rc.top = 35;
		rc.bottom = 55;

		if (_forward)
		{
			rc.left = 50;
			rc.right = 120;
		}
		else
		{
			rc.left = -70;
			rc.right = 0;
		}

		damage = 5;
	}
	else if (_aniName == "KICK")
	{
		rc.top = 25;
		rc.bottom = 85;

		if (_forward)
		{
			rc.left = 35;
			rc.right = 75;
		}
		else
		{
			rc.left = -25;
			rc.right = 15;
		}

		damage = 5;
	}
	else if (_aniName == "UPPERCUT")
	{
		rc.top = 5;
		rc.bottom = 65;

		if (_forward)
		{
			rc.left = 35;
			rc.right = 75;
		}
		else
		{
			rc.left = -30;
			rc.right = 10;
		}

		damage = 5;
	}
	else if (_aniName == "PUNCH")
	{
		rc.top = 30;
		rc.bottom = 65;

		if (_forward)
		{
			rc.left = 35;
			rc.right = 75;
		}
		else
		{
			rc.left = -30;
			rc.right = 10;
		}

		damage = 5;
	}
	else if (_aniName == "DUCKSWIPE")
	{
		rc.top = 45;
		rc.bottom = 60;

		if (_forward)
		{
			rc.left = 50;
			rc.right = 115;
		}
		else
		{
			rc.left = -65;
			rc.right = 0;
		}

		damage = 2;
	}
	else { _saveCurrAttackRect = {}; return; }

	// set rectangle by center
	float addX = position.x - (_saveCurrRect.right - _saveCurrRect.left) / 2;
	float addY = position.y - (_saveCurrRect.bottom - _saveCurrRect.top) / 2;
	rc.top += addY;
	rc.bottom += addY;
	rc.left += addX;
	rc.right += addX;

	_saveCurrAttackRect = { rc, (_currPowerup == PowerupType::Catnip
		|| _currPowerup == PowerupType::FireSword
		|| _currPowerup == PowerupType::IceSword
		|| _currPowerup == PowerupType::LightningSword) ? 100 : damage };
}

void Player::stopFalling(float collisionSize)
{
	BaseCharacter::stopFalling(collisionSize);
	if (_speed.x == 0 && !isDuck() && !isStanding() && !_isAttack && !isWeaponAnimation())
	{
		// If CC stopped falling (and he is not walking) he should stand
		_ani = _animations[_aniName = "STAND"];
		_ani->reset();
		_ani->mirrored = !_forward && !_isOnLadder;
		_ani->position = position;
		_ani->updateImageData();
	}
	_isOnLadder = false;
}
void Player::stopMovingLeft(float collisionSize)
{
	if (isClimbing()) return;
	if (_speed.x != 0)
	{
		_speed.x = 0;
		position.x += collisionSize;
		_leftCollision = true;
	}
	elevator = nullptr;
}
void Player::stopMovingRight(float collisionSize)
{
	if (isClimbing()) return;
	if (_speed.x != 0)
	{
		_speed.x = 0;
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

	_speed.y = -force;
}
void Player::jump()
{
	if (_currPowerup == PowerupType::Catnip)
		jump(SpeedY_SuperJump);
	else
		jump(SpeedY_RegularJump);
}

bool Player::checkForHurts()
{
	return false; // TODO: change this! let CC hurt!

	if (isTakeDamage() || _damageRest > 0) return false;

	pair<Rectangle2D, int> atkRc;

	for (BaseEnemy* enemy : ActionPlane::getEnemies())
	{
		if (enemy->isAttack())
		{
			atkRc = enemy->GetAttackRect();
			if (_lastAttackRect != atkRc.first && _saveCurrRect.intersects(atkRc.first))
			{
				_lastAttackRect = atkRc.first;
				_health -= atkRc.second;
				return true;
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
	
	for (FloorSpike* s : ActionPlane::getFloorSpikes())
	{
		if ((damage = s->getDamage()) > 0)
			if (_saveCurrRect.intersects(s->GetRect()))
			{
				_health -= damage;
				return true;
			}
	}
	
	for (GooVent* g : ActionPlane::getGooVents())
	{
		if ((damage = g->getDamage()) > 0)
			if (_saveCurrRect.intersects(g->GetRect()))
			{
				_health -= damage;
				return true;
			}
	}
	
	for (Laser* l : ActionPlane::getLasers())
	{
		if ((damage = l->getDamage()) > 0)
			if (_saveCurrRect.intersects(l->GetRect()))
			{
				_health -= damage;
				return true;
			}
	}

	_lastAttackRect = {};

	return false;
}
bool Player::collectItem(Item* item)
{
	// TODO: score animation when collect treasure

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
	case Item::Treasure_Skull_Purple:
		_score += Item::getTreasureScore(type);
		_collectedTreasures[type] += 1;
		return true;

	case Item::Ammo_Deathbag:	_weaponsAmount[ClawProjectile::Types::Pistol] += 25; return true;
	case Item::Ammo_Shot:		_weaponsAmount[ClawProjectile::Types::Pistol] += 5; return true;
	case Item::Ammo_Shotbag:	_weaponsAmount[ClawProjectile::Types::Pistol] += 10; return true;
	case Item::Ammo_Magic_5:	_weaponsAmount[ClawProjectile::Types::Magic] += 5; return true;
	case Item::Ammo_Magic_10:	_weaponsAmount[ClawProjectile::Types::Magic] += 10; return true;
	case Item::Ammo_Magic_25:	_weaponsAmount[ClawProjectile::Types::Magic] += 25; return true;
	case Item::Ammo_Dynamite:	_weaponsAmount[ClawProjectile::Types::Dynamite] += 5; return true;

	case Item::Health_Level:	ADD_HEALTH(5);
	case Item::Health_10:		ADD_HEALTH(10);
	case Item::Health_15:		ADD_HEALTH(15);
	case Item::Health_25:		ADD_HEALTH(25);

	case Item::MapPiece:
	case Item::NineLivesGem:
		_finishLevel = true;
		break;
	case Item::Warp:
	case Item::BossWarp:
		// impleted as `class Warp`
		break;

		// TODO: not impleted
	case Item::Curse_Ammo:		break;
	case Item::Curse_Magic:		break;
	case Item::Curse_Health:	break;
	case Item::Curse_Life:		break;
	case Item::Curse_Treasure:	break;
	case Item::Curse_Freeze:	break;

	case Item::Powerup_Catnip_White:
	case Item::Powerup_Catnip_Red:		SET_POWERUP(Catnip);
	case Item::Powerup_Invisibility:	/* TODO: not impleted */ break;
	case Item::Powerup_Invincibility:	/* TODO: not impleted */ break;
	case Item::Powerup_FireSword:		SET_POWERUP(FireSword);
	case Item::Powerup_LightningSword:	SET_POWERUP(LightningSword);
	case Item::Powerup_IceSword:		SET_POWERUP(IceSword);
	case Item::Powerup_ExtraLife:		_lives += 1; return true;
	}

	return false;
}

bool Player::isFalling() const
{
	return _speed.y > 0 && !_isOnLadder;
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
	_speed = {};
	_saveCurrRect = GetRect();
	_powerupLeftTime = 0;
	_dialogLeftTime = 0;
	_holdAltTime = 0;
	_currPowerup = PowerupType::None;
	_lastPowderKegExplos = nullptr;
	_raisedPowderKeg = nullptr;
	_lastAttackRect = {};
	_saveCurrAttackRect = {};
	_damageRest = 0;
	_forward = true;
	_freezeTime = 0;

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
		_currPowerup = PowerupType::None;
		_powerupSparkles.clear();
		AssetsManager::setBackgroundMusic(AudioManager::BackgroundMusicType::Level, false);
	}
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
