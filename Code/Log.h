#pragma once

#include <vector>
#include <string>
#include "tilelib.h"
#include <tuple>

using namespace std;

const int red = 0xCC0000FF; //rgba
const int cantAffordRed = 0xCC5555FF;
const int yellow = 0xFFFF00FF;
const int orange = 0xFF7700FF;
const int green = 0x009922FF;
const int blue = 0x0000BBFF;
const int equippedYellow = 0xBB8800FF;
const int white = 0xFFFFFFFF;
const int background = 0xFFFFFF44;
const int difficultyColors[3] = {green, yellow, orange};

/**
* Implements a combat log that is printed to the bottom of the page
*
* @author Andrew Socha
*/
class Log{
	public:
		Log();
		~Log();

		void print(string text, int color, int turn);
		string addArticle(string word, bool caps) const;
		vector<tuple<string, int, int>>& getLog();

	private:
		vector<tuple<string, int, int>> log;
};