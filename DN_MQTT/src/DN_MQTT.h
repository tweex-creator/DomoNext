/*
 Name:		DN_MQTT.h
 Created:	27/10/2022 10:39:51
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#ifndef _DN_MQTT_h
#define _DN_MQTT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#define NB_MAX_MQTT_JSON_SIZE  500
#define NB_MAX_SUB_TOPIC  20
#define NB_MAX_LEN_TOPIC  254

#define NB_MAX_MQTT_INTERFACE  1

#include <DN_WIFI.h>
#include <WiFiClient.h>

#include <DN_Memory.h>
#include <DN_DeviceId.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>


class DN_INTERFACEMQTT {
public:
	virtual void handleMqttMsg(const char* docMsgReceived) = 0;

};

class DN_MQTTclass
{
protected:
	DN_Memory& memoryManager;
	DN_WIFIClass& wifiManager;
	DN_DeviceId& deviceId;
	char mqtt_server[16];
	int mqttPort;
	char mqtt_topics[60];

	WiFiClient espClient;
	PubSubClient client;
	void setup_mqttServer(char ipServer[16], int port);
	void setupCallBack();

	unsigned long mqttConectionTimeoutInProgress;
	unsigned long mqttConectionTimeout;
	/// Envoie des messages
	StaticJsonDocument<NB_MAX_MQTT_JSON_SIZE> docMsgSend; //Le json qui contient le message a envoyer
	JsonArray data;//Pour stocker les tbl temporraire a la crea des msg 

	/// Reception des messages
	unsigned char nbReceiver;
	DN_INTERFACEMQTT* receiver[NB_MAX_MQTT_INTERFACE];

	/// topic sub
	char topicToSub[NB_MAX_SUB_TOPIC][NB_MAX_LEN_TOPIC];
	void subsribeToAll();
public:
	DN_MQTTclass(DN_Memory& memoryManager, DN_WIFIClass& wifiManager, DN_DeviceId& deviceId_);

	void callback(char* topic, byte* payload, unsigned int length);

	void getMQTT_IP(char ip[16]);
	void setMQTT_IP(const char ip[16]);

	void connectMainMqtt();
	bool isConnected();
	void startMessage(char msg_type[16]);

	void addMessage(const char* valueName, const char* value);
	void addMessage(const char* valueName, const int value);
	void addMessage(const char* valueName, const double value);

	void addArray(const char* arrayName);
	void addArrayMessage(const char* value);
	void addArrayMessage(int value);
	void addArrayMessage(double value);
	void sendMessage(const char* topic);

	void sendMessage(const char* topic, const char* msg);


	bool addMqttReceiver(DN_INTERFACEMQTT* receiver_);
	void rmMqttReceiver(DN_INTERFACEMQTT* receiver_);
	bool subscribe(const char topic[NB_MAX_LEN_TOPIC]);
	void unsubscribe(const char topic[NB_MAX_LEN_TOPIC]);
	void printSubscribtions();
	void log(const char log_type[24], const char* msg);
	void handle();
};

#endif

