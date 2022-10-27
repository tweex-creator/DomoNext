/*
 Name:		DN_Object.cpp
 Created:	27/10/2022 10:44:38
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_Object.h"


DN_Object* ptrToObj;


DN_Object::DN_Object(DN_MQTTclass& mqttBorker_, DN_WIFIClass& wifiManager_, DN_DeviceId& deviceId_) :brokerMqtt(mqttBorker_), deviceId(deviceId_), wifiManager(wifiManager_), data(2048)
{
}

void DN_Object::init()
{

	ptrToObj = this;
	connectServerState = -1;
	timeoutServerConnetion = 5000;
	brokerMqtt.addMqttReceiver(this);
	comCounter = 0;
	strcpy(lastSubId, "");
	subMqtt();
}

void DN_Object::subMqtt()
{
	char topic[64];
	char id[32];
	deviceId.getId(id);
	if (strcmp(id, lastSubId) != 0) {//si l'id n'a pas changé on re fait rien

		if (strcmp(id, "ZZZ") == 0) {
			deviceId.getMac(id);
		}
		sprintf(topic, "com/%s/updateValue/*/set", id);
		brokerMqtt.subscribe(topic);
		sprintf(topic, "com/%s/updateValue/*/set", lastSubId);
		brokerMqtt.unsubscribe(topic);
		sprintf(topic, "com/%s/pingPong", id);
		brokerMqtt.subscribe(topic);
		sprintf(topic, "com/%s/pingPong", lastSubId);
		brokerMqtt.unsubscribe(topic);
		sprintf(topic, "com/%s/kick", id);
		brokerMqtt.subscribe(topic);
		sprintf(topic, "com/%s/kick", lastSubId);
		brokerMqtt.unsubscribe(topic);
		sprintf(topic, "com/%s/multiCom", id);
		brokerMqtt.subscribe(topic);
		sprintf(topic, "com/%s/multiCom", lastSubId);
		brokerMqtt.unsubscribe(topic);
		sprintf(topic, "com/%s", lastSubId);
		char msg[21];
		sprintf(msg, "moved to id: %s", id);
		brokerMqtt.sendMessage(topic, msg);

		brokerMqtt.printSubscribtions();
	}
	deviceId.getId(lastSubId);//on met a jour le dernier id sauvegardé
}

void DN_Object::handleMqttMsg(JsonDocument& docMsgReceived) {

	if (!docMsgReceived["multiComHeader"].isNull()) {
		Serial.println("proceeding com");
		proceedCom(docMsgReceived);
		Serial.println("[COM] proceedCom setp 0 done 2");

	}

}


void DN_Object::reset()
{
	deviceId.reset();
}

bool DN_Object::isInJsonArray(const char* value, const JsonArray array)
{
	for (auto it = array.begin(); it != array.end(); ++it) {
		if (strcmp(value, (*it)) == 0) {
			return true;
		}
	}
	return false;
}

int DN_Object::initCom(char* externalDeviceId)
{
	unsigned int comNo = reserveComChanelAndComNo();// retourne le numero de la com != numero du chanel
	if (comNo != -1) {
		comLogger* com = getComFromNo(comNo);
		if (com == nullptr) {
			Serial.println("[COM] Fatal error (0)");
			return -1;
		}
		char comId[26];
		char objId[19];//si id = zzz prend la veleur de l'addr mac
		deviceId.getId(objId);
		if (strcmp(objId, "ZZZ") == 0) deviceId.getMac(objId);

		sprintf(comId, "%s_%d", objId, comNo);
		strcpy(com->externalDeviceId, externalDeviceId);
		strcpy(com->comId, comId);
		com->duplicationSafe = 0;
		char topic[54];
		sprintf(topic, "com/%s/multiCom", externalDeviceId);
		StaticJsonDocument<500> message;
		JsonObject multiComHeaderJson = message.createNestedObject("multiComHeader");
		multiComHeaderJson["deviceId"] = objId;
		multiComHeaderJson["comId"] = comId;
		multiComHeaderJson["comTimeout"] = 1000;
		multiComHeaderJson["step"] = 0;
		multiComHeaderJson["duplicationSafe"] = 0;
		multiComHeaderJson["endCom"] = false;

		char msg[1024];
		serializeJson(message, &msg, 1024);
		brokerMqtt.sendMessage(topic, msg);
		return comNo;

	}
	return -1;
}

