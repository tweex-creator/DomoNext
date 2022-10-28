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
	Serial.println("!!obj1!!");
}

unsigned long temps = 0;
int comNo = -1;
bool onlyOne = true;
unsigned long time_ = 0;
bool test =false;

void loop()
{
	DynamicJsonDocument msgDoc(64);

	if (test) Serial.println("loop 1 ");
	wifiManagerGlobal.handle();
	if (test) Serial.println("loop 2 ");
	mqttManagerGlobal.handle();
	if (test) Serial.println("loop 3 ");
	deviceIdGlobal.handle();
	if (test) Serial.println("loop 4 ");
	obj1.handle();
	if (test) Serial.println("loop 5 ");
	test = false;

	if (mqttManagerGlobal.isConnected()) {
		if (millis() - time_ > 20000) {
			time_ = millis();
			comNo = obj1.initCom("AC:0B:FB:DD:13:43", 6000);
			obj1.printComUsage();
		}
		if (comNo != -1) {
			if (obj1.availableMsgCom(comNo)) {
				test = true;
				char msg_[25];

				obj1.readMsgCom(comNo, msg_);
				const char* cstMsg = msg_;
				Serial.print("msg recus via com(");
				Serial.print(comNo);
				Serial.print("): ");
				Serial.println(cstMsg);
				/*deserializeJson(msgDoc, cstMsg, 25);
				const char* data = msgDoc["comStatus"];
				Serial.print("msg recus via com2(");
				Serial.print(comNo);
				Serial.print("): "); 
				Serial.println(data);
				/*if (msgDoc.containsKey("comStatus")) {
					const char* comStatus = msgDoc["comStatus"];
					if (strcmp(comStatus,"open") == 0) {
						Serial.println("open");
						obj1.sendMsgCom(comNo, "{\"dataTest\": true}",true);
						Serial.println("open2");
					}
				}*/			
			}
		}
	}
	if (test) Serial.println("loop 6 ");
}