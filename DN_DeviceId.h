// DN_DeviceId.h

#ifndef _DN_DEVICEID_h
#define _DN_DEVICEID_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <Dn_Memory.h>
#include <LittleFS.h>

#include "ArduinoJson-v6.19.4.h"
class DN_WIFIClass;

class DN_DeviceId
{
public:
	DN_DeviceId(DN_Memory& littleFsManager_);

	void saveToMemory();
	bool loadFromMemory();	

	void setId(const char newId[4]);
	void setName(const char newName[26]);
	void setMac(const char newMac[19]);

	void getId(char id_[4]);
	void getName(char name_[26]);
	void getMac(char mac_[19]);

	void reset();

	void handle();
private:
	char id[4];//! Id de 3 caractères + terminateur \0
	char name[26];//max 25 caractere 
	char mac[20];//max 19 caractere 

	DN_Memory& memoryManager;
	bool change;
};
#endif

