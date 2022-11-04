/*
 Name:		DN_ComCanal.cpp
 Created:	02/11/2022 16:42:54
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_ComCanal.h"

DN_ComCanal::DN_ComCanal(){
	closeCom();
}

bool DN_ComCanal::is_free()
{
	return !used;
}

bool DN_ComCanal::is_ComNo(int comNo_)
{
	return (this->comNo == comNo_ && !is_free());
}

bool DN_ComCanal::is_ComId(const char* comId)
{
	return ((strcmp(this->comId, comId) == 0) && !is_free());
}

bool DN_ComCanal::startCanal(const int comNo, const unsigned int timeOut)
{
	this->comNo = comNo;
	this->timeOut = timeOut;
	this->lastActivity = millis();
	used = true;

	return true;
}

void DN_ComCanal::setupCanal(const char externalDeviceId[19], const char comId[26], const int step, const int duplicationSafe)
{
	if (ready) {
		Serial.println("[ComCanal] Liberer d'abord le canal avant de pouvoir le configurer à nouveau");
		return;
	}
	strcpy(this->externalDeviceId, externalDeviceId);
	strcpy(this->comId, comId);
	this->step = step;
	this->duplicationSafe = duplicationSafe;
	ready = true;
}

bool DN_ComCanal::availableMsg()
{
	return newMessage;
}

char DN_ComCanal::getMessage(char* message, int& len)
{
	const int inMsgLen = strlen(message);
	if (!ready) {
		Serial.println("[Com entity] La com n'est pas setup, lecture des messages impossible");
		len = 1;
		strcpy(message, "");
		return -2;
	}
	if (!this->availableMsg()) {
		Serial.println("[Com entity] La com n'a pas de nouveau message");
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
	strncpy(this->inMessage, "", 1);
	newMessage = false;

	return 1;
}

void DN_ComCanal::handleNewMessage(const char* IncommingMessage)
{
	if (!ready) {
		Serial.println("[ComCanal] canal is not ready for reception");
		return;
	}
	DynamicJsonDocument data(300);
	DynamicJsonDocument multiComHeaderJson(300);
	DynamicJsonDocument doc(768);
	DeserializationError error;

	//char messageLocalCopy[500];

	int stepReceived;
	int duplicationSafeReceived;
	bool endComReceived;

	//strncpy(messageLocalCopy, IncommingMessage, 500);
	error = deserializeJson(doc, IncommingMessage);
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
	lastActivity = millis();
	return;
}

void DN_ComCanal::sendBrutMessage(const char* message, const int step)
{
	if (!ready) {
		Serial.println("[ComCanal] canal is not ready for sending"); 
		return;
	}
	char topic[11] = "testTopic";
	//snprintf(topic, 40, "com/%s/multiCom", externalDeviceId);
	mqttManager->sendMessage(topic, message);
	lastActivity = millis();
	duplicationSafe++;
	this->step = step;
}

void DN_ComCanal::setMqttManager(DN_MQTTclass* mqtt)
{
	this->mqttManager = mqtt;
}

bool DN_ComCanal::checkDuplicationSafe(const int receivedStep, const int receivedDuplicationSafe)
{
	if (receivedDuplicationSafe > this->duplicationSafe && this->step < receivedStep) {
		return true;
	}
	return false;
}

void DN_ComCanal::handle() {
	checkTimeOut();
}

void DN_ComCanal::checkTimeOut()
{
	if (!used) return;
	if (millis() - lastActivity < timeOut) return;
	lastActivity = millis();

	if (!endCom) {
		endCom = true;
		strcpy(inMessage, "{\"comStatus\": \"timed_out\"}");
		newMessage = true;
		//Serial.println(inMessage);
	}
	else {
		closeCom();
	}
}

void DN_ComCanal::closeCom()
{
	this->comNo = 0;
	this->timeOut = 0;
	this->lastActivity = 0;
	duplicationSafe = 0;
	step = 0;
	strcpy(this->externalDeviceId, "");
	strcpy(this->comId, "");
	newMessage = false;
	strcpy(inMessage, "");
	endCom = false;
	ready = false;
	used = false;
}
