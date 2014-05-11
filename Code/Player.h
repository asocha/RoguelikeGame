#include "Actor.h"
#include "World.h"
#include <functional>
#include <tuple>
#include <math.h>

/**
* Represents the player's character
*
* @author Andrew Socha
*/
class Player : public Actor{
	public:
		Player(World& world, FILE* file);
		Player(World& world, bool speedRun);
		~Player();

		void movePlayer(World& world);
		void pickupItem(Cell& cell, World& world);

		void saveGame(World& world);

	private:
		vector<Pickup*> inventory;
		Pickup* equipped[3];
		int maxHealth;
		int mana;
		int maxMana;
		int minDefenseBonus;
		int minAttackBonus;
		int gold;
		int experience;
		int level;
		int defPotionTimer;
		int attPotionTimer;
		PickupDef pickups[17];
		PickupDef spells[6];
		bool revealedWorld;
		FILE* file;
		bool speedRun;
		int startingTime;
		int lastRenderedTime;

		void initializePlayer(World& world);

		tuple<bool, int, int> renderInventory(World& world);
		tuple<bool, int, int> renderMouseover(World& world);
		void renderShop(World& world);

		void move(World& world, Coordint& coord, function<void ()> f);
		void updateWorldVisibility(World& world, int newVis);
		void gainExperience(int xp);

		bool useItem(int item);
		void unequipItem(int item, int equippedSlot);
		bool purchaseItem(World& world, int item);
		void sellItem(World& world, int item);

		void reduceDurability(int slot);
		void timePotions(int& timer, int& bonus);
		void everyTurn(World& world);

		void placePlayer(World& world);
		void die(Cell& cell, World& world); //replaces Actor's die function

		template <class T> void writeBytes(T object);
};