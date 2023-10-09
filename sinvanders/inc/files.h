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

#include<string>
#include<vector>


class Image;
class Info;

class SettingsFile
	{
public:
	SettingsFile(const string& name);
	void writeValue(int id, int value) const;
	bool readValue(int id, int& value) const;
private:
	string iFileName;
	};
	
class ContainerInterface
	{
public:
	virtual void addAnimation(AnimationInterface* anim) = 0;
	virtual void addImage(Image* image) = 0;
	};
	
class InfoReader
	{
public:
	InfoReader(ContainerInterface& add, const string& name, bool trySmallImages);
	Image* loadImage(const string& name) const;
	const string& file(const string& name) const;
	~InfoReader();
private:
	const Info& get(const string& name) const;
	bool parse(string& ss, int occur) const;
	void setPathSeparator();
private:
	ContainerInterface& iAdd;
	const bool iTrySmallImages;
	vector<Info*> iInfo;
	const string* iCurrentLine;
	string iFolder;
	};	
	
class InfoFile
	{
public:
	InfoFile();
	void set(const string& file);
	void next();
	const string& current() const;
private:
	string iCurrent;
	};	
		