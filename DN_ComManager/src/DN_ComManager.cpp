/*
 Name:		DN_ComManager.cpp
 Created:	28/10/2022 15:39:52
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_ComManager.h"
#include <DN_MQTT.h>
DN_ComManager::DN_ComManager(DN_DeviceId& device_id_, DN_MQTTclass* mqttManager): device_id(device_id_)
{
	currentComNo = 0;
	for (int i = 0; i < MAX_COM_CHANNEL; i++) {
		canaux[i].setMqttManager(mqttManager);
	}
}

int DN_ComManager::initCom(const char externalDeviceId[19],int timeOut)
{
	int comNo = this->getNewCom(timeOut); //On crée une nouvelle communication
	char comId[26]{};
	char localDeviceId[19];//max 19 si l'id n'est pas definie on prend l'addr mac
	if (comNo == -1) return -1;//Si la création de la communication a echoué, on retourne -1
	if(!this->is_ComNo_Valid(comNo)) return -1;

	device_id.getIdOrMac(localDeviceId);
	snprintf(comId, 26, "%s_%d", localDeviceId, comNo);//On genere un id de communication
	DN_ComCanal& com = getCom(comNo);
	com.setupCanal(externalDeviceId, comId);
	char message[200];
	/*Generation du message d'initialisation*/
	snprintf(message, 150, "{\"multiComHeader\": {\"deviceId\": \"%s\", \"comId\": \"%s\", \"step\":%d,\"duplicationSafe\":%d,\"endCom\":%d, \"TimeOut\" : %d}}", localDeviceId, comId, 0, 0, false, timeOut);
	com.sendMessage(message);

	return comNo;
}

bool DN_ComManager::messageAvailable(const int comNo)
{
	if (!is_ComNo_Valid(comNo)) return false;
	DN_ComCanal& com = getCom(comNo);
	return com.availableMsg();
}

void DN_ComManager::getMessage(const int comNo, char* res, int len)
{
	if (!is_ComNo_Valid(comNo)) return;
	DN_ComCanal& com = getCom(comNo);
	com.getMessage(res, len);
}

void DN_ComManager::handleMqtt(const char* msg)
{
	StaticJsonDocument<364> doc;
	StaticJsonDocument<100> multiComHeaderJson;
	DeserializationError error;

	error = deserializeJson(doc, msg, 500);
	if (error) {
		Serial.print(F("[ComManager] deserializeJson() routing failed: "));
		Serial.println(error.f_str());
		return;
	}
	if (doc["multiComHeader"].isNull()) {
		Serial.println("[DN_ComManager] multicom header missing (routing)");
		return;
	}
	multiComHeaderJson = doc["multiComHeader"];
	if (multiComHeaderJson["comId"].isNull()) {
		Serial.println("[DN_ComManager] multicom header miised comId (routing)");
		return;
	}
	const char* comId = multiComHeaderJson["comId"];
	if (!is_ComId_Valid(comId)) {
		Serial.println("[comManager] Reception d'un message dont le comId est inconnu");
		return;
	}

	DN_ComCanal& com =  getCom(comId);
	com.handleNewMessage(msg);
	Serial.println("[comManager] routage ok 1");

}

void DN_ComManager::handle()
{
	for (int i = 0; i < MAX_COM_CHANNEL; i++) {
		canaux[i].handle();
	}
}

int DN_ComManager::getNewCom(int timeOut)
{
	//Recherche d'une canal libre

	int channel_pos = -1;//La position du chanel dans le tbl canaux
	bool free_channel_find = false;

	while(channel_pos < MAX_COM_CHANNEL && !free_channel_find) {
		channel_pos++;

		if (canaux[channel_pos].is_free()) {
			free_channel_find = true;
			
		}
	}

	if (!free_channel_find) return -1;//Si aucun channel dispo on retourne -1
	this->currentComNo++;
	this->currentComNo %= 32000;//Toutes les 32 000 commuications, le comptage repart a 0(faible risque de double attribution)
	int comNo = this->currentComNo;

	if (timeOut < 500) { timeOut = 500; }//On limite a 500ms le timeOut minimum
	if(canaux[channel_pos].startCanal(comNo, timeOut)) return comNo;


	return -1;

}

void DN_ComManager::generateMultiComHeader(char header[150], const char comId[25], const int step, const int duplicationSafe, const bool endCom)
{
	snprintf(header, 150, "{ \"comId\": \"%s\", \"step\":%d,\"duplicationSafe\":%d,\"endCom\":%d,\"sucess\":%d}", comId, step, duplicationSafe, endCom);
}

bool DN_ComManager::is_ComNo_Valid(const int comNo)
{
	for (int i = 0; i < MAX_COM_CHANNEL; i++) {
		if (canaux[i].is_ComNo(comNo)) return true;
	}
	return false;
}

DN_ComCanal& DN_ComManager::getCom(const int comNo)
{
	if (!this->is_ComNo_Valid(comNo)) Serial.println("[comManager] Accès a une communication innexistante... risque de plantage elevé");
	unsigned int channel_pos = 0;//La position du chanel dans le tbl canaux
	while (channel_pos < MAX_COM_CHANNEL) {
		if (canaux[channel_pos].is_ComNo(comNo)) {
			return canaux[channel_pos];
		}
		channel_pos++;
	}
	Serial.println("[comManager] Accès a une communication innexistante...");
}

DN_ComCanal& DN_ComManager::getCom(const char* comId)
{
	if (!this->is_ComId_Valid(comId)) Serial.println("[comManager] Accès a une communication innexistante... risque de plantage elevé");
	unsigned int channel_pos = 0;//La position du chanel dans le tbl canaux
	while (channel_pos < MAX_COM_CHANNEL) {
		if (canaux[channel_pos].is_ComId(comId)) {
			return canaux[channel_pos];
		}
		channel_pos++;
	}
	Serial.println("[comManager] Accès a une communication innexistante...");
}

bool DN_ComManager::is_ComId_Valid(const char* comId)
{
	for (int i = 0; i < MAX_COM_CHANNEL; i++) {
		if (canaux[i].is_ComId(comId)) return true;
	}
	return false;
}
