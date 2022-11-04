/*
 Name:		DN_MQTT.cpp
 Created:	27/10/2022 10:39:51
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_MQTT.h"

#include <ESP8266WebServer.h>
DN_MQTTclass* handleWebAndCallBack = nullptr;

void handleMqttWeb(ESP8266WebServer& server) {
	if (server.hasArg("mqttIP")) {
		handleWebAndCallBack->setMQTT_IP(server.arg("mqttIP").c_str());
	}
}

void GenerateMqttWeb(ESP8266WebServer& server) {
	server.sendContent("<label> Adresse MQTT: </label>");
	server.sendContent("<input type=\"text\" name=\"mqttIP\" value=\"");
	char ip[16];
	handleWebAndCallBack->getMQTT_IP(ip);
	server.sendContent(ip);
	server.sendContent("\" required>");
	server.sendContent("<input type=\"submit\" value=\"Enregistrer\">");
}

void callbackHandle(char* topic, byte* payload, unsigned int length) {
	handleWebAndCallBack->callback(topic, payload, length);

}

DN_MQTTclass::DN_MQTTclass(DN_Memory& littleFsManagerProject, DN_WIFIClass& wifiManagerProject, DN_DeviceId& deviceId_) : memoryManager(littleFsManagerProject), wifiManager(wifiManagerProject), client(espClient), deviceId(deviceId_)
{
	client.setClient(espClient);
	char ipServer[16];
	getMQTT_IP(ipServer);
	mqttPort = 1883;

	this->setup_mqttServer("192.168.31.77", mqttPort);
	client.setCallback(callbackHandle);//Déclaration de la fonction de souscription

	handleWebAndCallBack = this;
	wifiManager.addWebElement(handleMqttWeb, GenerateMqttWeb);
	mqttConectionTimeoutInProgress = 0;
	mqttConectionTimeout = 15000;
	nbReceiver = 0;
	for (int i = 0; i < NB_MAX_SUB_TOPIC; i++) {
		strcpy(topicToSub[0], "");
	}

}

void DN_MQTTclass::subsribeToAll()
{
	for (int i = 0; i < NB_MAX_SUB_TOPIC; i++) {
		if (topicToSub[i][0] != '\0') {
			client.subscribe(topicToSub[i]);
			/*Serial.print("sub to '");
			Serial.print(topicToSub[i]);
			Serial.print("' : ");
			Serial.println(client.subscribe(topicToSub[i]));
		*/
		}
	}
}

void DN_MQTTclass::setup_mqttServer(char ipServer[16], int port)
{
	client.setServer(mqtt_server, mqttPort);
}

void DN_MQTTclass::getMQTT_IP(char ip[16])
{

	File f = memoryManager.openFile("/MqttManager/IP.txt", "r");
	char next;
	int pos = 0;
	next = f.read();
	while (f.available() > 0 && next != '\n') {
		ip[pos] = next;
		pos++;
		next = f.read();

	}
	ip[pos] = '\0';
	strcpy(mqtt_server, ip);
	f.close();
}

void DN_MQTTclass::setMQTT_IP(const char ip[16])
{
	File f = memoryManager.openFile("/MqttManager/IP.txt", "w");
	f.printf(ip);
	f.printf("\n");
	f.close();
	char ipg[16];
	getMQTT_IP(ipg);
}

void DN_MQTTclass::setupCallBack()
{
	client.setCallback(callbackHandle);//Déclaration de la fonction de souscription
}

void DN_MQTTclass::callback(char* topic, byte* payload, unsigned int length)
{
	DynamicJsonDocument docMsgReceived(1024); //Le json qui contient le message recus
	char msg[1024];

	for (int i = 0; i < length; i++) {
		msg[i] = (char)payload[i];

	}
	msg[length] = '\0';
	DeserializationError err = deserializeJson(docMsgReceived, (const char*)msg);
	if (err) {
		Serial.print(F("deserializeJson() d'un msg entrant failed: "));
		Serial.println(err.c_str());
		docMsgReceived.clear();
		docMsgReceived["JsonFormat"] = false;
		docMsgReceived["payload"] = msg;
	}
	else {
		docMsgReceived["JsonFormat"] = true;
	}
	docMsgReceived["topic"] = topic;


	if (docMsgReceived["JsonFormat"]) {

		for (int i = 0; i < NB_MAX_MQTT_INTERFACE; i++) {

			if (receiver[i] != nullptr) {
				serializeJson(docMsgReceived, msg);
				receiver[i]->handleMqttMsg(msg);
				Serial.println("handle msg mqtt done");
			}
		}
	}
	else {
		char payload[1024];
		strcpy(payload, "[object] ");
		strcat(payload, docMsgReceived["payload"]);
		log("JSON_FORMAT_ERROR", err.c_str());
	}

}

bool DN_MQTTclass::isConnected()
{
	return client.connected() && wifiManager.isConnected();
}

void DN_MQTTclass::handle()
{
	if (wifiManager.isConnected()) {
		if (client.connected() && !client.loop()) {
			Serial.println("ouch mqtt loop error");
		}
		else if (!client.connected()) {
			connectMainMqtt();
			//Serial.print("#");
		}
		else {
			//	Serial.print(".");
		}

	}
	else
	{
		//Serial.print("-");

	}


}

