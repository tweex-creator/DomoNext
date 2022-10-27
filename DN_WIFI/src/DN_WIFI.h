/*
 Name:		DN_WIFI.h
 Created:	27/10/2022 10:35:20
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#ifndef _DN_WIFI_h
#define _DN_WIFI_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>        // Include the mDNS library

#include <DN_Memory.h>
#include <DN_DeviceId.h>

class DN_DeviceId;

typedef void (*PtrFonctHandle)(ESP8266WebServer& server);
typedef void (*PtrFonctGenerate)(ESP8266WebServer& server);

class DN_WIFIClass
{
protected:
	ESP8266WebServer server;
	DN_Memory& memoryManager;
	DN_DeviceId& deviceId;
	//ASYNC step and timer
		//ScanSSId
	unsigned char stepScanSSID;
	bool scanInProgress;
	unsigned long int timerScanSSID;
	//Connect
	bool connectionInProgress;
	unsigned char stepConnection;
	unsigned char selectedNetwork;// le reseau au quel on essaie actuellement de se connecter (a partir de la fonction getBestRSSI)
	unsigned long wifiConectionTimeoutInProgress;
	unsigned long wifiConectionTimeout;
	unsigned long wifiAutoReconnectProgress;
	unsigned long wifiAutoReconnect;
	char ssid_current[36];
	char pswd_current[36];
	//AP gestion
	bool ap_open;
	bool ap_force_open;

	const char* aP_password;

	int nbAvailableNetwork;


	PtrFonctHandle listHandler[10];
	PtrFonctHandle listGenerator[10];
	int nbhandler;

	void getCreditentialSSID(char* res, unsigned char num);
	void getCreditentialPSWD(char* res, unsigned char num);
	void getBestRSSI(char* ssid, char* pswd, unsigned char rank);
	unsigned char getNbSavedCredidentials();
	int creditentialExist(const char* SSID);// retounre la postion ou est sauvegarde le reseau dont le SSID est passe en parametre, -1 si il n'est pas sauvegarde


public:
	//Devrait être privées mais un problème m'oblige a les laisser en public(voir constructeur) 
	void handleRt();
	void handleScan();
	void handleAddCreditential();
	void handleOtherActions();

	DN_WIFIClass(const char* AP_PSWD, DN_Memory& memoryManager, DN_DeviceId& deviceId);
	~DN_WIFIClass();

	void initAP();//Demarre le point d'accès WIFI permettant d'acceder a l'interface de confid lorsque l'objet n'est pas aconnecté au wifi
	void closeAP();
	void forceAP();//Force l'AP a rester ouvert meme si l'objet se connecte ou est deja connecté a un reseau wifi
	void unforceAP();

	void handle();//a appelé regulièrement pour assurer le bon fonctionnement de l'objet
	void connectMain();
	void scanSSID();
	bool scanSSIDDone();

	void addCreditential(const char* ssid, const char* pswd);//A faire Passer tout ca en json
	void rmCreditential(int pos);
	void printCreditential();


	void addWebElement(PtrFonctHandle, PtrFonctGenerate);

	void getIP(char* ip_);
	void getMac(char* mac_);

	bool isConnected();

};

extern DN_WIFIClass DN_WIFI;

#endif

