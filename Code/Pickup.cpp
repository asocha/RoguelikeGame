#include "Pickup.h"

/**
* Create a new pickup of a random type
*
* @param pickups			the list of pickup types for the game
* @param levelDifficulty	the difficulty that we are creating an item for (generally the difficulty of the current floor)
*/
Pickup::Pickup(PickupDef* pickups, int levelDifficulty){
	defNumber = pickFromTable(levelDifficulty, pickups, 15);
	if (defNumber == -1) defNumber = 6; //make gold if we weren't able to choose an item
	pickupDef = pickups[defNumber];

	durability = pickupDef.getDurability();
}

/**
* Create a specific pickup, such as chests or items loaded from saved games
*
* @param pickups		the list of pickup types for the game
* @param defNumber		the specific index of the item type in the pickups array
* @param durability		the current durability of the item (-1 for unequippable items)
*/
Pickup::Pickup(PickupDef* pickups, int defNumber, int durability){
	if (defNumber < 0 || defNumber > 16) defNumber = 6; //shouldn't happen but make gold if defNumber is bad
	this->defNumber = defNumber;
	pickupDef = pickups[defNumber];
	this->durability = durability;
}

/**
* Activate an item and remove it forever if it is a consumable
*
* @return	the slot in the equipped array where this item will be equipped (-1 if not equippable)
*/
int Pickup::use(){
	pickupDef.use();
	if (durability == -1){
		delete this;
		return -1;
	}
	return pickupDef.getEquipSlot();
}

/** Unequip an item */
void Pickup::unequip(){
	pickupDef.unequip();
}

/**
* Reduce the durability of an item by 1
*
* @return	the remaining durability
*/
int Pickup::reduceDurability(){
	return --durability;
}

int Pickup::getTile() const{
	return pickupDef.getTile();
}

int Pickup::getDurability() const{
	return durability;
}

/**
* Get the total price of an item
*
* @return	the price
*/
int Pickup::getPrice() const{
	if (durability == -1) return pickupDef.getBasePrice();
	return pickupDef.getBasePrice() * durability;
}

int Pickup::getDefNumber() const{
	return defNumber;
}

string Pickup::getName() const{
	return pickupDef.getName();
}

string Pickup::getDescription() const{
	return pickupDef.getDescription();
}

Pickup::~Pickup(){
}
