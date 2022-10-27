/*
 Name:		DN_Memory.h
 Created:	27/10/2022 10:02:09
 Author:	Herve
 Editor:	http://www.visualmicro.com
 Description: This library manage the non volatil memory
*/

#ifndef _DN_Memory_h
#define _DN_Memory_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

#include <LittleFS.h>

class DN_Memory
{
protected:
	bool loaded;

public:
	DN_Memory();
	~DN_Memory();
	void init();
	bool ready();
	File openFile(char* uri, char* mode);
};
#endif

