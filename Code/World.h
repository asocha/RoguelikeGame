#pragma once

#include "Cell.h"
#include <time.h>
#include <string>
#include "Coord.h"
#include "Actor.h"
#include "Log.h"
#include "Pickup.h"
#include <functional>

//tileset
const int playerTile = 0xAC;
const int empty = 0x1F;
const int outsideWorld = 0x14F;
const int hallwayWall = 0x1E;
const int columnWall = 0x1D;
const int wall = 0x14E;
const int stairs = 0x1A0;
const int shopTile = 0x13C;
const int inventoryHighlight = 0x18D;

const int tiles[48] = {playerTile, empty, outsideWorld, hallwayWall, columnWall, wall, stairs, shopTile, inventoryHighlight, //world
						0x24D, 0x222, 0x220, 0x250, 0x22C, 0x227, 0x1C, 0x22A, 0x229, 0x2C2, 0x2D3, 0x223, 0x254, 0x225, 0x266, 0x3D7, 0x3D8, //items
						0x393, 0x3A0, 0x367, 0x388, 0x34A, 0x3F, //spells
						0xE7, 0xAE, 0xCE, 0xE4, 0xB6, 0xBE, 0x9D, 0xBC, 0xC9, 0xEA, 0xC2, 0x8B, 0xEB, 0x88, 0xA1, 0x39D}; //monsters

//world size
const int worldsize = 50;

/**
* represents one floor of the game world
*
* @author Andrew Socha
*/
class World{
	public:
		World(int difficulty);
		~World();

		bool loadGame(PickupDef* pickups, FILE* file);
		template <class T> T readBytes(T& object);

		void renderViewport(Coordint& player);
		void renderMenu(int gameMode, FILE* file) const;
		void renderString(string string, int x, int y) const;

		Cell& getCell(Coordint& coord);
		int floodFill(Coordint& coord, int zone);
		void initializeWorld();
		void initializeMonsters(PickupDef* pickups);
		void initializeShop(PickupDef* pickups);
		void moveMonsters(Actor* player);

		Log& getLog();
		FILE* getFile() const;
		int getDifficulty() const;
		bool getWorldClear() const;
		bool getLoadingError() const;
		vector<Pickup*>& getShop();

		void changeZoom();
		void hadLoadingError();

		void loopInnerWorld(function<void (Cell& cell, int& i, int& j)> f);
		void loopWorld(function<void (Cell& cell, int& i, int& j)> f);

	private:
		Cell dummy; //dummy cell to return when outside the world
		Cell** world; //a 2d array that represents the world
		Log log; //combat log
		bool worldClear; //any monsters left?
		int difficulty;
		vector<Pickup*> shop;
		FILE* file;
		bool loadingError;
		int zoom;

		void moveMonster(Actor* monster, Actor* player);
		bool move(Coordint& monsterCoord, int newx, int newy, Actor* monster, bool isChasing);
		int getNearbyEmptyCells(Coordint& coord);
};