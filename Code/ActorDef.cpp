#include "ActorDef.h"

ActorDef::ActorDef(){
}

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

ActorDef::~ActorDef(){
}