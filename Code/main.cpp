//roguelike game created by Andrew Socha

#include "World.h"
#include "Player.h"

using namespace std;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	tl_init("My Game", 1280, 720, "tiles", 32, 3);

	World world(1);
	Player* player;
	FILE* file = fopen("saveGame.txt", "rb");
	int gameMode = 1; //1 for normal, 2 for speed run, 3 for load

	//menu screen
	while (true){
		tl_framestart(0);

		//up and down menu scrolling
		if (!file && (tl_keywentdown("up") || tl_keywentdown("w") || tl_keywentdown("down") || tl_keywentdown("s"))){
			gameMode = 3-gameMode;
		}
		else if (tl_keywentdown("up") || tl_keywentdown("w")){
			gameMode--;
			if (gameMode == 0) gameMode = 3;
		}
		else if (tl_keywentdown("down") || tl_keywentdown("s")){
			gameMode++;
			if (gameMode == 4) gameMode = 1;
		}
		else if (tl_keywentdown("enter") || tl_keywentdown("return")){ //begin a game
			break;
		}
		else if (tl_keywentdown("escape")){ //close game
			tl_shutdown();
		}

		world.renderMenu(gameMode, file);
	}

	if (gameMode == 3){ //load game
		player = new Player(world, file);
	}
	else if (gameMode == 1){ //normal game
		world.initializeWorld();
		player = new Player(world, false);
	}
	else{ //speed run
		world.initializeWorld();
		player = new Player(world, true);
	}

	if (file) fclose(file);
	
	//play game
	while(true){
		tl_framestart(0);
		
		world.renderViewport(player->getLocation());

		player->movePlayer(world);

		//close the game on "escape"
		if (tl_keywentdown("escape")){
			player->saveGame(world);
			world.getCell(player->getLocation()).setActor(0);
			delete player;
			tl_shutdown();
		}
	}
}