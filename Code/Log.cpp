#include "Log.h"

/** Consuctor: Create a new combat log for the game. */
Log::Log(){
}

/** Destructor */
Log::~Log(){
}

/**
* Print text to the log
*
* @param text	the text to add to the log
* @param color	the color the text will be when we print it (in RGBA)
* @param turn	the current turn number of the game
*/
void Log::print(string text, int color, int turn){
	log.push_back(tuple<string, int, int>(text, color, turn));
}

/**
* Add 'a' or 'an' to the start of a word
*
* @param word	the word
* @param caps	true if we want 'A' or 'An', false if we want 'a' or 'an'
* @return		the resultant string from adding 'a' or 'an' to the word
*/
string Log::addArticle(string word, bool caps) const{
	char first = word.at(0);
	if (first == 'A' || first == 'E' || first == 'I' || first == 'O' || first == 'U') {
		if (caps) return "An " + word;
		return "an " + word;
	}
	if (caps) return "A " + word;
	return "a " + word;
}

vector<tuple<string, int, int>>& Log::getLog(){
	return log;
}