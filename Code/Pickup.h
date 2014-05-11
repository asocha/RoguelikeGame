#pragma once

#include "PickupDef.h"
#include "Dice.h"
#include <vector>

/**
* Defines an item that the player can pick up and use in the game
*
* @author Andrew Socha
*/
class Pickup
{
	public:
		Pickup(PickupDef* pickups, int levelDifficulty);
		Pickup(PickupDef* pickups, int defNumber, int durability);
		~Pickup();

		int use();
		void unequip();
		int reduceDurability();

		int getTile() const;
		int getDurability() const;
		int getPrice() const;
		int getDefNumber() const;
		string getName() const;
		string getDescription() const;

	private:
		PickupDef pickupDef;
		int defNumber;
		int durability;
};


/**
* Chooses either a pickupDef from the pickupDef table or an actor from the actor table
* 
* @param levelDifficulty	the current game difficulty (starts at 1 and increases by 1 every floor)
* @param table				a pointer to the table we are choosing from
* @param tableSize			the size of that table
* @return					the index of the item or actor we have chosen in its respective table (0 to tableSize-1)
*/
template <class T> int pickFromTable(int levelDifficulty, T* table, int tableSize){
	vector<int> frequencies;
	int difficulty = 0;
	int frequency = 0;
	int totalFrequency = 0;

	for (int i = 0; i < tableSize; i++){
		difficulty = table[i].getDifficulty();

		//floor		maxDifficulty
		//1			5
		//2			7
		//3			8
		//4			9
		if (difficulty > levelDifficulty) frequency = levelDifficulty*25/difficulty/difficulty;
		else frequency = difficulty*25/levelDifficulty/levelDifficulty;
		//frequencies should create the most items at difficulty = levelDifficulty
		//and no items of where difficulty is far from levelDifficulty

		frequencies.push_back(frequency);
		totalFrequency += frequency;
	}

	if (!totalFrequency) return tableSize - 1; //nothing has a frequency should mean that the difficulty is excessively high
											   //so use all the highest difficulty monsters/items

	int item = rand() % totalFrequency;
	for (int i = 0; i < tableSize; i++){
		item -= frequencies[i];
		if (item < 0) return i;
	}

	return -1; //should never be reached
}