/*
 Name:		DN_ComManager.cpp
 Created:	28/10/2022 15:39:52
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_ComManager.h"
#include <DN_MQTT.h>
DN_ComManager::DN_ComManager()
{
	currentComNo = 0;
}

int DN_ComManager::initCom(const char externalDeviceId[19], const unsigned int timeOut)
{
	int comNo = this->getNewCom(timeOut); //On crée une nouvelle communication
	if (comNo == -1) return -1;//Si la création de la communication a echoué, on retourne -1
	if(!this->is_ComNo_Valid(comNo)) return -1;
	
	DN_ComCanal& com = getCom(comNo);

	/////ICI, initaliser le comId + envoyer le message

}

bool DN_ComManager::messageAvailable(const int comNo)
{
	return false;
}

int DN_ComManager::getNewCom(const int timeOut)
{
	//Recherche d'une canal libre
	unsigned int channel_pos = 0;//La position du chanel dans le tbl canaux
	bool free_channel_find;
	while(channel_pos < MAX_COM_CHANNEL && !free_channel_find) {
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

void DN_ComManager::generateMultiComHeader(const char header[150], const char comId[25], const int step, const int duplicationSafe, const bool endCom, const bool sucess)
{
	snprintf(header, 150, "{\"comId\": \"%s\", \"step\":%d,\"duplicationSafe\":%d,\"endCom\":%d,\"sucess\":%d}", comId, step, duplicationSafe, endCom, sucess);
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
	}
}
