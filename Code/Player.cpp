#include "Player.h"

int Actor::turn = 0;
int Actor::wasAttacked = 0;
int Actor::killedMonster;
Log* Actor::log;

//create a player that is loaded from a saved game
Player::Player(World& world, FILE* loadFile) : Actor(){
	initializePlayer(world);

	if (!world.loadGame(pickups, loadFile)){ //if there was an error loading game, create a new player instead
		world.initializeWorld();
		this->Player::Player(world, false);
		return;
	}

	if (world.getLoadingError()) goto error;

	//player info
	world.readBytes(maxHealth);
	world.readBytes(mana);
	world.readBytes(maxMana);
	world.readBytes(minDefenseBonus);
	world.readBytes(minAttackBonus);
	world.readBytes(gold);
	world.readBytes(experience);
	world.readBytes(level);
	world.readBytes(defPotionTimer);
	world.readBytes(attPotionTimer);
	if (world.readBytes(revealedWorld)) world.getCell(Coordint(-1,-1)).setVisibility(1); //set dummy cell's visibility

	world.readBytes(turn);
	world.readBytes(currentHealth);

	world.readBytes(location.x);
	world.readBytes(location.y);
	world.getCell(location).setActor(this);

	world.readBytes(actorDef.attack.bonus);
	world.readBytes(actorDef.defense.bonus);

	world.readBytes(lastRenderedTime);
	if (lastRenderedTime == -10) speedRun = false;
	else{
		speedRun = true;
		startingTime = time(NULL) - lastRenderedTime;
	}

	//player inventory
	int input, inventorySize, defNumber, durability;
	bool inputBool;
	world.readBytes(inventorySize);
	world.readBytes(input); //equipped size
	if (input != 3 || inventorySize > 18 || inventorySize < 0){ //detect error early
		world.hadLoadingError();
		goto error;
	}
	for (int i = 0; i < inventorySize; i++){
		world.readBytes(defNumber);
		world.readBytes(durability);
		if (!world.getLoadingError()) inventory.push_back(new Pickup(pickups, defNumber, durability));

		if (world.readBytes(inputBool) && durability > -1 && durability < 200 && !world.getLoadingError()){ //is equipped?
			useItem(i);
		}
	}

error:
	if (world.getLoadingError()){ //we reached end of file early... forget all the loading we did and make a new game
		world.~World();
		world.World::World(1);
		world.initializeWorld();
		this->Player::Player(world, false);
		log->print("Missing some save game data.", red, 0);
		log->print("Starting new game.", red, 0);
	}
	else{ //check for extra data at end of file
		char inputChar;
		world.readBytes(inputChar); //read end of file character

		file = world.getFile();

		if (!feof(file)){
			log->print("Extraneous data in save file.", red, turn);
			log->print("Attempting to load game anyways.", red, turn);
		}
	}
}

//create a new player for a new world
Player::Player(World& world, bool speedRun) : Actor(){
	initializePlayer(world);

	maxHealth = currentHealth = (1+speedRun)*actorDef.getMaxHealth(); //speedRunners have double starting health
	mana = maxMana = 100;
	minDefenseBonus = minAttackBonus = 0;
	gold = 0;
	experience = 0;
	level = 1;
	attPotionTimer = defPotionTimer = 0;
	revealedWorld = false;
	this->speedRun = speedRun;

	world.initializeMonsters(pickups);
	world.initializeShop(pickups);

	placePlayer(world);
	updateWorldVisibility(world, 2);
	if (speedRun){
		startingTime = time(NULL);
		lastRenderedTime = 100;
	}
}

