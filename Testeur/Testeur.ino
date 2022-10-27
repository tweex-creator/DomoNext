/*
 Name:		Testeur.ino
 Created:	12/10/2022 18:35:12
 Author:	Herve
*/

// the setup function runs once when you press reset or power the board
void setup() {

}

// the loop function runs over and over again until power down or reset
void loop() {
  
}
//COM 4

#include "../DN_Base3/DN_Value_int.h"
#include "../DN_Base3/DN_Value_Base.h"


#include "../DN_Base3/DN_LitlleFs.h"
DN_LitlleFsClass littleFsManagerGlobal;

#include "../DN_Base3/DN_DeviceId.h"
DN_DeviceId deviceIdGlobal(littleFsManagerGlobal);

#include "../DN_Base3/DN_WIFI.h"
DN_WIFIClass wifiManagerGlobal("123456789", littleFsManagerGlobal, deviceIdGlobal);

#include "../DN_Base3/DN_MQTT.h"
DN_MQTTclass mqttManagerGlobal(littleFsManagerGlobal, wifiManagerGlobal, deviceIdGlobal);

#include "../DN_Base3/DN_Object.h"
DN_Object obj1(mqttManagerGlobal, wifiManagerGlobal, deviceIdGlobal);
PubSubClient client;
WiFiClient espClient;

void setup() {


	Serial.begin(115200);
	WiFi.disconnect();
	deviceIdGlobal.loadFromMemory();
	wifiManagerGlobal.connectMain();
	mqttManagerGlobal.printSubscribtions();


}

unsigned long temps = 0;
void loop()
{
	wifiManagerGlobal.handle();
	mqttManagerGlobal.handle();
	deviceIdGlobal.handle();
	obj1.handle();

	if (mqttManagerGlobal.isConnected()) {
		if (millis() - temps > 10000) {
			temps = millis();
			Serial.println(obj1.initCom("AAC"));
			obj1.closeCom(rand() * 5);
			obj1.printComUsage();
		}
	}


}
