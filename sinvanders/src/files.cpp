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
 


#include <fstream>
#include <iostream>
#include <list>
#include <dirent.h>

#include "base.h" 
#include "images.h"

#include "files.h"


using namespace std;

class Info
	{
public:
	enum Type
		{
		EUnknown, EImage, EAudio, EAnimation, EMask
		};
	string iName;
	string iFile;
	Type iType;
	vector<int> iValues;
	
	~Info();
	};

Info::~Info()
	{
	iValues.clear();
	}


	
InfoReader::~InfoReader()
	{
	for(vector<Info*>::const_iterator i = iInfo.begin(); i != iInfo.end(); i++)
		{
		delete (*i);
		}
	iInfo.clear();
	}	
	
const Info& InfoReader::get(const string& name) const
	{
	vector<Info*>::const_iterator i = iInfo.begin();
	for(; i != iInfo.end(); i++)
		{
		if((*i)->iName.compare(name) == 0)
			return *(*i);
		}
	throw InvanderException("name not found:\'" + name + "\'");
	return *(*i); //avoid warning in  some systems, generate a warning in some :-(	
	}
	
void trimmed(string& tgt, const string& src, int pos, int len)
	{
	tgt.clear();
	for(int i = pos ; i < len ; i++)
		{
		const char c = src[i];
		if(c > ' ')
			tgt += c;
		}
	}	
	
bool InfoReader::parse(string& ss, int occur) const
	{
	const string& str = *iCurrentLine;
	int start = 0;
	int end = 0;
	int endHere = str.find(';');
	if(endHere == string::npos)
		return false;
	for(;;)
		{
		int found = str.find(',', start);
		if(found == string::npos || found > endHere)
			end = endHere;
		else
			end = found;
		if(occur <= 0)
			{
			if(end > start)
				{
				trimmed(ss, str, start, end);
				//ss = str.substr(start, end - start);
				//std::remove(ss.begin(), ss.end(), ' ');
				return true;
				}
			return false;
			}			
		--occur;
		start = end + 1;
		}
	}	
	
InfoReader::InfoReader(ContainerInterface& add, const string& name, bool trySmallImages) : iAdd(add), iTrySmallImages(trySmallImages)
	{
	ifstream file;
	file.open(name.c_str());
	if(!file.is_open())
		throw InvanderException("Info not found");
	string line;
	while(std::getline(file, line))
		{
		iCurrentLine = &line;
		string type;
		if(!parse(type, 0))
			continue; //empty line
		Info* info = new Info();
		iInfo.push_back(info);
		parse(info->iName, 1);
		parse(info->iFile, 2);
		if(type == "i")
			{
			string rate;
			if(!parse(rate, 3))
				info->iType = Info::EImage;
			else
				{
				info->iType = Info::EAnimation;
				info->iValues.push_back(atoi(rate.c_str()));
				string frames;
				parse(frames, 4);
				info->iValues.push_back(atoi(frames.c_str()));
				}
			}
		else if(type== "m")
			{
			info->iType = Info::EMask;
			}
		else if(type == "a")
			{
			info->iType = Info::EAudio;
			}
		else if(type == "d")
			{
			string property;
			parse(property, 1);
			if(property == "datafolder")
				parse(iFolder, 2);
			}
		else
			{
			throw InvanderException("Info corrupted");
			}
		}
	file.close();
	if(iFolder.size() > 0 && iInfo.size() > 0)
		{
		setPathSeparator();
		}
	}
	
void InfoReader::setPathSeparator()
	{	
	ifstream file;
	string name = iFolder + "/license.txt";
	file.open(name.c_str());
	char separator = 0;
	if(file.is_open())
		{
		separator= '/';
		}
	else
		{
		name = iFolder + "\\license.txt";
		file.open(name.c_str());
		if(file.is_open())
			{
			separator = '\\';
			}
		else
			{
			throw InvanderException("No license found!");
			}
		}
	file.close();
	for(vector<Info*>::iterator ii = iInfo.begin(); ii != iInfo.end(); ii++)
		{
		(*ii)->iFile = iFolder + separator + (*ii)->iFile;
		}
	}
	
const string& InfoReader::file(const string& name) const	
	{
	return get(name).iFile;
	}
	