//create the pickups and spells tables and set player data that is always the same
void Player::initializePlayer(World& world){
	actorDef = ActorDef(playerTile, "Player", 0, Dice(0,0,100), Dice(2,4,0), Dice(2,2,0)); //100 health, 4-8 attack, 2-4 defense
	defNumber = -1;
	log = &(world.getLog());
	equipped[0] = equipped[1] = equipped[2] = 0;

	//list of pickup types (tile, "name", "description", equip slot, durability, base price, difficulty, on use function, on unequip function)
	const PickupDef mysticPot(0x24D,		"Mystical Potion",		"Grants many magical bonuses.",						-1,	Dice(0,0,-1),	25,		8,	[&](){currentHealth+=10; maxHealth+=10; mana+=10; maxMana+=10; actorDef.attack.bonus+=1; actorDef.defense.bonus+=1;},	[&](){/*Does nothing (not equippable)*/});
	const PickupDef attackPot2(0x222,		"Super Attack Potion",	"Grants +2 Attack.",								-1,	Dice(0,0,-1),	20,		6,	[&](){actorDef.attack.bonus+=2;},																						[&](){});
	const PickupDef healthPot2(0x220,		"Super Health Potion",	"Restores 30 health. Increases max health by 5.",	-1,	Dice(0,0,-1),	20,		4,	[&](){currentHealth+=30; maxHealth+=5; if (currentHealth>maxHealth) currentHealth = maxHealth;},						[&](){});
	const PickupDef basicSword(0x250,		"Basic Sword",			"Attack Dice: 5d2",									0,	Dice(4,4,8),	0.5,	3,	[&](){actorDef.attack.amount=5; actorDef.attack.sides=2;},																[&](){actorDef.attack.amount=4; actorDef.attack.sides=2;});
	const PickupDef defensePot(0x22C,		"Defense Potion",		"Grants +1 Defense.",								-1,	Dice(0,0,-1),	10,		2,	[&](){actorDef.defense.bonus+=1;},																						[&](){});
	const PickupDef healthPot(0x227,		"Health Potion",		"Restores 20 health.",								-1,	Dice(0,0,-1),	10,		1,	[&](){currentHealth+=20; if (currentHealth>maxHealth) currentHealth = maxHealth;},										[&](){});
	const PickupDef gold(0x1C,				"Gold Coins",			"",													-1,	Dice(0,0,-1),	10,		1,	[&](){/*Does nothing (currency)*/},																						[&](){});
	const PickupDef manaPot(0x22A,			"Mana Potion",			"Restores 40 mana.",								-1,	Dice(0,0,-1),	10,		1,	[&](){mana+=40; if (mana>maxMana) mana = maxMana;},																		[&](){});
	const PickupDef attackPot(0x229,		"Attack Potion",		"Grants +1 Attack.",								-1,	Dice(0,0,-1),	10,		2,	[&](){actorDef.attack.bonus+=1;},																						[&](){});
	const PickupDef shield(0x2C2,			"Shield",				"Grants an additional Defense die.",				1,	Dice(4,4,10),	0.6,	4,	[&](){actorDef.defense.amount=3;},																						[&](){actorDef.defense.amount=2;});
	const PickupDef breastplate(0x2D3,		"Breastplate",			"Grants your Defense dice an additional side.",		2,	Dice(5,5,10),	0.7,	5,	[&](){actorDef.defense.sides=3;},																						[&](){actorDef.defense.sides=2;});
	const PickupDef manaPot2(0x223,			"Super Mana Potion",	"Restores 75 mana. Increases max mana by 5.",		-1,	Dice(0,0,-1),	20,		5,	[&](){mana+=75; maxMana+=5; if (mana>maxMana) mana = maxMana;},															[&](){});
	const PickupDef superiorSword(0x254,	"Superior Sword",		"Attack Dice: 5d3",									0,	Dice(5,5,5),	0.8,	7,	[&](){actorDef.attack.amount=5; actorDef.attack.sides=3;},																[&](){actorDef.attack.amount=4; actorDef.attack.sides=2;});
	const PickupDef defensePot2(0x225,		"Super Defense Potion",	"Grants +2 Defense.",								-1,	Dice(0,0,-1),	20,		7,	[&](){actorDef.defense.bonus+=2;},																						[&](){});
	const PickupDef legendaryDagger(0x266,	"Legendary Dagger",		"Attack Dice: 5d4",									0,	Dice(6,6,0),	1,		9,	[&](){actorDef.attack.amount=5; actorDef.attack.sides=4;},																[&](){actorDef.attack.amount=4; actorDef.attack.sides=2;});
	const PickupDef chest(0x3D7,			"Chest",				"",													-1,	Dice(0,0,-1),	100,	100,[&](){},																												[&](){});
	const PickupDef openedChest(0x3D8,		"Opened Chest",			"",													-1,	Dice(0,0,-1),	100,	100,[&](){},																												[&](){});

	PickupDef myPickups[17] = {mysticPot, attackPot2, healthPot2, basicSword, defensePot, healthPot, gold, manaPot, attackPot, shield, breastplate, manaPot2, superiorSword, defensePot2, legendaryDagger, chest, openedChest};
	for (int i = 0; i < 17; i++){
		pickups[i] = myPickups[i];
	}


	//list of spells types (tile, "name", "description", -1, -1, mana cost, 0, on use function, empty function)
	//spells are implemented as infinite use items
	const PickupDef conjure(0x393,	"Conjure Item",		"Creates a random item.",							-1,	Dice(0,0,-1),	60,		0,	[&](){
		if (inventory.size() < 18){
			Pickup* newPickup = new Pickup(pickups, world.getDifficulty());
			while (newPickup->getName() == "Gold Coins"){
				delete newPickup;
				newPickup = new Pickup(pickups, world.getDifficulty());
			}
			inventory.push_back(newPickup);
		}
	},		[&](){});
	const PickupDef buff(0x3A0,		"Buff",				"Increase a random stat.",							-1,	Dice(0,0,-1),	40,		0,	[&](){
		int stat = rand() % 7;
		if (stat == 0){ maxHealth += 10; currentHealth += 10; this->log->print("You gained Health!", white, turn);}
		else if (stat == 1){ maxMana += 10; mana += 10; this->log->print("You gained Mana!", white, turn);}
		else if (stat == 2){ this->gold += 10; this->log->print("You gained Gold!", white, turn);}
		else if (stat == 3){ this->log->print("You gained Experience!", white, turn); gainExperience(level*10); }
		else if (stat == 4){ this->log->print("You gained a Level!", white, turn); gainExperience(10*floor(2*pow(1.5,double(level)))); }
		else if (stat == 5){ actorDef.attack.bonus++; this->log->print("You gained Attack!", white, turn);}
		else{ actorDef.defense.bonus++; this->log->print("You gained Defense!", white, turn);}
	},		[&](){});
	const PickupDef fullHeal(0x367,	"Full Heal",		"Restores all health. REDUCES MAX MANA BY 50.",		-1,	Dice(0,0,-1),	100,	0,	[&](){
		currentHealth = maxHealth;
		maxMana -= 50;
	},		[&](){});
	const PickupDef reveal(0x388,	"Reveal",			"Reveals the entire level.",						-1,	Dice(0,0,-1),	35,		0,	[&](){
		world.loopWorld([&](Cell& cell, int& i, int& j){
			cell.setVisibility(2);
		});
		world.getCell(Coordint(-1,-1)).setVisibility(1);
		revealedWorld = true;
	},		[&](){});
	const PickupDef summon(0x34A,	"Summon Monsters",	"Summons elite monsters with powerful treasures.",	-1,	Dice(0,0,-1),	60,		0,	[&](){
		for (int x = location.x-5; x <= location.x+5; x++){
			for (int y = location.y-5; y <= location.y+5; y++){ //for all tiles within 5 x and y
				if (y == location.y - 1 && x > location.x - 2 && x < location.x+2) y +=3; //except those within 1 x and y
				Cell& cell = world.getCell(Coordint(x,y));

				if (cell.getTile() == empty && !cell.getActor() && !cell.getPickup() && rand() % 5 == 0){
					cell.setActor(new Actor(Coordint(x,y), world.getDifficulty()+1));
					cell.setPickup(new Pickup(pickups, world.getDifficulty()+1));
				}
			}
		}
	},		[&](){});
	const PickupDef random(0x3F,	"Random Magic",		"Casts one of your other spells at random.",		-1,	Dice(0,0,-1),	35,		0,	[&](){
		int spell = rand() % 5;
		this->log->print("You randomly casted " + spells[spell].getName() + ".", white, turn);
		spells[spell].use();
	},		[&](){});

	PickupDef mySpells[6] = {conjure, buff, fullHeal, reveal, summon, random};
	for (int i = 0; i < 6; i++){
		spells[i] = mySpells[i];
	}
}

