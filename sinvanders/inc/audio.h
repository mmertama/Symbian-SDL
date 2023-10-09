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
 
#ifndef AUDIO_H
#define AUDIO_H

#include <string>
#include <exception>

class AudioPlr;

class AudioException : public std::exception
    {
public:
    AudioException(const std::string& reason);
    const char* what() const throw();
    ~AudioException() throw() {}
private:
    std::string iReason;
    };

class Audio
    {
public:
    Audio();
    ~Audio();
    void stop(int handle);
    void set(const std::string& string, int handle);
    void play(int handle, bool loop);
    void resume();
    void pause();
private:
    AudioPlr* iAudioPlr;
    };

#endif
