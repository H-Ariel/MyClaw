#pragma once

#include "../BaseBoss.h"


class AquatisTentacle;
class AquatisStalactite;

class Aquatis : public BaseBoss
{
public:
	static Aquatis* createAquatis(const WwdObject& obj);
	static Aquatis* getInstance() { return _Aquatis; }

	~Aquatis();

	void Logic(uint32_t elapsedTime) override;
	pair<Rectangle2D, int> GetAttackRect() override;
	bool checkForHurts() override;

	void activateNextStalactite();

	AquatisTentacle* addTentacle(const WwdObject& obj);
	AquatisStalactite* addStalactite(const WwdObject& obj);


private:
	Aquatis(const WwdObject& obj);

	static constexpr int STALACTITES_COUNT = 5;

	vector<AquatisTentacle*> tentaclesList; // list of tentacles that can hurt CC
	AquatisStalactite* stalactitesList[STALACTITES_COUNT]; // list of stalactites that can hurt Aquatis
	int currStalactiteIdx;

	static Aquatis* _Aquatis;
};
