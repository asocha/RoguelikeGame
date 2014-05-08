#include "World.h"

World::World(int difficulty){
	srand(time(NULL));
	dummy = Cell("wall", outsideWorld);
	worldClear = false;
	this->difficulty = difficulty;

	//create the world 2d array
	world = new Cell*[worldsize];
	for (int i = 0; i < worldsize; i++){
		world[i] = new Cell[worldsize];
	}

	zoom = 1;
}

//load game world from file
bool World::loadGame(PickupDef* pickups, FILE* filePtr){
	file = filePtr;
	loadingError = false;

	int input;
	bool inputBool;
	char inputChar;

	if (readBytes(input) != 31686464){ //verify file id is correct
		log.print("Invalid save file ID.", red, 0);
		log.print("Starting new game.", red, 0);
		return false;
	}
	if (readBytes(input) != 1){ //verify version # is valid (only version 1 is valid for now)
		log.print("Invalid save file version number.", red, 0);
		log.print("Starting new game.", red, 0);
		return false;
	}

	//world info
	readBytes(input); //world size
	readBytes(difficulty);
	readBytes(worldClear);
	
	//shop info
	readBytes(input); //shop size
	if (input < 0 || input > 100){ //detect error early
		loadingError = true;
		return true;
	}

	int defNumber, durability;
	for (int i = 0; i < input; i++){
		if (readBytes(inputBool)){ //is there a pickup in this store slot?
			readBytes(defNumber);
			readBytes(durability);

			if (!loadingError) shop.push_back(new Pickup(pickups, defNumber, durability));
		}
		else shop.push_back(0);
	}

	//world info
	int chaseCount, health;
	Actor* actor;
	Pickup* pickup;
	string type;
	loopWorld([&](Cell& cell, int& i, int& j){
		//cell info
		cell.setTile(readBytes(input));
		
		readBytes(input); //length of type string
		if (input > 8 || input < 0){ //try to detect an error
			loadingError = true;
			goto error;
		}
		type = "";
		for (int a = 0; a < input; a++){ //read type string 1 char at a time
			type += readBytes(inputChar);
		}
		cell.setType(type);

		cell.setVisibility(readBytes(input));

		//actor info
		if (readBytes(inputBool)){
			readBytes(defNumber);
			readBytes(chaseCount);
			readBytes(health);

			if (defNumber != -1 && !loadingError){ //if not player
				actor = new Actor(defNumber, chaseCount, health);
				cell.setActor(actor);
			
				readBytes(actor->getLocation().x);
				readBytes(actor->getLocation().y);
				readBytes(actor->getHome().x);
				readBytes(actor->getHome().y);
			}
			else{ //is player, ignore data (loaded in player class)
				readBytes(input);
				readBytes(input);
				readBytes(input);
				readBytes(input);
			}
		}

		//pickup info
		if (readBytes(inputBool)){
			readBytes(defNumber);
			readBytes(durability);
			if (!loadingError) {
				pickup = new Pickup(pickups, defNumber, durability);
				cell.setPickup(pickup);
			}
		}
		
error:	if (loadingError) i = j = worldsize; //end early if we've already had an error
	});

	return true;
}

template <class T> T World::readBytes(T& object){
	if (!loadingError && fread(&object, 1, 1, file) != 1){
		loadingError = true;
	}
	for (int i = 1; i < sizeof(T); i++){
		object = object << 8;
		if (!loadingError && fread(&object, 1, 1, file) != 1){
			loadingError = true;
		}
	}
	return object;
}

//get a Cell at a specific coordinate
Cell& World::getCell(Coordint& coord){
	int x = coord.x;
	int y = coord.y;
	if (x >= 0 && x < worldsize && y >= 0 && y < worldsize){
		return world[x][y];
	}
	return dummy;
}