//places the player at the first empty tile in the world
void Player::placePlayer(World& world){
	world.loopInnerWorld([&](Cell& cell, int& i, int& j){
		if (cell.getTile() == empty){
			//found a cell to place the player

			//remove all monsters and pickups within 5 tiles of the player's initial spawn location
			for (int x = i-5; x <= i+5; x++){
				for (int y = j-5; y <= j+5; y++){
					Cell& cell2 = world.getCell(Coordint(x,y));

					delete cell2.getActor();
					cell2.setActor(0);

					delete cell2.getPickup();
					cell2.setPickup(0);
				}
			}

			//place the player
			location = Coordint(i,j);
			cell.setActor(this);

			i = j = worldsize; //break out of loopInnerWorld
		}
	});
}

//check if player pressed a movement key, also renders the player's stats and inventory
void Player::movePlayer(World& world){
	tuple<bool, int, int> mouseInInventory = renderInventory(world);

	//check for movement
	if (tl_keywentdown("up") || tl_keywentdown("w")){
		move(world, location.up(), [this](){this->location.y--;});
	}
	else if (tl_keywentdown("down") || tl_keywentdown("s")){
		move(world, location.down(), [this](){this->location.y++;});
	}
	else if (tl_keywentdown("left") || tl_keywentdown("a")){
		move(world, location.left(), [this](){this->location.x--;});
	}
	else if (tl_keywentdown("right") || tl_keywentdown("d")){
		move(world, location.right(), [this](){this->location.x++;});
	}
	else if (tl_keywentdown("z")){
		world.changeZoom();
	}
	else if (world.getCell(location).getTile() == stairs && (tl_keywentdown("return") || tl_keywentdown("enter"))){ //player uses stairs to go to the next level
		int newDif = world.getDifficulty() + 1;
		world.~World(); //clear the old world
		world.World::World(newDif); //create the new world

		world.initializeWorld();
		world.initializeMonsters(pickups);
		world.initializeShop(pickups);

		placePlayer(world);

		revealedWorld = false;

		everyTurn(world);
		updateWorldVisibility(world,2);

		tl_play("newLevel.sfs");
		if (speedRun) startingTime = time(NULL);
	}
	else if (tl_buttonwentdown() && std::get<0>(mouseInInventory)){ //std::get<i> returns ith item in tuple
																	//player is clicking on an item or spell
		int item = std::get<1>(mouseInInventory);
		if (std::get<2>(mouseInInventory) == -1){
			if (item > 20){ //purchase from shop
				if (!purchaseItem(world, item-30)) return; //return if we tried to purchase something and couldn't
			}
			else{
				if (world.getCell(location).getTile() == shopTile && item >= 0){ //sell to shop
					sellItem(world, item);
				}
				else{ //equip or use item, or cast spell
					if (!useItem(item)) return;
				}
			}
		}
		else{ //unequip
			unequipItem(item, std::get<2>(mouseInInventory));
			if (world.getCell(location).getTile() == shopTile) sellItem(world, item); //sell if at shop
		}

		everyTurn(world);
	}
	else if (tl_buttonwentdown(3) && std::get<0>(mouseInInventory) && !world.getCell(location).getPickup()){ //player is dropping an item with the right mouse button
		int item = std::get<1>(mouseInInventory);
		if (item < 18){ //make sure we're clicking on inventory and not shop
			if (std::get<2>(mouseInInventory) != -1){ //unequip item first
				unequipItem(item, std::get<2>(mouseInInventory));
			}

			log->print("You dropped " + log->addArticle(inventory.at(item)->getName(), false) + ".", white, turn);
			world.getCell(location).setPickup(inventory.at(item)); //place the item on the player's current location
			inventory.erase(inventory.begin()+item);

			everyTurn(world);
		}
	}
	else{ //player casts a spell using hotkeys (1-6)
		for (int i = 1; i < 7; i++){
			if (tl_keywentdown(&std::to_string(long double(i)).at(0))){
				if (useItem(i-13)) everyTurn(world);
			}
		}
	}

	if (speedRun && time(NULL) - startingTime > 99) die(world.getCell(location), world);
}

