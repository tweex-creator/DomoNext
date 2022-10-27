// DN_LitlleFs.h

#ifndef _DN_LITLLEFS_h
#define _DN_LITLLEFS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <LittleFS.h>

class DN_LitlleFsClass
{
 protected:
	 bool loaded;

 public:
	 DN_LitlleFsClass();
	 ~DN_LitlleFsClass();
	void init();
	bool ready();
	File openFile(char* uri, char* mode);
};


#endif

