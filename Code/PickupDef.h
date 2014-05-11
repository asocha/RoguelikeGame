#pragma once

#include <string>
#include <functional>
#include "Dice.h"
class World;

using namespace std;

/**
* Defines properties of pickups that are the same for all pickups of that type
*
* @author Andrew Socha
*/
class PickupDef
{
	public:
		PickupDef();
		PickupDef(int tile, string name, string description, int equipSlot, Dice durability, double basePrice, int difficulty, function<void ()> onUse, function<void ()> onUnequip);
		~PickupDef();

		void use();
		void unequip();

		int getTile() const;
		int getEquipSlot() const;
		string getName() const;
		string getDescription() const;
		int getDurability() const; //rolls the durability dice
		double getBasePrice() const;
		int getDifficulty() const;

	private:
		int tile;
		string name;
		string description;
		int equipSlot;
		Dice durability;
		double basePrice;
		int difficulty;
		function<void ()> onUse; //function that is called when the item is used
		function<void ()> onUnequip;
};

