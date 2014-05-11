#pragma once

#include <windows.h>

/**
* Simulates dice for random number generation.
*
* @author Andrew Socha
*/
class Dice{
	public:
		Dice();
		Dice(int sides, int amount, int bonus);
		~Dice();
		int roll() const;

		int sides; //sides of the dice
		int amount; //number of dice rolled
		int bonus; //additional constant added to the result
};