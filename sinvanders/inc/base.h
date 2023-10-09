/*
    SDL SPACE INVADERS - The Game
    Copyright (C) 2009  Markus Mertama

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */
 

#include<exception>
#include<string>

using namespace std;

#ifndef BASE_H
#define BASE_H



class Size
    {
public:
    int w, h;
    };
    
class Position
    {
public:
    int x, y;
    };  
    
class Rect  
    {
public:
    int x, y, w, h;
public:
    inline Position br() const;
    inline bool isIn(const Position& pos) const;
    };
	

inline Position Rect::br() const
	{
	Position p = {x + w, y + h};
	return p;
	}
	
inline bool Rect::isIn(const Position& pos) const
	{
	return pos.x >= x && pos.y >= y && pos.x < x + w && pos.y < y + h; 
	}	
	
class Mapper
	{
public:
	inline static int toScreen(int x, int w);
	inline static int toView(int x, int w);
	};
	
inline int Mapper::toScreen(int x, int w) 
	{
//unfortunate optimization here, but I had to write this as this function is called 
//often and I wanted to avoid division here - this code really assumes that 
//view size is always 256 x 256
	return  (x * (w << 8)) >> 16; //((width << 16) / 255) >> 16  
	}
	
inline int Mapper::toView(int x, int w) 
	{
	return (x << 8) / w;
	}
	
class InvanderException : public std::exception
    {
public:
    InvanderException(const string& aReason);
    const char* what() const throw();
	~InvanderException() throw() {}
private:
    std::string iReason;
    };
	
#endif
		