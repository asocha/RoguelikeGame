#pragma once

#include "Cell.h"
#include "World.h"

/** Default Constructor: Create a wall cell */
Cell::Cell(){
	type = "wall";
	tile = wall;
	zone = 0;
	visibility = 0;
	actor = 0;
	pickup = 0;
}

/**
* Create a cell of the game world
*
* @param type	the type of cell we are creating
* @param tile	the tile number of the tile used to render the cell
*/
Cell::Cell(string type, int tile){
	this->type = type;
	this->tile = tile;
	this->zone = 0;
	this->visibility = 0;
	this->actor = 0;
	this->pickup = 0;
}

void Cell::setType(string type){
	this->type = type;
}

string Cell::getType() const{
	return type;
}

void Cell::setTile(int tile){
	this->tile = tile;
}

int Cell::getTile() const{
	return tile;
}

void Cell::setZone(int zone){
	this->zone = zone;
}

int Cell::getZone() const{
	return zone;
}

void Cell::setVisibility(int visibility){
	this->visibility = visibility;
}

int Cell::getVisibility() const{
	return visibility;
}

void Cell::setActor(Actor* actor){
	this->actor = actor;
}

Actor* Cell::getActor() const{
	return actor;
}

void Cell::setPickup(Pickup* pickup){
	this->pickup = pickup;
}

Pickup* Cell::getPickup() const{
	return pickup;
}

/** Destructor */
Cell::~Cell(){
}