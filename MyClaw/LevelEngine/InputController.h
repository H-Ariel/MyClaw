#pragma once


class Player;

class InputController
{
public:
	InputController(Player* player);

	// used to move player
	void keyUp(int key);
	void keyDown(int key);


private:
	Player* player;
};