//create the entire world - empty rooms and walls
void World::initializeWorld(){
restartWorld:
	int x, y;
	for (int i = 0; i < (worldsize-5)*(worldsize-5); i++){ //set some tiles to empty space
		x = 1 + rand() % (worldsize-2); //1 to (worldsize-2) so that there is always a 1-tile border of wall tiles
		y = 1 + rand() % (worldsize-2);
		world[x][y].setType("empty");
		world[x][y].setTile(empty);
	}

	//floodfill to create a single empty zone
	int zone = 1, size = 0, largestSize = 0, largestZone = 0;
	//find the largest zone of empty tiles
	loopInnerWorld([&](Cell& cell, int& i, int& j){
		size = floodFill(Coordint(i,j), zone);
		if (size > largestSize){
			largestSize = size;
			largestZone = zone;
		}
		zone++;
	});
	//make smaller zones into wall
	loopInnerWorld([&](Cell& cell, int& i, int& j){
		if (cell.getZone() != largestZone){
			cell.setType("wall");
			cell.setTile(wall);
		}
	});

	//set special wall tiles
	Cell* stairsCell;
	bool placedShop = false;
	bool placedStairs = false;
	int tile, count;
	loopWorld([&](Cell& cell, int& i, int& j){
		tile = cell.getTile();
		count = getNearbyEmptyCells(Coordint(i,j));
				
		//set all walls that are surrounded by empty space to a special tile
		if (count == 8 && tile == wall) cell.setTile(columnWall);

		//set all walls that border empty space to a special tile
		else if (count && tile == wall) cell.setTile(hallwayWall);

		//found an empty space surrounded by empty space
		else if (count == 9){
			if (!placedShop){ //place shop
				cell.setTile(shopTile);
				cell.setType("shop");
				placedShop = true;
			}
			else{ //set a possible location for stairs
				stairsCell = &cell;
				placedStairs = true;
			}
		}
	});

	//didn't find a suitable stairs location, recreate world
	if (!placedStairs){
		loopWorld([&](Cell& cell, int& i, int& j){
			cell.setTile(wall);
			cell.setType("wall");
			cell.setZone(0);
		});
		goto restartWorld; 
	}

	//create stairs
	stairsCell->setTile(stairs);
	stairsCell->setType("stairs");
}

//count the number of EMPTY cells adjacent/diagonal to a given cell (includes the given cell) (called by initializeWorld)
int World::getNearbyEmptyCells(Coordint& coord){
	int count = 0;
	for (int i = coord.x - 1; i <= coord.x + 1; i++){
		for (int j = coord.y - 1; j <= coord.y + 1; j++){
			if (getCell(Coordint(i,j)).getTile() == empty) count++;
		}
	}
	return count;
}

//used to find the largest zone of empty tiles
int World::floodFill(Coordint& coord, int zone){
	Cell& cell = getCell(coord);
	if (cell.getTile() == wall || cell.getZone() != 0) return 0;
	
	cell.setZone(zone);

	int visited = floodFill(coord.left(), zone) + 1;
	visited += floodFill(coord.right(), zone);
	visited += floodFill(coord.up(), zone);
	visited += floodFill(coord.down(), zone);
	return visited;
}

//render a String somewhere on the screen
void World::renderString(string string, int x, int y) const{
	for (int i = 0; i < string.length(); i++){
		tl_rendertile(string.at(i), x + i, y);
	}
}

//render the starting menu
void World::renderMenu(int gameMode, FILE* file) const{
	string normalGame = "Normal Game";
	string speedRun = "Speed Run";
	string loadGame = "Load Game";
	string createdBy = "Created By: Andrew Socha";

	if (gameMode == 1){ tl_color(green); normalGame = ">" + normalGame; }
	renderString(normalGame, tl_xres() / 2 - 5 - (gameMode==1), tl_yres()/2 - 1);

	if (gameMode == 2){ tl_color(green); speedRun = ">" + speedRun; }
	else tl_color(white);
	renderString(speedRun, tl_xres() / 2 - 5 - (gameMode==2), tl_yres()/2);

	if (!file) tl_color(background);
	else if (gameMode != 3) tl_color(white);
	else{ tl_color(green); loadGame = ">" + loadGame; }
	renderString(loadGame, tl_xres() / 2 - 5 - (gameMode==3), tl_yres()/2 + 1);
	tl_color(white);

	renderString(createdBy,  tl_xres() / 2 - 12, tl_yres() - 3);

	//render border
	for (int i = 0; i < tl_xres(); i++){
		tl_rendertile(tiles[i % 48], i, 0);
		tl_rendertile(tiles[(48 - i) % 48], i, tl_yres() - 1);
	}
	for (int i = 1; i < tl_yres() - 1; i++){
		tl_rendertile(tiles[(48 - i) % 48], 0, i);
		tl_rendertile(tiles[i % 48], tl_xres() - 1, i);
	}
}

