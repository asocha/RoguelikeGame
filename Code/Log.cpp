#include "Log.h"

Log::Log(){
}

Log::~Log(){
}

//print text to the log
void Log::print(string text, int color, int turn){
	log.push_back(tuple<string, int, int>(text, color, turn));
}

//add 'a' or 'an' to the start of a word
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