//player movement (called by movePlayer)
void Player::move(World& world, Coordint& coord, function<void ()> f){
	Cell& cell = world.getCell(coord);
	if (cell.getActor()){
		killedMonster = 0;
		attack(*(cell.getActor()), cell, world); //attack monster
		reduceDurability(0);

		if (killedMonster) gainExperience(killedMonster);
	}
	else if (cell.getType() != "wall"){ //move
		updateWorldVisibility(world, 1);

		world.getCell(location).setActor(0); //set current location's actor to null
		cell.setActor(this); //set new location's actor to the player
		f(); //change player's location
		
		Pickup* pickup = cell.getPickup();
		if (pickup) pickupItem(cell, world); //pickup any item in the new cell

		updateWorldVisibility(world, 2);
	}
	everyTurn(world);
}

//update the visibility variable on all tiles near the player that are in line of sight
//such that the player only sees nearby tiles
void Player::updateWorldVisibility(World& world, int newVis){
	if (revealedWorld) return;

	double ratio, xOffset, yOffset, x, y;
	int numCells, xSign, ySign, prevX = 0, prevY = 0;
	for (double i = 0; i < 6.28318531; i += 6.28318531/48){
		x = cos(i) * 5.2;
		y = sin(i) * 5.2;
		
		if (prevX != int(x) || prevY != int(y)){ //make sure we have a new point
			prevX = x;
			prevY = y;

			xSign = (x > 0) - (x < 0);
			ySign = (y > 0) - (y < 0);
			x = floor(abs(x));
			y = floor(abs(y));

			numCells = x + y; //# cells drawn
			ratio = y / x; //ratio of y to x
				
			x = y = 0.5;
			for (int j = 0; j <= numCells; j++){
				Cell& cell = world.getCell(location + Coordint(x*xSign, y*ySign));
				cell.setVisibility(newVis);
				if (cell.getTile() == hallwayWall) break; //wall blocking view
				
				if (ratio*x > y) y++;
				else x++;
			}
		}
	}
}

