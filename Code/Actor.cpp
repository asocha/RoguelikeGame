#include "Actor.h"

//list of monster types (remember dice are 'number of sides, number of dice, bonus')
//tile, "name", difficulty, health, attack, defense
const ActorDef dragon(			0xE7, "Dragon",				8,	Dice(6, 7, 10),	Dice(3, 4, 4), Dice(4, 2, 3)); //+2 health dice
const ActorDef troll(			0xAE, "Troll",				7,	Dice(6, 5, 10),	Dice(3, 4, 4), Dice(3, 2, 2)); //+2 att dice
const ActorDef goblinWarrior(	0xCE, "Goblin Warrior",		6,	Dice(4, 3, 5),	Dice(3, 2, 4), Dice(3, 2, 2)); //+1 def
const ActorDef serpent(			0xE4, "Serpent",			5,	Dice(3, 3, 3),	Dice(3, 2, 4), Dice(3, 2, 1));
const ActorDef mudMonster(		0xB6, "Mud Monster",		4,	Dice(3, 3, 3),	Dice(3, 1, 3), Dice(4, 2, 1)); //-1 def just for this
const ActorDef lizardmanWarrior(0xBE, "Lizardman Warrior",	3,	Dice(4, 3, 5),	Dice(3, 2, 4), Dice(5, 1, 1));
const ActorDef eye(				0x9D, "Eye",				2,	Dice(3, 3, 3),	Dice(3, 2, 4), Dice(5, 1, 1));
const ActorDef lizardman(		0xBC, "Lizardman",			1,	Dice(3, 3, 3),	Dice(3, 1, 3), Dice(5, 1, 1));
const ActorDef goblin(			0xC9, "Goblin",				2,	Dice(4, 3, 5),	Dice(3, 1, 3), Dice(5, 1, 1));
const ActorDef shadowcaster(	0xEA, "Shadowcaster",		3,	Dice(3, 3, 3),	Dice(5, 3, 4), Dice(5, 1, 1));
const ActorDef slimeMonster(	0xC2, "Slime Monster",		4,	Dice(4, 3, 5),	Dice(3, 1, 3), Dice(3, 2, 1));
const ActorDef hawk(			0x8B, "Hawk",				5,	Dice(3, 3, 3),	Dice(3, 2, 4), Dice(4, 2, 1)); //-1 def just for this
const ActorDef spectre(			0xEB, "Spectre",			6,	Dice(6, 5, 10),	Dice(3, 2, 4), Dice(5, 1, 2)); //+1 def
const ActorDef ooze(			0x88, "Ooze",				7,	Dice(6, 5, 10),	Dice(3, 3, 3), Dice(4, 2, 2)); //+2 att dice, -1 def just for this
const ActorDef beholder(		0xA1, "Beholder",			8,	Dice(4, 5, 5),	Dice(5, 5, 4), Dice(3, 2, 2)); //+2 health dice
const ActorDef death(			0x39D,"Lingering Death",	9,	Dice(7, 7, 10), Dice(5, 5, 5), Dice(4, 3, 4)); //+even more

const ActorDef monsters[16] = {dragon, troll, goblinWarrior, serpent, mudMonster, lizardmanWarrior,
	eye, lizardman, goblin, shadowcaster, slimeMonster, hawk, spectre, ooze, beholder, death};

/** Default Constructor: used by the player actor */
Actor::Actor(){
	//stats set in Player.cpp
}

/**
* Creates a new monster for the game
*
* @param location			the initial location of the monster
* @param levelDifficulty	the difficulty that we are creating this monster for (typically the current floor's difficulty)
*/
Actor::Actor(Coordint location, int levelDifficulty){
	this->location = location;
	this->home = location;
	chaseCount = 0;

	//create a monster of a random type
	defNumber = pickFromTable(levelDifficulty, monsters, 16);
	if (defNumber == -1) defNumber = 7;
	actorDef = monsters[defNumber];

	currentHealth = actorDef.getMaxHealth();
}

/**
* Creates a monster that is loaded from a saved game
*
* @param defNumber		the index of the ActorDef for this monster type
* @param chaseCount		how many steps this monster has taken chasing the player
* @param health			the current health of the monster
*/
Actor::Actor(int defNumber, int chaseCount, int health){
	this->chaseCount = chaseCount;
	this->defNumber = defNumber;
	if (defNumber > -1 && defNumber < 16) actorDef = monsters[defNumber];

	currentHealth = health;
}

int Actor::getTile() const{
	return actorDef.getTile();
}

int Actor::getTurn(){
	return turn;
}

int Actor::getChaseCount() const{
	return chaseCount;
}

int Actor::getDefNumber() const{
	return defNumber;
}

int Actor::getHealth() const{
	return currentHealth;
}

string Actor::getName() const{
	return actorDef.getName();
}

int Actor::getDifficulty() const{
	return actorDef.getDifficulty();
}

/** Increase a monster's chase count by 1 */
void Actor::incrementChaseCount(){
	chaseCount++;
}

/**
* This actor attacks another actor, dealing damage to it
*
* @param target		a referense to the actor we are attacking
* @param cell		the cell where the actor we are attacking is located
* @param world		the current game world
*/
void Actor::attack(Actor& target, Cell& cell, World& world){
	tl_play("attack.sfs");

	int damage = actorDef.getAttack() - target.actorDef.getDefense();
	if (damage < 0) damage = 0;
	
	target.currentHealth -= damage;

	if (actorDef.getName() == "Player"){
		if (damage) log->print("You dealt " + std::to_string(long double(damage)) + " damage to " + log->addArticle(target.actorDef.getName(), false) + ".", yellow, turn); //player attacks monster
		else log->print("You attacked " + log->addArticle(target.actorDef.getName(), false) + " and missed.", yellow, turn); //player misses monster
	}
	else{
		if (damage) log->print(log->addArticle(actorDef.getName(), true) + " dealt " + std::to_string(long double(damage)) + " damage to you.", yellow, turn); //monster attacks player
		else log->print(log->addArticle(actorDef.getName(), true) + " attacked you and missed.", yellow, turn); //monster misses player
		wasAttacked++;
	}
	
	if (target.currentHealth < 1){
		killedMonster = target.getDifficulty();
		target.die(cell, world);
	}
}

/**
* A monster dies and is removed from the world
*
* @param cell	the cell where the monster is located
* @param world	the current game world
*/
void Actor::die(Cell& cell, World& world){
	tl_play("monsterDeath.sfs");

	cell.setActor(0);
	log->print("You have slain " + log->addArticle(actorDef.getName(), false) + ".", yellow, turn);
	delete this;
}

Coordint& Actor::getLocation(){
	return location;
}

Coordint& Actor::getHome(){
	return home;
}

/**
* A monster gets a new home, resetting its chase count
*
* @param home	the location of the monster's new home
*/
void Actor::setHome(Coordint& home){
	this->home = home;
	chaseCount = 0;
}

Actor::~Actor(){
}