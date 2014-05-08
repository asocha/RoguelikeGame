//Defines an Actor, which can be a monster, player, or other character in the world.
#pragma once

#include "Coord.h"
#include "Dice.h"
#include "ActorDef.h"
#include "Log.h"
#include "Cell.h"
#include <cmath>
#include "Pickup.h"
class World;

class Actor{
	public:
		Actor();
		Actor(Coordint location, int levelDifficulty);
		Actor(int defNumber, int chaseCount, int health);
		~Actor();

		void attack(Actor& target, Cell& cell, World& world);
		void incrementChaseCount();

		int getTile() const;
		static int getTurn();
		int getChaseCount() const;
		string getName() const;
		int getDifficulty() const;
		int getDefNumber() const;
		int getHealth() const;
		Coordint& getLocation();
		Coordint& getHome();

		void setHome(Coordint& home);

	protected:
		Coordint location;
		int currentHealth;
		static int wasAttacked; //used to tell the Player class how many times it was attacked this turn
		static int killedMonster; //used to tell the Player class if it killed a monster (and the difficulty of that monster)
		static int turn;
		ActorDef actorDef;
		int defNumber;
		static Log* log;

	private:
		virtual void die(Cell& cell, World& world);
		Coordint home;
		int chaseCount;
};