//render the player's stats, spells, and inventory, as well as the game timer in speed run mode
tuple<bool, int, int> Player::renderInventory(World& world){
	//render remaining time
	if (speedRun){
		int timer = 100 + startingTime - time(NULL);
		if (timer < 20){
			if (lastRenderedTime != timer) tl_play("timer.sfs");
			tl_color(red);			
		}
		else if (timer < 40) tl_color(orange);
		lastRenderedTime = timer;
		world.renderString(std::to_string(long double(timer)), 0, 0);
	}

	//render stats
	string health = "Hlth:" + std::to_string(long double(currentHealth)) + "/" + std::to_string(long double(maxHealth));
	string manaStr = "Mana:" + std::to_string(long double(mana)) + "/" + std::to_string(long double(maxMana));
	string attack = "Atck:" + std::to_string(long double(actorDef.attack.amount)) + "d" + std::to_string(long double(actorDef.attack.sides)) + "+" + std::to_string(long double(actorDef.attack.bonus));
	string defense = "Dfnc:" + std::to_string(long double(actorDef.defense.amount)) + "d" + std::to_string(long double(actorDef.defense.sides)) + "+" + std::to_string(long double(actorDef.defense.bonus));
	string gold = "Gold:" + std::to_string(long double(this->gold));
	string lvl = "Level:" + std::to_string(long double(level));
	string xp = "XP:" + std::to_string(long double(experience)) + "/" + std::to_string(long double(10*floor(2*pow(1.5,double(level)))));
	string floor = "Floor:" + std::to_string(long double(world.getDifficulty()));

	tl_scale(2);

	int xStart = tl_xres() - tl_xres()%2 - 12;

	tl_color(red);
	world.renderString(health, xStart, 1);

	tl_color(blue);
	world.renderString(manaStr, xStart, 2);
	
	tl_color(white);
	world.renderString(attack, xStart, 3);
	world.renderString(defense, xStart, 4);
	world.renderString(gold, xStart, 5);
	world.renderString(lvl, xStart, 7);
	world.renderString(xp, xStart, 8);
	world.renderString(floor, xStart, 9);

	tl_scale(1);

	tuple<bool, int, int> toReturn = renderMouseover(world);
	if (world.getCell(location).getTile() == shopTile) renderShop(world);

	//render spells
	for (int i = 0; i < 6; i++){
		tl_rendertile(spells[i].getTile(), tl_xres() - 6 + i, 5);
	}

	//render inventory
	for (int i = 0; i < inventory.size(); i++){
		for (int j = 0; j < 3; j++){
			if (inventory.at(i) == equipped[j]) tl_color(equippedYellow);
		}
		tl_rendertile(inventory.at(i)->getTile(), tl_xres() - 6 + i%6, 7 + i/6);
		tl_color(white);
	}

	return toReturn;
}

