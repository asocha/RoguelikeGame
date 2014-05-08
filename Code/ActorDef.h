//defines an Actor type that all Actors of that type share
#pragma once

#include <string>
#include "Dice.h"

using namespace std;

class ActorDef{
	public:
		ActorDef();
		ActorDef(int tile, string name, int difficulty, Dice maxHealth, Dice attack, Dice defense);
		~ActorDef();

		int getTile() const;
		int getDifficulty() const;
		string getName() const;

		//the following functions roll dice and return a value
		int getMaxHealth() const;
		int getAttack() const;
		int getDefense() const;

		Dice maxHealth;
		Dice attack;
		Dice defense;

	private:
		int tile;
		string name;
		int difficulty;
};