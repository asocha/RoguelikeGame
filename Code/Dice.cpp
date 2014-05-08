#include "Dice.h"

Dice::Dice(){
	sides = 0;
	amount = 0;
	bonus = 0;
}

Dice::Dice(int sides, int amount, int bonus){
	this->sides = sides;
	this->amount = amount;
	this->bonus = bonus;
}

int Dice::roll() const{
	int total = bonus;
	for (int i = 0; i < amount; i++){
		total += (rand() % sides) + 1;
	}
	return total;
}

Dice::~Dice(){
}