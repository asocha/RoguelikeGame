#include "PickupDef.h"

PickupDef::PickupDef(){
}

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

//activate an item by calling its onUse function
void PickupDef::use(){
	onUse();
}

//unequip an item by calling its onUnequip function
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