void DN_Object::proceedCom(JsonDocument& respons)
{
	Serial.println("[COM] multicom (proceedCom)");

	if (respons["multiComHeader"].isNull()) {
		Serial.println("[COM] multicom header missing (proceedCom)");
		return;
	}

	//Recuperaton du multiComHeader

	DynamicJsonDocument multiComHeaderJson(200);
	Serial.print("mem:");
	Serial.println(multiComHeaderJson.memoryUsage());
	multiComHeaderJson = respons["multiComHeader"];

	Serial.print("mem:");
	Serial.println(multiComHeaderJson.memoryUsage());


	//Verification de la présence des éléments obligatoires
	if (multiComHeaderJson["step"].isNull() || multiComHeaderJson["duplicationSafe"].isNull() || multiComHeaderJson["comId"].isNull() || multiComHeaderJson["endCom"].isNull()) {
		Serial.println("[COM] multicom header missed endCom, step, duplicationSafe or comId (proceedCom)");
		return;
	}

	//Extraction des élements obligatoires
	const char* comId = multiComHeaderJson["comId"];
	int step = multiComHeaderJson["step"];
	int duplicationSafe = multiComHeaderJson["duplicationSafe"];
	bool endCom = multiComHeaderJson["endCom"];
	comLogger* com;
	com = getComFromId(comId);
	if (com == nullptr) {
		if (step == 0) {
			Serial.println("[COM] proceedCom setp 0");
			proceedNewCom(multiComHeaderJson);
			Serial.println("[COM] proceedCom setp 0 done0");

		}
		Serial.println("[COM] proceedCom setp 0 done 1");
		return;
		Serial.println("[COM] proceedCom setp 0 done1.1");

	}

	if (!checkDuplicationSafe(com, step, duplicationSafe)) {
		Serial.println("[COM] Reception doublon");
		return;
	}
	yieldComLogger(com);

	if (step == 1) {
		Serial.println("[COM] proceedCom setp 1");

		//Verification de la présence des éléments obligatoires
		if (multiComHeaderJson["success"].isNull()) {
			Serial.println("[COM] multicom header missed succeess (proceedCom setp == 1)");
			return;
		}
		bool success = multiComHeaderJson["success"];
		if (success) {
			strcpy(com->lastReceived, "{\"comInit\": true}"); // on place un message vide
			com->newMessageFlag = 1;
		}
		else {
			strcpy(com->lastReceived, "{\"comInit\": false}"); // on place un message vide
			com->newMessageFlag = 1;
		}
	}

	if (endCom) {
		com->endCom = true;
	}

}

void DN_Object::proceedNewCom(JsonDocument& multiComHeaderJson)
{
	if (multiComHeaderJson["deviceId"].isNull() || multiComHeaderJson["comTimeout"].isNull()) {
		Serial.println("[COM] multiComHeaderJson missed deviceId or comTimeout");
		return;
	}
	Serial.println("[COM] retrieving com data");
	brokerMqtt.sendMessage("topic", "msg");

	const char* comId = multiComHeaderJson["comId"];
	const char* externalDeviceId = multiComHeaderJson["deviceId"];
	int duplicationSafe = multiComHeaderJson["duplicationSafe"];
	int step = multiComHeaderJson["step"];
	int comNo = reserveComChanelAndComNo();
	int comTimeout = multiComHeaderJson["comTimeout"];
	comLogger* com;
	DynamicJsonDocument message(300);
	JsonObject multiComHeaderJsonSend = message.createNestedObject("multiComHeader");;
	char topic[54];
	Serial.println("[COM] retrieving com data done");
	Serial.println(comNo);
	char msg[1024];

	if (comNo != (-1)) {
		Serial.println("weeeeeeeeeeeeeel");

		com = getComFromNo(comNo);
		if (com == nullptr) { Serial.println("[COM] Erreur lors de la recuperation de la com Crée (proceedNewCom)"); return; }
		setupComLogger(com, comId, externalDeviceId, comTimeout, duplicationSafe, step);
		multiComHeaderJsonSend["comId"] = comId;
		multiComHeaderJsonSend["step"] = 1;
		multiComHeaderJsonSend["success"] = true;
		multiComHeaderJsonSend["duplicationSafe"] = duplicationSafe + 1;
		multiComHeaderJsonSend["endCom"] = false;
		sprintf(topic, "com/%s/multiCom", externalDeviceId);
		serializeJson(message, &msg, 1024);
		brokerMqtt.sendMessage(topic, msg);
		Serial.println("[COM] Com done");

	}
	else {
		Serial.println("[COM] unable to open a connexion (missing channel)");
		multiComHeaderJsonSend["comId"] = comId;
		multiComHeaderJsonSend["step"] = 1;
		multiComHeaderJsonSend["success"] = false;
		multiComHeaderJsonSend["duplicationSafe"] = duplicationSafe + 1;
		multiComHeaderJsonSend["endCom"] = true;
		sprintf(topic, "com/%s/multiCom", externalDeviceId);
		serializeJson(message, &msg, 1024);
		brokerMqtt.sendMessage(topic, msg);
	}
}

