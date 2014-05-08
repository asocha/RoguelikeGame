//Coord: signifies an X,Y location in the world
#pragma once

template <class T> struct Coord {
	Coord<T>(){ x = 0; y = 0; }
	Coord<T>(T x, T y){ this->x = x; this->y = y; }
	T x, y; 

	Coord operator+(const Coord &o) { return Coord(x+o.x, y+o.y); }

	Coord up(){ return Coord(x, y-1); }
	Coord down(){ return Coord(x, y+1); }
	Coord left(){ return Coord(x-1, y); }
	Coord right(){ return Coord(x+1, y); }
};

typedef Coord<int> Coordint;