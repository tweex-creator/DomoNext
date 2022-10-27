

//COM 4

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
	Serial.println("!!obj2!!");
	WiFi.disconnect();
	deviceIdGlobal.loadFromMemory();
	wifiManagerGlobal.connectMain();


	obj1.init();
	Serial.println("!!obj2!!");

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

}