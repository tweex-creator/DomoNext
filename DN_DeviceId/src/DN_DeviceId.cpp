/*
 Name:		DN_DeviceId.cpp
 Created:	27/10/2022 10:27:11
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_DeviceId.h"
#include <ESP8266WebServer.h>

DN_DeviceId* deviceIdForWeb;
void handleDeviceIdWeb(ESP8266WebServer& server) {
	if (server.hasArg("DeviceIdNewName")) {
		deviceIdForWeb->setName(server.arg("DeviceIdNewName").c_str());
	}

}

void GenerateDeviceIdWeb(ESP8266WebServer& server) {
	server.sendContent("<label> Nom de l'objet: </label>");
	server.sendContent("<input type=\"text\" name=\"DeviceIdNewName\" value=\"");
	char name[26];
	deviceIdForWeb->getName(name);
	server.sendContent(name);
	server.sendContent("\" required>");
	server.sendContent("<input type=\"submit\" value=\"Enregistrer\">");
}

DN_DeviceId::DN_DeviceId(DN_Memory& littleFsManager_) :memoryManager(littleFsManager_)
{
	deviceIdForWeb = this;
	change = false;
	if (!loadFromMemory()) {
		reset();
	}
}

void DN_DeviceId::reset()
{
	strcpy(id, "ZZZ"); // L'i ZZZ correspond a un id non defini
	strcpy(name, "no name");
	change = true;

	saveToMemory();
}

void DN_DeviceId::handle()
{
	saveToMemory();
}

bool DN_DeviceId::loadFromMemory() {
	StaticJsonDocument<200> doc;

	File f = memoryManager.openFile("/Id/id.txt", "r");
	DeserializationError error = deserializeJson(doc, f);

	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("deserializeJson() failed: "));
		Serial.println(error.f_str());
		f.close();
		return false;
	}
	else {
		Serial.println("data load:");
		strcpy(this->id, doc["id"]);
		strcpy(this->name, doc["name"]);
		Serial.print("id:");
		Serial.println(id);
		Serial.print("name:");

		Serial.println(name);
		f.close();

		return true;

	}
}

void DN_DeviceId::saveToMemory()
{
	if (change) {
		File f = LittleFS.open("/Id/id.txt", "w");
		StaticJsonDocument<64> doc;
		doc["id"] = id;
		doc["name"] = name;
		serializeJson(doc, f);
		f.close();
		f = LittleFS.open("/Id/id.txt", "r");
		f.close();
		//Serial.println("saved");
		change = false;
	}

}

void DN_DeviceId::setId(const char newId[4])
{
	strcpy(id, newId);
	change = true;
}

void DN_DeviceId::setName(const char newName[26])
{
	strcpy(name, newName);
	change = true;
}

void DN_DeviceId::setMac(const char newMac[19])
{
	strcpy(mac, newMac);
	change = true;
}

void DN_DeviceId::getId(char id_[4])
{
	strcpy(id_, id);
}

void DN_DeviceId::getName(char name_[26])
{
	strcpy(name_, name);
}

void DN_DeviceId::getMac(char mac_[19])
{
	strcpy(mac_, mac);

}

void DN_DeviceId::getIdOrMac(char id_Mac[19])
{
	this->getId(id_Mac);
	if (strcmp(id_Mac, "ZZZ") != 0) return;
	this->getMac(id_Mac);

}