//render mouseover inventory highlight and mouseover tooltip
tuple<bool, int, int> Player::renderMouseover(World& world){
	bool inInventory = false;
	int inventorySlot = -1;
	int equippedSlot = -1;

	//render inventory scrollover highlight and get mouseover information
	int x = tl_mousex();
	int y = tl_mousey();
	int item = x - tl_xres() + 6 + (y - 7) * 6; //figure out which item we're hovering over
	string name = "";
	string description = "";
	int descriptionColor = green;
	if (0 <= item && item < inventory.size() && x > tl_xres() - 7 && x < tl_xres()){ //hovering over inventory item
		inInventory = true;
		inventorySlot = item;
		for (int i = 0; i < 3; i++){
			if (inventory.at(item) == equipped[i]) equippedSlot = i;
		}
		tl_rendertile(inventoryHighlight, x, y);

		//get mouseovered (inventory) item's name and description
		name = inventory.at(item)->getName();
		if (inventory.at(item)->getDurability() != -1) name += "    Durability: " + std::to_string(long double(inventory.at(item)->getDurability()));
		name += "    Value: " + std::to_string(long double(inventory.at(item)->getPrice() * 3 / 4));
		description = inventory.at(item)->getDescription();
	}
	else if (0 <= item + 12 && item + 12 < 6 && x > tl_xres() - 7 && x < tl_xres()){ //hovering over spell
		//spells are 12 above inventory

		inInventory = true; //pretend we're in the inventory
		inventorySlot = item;

		tl_rendertile(inventoryHighlight, x, y);

		//get mouseovered (spell) item's name and description
		name = "[" + std::to_string(long double(item+13)) + "] " + spells[item+12].getName() +
			   "     Mana: " + std::to_string(long double(spells[item+12].getBasePrice()));
		description = spells[item+12].getDescription();
	}
	else if (world.getCell(location).getTile() == shopTile && 0 <= item - 30 && x > tl_xres() - 7 && x < tl_xres()){ //hovering over shop item
		vector<Pickup*>& shop = world.getShop();
		if (item - 30 < shop.size() && shop[item-30]){ //shop is offset 30 item spaces from the inventory
			inInventory = true; //pretend we're in the inventory
			inventorySlot = item;

			tl_rendertile(inventoryHighlight, x, y);

			//get mouseovered (shop) item's name and description
			name = shop[item-30]->getName();
			if (shop[item-30]->getDurability() != -1) name += "    Durability: " + std::to_string(long double(shop[item-30]->getDurability()));
			name += "     Price: " + std::to_string(long double(shop[item-30]->getPrice()));
			description = shop[item-30]->getDescription();
		}
	}
	else if (x < tl_xres() - 6){ //get mouseovered (world) object's name and description
		int objectx = location.x - tl_xres()/2 - tl_xres()%2 + 4 + x;
		int objecty = location.y - tl_yres()/2 - tl_yres()%2 + 2 + y;

		Cell& cell = world.getCell(Coordint(objectx,objecty));
		if (cell.getVisibility() == 2){
			Actor* monster = cell.getActor();
			Pickup* pickup = cell.getPickup();

			if (monster){
				name = monster->getName();
				if (name == "Player") tl_color(blue);
				else{
					tl_color(red);
					description = "Difficulty: " + std::to_string(long double(monster->getDifficulty()));
					descriptionColor = difficultyColors[(monster->getDifficulty() - 1) / 3];
				}
			}
			else if (pickup){
				name = pickup->getName();
				if (pickup->getDurability() != -1) name += "    Durability: " + std::to_string(long double(pickup->getDurability()));
				if (name != "Gold Coins" && name != "Chest" && name != "Opened Chest") name += "    Value: " + std::to_string(long double(pickup->getPrice() * 3 / 4));
				description = pickup->getDescription();
			}
		}
	}

	if (world.getCell(location).getTile() == stairs){ //player is standing on stairs, render their info instead of mouseover info
		name = "Stairs";
		description = "Press 'Enter' to go to Level " + std::to_string(long double(world.getDifficulty()+1)) + ".";
		tl_color(white);
		descriptionColor = green;
	}
	else if (world.getCell(location).getPickup() && inventory.size() == 18){ //player is standing on a pickup with a full inventory
		name = "Your inventory is full!";
		description = "Use, drop, or sell an item to make room.";
		tl_color(red);
		descriptionColor = green;
	}

	//render mouseovered object's name and description
	tl_scale(2);
	world.renderString(name, 0, tl_yres() - 6 - tl_yres()%2);
	tl_color(descriptionColor);
	world.renderString(description, 0, tl_yres() - 5 - tl_yres()%2);
	tl_scale(1);
	tl_color(white);

	return tuple<bool, int, int>(inInventory, inventorySlot, equippedSlot);
}

//render the shop
void Player::renderShop(World& world){
	vector<Pickup*>& shop = world.getShop();

	tl_scale(2);
	world.renderString("Shop", tl_xres() - tl_xres()%2 - 8, 22);
	tl_scale(1);

	for (int i = 0; i < shop.size(); i++){
		if (shop[i]){
			if (shop[i]->getPrice() > gold) tl_color(cantAffordRed); //can't afford it, highlight red
			tl_rendertile(shop[i]->getTile(), tl_xres() - 6 + i%6, 12 + i/6);
			tl_color(white);
		}
	}
}

//gain experience for killing a monster (killedMonster set by actor: attack())
void Player::gainExperience(int xp){
	experience += xp;
	
	if (experience >= 10*floor(2*pow(1.5,double(level)))){ //gain level
		experience -= 10*floor(2*pow(1.5,double(level)));
		level++;

		tl_play("playerLevelUp.sfs");
		this->log->print("You are now level " + std::to_string(long double(level)) + "!", blue, turn);

		//gain 5 health and mana per level, 1 attack per 2 levels, and 1 defense per 2 levels
		maxHealth += 5;
		currentHealth += 5;
		maxMana += 5;
		mana += 5;

		if (level % 2){
			actorDef.defense.bonus++;
			minDefenseBonus++;
		}
		else{
			actorDef.attack.bonus++;
			minAttackBonus++;
		}
	}
}

