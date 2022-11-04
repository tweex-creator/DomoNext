/*
 Name:		DN_Object.h
 Created:	27/10/2022 10:44:38
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#ifndef _DN_Object_h
#define _DN_Object_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#define MAX_COM_CHANNELS 5
#define MAX_EXTERN_VALUE 10

#include <DN_WIFI.h>
#include <DN_MQTT.h>
#include <ArduinoJson.h>
#include <DN_ComManager.h>

class DN_Object;

struct comLogger {//stock les informations sur les communications en cours
	bool used;//indique si la com est actuellement utilisée
	unsigned int comNum;//numero local de la com( different de l'emplacement dans le tbl askMap) voir getComId	
	unsigned long lastActivity;//stock le millis() de la dernière activité. Permet de liberer la ressource si elle n'est plus utilisé.
	bool sendAllow; //permet de ne pas permettre au programme d'envoyer un nouveau message tant qu'il n'a pas recus de reponse
	char comId[26];//L'id de la communication(le meme sur les deux appareils)
	int comTimeout;
	char externalDeviceId[19];
	int duplicationSafe;
	int step;//Inidque la progression de la communication, 0: demande d'ouverture, 2: reponse , +1 a chque message, -2 com timedOut
	char lastSend[500];
	char lastReceived[500];
	bool newMessageFlag;// true si un nouveau message n'a pas été lue, repasse à 0 à la lecture
	bool endCom; // indique si la communication est terminée et donc si il faut renvoyer une réponse

};

class DN_Object :DN_INTERFACEMQTT
{
public:
	DN_ComManager comManager;
	DN_Object(DN_MQTTclass& mqttBorker, DN_WIFIClass& wifiManager_, DN_DeviceId& deviceId_);

	void handle();

	void handleMqttMsg(const char* message);//Tous les msg passe par ici et son d'abord traiter pour les fonctions interne, ceux qui sont destiner a l'objet sont transmis a handleMqttMsgEndObject
	void reset();
	bool isInJsonArray(const char* value, const JsonArray array);


	int initCom(char* externalDeviceId, const unsigned int timeOut = 5000);

	bool availableMsgCom(int comNo);//True si un message attend d'etre lu sur la communication
	bool readMsgCom(int comNo, char* message/*OUT*/, bool ghost = false/*si vraie, le message ne sera pas marqué comme lu. ! Vous ne pourrez pas envoyer de message tant qu'il ne sera pas lu*/);//retourne faux si le message n'a pas pu être lue
	void closeCom(const char comId[26]);
	void sendMsgCom(int comNo, char* msg, bool endCom);
	void closeCom(int comNo); //Ferme la connexion dont le num est passée en paramètre
	
	
	void closeChannel(int channelNo); //Deprecated, Ferme la connexion dont le num est passée en paramètre
	
	void printComUsage();

	

	void init();


protected:
	//virtual void handleMqttMsgEndObject(const DynamicJsonDocument& docMsgReceived) = 0;
	void handleMqttMsgEndObject(DynamicJsonDocument& docMsgReceived);

private:

	void subMqtt();//Souscript a tous les topics utils à l'objet, doit être rappeler en cas de changement d'id pour mettre a jour les topics souscrits
	char lastSubId[4];//stock le dernier Id au quel on a souscrit pour pouvoir se desabonner en cas de changement. 
	int comCounter;
	int getLastComCounter();
	int getNewComCounter();

	int reserveComChanelAndComNo();//reserve un chanel de communication. et y cree une communication d'on l'id et renvoyé. si plus de chanel dispo -1


	//gestion des comLogger

	comLogger* getComFromNo(int comNo);
	comLogger* getComFromId(const char comId[26]);
	void initComLogger(comLogger* comLogger, int comNum);
	void setupComLogger(comLogger* com, const char comId[26], const char  externalDeviceId[19], const int comTimeout, const int duplicationSafe, const int step);
	bool isComLoggerAlive(comLogger* com); //retourne vraie si la com n'a pas timeout faux siinon
	void yieldComLogger(comLogger* com);// remet le timeOut de la communication à 0;
	void freeComLogger(comLogger* com);//libère la connection (attention il ne s'agit pas d'un moyend efermer proprement une connexion)
	bool checkDuplicationSafe(const comLogger* comLocal, const int receivedStep, const int receivedDuplicationSafe);
	void updateComLoggerLastSend(comLogger* com, const char lastSend[500]);
	void handleTimeOut();



	void proceedCom(JsonDocument& respons);
	void proceedNewCom(JsonDocument& multiComHeaderJson);
	void forceCloseCom(comLogger* com);

	void sendJsonDocOverMqtt(JsonDocument& message, char* topic);
	DN_MQTTclass& brokerMqtt;
	DN_DeviceId& deviceId;
	DN_WIFIClass& wifiManager;
	DynamicJsonDocument data;

	comLogger comMap[MAX_COM_CHANNELS];


	unsigned long timeoutServerConnetionCurrent;
	unsigned long timeoutServerConnetion;

	int connectServerState;//-2: deconnecté sans tentatives de reconnexion auto, -1: deconnecté, 0: connection en cours, 1: connecté, 2:recuperation d'un id




};

#endif

