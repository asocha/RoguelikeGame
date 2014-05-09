#include "Dice.h"

/** Default Constuctor */
Dice::Dice(){
	sides = 0;
	amount = 0;
	bonus = 0;
}

/**
* Create some Dice
*
* @param sides	the number of sides each die will have
* @param amount	the number of dice we are creating
* @param bonus	a bonus amount to add to the total dice roll everytime we roll them
*/
Dice::Dice(int sides, int amount, int bonus){
	this->sides = sides;
	this->amount = amount;
	this->bonus = bonus;
}

/**
* Roll the dice
*
* @return	the total dice roll
*/
int Dice::roll() const{
	int total = bonus;
	for (int i = 0; i < amount; i++){
		total += (rand() % sides) + 1;
	}
	return total;
}

/** Destructor */
Dice::~Dice(){
}