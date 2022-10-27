// 
// 
// 

#include "DN_LitlleFs.h"

DN_LitlleFsClass::DN_LitlleFsClass()
{
 
    if (!LittleFS.begin()) {
        Serial.println("An Error has occurred while mounting LittleFS");
        loaded = false;
        return;
    }
    loaded = true;
}

DN_LitlleFsClass::~DN_LitlleFsClass()
{
	LittleFS.end();

}

void DN_LitlleFsClass::init()
{
}

bool DN_LitlleFsClass::ready()
{
    if (!loaded) Serial.println("!! LittleFS n'a pas pu etre charge !!");
    return loaded;
}

File DN_LitlleFsClass::openFile(char* uri, char* mode)
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