void DN_MQTTclass::connectMainMqtt()
{

	if (millis() - mqttConectionTimeoutInProgress > mqttConectionTimeout && !client.connected() && wifiManager.isConnected()) {
		mqttConectionTimeoutInProgress = millis();
		client.setServer(mqtt_server, 1883);
		client.setCallback(callbackHandle);
		Serial.print("[DN_MQTT] Connection MQTT ");
		char mac[32];
		deviceId.getMac(mac);
		Serial.print(mac);
		if (client.connect(mac)) {
			Serial.println(" | MQTT connection OK");
			subsribeToAll();
			//Serial.println("sub all ok");

		}
		else {
			Serial.print(" | echec, code erreur= ");
			Serial.print(client.state());
			Serial.println(" --> nouvel essai dans 2s");
		}
	}
}

void DN_MQTTclass::startMessage(char msg_type[16])
{
	docMsgSend.clear();
	char id_[4];
	deviceId.getId(id_);
	docMsgSend["deviceId"] = id_;
	//docMsgSend["msg_type"] = msg_type;
}

void DN_MQTTclass::addMessage(const char* valueName, const  char* value)
{
	docMsgSend[valueName] = value;
}

void DN_MQTTclass::addMessage(const char* valueName, int value)
{
	docMsgSend[valueName] = value;
}

void DN_MQTTclass::addMessage(const char* valueName, double value)
{
	docMsgSend[valueName] = value;

}

void DN_MQTTclass::addArray(const char* arrayName)
{
	data = docMsgSend.createNestedArray(arrayName);
}

void DN_MQTTclass::addArrayMessage(const char* value)
{
	data.add(value);
}

void DN_MQTTclass::addArrayMessage(int value)
{
	data.add(value);
}

void DN_MQTTclass::addArrayMessage(double value)
{
	data.add(value);
}

void DN_MQTTclass::sendMessage(const char* topic)
{
	char msg[1024];

	serializeJson(docMsgSend, &msg, 1024);
	client.publish_P(topic, msg, false);
}

void DN_MQTTclass::sendMessage(const char* topic, const char* msg)
{
	client.publish_P(topic, msg, false);
}

bool  DN_MQTTclass::addMqttReceiver(DN_INTERFACEMQTT* receiver_)
{
	for (int i = 0; i < NB_MAX_MQTT_INTERFACE; i++) {
		if (receiver[i] == nullptr) {
			receiver[i] = receiver_;
			return true;
		}
	}
	log("CORE_ERROR", "[DN_MQTT] No space left for topicReceiver");
	return false;

}

void DN_MQTTclass::rmMqttReceiver(DN_INTERFACEMQTT* receiver_)
{
	for (int i = 0; i < NB_MAX_MQTT_INTERFACE; i++) {
		if (receiver[i] == receiver_) {
			receiver[i] = nullptr;
		}
	}
}

bool DN_MQTTclass::subscribe(const char topic[NB_MAX_LEN_TOPIC])
{
	for (int i = 0; i < NB_MAX_SUB_TOPIC; i++) {
		if (topicToSub[i][0] != '\0') {
			if (strcmp(topicToSub[i], topic) == 0) {

				return true;//déja subscribe
			}
		}
	}
	for (int i = 0; i < NB_MAX_SUB_TOPIC; i++) {

		if (strcmp(topicToSub[i], "") == 0) {
			strcpy(topicToSub[i], topic);

			//Serial.print("sub: ");
			//Serial.println(client.subscribe(topic));

			//subsribeToAll();
			return true;
		}
	}
	log("CORE_ERROR", "[DN_MQTT] No topic space left for subscribtion");
	return false;//plus de place pour de nouveau topic, augmenter  NB_MAX_SUB_TOPIC
}

void DN_MQTTclass::unsubscribe(const char topic[NB_MAX_LEN_TOPIC])
{
	for (int i = 0; i < NB_MAX_SUB_TOPIC; i++) {
		if (topicToSub[i][0] != '\0') {
			if (strcmp(topicToSub[i], topic) == 0) {
				topicToSub[i][0] = '\0';
			}
		}
	}
	client.unsubscribe(topic);
}

void DN_MQTTclass::printSubscribtions()
{
	Serial.println("topics: ");
	for (int i = 0; i < NB_MAX_SUB_TOPIC; i++) {
		if (topicToSub[i][0] != '\0') {
			char serPrint[40];
			sprintf(serPrint, "topic[%d]: %s", i, topicToSub[i]);
			Serial.println(serPrint);
		}
	}
	Serial.println("topics end");

}



void DN_MQTTclass::log(const char log_type[24], const char* msg)
{
	if (strcmp(log_type, "CORE_ERROR") == 0 || strcmp(log_type, "JSON_FORMAT_ERROR") == 0 || strcmp(log_type, "CORE_WARNING") == 0) {
		char topic[32];
		strcpy(topic, "log/");
		strcat(topic, log_type);
		this->startMessage("log");
		this->addMessage("descritpion", msg);
		this->sendMessage(topic);
		Serial.print("[LOG] ");
		Serial.print(log_type);
		Serial.print(" | ");
		Serial.println(msg);
	}
	else {
		Serial.println("Ce type de log n'est pas accepte");
	}
}