void DN_Object::sendJsonDocOverMqtt(JsonDocument& message, char* topic)
{
	char msg[1024];
	serializeJson(message, &msg, 1024);
	brokerMqtt.sendMessage(topic, msg);
}

void DN_Object::closeCom(int comNo)
{
	comLogger* com;
	com = getComFromNo(comNo);
	if (com != nullptr) {
		com->used = false;
	}
}

void DN_Object::closeChannel(int channelNo)
{
	if (channelNo >= MAX_COM_CHANNELS) return;
	comMap[channelNo].used = false;
	strcpy(comMap[channelNo].comId, "");
	comMap[channelNo].comNum = -1;
	comMap[channelNo].duplicationSafe = 0;
	strcpy(comMap[channelNo].externalDeviceId, "");
	comMap[channelNo].step = 0;
	strcpy(comMap[channelNo].lastSend, "");
}



int DN_Object::getLastComCounter()
{
	return comCounter;
}

int DN_Object::getNewComCounter()
{
	comCounter++;
	if (comCounter > 30000) comCounter = 0;
	return comCounter;
}

int DN_Object::reserveComChanelAndComNo()
{
	int i = 0;
	while (i < MAX_COM_CHANNELS) {
		if (!comMap[i].used) {
			int comNum = getNewComCounter();
			initComLogger(&comMap[i], comNum);
			return comNum;
		};
		i++;
	}
	return -1;
}


comLogger* DN_Object::getComFromNo(int comNo)
{
	int i = 0;
	while (i < MAX_COM_CHANNELS) {
		if (comMap[i].comNum == comNo) {
			if (comMap[i].used == true) return &comMap[i];
			return nullptr;
		};
		i++;
	}
	return nullptr;
}

comLogger* DN_Object::getComFromId(const char comId[26])
{
	int i = 0;
	while (i < MAX_COM_CHANNELS) {
		if (strcmp(comMap[i].comId, comId) == 0) {
			if (comMap[i].used == true) return &comMap[i];
			return nullptr;
		};
		i++;
	}
	return nullptr;
}

void DN_Object::initComLogger(comLogger* comLogger, int comNum)
{
	comLogger->used = true;
	comLogger->lastActivity = millis();
	comLogger->comNum = comNum;
	comLogger->endCom = false;
	comLogger->newMessageFlag = false;
	strcpy(comLogger->lastSend, "");
	strcpy(comLogger->lastReceived, "");

}

void DN_Object::setupComLogger(comLogger* com, const char  comId[26], const char  externalDeviceId[19], const int comTimeout, const int duplicationSafe, const int step)
{

	strcpy(com->comId, comId);
	com->comTimeout = comTimeout;
	com->duplicationSafe = duplicationSafe;
	strcpy(com->externalDeviceId, externalDeviceId);
	com->step = step;
}

bool DN_Object::isComLoggerAlive(comLogger* com)
{
	return millis() - com->lastActivity < com->comTimeout;
}

void DN_Object::yieldComLogger(comLogger* com)
{
	com->lastActivity = millis();
}

void DN_Object::freeComLogger(comLogger* com)
{
	com->used = false;
	com->comNum = -2;
	com->lastActivity = 0;
	strcpy(com->lastSend, "");
	strcpy(com->externalDeviceId, "");
}

bool DN_Object::checkDuplicationSafe(const comLogger* comLocal, const int receivedStep, const int receivedDuplicationSafe)
{
	if (receivedDuplicationSafe > comLocal->duplicationSafe && comLocal->step < receivedStep) {
		return true;
	}
	return false;
}

