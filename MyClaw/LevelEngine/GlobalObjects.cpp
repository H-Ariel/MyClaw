#include "GlobalObjects.h"
#include "Objects/Player/Player.h"
#include "ActionPlane.h"
#include "ClawLevelEngine.h"


shared_ptr<Player> GlobalObjects::player;
shared_ptr<PhysicsManager> GlobalObjects::physics;
shared_ptr<CheatsManager> GlobalObjects::cheats;
ClawLevelEngine* GlobalObjects::clawLevelEngine = nullptr;


D2D1_POINT_2F& GlobalObjects::playerPosition() { return player->position; }
Rectangle2D GlobalObjects::getPlayerRect() { return player->GetRect(); }
pair<Rectangle2D, int> GlobalObjects::getPlayerAttackRect() { return player->GetAttackRect(); }
bool GlobalObjects::isPlayerTakeDamage() { return player->isTakeDamage(); }
bool GlobalObjects::isPlayerDuck() { return player->isDuck(); }
bool GlobalObjects::isPlayerAttack() { return player->isAttack(); }
bool GlobalObjects::isPlayerGhost() { return player->isGhost(); }
bool GlobalObjects::isPlayerSqueezed() { return player->isSqueezed(); }
bool GlobalObjects::isPlayerFreeze() { return player->isFreeze(); }


void GlobalObjects::addObjectToActionPlane(BasePlaneObject* obj) { clawLevelEngine->getActionPlane()->addPlaneObject(obj); }

const vector<PowderKeg*>& GlobalObjects::getActionPlanePowderKegs() { return clawLevelEngine->getActionPlane()->getPowderKegs(); }
const vector<BaseEnemy*>& GlobalObjects::getActionPlaneEnemies() { return clawLevelEngine->getActionPlane()->getEnemies(); }
const vector<Projectile*>& GlobalObjects::getActionPlaneProjectiles() { return clawLevelEngine->getActionPlane()->getProjectiles(); }
const vector<BaseDamageObject*>& GlobalObjects::getActionPlaneDamageObjects() { return clawLevelEngine->getActionPlane()->getDamageObjects(); }

void GlobalObjects::addTimer(Timer* timer) { clawLevelEngine->addTimer(timer); }
