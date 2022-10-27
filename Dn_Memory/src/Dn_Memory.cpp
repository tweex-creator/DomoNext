/*
 Name:		DN_Memory.cpp
 Created:	27/10/2022 10:02:09
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_Memory.h"



DN_Memory::DN_Memory()
{

    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
        loaded = false;
        return;
    }
    loaded = true;
}

DN_Memory::~DN_Memory()
{
    LittleFS.end();

}

void DN_Memory::init()
{
}

bool DN_Memory::ready()
{
    if (!loaded) Serial.println("!! LittleFS n'a pas pu etre charge !!");
    return loaded;
}

File DN_Memory::openFile(char* uri, char* mode)
{
    if (ready()) {
        // Serial.println("[LITLLEFS] acess");
        File file = LittleFS.open(uri, mode);
        if (!file) {
            Serial.print("Failed to open file ");
            Serial.print(uri);
            Serial.println(" for reading");
        }
        return file;
    }
    return File();
}