#pragma once

/**
* Signifies an X,Y location in the world
*
* @author Andrew Socha
*/
template <class T> struct Coord {
	/** Default Constructor */
	Coord<T>(){ x = 0; y = 0; }

	/**
	* Creates a coordinate object, representing a location in the game world
	* 
	* @param x	the x coordinate
	* @param y	the y coordinate
	*/
	Coord<T>(T x, T y){ this->x = x; this->y = y; }
	T x, y; 

	/**
	* Add another coordinate to this one
	* 
	* @param coord	the 2nd coordinate
	* @return		the resulting coordinate
	*/
	Coord operator+(const Coord &coord) { return Coord(x+coord.x, y+coord.y); }

	/**
	* Get the coordinate above this one
	* 
	* @return	the resulting coordinate
	*/
	Coord up(){ return Coord(x, y-1); }

	/**
	* Get the coordinate below this one
	* 
	* @return	the resulting coordinate
	*/
	Coord down(){ return Coord(x, y+1); }

	/**
	* Get the coordinate to the left of this one
	* 
	* @return	the resulting coordinate
	*/
	Coord left(){ return Coord(x-1, y); }

	/**
	* Get the coordinate to the right of this one
	* 
	* @return	the resulting coordinate
	*/
	Coord right(){ return Coord(x+1, y); }
};

typedef Coord<int> Coordint;