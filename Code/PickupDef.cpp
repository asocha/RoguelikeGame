#include "PickupDef.h"

/** Default Constructor */
PickupDef::PickupDef(){
}

/**
* Creates an item definition, which includes all the information on a specific item type that is the same for all instances of that item
*
* @param tile			the tile number of the tile used to render that item
* @param name			the item's name
* @param description	the item's description
* @param equipSlot		which slot that item would be equipped into (0-2, or -1 if not equippable)
* @param durability		the dice that determine the item's initial durability
* @param basePrice		the total price, or the price per durability for items with durability, that the shop sells this item for
* @param difficulty		the relative power of this type of item (1-9)
* @param onUse			a function that implements what happens when this item is used or equipped (may be empty for unusable items, such as gold)
* @param onUnequip		a function that implements what happens when this item is unequipped (may be empty if not equippable)
*/
PickupDef::PickupDef(int tile, string name, string description, int equipSlot, Dice durability, double basePrice, int difficulty, function<void ()> onUse, function<void ()> onUnequip){
	this->tile = tile;
	this->name = name;
	this->description = description;
	this->equipSlot = equipSlot;
	this->durability = durability;
	this->basePrice = basePrice;
	this->difficulty = difficulty;
	this->onUse = onUse;
	this->onUnequip = onUnequip;
}

/** Activate an item by calling its onUse function */
void PickupDef::use(){
	onUse();
}

/** Unequip an item by calling its onUnequip function */
void PickupDef::unequip(){
	onUnequip();
}

int PickupDef::getTile() const{
	return tile;
}

int PickupDef::getEquipSlot() const{
	return equipSlot;
}

string PickupDef::getName() const{
	return name;
}

string PickupDef::getDescription() const{
	return description;
}

/**
* Roll an item type's durability dice to get an item's initial durability
*
* @return	the initial durability for an item
*/
int PickupDef::getDurability() const{
	return durability.roll();
}

double PickupDef::getBasePrice() const{
	return basePrice;
}

int PickupDef::getDifficulty() const{
	return difficulty;
}

PickupDef::~PickupDef(){
}