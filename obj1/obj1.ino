/*
 Name:		obj1.ino
 Created:	27/10/2022 11:37:24
 Author:	Herve
*/

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
}

unsigned long temps = 0;
int comNo = -1;
bool onlyOne = true;

void loop()
{	
	wifiManagerGlobal.handle();
	mqttManagerGlobal.handle();
	deviceIdGlobal.handle();
	obj1.handle();

	if (true) {

		if (mqttManagerGlobal.isConnected()) {
			if (onlyOne) {
				Serial.println("send");
				onlyOne = false;
				obj1.closeChannel(1);
				comNo = obj1.initCom("AC:0B:FB:DD:13:43");
			}
		}
		if (comNo != -1) {
			if (obj1.availableMsgCom(comNo)) {
				Serial.println("msg available");
				char msg[500];
				obj1.readMsgCom(comNo, msg);
				Serial.print("msg recus via com(");
				Serial.print(comNo);
				Serial.print("): ");
				Serial.println(msg);
			}
		}
	}
}