//player picks up an item, placing it in his inventory
void Player::pickupItem(Cell& cell, World& world){
	Pickup* pickup = cell.getPickup();
	string name = pickup->getName();

	if (name == "Gold Coins"){ //found gold
		tl_play("pickup.sfs");
		int gold = rand() % 9 + 2; //2-10 gold
		this->gold += gold;
		log->print("You found " + std::to_string(long double(gold)) + " Gold Coins.", white, turn);
		delete pickup;
		cell.setPickup(0);
	}
	else if (name == "Chest"){ //opened a chest
		if (inventory.size() == 18) return; //don't actually open it if inventory is full
		tl_play("pickup.sfs");
		log->print("You found a Chest!", green, turn);
		delete pickup;

		for (int i = 1; i < 4; i++){
			if (rand() % i == 0 && inventory.size() < 18){ //pick up 1-3 items
				cell.setPickup(new Pickup(pickups, world.getDifficulty()));
				pickupItem(cell, world);
			}
		}

		//pick up gold
		cell.setPickup(new Pickup(pickups, 6, -1));
		pickupItem(cell, world);

		//replace the chest with an opened chest
		cell.setPickup(new Pickup(pickups, 16, -1));
	}
	else if (inventory.size() < 18 && name != "Opened Chest"){ //found any other item (ignore opened chests)
		tl_play("pickup.sfs");
		log->print("You found " + log->addArticle(name, false) + ".", white, turn);
		inventory.push_back(pickup);
		cell.setPickup(0);
	}
}

//the player uses an item at a certain location in his/her inventory
bool Player::useItem(int item){
	if (item < 0){ //cast a spell
		if (mana < spells[item+12].getBasePrice()) return false;
		tl_play("magic.sfs");
		log->print("You casted " + spells[item+12].getName() + ".", white, turn);
		spells[item+12].use();
		mana -= spells[item+12].getBasePrice();
	}
	else{ //use an item
		log->print("You used " + log->addArticle(inventory.at(item)->getName(), false) + ".", white, turn);
		int equipSlot = inventory.at(item)->use();
		if (equipSlot == -1) inventory.erase(inventory.begin()+item); //consumable
		else equipped[equipSlot] = inventory.at(item); //equippable
	}
	return true;
}

//the player unequips an item at a certain location in his/her inventory
void Player::unequipItem(int item, int equippedSlot){
	log->print("You unequipped " + log->addArticle(inventory.at(item)->getName(), false) + ".", white, turn);
	equipped[equippedSlot] = 0;
	inventory.at(item)->unequip();
}

//the player purchases an item from the shop
bool Player::purchaseItem(World& world, int item){
	vector<Pickup*>& shop = world.getShop();
	Pickup* pickup = shop.at(item);
	if (inventory.size() < 18 && gold >= pickup->getPrice()){
		tl_play("pickup.sfs");

		log->print("You bought " + log->addArticle(pickup->getName(), false) + ".", white, turn);
		inventory.push_back(pickup);
		gold -= pickup->getPrice();
		shop[item] = 0;

		return true;
	}

	return false;
}

//the player sells an item to the shop
void Player::sellItem(World& world, int item){
	log->print("You sold " + log->addArticle(inventory.at(item)->getName(), false) + ".", white, turn);
	gold += inventory.at(item)->getPrice() * 3 / 4;
	world.getShop().push_back(inventory.at(item));
	inventory.erase(inventory.begin()+item);
}

//reduce the durability of either the player's weapon or his/her armor pieces and break the item if the durability is zero
void Player::reduceDurability(int slot){
	Pickup* item = equipped[slot];
	if (item){
		int remainingDurability = item->reduceDurability();
		if (remainingDurability == 5){ //low durability
			log->print("Your " + item->getName() + " is low on durability!", orange, turn);
		}
		else if (!remainingDurability){ //broken
			log->print("Your " + item->getName() + " broke!", red, turn);
			item->unequip();

			//find the item in our inventory and remove it
			for (int i = 0; i < inventory.size(); i++){
				if (inventory.at(i) == item) inventory.erase(inventory.begin()+i);
			}

			equipped[slot] = 0;
			delete item;
		}
	}
}

//the player's used attack/defense potions slowly expire over 100 turns
void Player::timePotions(int& timer, int& bonus){
	if (timer){
		timer--;

		if (!timer){ //potion expired
			bonus--;
		}
	}
	else timer = 99; //just used a potion, set timer
}

//stuff that is done every time the player does something
void Player::everyTurn(World& world){
	world.moveMonsters(this); //all monsters move

	if (actorDef.defense.bonus > minDefenseBonus) timePotions(defPotionTimer, actorDef.defense.bonus); //potions reduce duration
	if (actorDef.attack.bonus > minAttackBonus) timePotions(attPotionTimer, actorDef.attack.bonus);

	for (int i = 0; i < wasAttacked; i++){ //reduce armor durability for each time the player was attacked this turn
		reduceDurability(1);
		reduceDurability(2);
	}
	wasAttacked = 0;

	turn++;
}