//render the screen
void World::renderViewport(Coordint& player){
	//render the world centered on the player
	tl_scale(zoom);
	int visibility;
	for (int j = 0; j <= tl_yres(); j++){
		for (int i = 0; i <= tl_xres() - (7 * zoom); i++){
			Cell& cell = getCell(Coordint(player.x - (tl_xres() - (7*zoom))/2 + i, player.y - (tl_yres() - (3*zoom))/2 + j));
			visibility = cell.getVisibility();
			if (visibility){
				if (visibility == 1 || (j > tl_yres() - (3*zoom) - 1 - (zoom-1)*tl_yres()%2 && i < 24*zoom)) tl_color(background); //visibility == 1 or behind combat log
				else tl_color(white);
				tl_rendertile(cell.getTile(), i, j); //render tile

				if (visibility == 2){
					Pickup* pickup = cell.getPickup();
					if (pickup) tl_rendertile(pickup->getTile(), i, j); //render pickup on tile

					Actor* actor = cell.getActor();
					if (actor) tl_rendertile(actor->getTile(), i, j); //render actor on tile
				}
			}
		}
	}
	tl_color(white);

	//render the most recent messages from the log at the bottom of the screen
	tl_scale(2);
	vector<tuple<string, int, int>>& log = this->log.getLog();
	int currentTurn = getCell(player).getActor()->getTurn();

	int alphaChange;
	for (int i = log.size() - 1; (i > int(log.size()) - 5 - tl_yres()%2) && (i >= 0); i--){ //print 4 or 5 strings depending on if a 5th can fit (tl_yres()%2)
		alphaChange = 0x15*(currentTurn - std::get<2>(log.at(i)));
		if (alphaChange > 0xFF) alphaChange = 0xFF;
		tl_color(std::get<1>(log.at(i)) - alphaChange); //adjust color's alpha based on the difference between the current turn and the turn event occured
		renderString(std::get<0>(log.at(i)), 0, tl_yres() - log.size() + i);
	}
	tl_scale(1);
	tl_color(white);


	//the player's stats and inventory, as well as the mouseover tooltip and shop, are rendered by the Player class during movePlayer()
}

//add monsters (and pickups) to the world
void World::initializeMonsters(PickupDef* pickups){
	int count;
	loopInnerWorld([&,this](Cell& cell, int& i, int& j){
		count = getNearbyEmptyCells(Coordint(i,j));
		//found a dead end, maybe place a chest in it
		if (rand() % 2 && count == 2 && cell.getTile() == empty){
			cell.setPickup(new Pickup(pickups, 15, -1)); //add chest
		}

		//at empty tiles, sometimes create a new monster and place it there
		else if (cell.getTile() == empty && rand() % 30 == 0){
			cell.setActor(new Actor(Coordint(i,j), difficulty));
			if (rand() % 2) cell.setPickup(new Pickup(pickups, difficulty)); //add pickup
		}
	});
}

//add items to the shop
void World::initializeShop(PickupDef* pickups){
	for (int i = 0; i < 12; i++){
		shop.push_back(new Pickup(pickups, difficulty));
		while (shop[i]->getName() == "Gold Coins"){ //prevent the shop from selling gold
			delete shop[i];
			shop[i] = new Pickup(pickups, difficulty);
		}
	}
}

//move all monsters (called whenever the player moves)
void World::moveMonsters(Actor* player){
	vector<Actor*> actors; //vector of all monsters that need to move... I place monsters here as I find them to ensure that a monster doesn't move twice

	//find the monsters
	loopInnerWorld([&](Cell& cell, int& i, int& j){
		Actor* actor = cell.getActor();
		if (actor && actor != player) actors.push_back(actor);
	});

	//check if all monsters are slain
	if (!actors.size() && !worldClear){
		worldClear = true;
		log.print("You have slain all enemies here, hero.", green, player->getTurn());
	}
	else if (actors.size() && worldClear){ //player summoned more monsters with the Summon Monsters spell... so the world is no longer clear.
		worldClear = false;
	}

	//move the monsters
	for (int i = 0; i < actors.size(); i++){
		moveMonster(actors.at(i), player);
	}
}

