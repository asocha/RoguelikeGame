#include "ActorDef.h"

/** Default Constuctor */
ActorDef::ActorDef(){
}

/**
* Creates an actor definition, which includes all the information on a specific actor type that is the same for all instances of that actor
*
* @param tile			the tile number of the tile used to render that actor
* @param name			the actor's name
* @param difficulty		the relative power of this type of actor (1-9)
* @param maxHealth		the dice used to determine the maximum health of the actor
* @param attack			the dice used to determine the damage of the actor each time it attacks
* @param defense		the dice used to determine the defense of the actor each time it is attacked
*/
ActorDef::ActorDef(int tile, string name, int difficulty, Dice maxHealth, Dice attack, Dice defense){
	this->tile = tile;
	this->name = name;
	this->difficulty = difficulty;
	this->maxHealth = maxHealth;
	this->attack = attack;
	this->defense = defense;
}

int ActorDef::getTile() const{
	return tile;
}

int ActorDef::getDifficulty() const{
	return difficulty;
}

int ActorDef::getMaxHealth() const{
	return maxHealth.roll();
}

int ActorDef::getAttack() const{
	return attack.roll();
}

int ActorDef::getDefense() const{
	return defense.roll();
}

string ActorDef::getName() const{
	return name;
}

/** Destructor */
ActorDef::~ActorDef(){
}