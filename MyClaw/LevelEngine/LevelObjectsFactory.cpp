#include "LevelObjectsFactory.h"

#include "Objects/PowderKeg.h"
#include "Objects/Checkpoint.h"
#include "Objects/Elevators/Elevator.h"
#include "Objects/Crate.h"
#include "Objects/SoundTrigger.h"
#include "Objects/AmbientSound.h"
#include "Objects/Pegs.h"
#include "Objects/DoNothing.h"
#include "Objects/Rope.h"
#include "Objects/SpringBoard.h"
#include "Objects/Statue.h"
#include "Objects/GroundBlower.h"
#include "Objects/ConveyorBelt.h"
#include "Objects/Cannons/Cannon.h"
#include "Objects/Cannons/TowerCannon.h"
#include "Objects/Cannons/SkullCannon.h"
#include "Objects/GooVent.h"
#include "Objects/CrabNest.h"
#include "Objects/TProjectilesShooter.h"
#include "Objects/FloorSpike.h"
#include "Objects/Laser.h"
#include "Objects/Stalactite.h"
#include "Objects/LavaMouth.h"
#include "Objects/LavaHand.h"
#include "Enemies/Officer.h"
#include "Enemies/Soldier.h"
#include "Enemies/Rat.h"
#include "Enemies/PunkRat.h"
#include "Enemies/RobberThief.h"
#include "Enemies/CutThroat.h"
#include "Enemies/Seagull.h"
#include "Enemies/Bosses/LeRauxe.h"
#include "Enemies/Bosses/Katherine.h"
#include "Enemies/TownGuard.h"
#include "Enemies/Bosses/Wolvington.h"
#include "Enemies/RedTailPirate.h"
#include "Enemies/BearSailor.h"
#include "Enemies/CrazyHook.h"
#include "Enemies/HermitCrab.h"
#include "Enemies/PegLeg.h"
#include "Enemies/Mercat.h"
#include "Enemies/Siren.h"
#include "Enemies/Fish.h"
#include "Enemies/Bosses/Gabriel/Gabriel.h"
#include "Enemies/Bosses/Marrow/Marrow.h"
#include "Enemies/Chameleon.h"
#include "Enemies/Bosses/Aquatis/Aquatis.h"
#include "Enemies/Bosses/Aquatis/AquatisCrack.h"
#include "Enemies/Bosses/Aquatis/AquatisTentacle.h"
#include "Enemies/Bosses/Aquatis/AquatisStalactite.h"
#include "Enemies/Bosses/RedTail.h"
#include "Enemies/TigerGuard.h"
#include "Enemies/Bosses/LordOmar/LordOmar.h"


#ifdef _DEBUG
#define NO_ENEMIES
#define NO_OBSTACLES
#endif


