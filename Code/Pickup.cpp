#include "Pickup.h"

//create a new pickup
Pickup::Pickup(PickupDef* pickups, int levelDifficulty){
	defNumber = pickFromTable(levelDifficulty, pickups, 15);
	if (defNumber == -1) defNumber = 6; //make gold if we weren't able to choose an item
	pickupDef = pickups[defNumber];

	durability = pickupDef.getDurability();
}

//create a specific pickup, such as those loaded from saved games or chests
Pickup::Pickup(PickupDef* pickups, int defNumber, int durability){
	if (defNumber < 0 || defNumber > 16) defNumber = 6; //shouldn't happen but make gold if defNumber is bad
	this->defNumber = defNumber;
	pickupDef = pickups[defNumber];
	this->durability = durability;
}

//activate an item and remove it forever if it is not an equippable
int Pickup::use(){
	pickupDef.use();
	if (durability == -1){
		delete this;
		return -1;
	}
	return pickupDef.getEquipSlot();
}

//unequip an item
void Pickup::unequip(){
	pickupDef.unequip();
}

//reduce the durability of an item by 1
int Pickup::reduceDurability(){
	return --durability;
}

int Pickup::getTile() const{
	return pickupDef.getTile();
}

int Pickup::getDurability() const{
	return durability;
}

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