void DN_Object::updateComLoggerLastSend(comLogger* com, const char lastSend[500])
{
	strcpy(com->lastSend, lastSend);
}

bool DN_Object::availableMsgCom(int comNo)
{
	comLogger* com = getComFromNo(comNo);
	if (com == nullptr) return false;
	return com->newMessageFlag;
}

bool DN_Object::readMsgCom(int comNo, char* message, bool ghost)
{
	comLogger* com = getComFromNo(comNo);
	if (com == nullptr) {
		strcpy(message, "");
		return false;
	}
	if (com->newMessageFlag) {
		strcpy(message, com->lastReceived);
		if (!ghost) com->newMessageFlag = false;
	}
	return true;
}

void DN_Object::closeCom(const char comId[26])
{
}

void DN_Object::printComUsage()
{
	Serial.print("Last comNo given: ");
	Serial.println(getLastComCounter());



	const int chanelNumber = MAX_COM_CHANNELS;
	const int casePerChannel = 9;
	char line[100];
	strcpy(line, "---------");
	for (int i = 0; i < casePerChannel * chanelNumber; i++) {
		strcat(line, "-");
	}
	Serial.println(line);
	strcpy(line, "Ch.No -> ");
	for (int i = 0; i < chanelNumber; i++) {
		strcat(line, "|    ");
		char cvalue[2];
		sprintf(cvalue, "%d", i);
		strcat(line, cvalue);
		strcat(line, "   ");
	}
	Serial.println(line);

	strcpy(line, "used  -> ");
	for (int i = 0; i < chanelNumber; i++) {
		if (comMap[i].used) {
			strcat(line, "|  TRUE  ");
		}
		else {
			strcat(line, "|  FALSE ");
		}
	}
	Serial.println(line);

	strcpy(line, "Com.No-> ");
	for (int i = 0; i < chanelNumber; i++) {
		if (comMap[i].comNum < 10) {
			strcat(line, "|    ");
			char cvalue[2];
			sprintf(cvalue, "%d", comMap[i].comNum);
			strcat(line, cvalue);
			strcat(line, "   ");

		}
		else if (comMap[i].comNum < 100) {
			strcat(line, "|   ");
			char cvalue[3];
			sprintf(cvalue, "%d", comMap[i].comNum);
			strcat(line, cvalue);
			strcat(line, "   ");
		}
		else {
			strcat(line, "|   ");
			char cvalue[4];
			sprintf(cvalue, "%d", comMap[i].comNum);
			strcat(line, cvalue);
			strcat(line, "  ");
		}
	}
	Serial.println(line);
	strcpy(line, "---------");
	for (int i = 0; i < casePerChannel * chanelNumber; i++) {
		strcat(line, "-");
	}
	Serial.println(line);
}


void DN_Object::handle()
{
	/*if (connectServerState != 1) {
		//connectServer();
	}
	else {

	}*/

}

void DN_Object::handleMqttMsgEndObject(DynamicJsonDocument& docMsgReceived)
{
}



/*void DN_Object::handleSyncValueRespons(DynamicJsonDocument& respons)
{

	if (data.containsKey("valuesList")) {
		JsonArray valuesListSer;
		if (!respons["valuesList"].isNull()) {
			valuesListSer = respons["valuesList"];

		}
		else {
			Serial.println("no value saved on server");
		}
		int arraySizeSer = valuesListSer.size();

		JsonArray valuesListLocal = data["valuesList"];
		int arraySizeLocal = valuesListLocal.size();
		for (auto it = valuesListLocal.begin(); it != valuesListLocal.end(); ++it) {
			if (!isInJsonArray((*it), valuesListSer)) {
				char idValue[4];
				strcpy(idValue, (*it));
				const char* valueId = data[idValue]["valueId"];
				const char* valueName = data[idValue]["valueName"];
				int lifeTime = data[idValue]["lifeTime"];
				addValueOnServer(valueId, valueName, lifeTime);


			}
		}
		for (auto it = valuesListSer.begin(); it != valuesListSer.end(); ++it) {
			if (!isInJsonArray((*it), valuesListLocal)) {
				const char* id = (*it);
				removeValueOnServer(id);
				Serial.print("supression de la value (sur le serveur) : ");
				Serial.println(id);
			}
		}
	}


}


*/