BasePlaneObject* LevelObjectsFactory::createObject(const WwdObject& obj) {
	/* Objects */
	if (obj.logic == "FrontCandy" || obj.logic == "FrontAniCandy" ||
		obj.logic == "BehindCandy" || obj.logic == "BehindAniCandy" ||
		obj.logic == "DoNothing" || obj.logic == "DoNothingNormal" ||
		obj.logic == "AniCycle" || obj.logic == "AniCycleNormal" ||
		obj.logic == "Sign" || obj.logic == "GooCoverup")
	{
		return DBG_NEW DoNothing(obj);
	}
	else if (obj.logic == "GlobalAmbientSound")
	{
		return DBG_NEW GlobalAmbientSound(obj, _wwd->levelNumber);
	}
	else if (obj.logic == "AmbientSound" || obj.logic == "SpotAmbientSound")
	{
		return DBG_NEW AmbientSound(obj);
	}
	else if (contains(obj.logic, "SoundTrigger"))
	{
		return DBG_NEW SoundTrigger(obj);
	}
	else if (obj.logic == "BehindCrate" || obj.logic == "FrontCrate")
	{
		return DBG_NEW Crate(obj);
	}
	else if (obj.logic == "FrontStackedCrates" || obj.logic == "BackStackedCrates")
	{
		return DBG_NEW StackedCrates(obj);
	}
	else if (obj.logic == "FrontStatue" || obj.logic == "BehindStatue")
	{
		return DBG_NEW Statue(obj);
	}
	else if (obj.logic == "PowderKeg")
	{
		return DBG_NEW PowderKeg(obj);
	}
	else if (endsWith(obj.logic, "Elevator"))
	{
		return Elevator::create(obj, _wwd->levelNumber);
	}
	else if (endsWith(obj.logic, "Checkpoint"))
	{
		return DBG_NEW Checkpoint(obj, _wwd->levelNumber);
	}
	else if (obj.logic == "StartSteppingStone")
	{
		return DBG_NEW StartSteppingStone(obj);
	}
	else if (startsWith(obj.logic, "TogglePeg") || startsWith(obj.logic, "SteppingStone"))
	{
		return DBG_NEW TogglePeg(obj);
	}
	else if (obj.logic == "CrumblingPeg")
	{
		return DBG_NEW CrumblingPeg(obj);
	}
	else if (obj.logic == "BreakPlank")
	{
		int idx = -1;
		if (_wwd->levelNumber == 5) idx = 509;
		else if (_wwd->levelNumber == 11) idx = 39;
		//else throw Exception("invalid level");
		WwdTileDescription& tileDesc = _wwd->tileDescriptions[idx];
		float topOffset = (float)tileDesc.rect.top;
		return DBG_NEW BreakPlank(obj, topOffset);
	}
	else if (obj.logic == "TreasurePowerup" || obj.logic == "GlitterlessPowerup"
		|| obj.logic == "SpecialPowerup" || obj.logic == "AmmoPowerup"
		|| obj.logic == "BossWarp" || obj.logic == "HealthPowerup"
		|| obj.logic == "EndOfLevelPowerup" || obj.logic == "MagicPowerup"
		/*|| obj.logic == "CursePowerup"*/)
	{
		return Item::getItem(obj, true);
	}
	else if (obj.logic == "AniRope")
	{
		return DBG_NEW Rope(obj);
	}
	else if (obj.logic == "SpringBoard" || obj.logic == "WaterRock")
	{
		return DBG_NEW SpringBoard(obj, _wwd->levelNumber);
	}
	else if (obj.logic == "GroundBlower")
	{
		return DBG_NEW GroundBlower(obj);
	}
	else if (obj.logic == "ConveyorBelt")
	{
		return DBG_NEW ConveyorBelt(obj);
	}

	/* Enemies */

#ifndef NO_ENEMIES
	else if (obj.logic == "CrabNest")
	{
		return DBG_NEW CrabNest(obj);
	}
	else if (obj.logic == "Officer")
	{
		return DBG_NEW Officer(obj);
	}
	else if (obj.logic == "Soldier")
	{
		return DBG_NEW Soldier(obj);
	}
	else if (obj.logic == "Rat")
	{
		return DBG_NEW Rat(obj);
	}
	else if (obj.logic == "PunkRat")
	{
		return DBG_NEW PunkRat(obj);
	}
	else if (obj.logic == "RobberThief")
	{
		return DBG_NEW RobberThief(obj);
	}
	else if (obj.logic == "CutThroat")
	{
		return DBG_NEW CutThroat(obj);
	}
	else if (obj.logic == "Seagull")
	{
		return DBG_NEW Seagull(obj);
	}
	else if (obj.logic == "TownGuard1" || obj.logic == "TownGuard2")
	{
		return DBG_NEW TownGuard(obj);
	}
	else if (obj.logic == "RedTailPirate")
	{
		return DBG_NEW RedTailPirate(obj);
	}
	else if (obj.logic == "BearSailor")
	{
		return DBG_NEW BearSailor(obj);
	}
	else if (obj.logic == "CrazyHook")
	{
		return DBG_NEW CrazyHook(obj);
	}
	else if (obj.logic == "HermitCrab")
	{
		return DBG_NEW HermitCrab(obj);
	}
	else if (obj.logic == "PegLeg")
	{
		return DBG_NEW PegLeg(obj);
	}
	else if (obj.logic == "Mercat")
	{
		return DBG_NEW Mercat(obj);
	}
	else if (obj.logic == "Siren")
	{
		return DBG_NEW Siren(obj);
	}
	else if (obj.logic == "Fish")
	{
		return DBG_NEW Fish(obj);
	}
	else if (obj.logic == "Chameleon")
	{
		return DBG_NEW Chameleon(obj);
	}
	else if (obj.logic == "TigerGuard")
	{
		return DBG_NEW TigerGuard(obj);
	}
#endif

	/* Obstacles */

#ifndef NO_OBSTACLES
	else if (obj.logic == "TowerCannonLeft" || obj.logic == "TowerCannonRight")
	{
		return DBG_NEW TowerCannon(obj);
	}
	else if (obj.logic == "SkullCannon")
	{
		return DBG_NEW SkullCannon(obj);
	}
	else if (obj.logic == "TProjectile")
	{
		return DBG_NEW TProjectilesShooter(obj, _wwd->levelNumber);
	}
	else if (obj.logic == "GooVent")
	{
		return DBG_NEW GooVent(obj);
	}
	else if (startsWith(obj.logic, "FloorSpike"))
	{
		return DBG_NEW FloorSpike(obj);
	}
	else if (startsWith(obj.logic, "SawBlade"))
	{
		return DBG_NEW SawBlade(obj);
	}
	else if (obj.logic == "LavaGeyser")
	{
		return DBG_NEW LavaGeyser(obj);
	}
	else if (obj.logic == "Laser")
	{
		return DBG_NEW Laser(obj);
	}
	else if (obj.logic == "LavaMouth")
	{
		return DBG_NEW LavaMouth(obj);
	}
	else if (obj.logic == "Stalactite")
	{
		return DBG_NEW Stalactite(obj);
	}
	else if (obj.logic == "LavaHand")
	{
		return DBG_NEW LavaHand(obj);
	}
#endif

	/* Bosses and their objects */
#pragma region Bosses
	else if (obj.logic == "Raux")
	{
		return DBG_NEW LeRauxe(obj);
	}
	else if (obj.logic == "Katherine")
	{
		return DBG_NEW Katherine(obj);
	}
	else if (obj.logic == "Wolvington")
	{
		return DBG_NEW Wolvington(obj);
	}
	else if (obj.logic == "Gabriel")
	{
		return DBG_NEW Gabriel(obj);
	}
	else if (obj.logic == "GabrielCannon")
	{
		return DBG_NEW GabrielCannon(obj);
	}
	else if (obj.logic == "CannonSwitch")
	{
		return DBG_NEW GabrielCannonSwitch(obj);
	}
	else if (obj.logic == "CannonButton")
	{
		return DBG_NEW GabrielCannonButton(obj);
	}
	else if (obj.logic == "Marrow")
	{
		return DBG_NEW Marrow(obj);
	}
	else if (obj.logic == "Parrot")
	{
		return DBG_NEW MarrowParrot(obj);
	}
	else if (obj.logic == "MarrowFloor")
	{
		return DBG_NEW MarrowFloor(obj);
	}
	else if (obj.logic == "Aquatis")
	{
		return Aquatis::createAquatis(obj);
	}
	else if (obj.logic == "Tentacle")
	{
		return Aquatis::getInstance()->addTentacle(obj);
	}
	else if (obj.logic == "AquatisCrack")
	{
		return DBG_NEW AquatisCrack(obj);
	}
	else if (obj.logic == "AquatisDynamite")
	{
		return Item::getItem(obj, true);
	}
	else if (obj.logic == "AquatisStalactite")
	{
		return Aquatis::getInstance()->addStalactite(obj);
	}
	else if (obj.logic == "RedTail")
	{
		return DBG_NEW RedTail(obj);
	}
	else if (obj.logic == "RedTailSpikes")
	{
		return DBG_NEW RedTailSpikes(obj);
	}
	else if (obj.logic == "RedTailWind")
	{
		return DBG_NEW RedTailWind(obj);
	}
	else if (obj.logic == "Omar")
	{
		return DBG_NEW LordOmar(obj);
	}
#pragma endregion

	//	throw Exception("TODO: logic=" + obj.logic);

	return nullptr;
}