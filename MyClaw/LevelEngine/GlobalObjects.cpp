#include "GlobalObjects.h"
#include "Objects/Player/Player.h"
#include "ActionPlane.h"
#include "ClawLevelEngine.h"


shared_ptr<Player> GlobalObjects::player;
shared_ptr<PhysicsManager> GlobalObjects::physics;
shared_ptr<CheatsManager> GlobalObjects::cheats;
ActionPlane* GlobalObjects::actionPlane = nullptr;
ClawLevelEngine* GlobalObjects::clawLevelEngine = nullptr;


D2D1_POINT_2F& GlobalObjects::getPlayerPosition() { return player->position; }
Rectangle2D GlobalObjects::getPlayerRect() { return player->GetRect(); }
pair<Rectangle2D, int> GlobalObjects::getPlayerAttackRect() { return player->GetAttackRect(); }
bool GlobalObjects::isPlayerTakeDamage() { return player->isTakeDamage(); }
bool GlobalObjects::isPlayerDuck() { return player->isDuck(); }
bool GlobalObjects::isPlayerAttack() { return player->isAttack(); }
bool GlobalObjects::isPlayerGhost() { return player->isGhost(); }
bool GlobalObjects::isPlayerSqueezed() { return player->isSqueezed(); }
bool GlobalObjects::isPlayerFreeze() { return player->isFreeze(); }
void GlobalObjects::squeezePlayer(D2D1_POINT_2F pos, bool mirror) { player->squeeze(pos, mirror); }
void GlobalObjects::unsqueezePlayer() { player->unsqueeze(); }


void GlobalObjects::addObjectToActionPlane(BasePlaneObject* obj) { actionPlane->addPlaneObject(obj); }

const vector<PowderKeg*>& GlobalObjects::getActionPlanePowderKegs() { return actionPlane->getPowderKegs(); }
const vector<BaseEnemy*>& GlobalObjects::getActionPlaneEnemies() { return actionPlane->getEnemies(); }
const vector<Projectile*>& GlobalObjects::getActionPlaneProjectiles() { return actionPlane->getProjectiles(); }
const vector<BaseDamageObject*>& GlobalObjects::getActionPlaneDamageObjects() { return actionPlane->getDamageObjects(); }

void GlobalObjects::addTimer(Timer* timer) { clawLevelEngine->addTimer(timer); }
