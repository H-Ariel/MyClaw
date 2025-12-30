#pragma once

#include "BasePlaneObject.h"
#include "OneTimeAnimation.h"


class Item : public BaseDynamicPlaneObject
{
public:
	enum Type : int8_t
	{
		None = -1,
		Default = 0,
		Treasure_Goldbars,
		Treasure_Rings_Red,
		Treasure_Rings_Green,
		Treasure_Rings_Blue,
		Treasure_Rings_Purple,
		Treasure_Necklace,
		Treasure_Crosses_Red,
		Treasure_Crosses_Green,
		Treasure_Crosses_Blue,
		Treasure_Crosses_Purple,
		Treasure_Scepters_Red,
		Treasure_Scepters_Green,
		Treasure_Scepters_Blue,
		Treasure_Scepters_Purple,
		Treasure_Geckos_Red,
		Treasure_Geckos_Green,
		Treasure_Geckos_Blue,
		Treasure_Geckos_Purple,
		Ammo_Deathbag,
		Ammo_Shot,
		Ammo_Shotbag,
		Powerup_Catnip_White,
		Powerup_Catnip_Red,
		Health_Level,
		Health_25,
		Health_10,
		Health_15,
		Ammo_Magic_5,
		Ammo_Magic_10,
		Ammo_Magic_25,
		MapPiece,
		Warp,
		Treasure_Coins,
		Ammo_Dynamite,
		Curse_Ammo,
		Curse_Magic,
		Curse_Health,
		Curse_Life,
		Curse_Treasure,
		Curse_Freeze,
		Treasure_Chalices_Red,
		Treasure_Chalices_Green,
		Treasure_Chalices_Blue,
		Treasure_Chalices_Purple,
		Treasure_Crowns_Red,
		Treasure_Crowns_Green,
		Treasure_Crowns_Blue,
		Treasure_Crowns_Purple,
		Treasure_Skull_Red,
		Treasure_Skull_Green,
		Treasure_Skull_Blue,
		Treasure_Skull_Purple,
		Powerup_Invisibility,
		Powerup_Invincibility,
		Powerup_ExtraLife,
		Powerup_FireSword,
		Powerup_LightningSword,
		Powerup_IceSword,
		BossWarp,
		Level2_Gem,

		NineLivesGem = 100
	};

	~Item();

	void Logic(uint32_t elapsedTime) override;
	void Draw() override;

	void stopFalling(float collisionSize) override;

	Type getType() const { return _type; }
	int getDuration() const { return _duration; }
	OneTimeAnimation* getTreasureScoreAnimation() const;
	void playItemSound() const;
	
	static int getTreasureScore(Type treasure);

	static Item* getItem(const WwdObject& obj, bool isFromMap, int8_t type);
	static Item* getItem(const WwdObject& obj, bool isFromMap);
	static Item* getItem(const WwdObject& obj, int8_t type);
	static Item* getItem(const WwdObject& obj);

protected:
	Item(const WwdObject& obj, int8_t type, bool isFromMap);

	const Type _type;

private:
	shared_ptr<UIAnimation> _glitterAnimation;
	int _duration; // (in milliseconds). used in Catnip, Fire-Sword, etc.
	Timer _respawnTimer; // (in milliseconds)
	bool _respawning;


	static const initializer_list<Item::Type> UpdateFramesTypes;
};


class Warp : public Item
{
public:
	Warp(const WwdObject& obj, int8_t type);
	void Logic(uint32_t elapsedTime) override;

	bool isBossWarp() const { return _type == Type::BossWarp; }
	bool isActivated() const { return _activateWarp; }
	void deactivateWarp() { _activateWarp = false; }
	D2D1_POINT_2F getDestination() const { return _destination; }

private:
	const D2D1_POINT_2F _destination;
	const bool _oneTimeWarp; // flag to determine whether to delete an object after using it
	bool _activateWarp;
};