//the player dies - game ends
void Player::die(Cell& cell, World& world){
	remove("saveGame.txt");
	tl_play("death.sfs");

	currentHealth = 0; //make sure it isn't displaying negative
	spells[3].use(); //reveal the world

	while (!tl_keywentdown("escape")){ //wait until player hits escape
		tl_framestart(0);

		if (tl_keywentdown("z")){
			world.changeZoom();
		}
		
		//freeze the speed run timer
		if (speedRun && lastRenderedTime != 100 + startingTime - time(NULL)){
			startingTime++;
		}

		renderInventory(world);
		world.renderViewport(location);
	
		tl_color(red);
		world.renderString("You have died.", tl_xres()/2 - 10, tl_yres()/2 - 2);
		world.renderString("GAME OVER!", tl_xres()/2 - 8, tl_yres()/2 - 1);
		world.renderString("Press 'Esc' to quit.", tl_xres()/2 - 13, tl_yres()/2 + 1);
		tl_color(white);
	}

	world.getCell(location).setActor(0);
	delete this;
	tl_shutdown();
}

//save the current game to file
void Player::saveGame(World& world){
	file = fopen("saveGame.txt", "wb");

	writeBytes(31686464); //file ID
	writeBytes(1); //version #

	//world info
	writeBytes(worldsize);
	writeBytes(world.getDifficulty());
	writeBytes(world.getWorldClear());

	//shop info
	vector<Pickup*>& shop = world.getShop();
	writeBytes(shop.size());
	for (int i = 0; i < shop.size(); i++){
		if (shop[i]){
			writeBytes(true);
			writeBytes(shop[i]->getDefNumber());
			writeBytes(shop[i]->getDurability());
		}
		else writeBytes(false);
	}

	//world info
	string type;
	Actor* actor;
	Pickup* pickup;
	world.loopWorld([&](Cell& cell, int& i, int& j){
		//cell info
		writeBytes(cell.getTile());

		type = cell.getType();
		writeBytes(type.length());
		for (int a = 0; a < type.length(); a++){
			writeBytes(type[a]);
		}

		writeBytes(cell.getVisibility());

		//actor info
		actor = cell.getActor();
		if (actor){
			writeBytes(true);
			writeBytes(actor->getDefNumber());
			writeBytes(actor->getChaseCount());
			writeBytes(actor->getHealth());
			writeBytes(actor->getLocation().x);
			writeBytes(actor->getLocation().y);
			writeBytes(actor->getHome().x);
			writeBytes(actor->getHome().y);
		}
		else writeBytes(false);

		//pickup info
		pickup = cell.getPickup();
		if (pickup){
			writeBytes(true);
			writeBytes(pickup->getDefNumber());
			writeBytes(pickup->getDurability());
		}
		else writeBytes(false);
	});

	//player info
	writeBytes(maxHealth);
	writeBytes(mana);
	writeBytes(maxMana);
	writeBytes(minDefenseBonus);
	writeBytes(minAttackBonus);
	writeBytes(gold);
	writeBytes(experience);
	writeBytes(level);
	writeBytes(defPotionTimer);
	writeBytes(attPotionTimer);
	writeBytes(revealedWorld);

	writeBytes(turn);
	writeBytes(currentHealth);
	writeBytes(location.x);
	writeBytes(location.y);
	writeBytes(actorDef.attack.bonus);
	writeBytes(actorDef.defense.bonus);

	if (speedRun) writeBytes(int(time(NULL) - startingTime));
	else writeBytes(-10);

	//player inventory
	writeBytes(inventory.size());
	writeBytes(3); //equipped size
	bool isEquipped;
	for (int i = 0; i < inventory.size(); i++){
		writeBytes(inventory[i]->getDefNumber());
		writeBytes(inventory[i]->getDurability());

		//isEquipped?
		isEquipped = false;
		for (int j = 0; j < 3; j++){
			if (equipped[j] == inventory[i]) isEquipped = true;
		}
		writeBytes(isEquipped);
	}

	fclose(file);
}

//write data out to file in an endianness-agnostic manner
template <class T> void Player::writeBytes(T object){
	T shiftedO;
	for (int i = 1; i <= sizeof(T); i++){
		shiftedO = (object >> 8*(sizeof(T) - i));
		fwrite(&shiftedO, 1, 1, file);
	}
}

Player::~Player(){
	for (int i = 0; i < inventory.size(); i++){
		delete inventory.at(i);
	}
}