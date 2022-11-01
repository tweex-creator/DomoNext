/*
 Name:		DN_ComEntity.cpp
 Created:	28/10/2022 16:02:33
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_ComEntity.h"

bool DN_ComCanal::is_free()
{
	return free;
}

bool DN_ComCanal::is_ComNo(int comNo)
{
	return (this->comNo == comNo && !is_free());
}

bool DN_ComCanal::startCanal(const int comNo, const unsigned int timeOut)
{
	if (!is_free) return false;
	this->free = false;
	this->comNo = comNo;
	this->timeOut = timeOut;
	this->lastActivity = millis();
	return true;
}

void DN_ComCanal::setupCanal(const char externalDeviceId[19], const char comId[26])
{
	if (ready) {
		Serial.println("[ComCanal] Liberer d'abord le canal avant de pouvoir le configurer à nouveau");
		return;
	}
	strcpy(this->externalDeviceId, externalDeviceId);
	strcpy(this->comId, comId);
	ready = true;
}

bool DN_ComCanal::availableMsg()
{
	return newMessage;
}

char DN_ComCanal::getMessage(char* message, int& len)
{
	const char inMsgLen = strlen(message);
	if (!ready) {
		Serial.println("[Com entity] La com n'est pas setup, lecture des messages impossible");
		len = 1;
		strcpy(message, "");
		return -2;
	}
	if (!this->availableMsg()) {
		len = 1;
		strcpy(message, "");
		return 0;
	}

	if (inMsgLen > len) {
		len = inMsgLen;
		strcpy(message, "");
		return -1;
	}

	strncpy(message, this->inMessage, len);
	len = inMsgLen;
	return 1;
}

void DN_ComCanal::handleNewMessage(const char* IncommingMessage, const int len)
{
	StaticJsonDocument<364> doc;
	StaticJsonDocument<100> multiComHeaderJson;
	StaticJsonDocument<300> data;
	DeserializationError error;

	char messageLocalCopy[500];

	int stepReceived;
	int duplicationSafeReceived;
	bool endComReceived;

	strncpy(messageLocalCopy, IncommingMessage, 500);
	error = deserializeJson(doc, messageLocalCopy, 500);
	if (error) {
		Serial.print(F("[ComCanal] deserializeJson() handleNewMessage failed: "));
		Serial.println(error.f_str());
		return;
	}

	if (doc["multiComHeader"].isNull()) {
		Serial.println("[DN_ComCanal] multicom header missing (handleNewMessage)");
		return;
	}
	multiComHeaderJson = doc["multiComHeader"];

	if (multiComHeaderJson["step"].isNull() || multiComHeaderJson["duplicationSafe"].isNull() || multiComHeaderJson["comId"].isNull() || multiComHeaderJson["endCom"].isNull()) {
		Serial.println("[DN_ComCanal] multicom header missed endCom, step, duplicationSafe or comId");
		Serial.println("----");
		Serial.println(IncommingMessage);
		Serial.println("----");

		return;
	}
	stepReceived = multiComHeaderJson["step"];
	duplicationSafeReceived = multiComHeaderJson["duplicationSafe"];
	endComReceived = multiComHeaderJson["endCom"];
	//Verification pas de doublon
	if (!this->checkDuplicationSafe(stepReceived, duplicationSafeReceived)) {
		Serial.println("[DN_ComCanal] Reception doublon");
		return;
	}
	this->duplicationSafe = duplicationSafeReceived;
	this->step = stepReceived;



	if (!doc["data"].isNull()) {
		data = doc["data"];
	}
	else {
		Serial.println("[DN_ComCanal] Pas de contenu (handleNewMessage)");
	}
	if (endComReceived) {
		data["comStatus"] = "closed";// la communication a été fermée par l'objet distant

	}
	else {
		data["comStatus"] = "open";// l'objet distant attend une reponse
	}
	
	serializeJson(data, inMessage);
	newMessage = true;
	return;
}


bool DN_ComCanal::checkDuplicationSafe(const int receivedStep, const int receivedDuplicationSafe)
{
	if (receivedDuplicationSafe > this->duplicationSafe && this->step < receivedStep) {
		return true;
	}
	return false;
}