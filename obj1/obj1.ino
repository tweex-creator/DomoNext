/*
 Name:		obj1.ino
 Created:	27/10/2022 11:37:24
 Author:	Herve
*/
#include "Arduino.h"
#include <DN_Value_int.h>

#include <DN_Memory.h>
DN_Memory memoryManagerGlobal;

#include <DN_DeviceId.h>
DN_DeviceId deviceIdGlobal(memoryManagerGlobal);

#include <DN_WIFI.h>
DN_WIFIClass wifiManagerGlobal("123456789", memoryManagerGlobal, deviceIdGlobal);

#include <DN_MQTT.h>
DN_MQTTclass mqttManagerGlobal(memoryManagerGlobal, wifiManagerGlobal, deviceIdGlobal);

#include <DN_Object.h>
DN_Object obj1(mqttManagerGlobal, wifiManagerGlobal, deviceIdGlobal);


PubSubClient client;
WiFiClient espClient;


void setup() {


	Serial.begin(115200);
	Serial.println("!!obj1!!");

	WiFi.disconnect();
	deviceIdGlobal.loadFromMemory();
	wifiManagerGlobal.connectMain();


	obj1.init();
	Serial.println("!!obj1!!");
}

unsigned long temps = 0;
bool first = true;
int comNo = -1;
bool onlyOne = true;
bool test =false;

void loop()
{
	wifiManagerGlobal.handle();
	mqttManagerGlobal.handle();
	deviceIdGlobal.handle();
	obj1.handle();

	if (mqttManagerGlobal.isConnected()) {
		if (millis() - temps > 2000 ||first) {
			first = false;

			temps = millis();
			comNo = obj1.comManager.initCom("AC:0B:FB:DD:13:43", 1800);
			Serial.print(comNo);
			Serial.println(": send");
			//obj1.printComUsage();
		}
		if (!obj1.comManager.is_ComNo_Valid(comNo)) {
			comNo = -1;
		}
		if (comNo != -1 && obj1.comManager.messageAvailable(comNo)) {
			char received[500];
			obj1.comManager.getMessage(comNo, received, 500);
			Serial.println("Message recus: ");
			Serial.println(received);
			test = true;
		}
	}
}