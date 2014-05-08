//Cell: a piece of the world
#pragma once

#include <string>
class Pickup;
class Actor;

using namespace std;

class Cell{
	public:
		Cell();
		Cell(string type, int tile);
		~Cell();

		void setType(string type);
		string getType() const;

		void setTile(int tile);
		int getTile() const;

		void setZone(int zone);
		int getZone() const;

		void setVisibility(int visibility);
		int getVisibility() const;

		void setActor(Actor* actor);
		Actor* getActor() const;

		void setPickup(Pickup* pickup);
		Pickup* getPickup() const;

	private:
		string type; //wall, empty, etc
		int tile; //tile number from tiles.png
		int zone; //used to find largest set of empty tiles
		int visibility; //0 = fully hidden, 1 = all but monsters visible, 2 = fully visible
		Actor* actor; //the monster, player, etc. at this tile
		Pickup* pickup; //an item at this location
};