Image* InfoReader::loadImage(const string& name) const
	{
	const Info& info = get(name);
	Image* image = NULL;
	if(info.iType == Info::EAnimation)
		{
		AnimationImage* a = new AnimationImage();
		iAdd.addImage(a);
		iAdd.addAnimation(a);
		a->loadBmp(info.iFile, iTrySmallImages);
		a->setFrameRate(info.iValues[0]);
		a->setFrameCount(info.iValues[1]);
		image = a;
		}
	else if(info.iType == Info::EImage)
		{
		image = new Image();
		iAdd.addImage(image);
		image->loadBmp(info.iFile, iTrySmallImages);
		}
	else if(info.iType == Info::EMask)
		{
		image = new Image();
		iAdd.addImage(image);
		image->loadBmp(info.iFile, iTrySmallImages, true);
		}
	else
		{
		throw InvanderException(name + "info");
		}
	return image;
	}	


class intread : public ifstream
	{
public:
	intread(const string& name);
	intread& operator>>(int& value);
private:
	int read();
	};
	
intread::intread(const string& name) : ifstream(name.c_str(), ios::binary)
	{
	}
	
intread& intread::operator>>(int& value)
	{
	value = read();
	return *this;
	}
	
int intread::read()
	{
	int value;
	ifstream::read(reinterpret_cast<char*>(&value), sizeof(int));
	return value;
	}
	
class intwrite : public ofstream
	{
public:
	intwrite(const string& name);
	intwrite& operator<<(const int& value);
private:
	void write(int value);
	};
	
intwrite::intwrite(const string& name) : ofstream(name.c_str(), ios::binary)
	{
	}
	
intwrite& intwrite::operator<<(const int& value)
	{
	write(value);
	return *this;
	}	
		
void intwrite::write(int value)
	{
	ofstream::write(reinterpret_cast<const char*>(&value), sizeof(int));
	}	
	

	

bool SettingsFile::readValue(int id, int& value) const
	{
	intread ii(iFileName);
	if(ii.is_open())
		{
		int count;
		ii >> count;
		for(int i = 0 ; i < count; i++)
			{
			int tid;
			ii >> tid;
			int tvalue;
			ii >> tvalue;
			if(id == tid)
				{
				value = tvalue;
				ii.close();
				return true;
				}
			}
		ii.close();
		}
	return false;
	}
	
void SettingsFile::writeValue(int id, int value) const
	{
	intread ii(iFileName);
	vector<int> values;
	bool found = false;
	if(ii.is_open())
		{
		int count;
		ii >> count;
		for(int i = 0 ; i < count; i++)
			{
			int tid;
			ii >> tid;
			int tvalue;
			ii >> tvalue;
			if(id == tid)
				{
				tvalue = value;
				found = true;
				}
			values.push_back(tid);
			values.push_back(tvalue);
			}
		ii.close();
		}
	if(!found)
		{
 		values.push_back(id);
		values.push_back(value);
		}
	intwrite oo(iFileName);
	if(oo.is_open())
		{
		oo << values.size() / 2;
		for(vector<int>::const_iterator it = values.begin(); it != values.end();)
			{
			oo << (*it);
			++it;
			oo << (*it);
			++it;
			}
		oo.close();
		}
	values.clear();
	}

SettingsFile::SettingsFile(const string& name) : iFileName(name)
	{
	}
	
InfoFile::InfoFile() : iCurrent("sinvander_info.txt")	
	{
	ifstream is("infos");
	if(is.is_open())
		{
		is >> iCurrent;
		}
	is.close();
	}
	
void InfoFile::set(const string& current)
	{
	iCurrent = current;
	}	

const string& InfoFile::current() const
	{
	return iCurrent;
	}	
	
void InfoFile::next()
	{
	DIR* dir = ::opendir(".");
	if(dir == NULL)
		throw InvanderException("Cannot open dir");
	list<string> names;
	const string ext("_info.txt");
	const int sz = ext.size();
	for(;;)
		{
		dirent* de = ::readdir(dir);
		if(de == NULL)
			break;
		string s(de->d_name);
		if(s.size() > sz && ext.compare(0, sz, s, s.size() - sz, sz) == 0)
			{
			names.push_back(s);
			}
		}
	::closedir(dir);
	if(names.size() > 1)
		{
		names.sort();
		for(list<string>::const_iterator it = names.begin(); it != names.end(); it++)
			{
			if(*it == iCurrent)
				{
				it++;
				if(it != names.end())
					iCurrent = *it;
				else
					iCurrent = *(names.begin());
				ofstream os("infos");
				os << iCurrent;
				os.close();
				return;
				}
			}
		}
	}

	