//a Monster moves or attacks (called by moveMonsters)
void World::moveMonster(Actor* monster, Actor* player){
	Coordint& playerCoord = player->getLocation();
	Coordint& monsterCoord = monster->getLocation();
	int xdist = abs(playerCoord.x - monsterCoord.x);
	int ydist = abs(playerCoord.y - monsterCoord.y);
	int distance = xdist + ydist;

	if ((distance > 4 && monster->getChaseCount() > 9) || monster->getChaseCount() > 14) monster->setHome(monsterCoord); //set new home for the monster

	if (distance == 1) monster->attack(*player, world[playerCoord.x][playerCoord.y], *this); //monster attacks player
	else if (distance < 5 && monster->getChaseCount() < 10){ //monster moves towards nearby player
		bool hasTriedADirection = false;
		if (xdist > ydist || (xdist == ydist && rand() % 2)){ //monster tries to move horizonally
moveHor:	if (monsterCoord.x > playerCoord.x){ //monster tries to move left
				if (!move(monsterCoord, monsterCoord.x-1, monsterCoord.y, monster, true) && !hasTriedADirection){
					//failed to move left... move vertically instead
					hasTriedADirection = true;
					goto moveVert;
				}
			}
			else if (monsterCoord.x < playerCoord.x){ //monster tries to move right
				if (!move(monsterCoord, monsterCoord.x+1, monsterCoord.y, monster, true) && !hasTriedADirection){
					//failed to move right... move vertically instead
					hasTriedADirection = true;
					goto moveVert;
				}
			}
		}
		else{ //monster tries to move vertically
moveVert:	if (monsterCoord.y > playerCoord.y){ //monster tries to move up
				if (!move(monsterCoord, monsterCoord.x, monsterCoord.y-1, monster, true) && !hasTriedADirection){
					//failed to move up... move horizontally instead
					hasTriedADirection = true;
					goto moveHor;
				}
			}
			else if (monsterCoord.y < playerCoord.y){ //monster tries to move down
				if (!move(monsterCoord, monsterCoord.x, monsterCoord.y+1, monster, true) && !hasTriedADirection){
					//failed to move down... move horizontally instead
					hasTriedADirection = true;
					goto moveHor;
				}
			}
		}
	}
	else{ //monster moves randomly
		for (int i = 0; i < 4; i++){ //attempt to move 4 times... we must randomly choose an empty space to actually move
			int direction = rand() % 4;
			if (direction == 0){
				if (move(monsterCoord, monsterCoord.x, monsterCoord.y-1, monster, monster->getChaseCount() > 9)) return;
			}
			else if (direction == 1){
				if (move(monsterCoord, monsterCoord.x, monsterCoord.y+1, monster, monster->getChaseCount() > 9)) return;
			}
			else if (direction == 2){
				if (move(monsterCoord, monsterCoord.x+1, monsterCoord.y, monster, monster->getChaseCount() > 9)) return;
			}
			else{
				if (move(monsterCoord, monsterCoord.x-1, monsterCoord.y, monster, monster->getChaseCount() > 9)) return;
			}
		}
	}
}

//move a monster (called by moveMonster)
bool World::move(Coordint& monsterCoord, int newx, int newy, Actor* monster, bool isChasing){
	Coordint& home = monster->getHome();
	int xdist = abs(home.x - newx);
	int ydist = abs(home.y - newy);
	int distance = xdist + ydist;

	if ((isChasing || distance < 5) && world[newx][newy].getTile() == empty && world[newx][newy].getActor() == 0){ //monster moves
		world[newx][newy].setActor(monster);
		world[monsterCoord.x][monsterCoord.y].setActor(0);
		monsterCoord = Coordint(newx,newy);
		if (isChasing) monster->incrementChaseCount(); //monster slowly tires of chasing
		return true;
	}

	return false;
}

Log& World::getLog(){
	return log;
}

FILE* World::getFile() const{
	return file;
}

int World::getDifficulty() const{
	return difficulty;
}

bool World::getWorldClear() const{
	return worldClear;
}

bool World::getLoadingError() const{
	return loadingError;
}

void World::hadLoadingError(){
	loadingError = true;
}

void World::changeZoom(){
	zoom = 3 - zoom; //alternate between 1 and 2
}

vector<Pickup*>& World::getShop(){
	return shop;
}

//loops through the world, excluding the 1-tile border of walls
void World::loopInnerWorld(function<void (Cell& cell, int& i, int& j)> f){
	for (int i = 1; i < worldsize - 1; i++){
		for (int j = 1; j < worldsize - 1; j++){
			f(world[i][j], i, j);
		}
	}
}

//loops through the world
void World::loopWorld(function<void (Cell& cell, int& i, int& j)> f){
	for (int i = 0; i < worldsize; i++){
		for (int j = 0; j < worldsize; j++){
			f(world[i][j], i, j);
		}
	}
}

World::~World(){
	for (int i = 0; i < worldsize; i++){
		for (int j = 1; j < worldsize - 1; j++){
			Actor* actor = world[i][j].getActor();
			if (actor && actor->getTile() != playerTile) delete actor;
			delete world[i][j].getPickup();
		}
		delete[] world[i];
	}
	delete[] world;

	for (int i = 0; i < shop.size(); i++){
		delete shop[i];
	}
}