/*
 Name:		DN_WIFI.cpp
 Created:	27/10/2022 10:35:20
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_WIFI.h"
#include <string.h>

DN_WIFIClass* redirectorHandle;
void handleRootRedirector() {
	redirectorHandle->handleRt();
}

void handleScanRedirector() {
	redirectorHandle->handleScan();
}

void handleAddCreditentialRedirector() {
	redirectorHandle->handleAddCreditential();
}

DN_WIFIClass::DN_WIFIClass(const char* AP_PSWD, DN_Memory& littleFsManagerProject, DN_DeviceId& devicedId_) :aP_password(AP_PSWD), server(80), memoryManager(littleFsManagerProject), deviceId(devicedId_)
{

	for (int i = 0; i < 10; i++) {
		listHandler[i] = NULL;
		listGenerator[i] = NULL;
	}
	char macForDeviceId[20];
	getMac(macForDeviceId);
	deviceId.setMac(macForDeviceId);
	nbhandler = 0;
	stepScanSSID = 0;
	redirectorHandle = this;
	scanInProgress = false;
	connectionInProgress = false;
	stepConnection = 0;
	nbAvailableNetwork = 0;
	selectedNetwork = 0;
	wifiConectionTimeoutInProgress = 0;
	wifiConectionTimeout = 10000;//un timeout de 10 seconde pour se connecter au reseau wifi
	ap_open = false;
	ap_force_open = false;
	wifiAutoReconnectProgress = 0;
	wifiAutoReconnect = 20000;//L'appareil essaie de se reconnecter au reseau toutes les x secondes
	WiFi.mode(WIFI_STA);
	server.on("/", handleRootRedirector);
	server.on("/scan", handleScanRedirector);
	server.begin();
	Serial.println("HTTP server started");
	server.on("/addCreditential", HTTP_GET, handleAddCreditentialRedirector);

}

DN_WIFIClass::~DN_WIFIClass()
{
	redirectorHandle = NULL;
}

void DN_WIFIClass::initAP()
{
	WiFi.mode(WIFI_AP_STA);

	Serial.println();
	Serial.print("[WIFI]Configuring access point...");
	char name[26];;
	deviceId.getName(name);
	WiFi.softAP(name, aP_password);
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
	ap_open = true;

}

void DN_WIFIClass::closeAP()
{
	if (!ap_force_open) {
		Serial.println("[WIFI] fermeture AP");
		WiFi.softAPdisconnect(true);
		WiFi.enableAP(false);
		ap_open = false;
	}
}

void DN_WIFIClass::forceAP()
{
	ap_force_open = true;
	if (ap_open) {
		this->initAP();
	}
}

void DN_WIFIClass::unforceAP()
{
	ap_force_open = false;

}

void DN_WIFIClass::handle()
{
	if (ap_force_open) {
		if (!ap_open)
		{
			this->initAP();
		}
	}
	else if (!ap_force_open) {
		if (WiFi.status() == WL_CONNECTED && ap_open)
		{
			this->closeAP();
		}

		if (WiFi.status() != WL_CONNECTED && !ap_open)
		{
			this->initAP();
			connectMain();
		}
	}

	if (WiFi.status() != WL_CONNECTED && !connectionInProgress)
	{
		if (millis() - wifiAutoReconnectProgress > wifiAutoReconnect) {
			wifiAutoReconnectProgress = millis();
			connectMain();

		}
	}


	if (scanInProgress) {
		scanSSID();
	}
	if (connectionInProgress) {
		connectMain();
	}
	server.handleClient();
}

void DN_WIFIClass::connectMain()
{
	switch (stepConnection)
	{
	case 0:
		connectionInProgress = true;
		selectedNetwork = 0;
		Serial.println("[WIFI] lancement tentative de connection");

		if (!scanInProgress) { // on lance un scan des reseau disponible
			scanSSID();
		}
		stepConnection = 1;

		break;
	case 1:

		if (!scanInProgress) { //Une fois le scan termin� on recup�re le reseau enregistr� avec le RSSID le plus fort
			stepConnection = 2;
		}
		break;
	case 2:
		do {
			getBestRSSI(ssid_current, pswd_current, selectedNetwork);
			if (strcmp(ssid_current, "") == 0) {
				selectedNetwork++;
			}

		} while (strcmp(ssid_current, "") == 0 && nbAvailableNetwork > selectedNetwork);

		if (nbAvailableNetwork <= selectedNetwork) {
			stepConnection = 4;
		}
		else {
			Serial.print("[WIFI] Connection -");
			Serial.print(" ssid:");

			Serial.print(ssid_current);
			Serial.print(" pswd:");
			Serial.println(pswd_current);
			char name[26];;
			deviceId.getName(name);
			WiFi.hostname(name);
			WiFi.disconnect();
			WiFi.begin(ssid_current, pswd_current);
			Serial.println("[WIFI] Connecting...");
			wifiConectionTimeoutInProgress = millis();
			stepConnection = 3;
		}
		break;
	case 3:

		if (WiFi.status() == WL_CONNECTED)
		{
			Serial.print("[WIFI] Connected, IP address: ");
			Serial.println(WiFi.localIP());
			stepConnection = 5;
			char id[4];
			deviceId.getId(id);
			if (!MDNS.begin(id)) {             // Start the mDNS responder for esp8266.local
				Serial.println("[WIFI] Error setting up MDNS responder!");
			}
			else {
				Serial.println("[WIFI] mDNS started");

			}

		}
		if (millis() - wifiConectionTimeoutInProgress > wifiConectionTimeout) {
			Serial.println("[WIFI] Connection impossible! ");
			wifiConectionTimeoutInProgress = millis();
			selectedNetwork++;
			stepConnection = 2;
		}
		break;
	default:

		connectionInProgress = false;
		stepConnection = 0;
		break;
	}
}

void DN_WIFIClass::scanSSID()
{
	scanInProgress = true;
	switch (stepScanSSID)
	{
	case 0:
		//Serial.println("Scan start ... ");
		timerScanSSID = millis();
		stepScanSSID++;
		break;
	case 1:
		if (millis() - timerScanSSID > 200)
		{
			//Serial.println("	Scan en cours... ");
			WiFi.scanNetworks(true);
			timerScanSSID = millis();
			stepScanSSID++;

		}
		break;
	case 2:
		if (millis() - timerScanSSID > 1000)
		{
			nbAvailableNetwork = WiFi.scanComplete();
			timerScanSSID = millis();

			if (nbAvailableNetwork >= 0)
			{

				//Serial.printf("%d reseaux trouves(", nbAvailableNetwork);
				/*for (int i = 0; i < nbAvailableNetwork; i++)
				{
					Serial.printf("%s[%d], ", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
				}*/
				//Serial.printf(")\n", nbAvailableNetwork);
				stepScanSSID++;


			}
			else if (nbAvailableNetwork == -1) {
				//Serial.println("	Scan en cours... ");
			}
			else if (nbAvailableNetwork == -2) {
				Serial.println("	Une erreur c'est produite durant le scan!");
				nbAvailableNetwork = 0;
				stepScanSSID++;
			}
		}
		break;
	case 3:
		scanInProgress = false;
		stepScanSSID = 0;
		break;
	default:
		break;
	}




}

bool DN_WIFIClass::scanSSIDDone()
{
	return !scanInProgress;
}

void DN_WIFIClass::getBestRSSI(char* ssid, char* pswd, unsigned char rank)
{
	if (nbAvailableNetwork > rank) {
		unsigned char* rankRSSI = new unsigned char[nbAvailableNetwork];
		for (unsigned char i = 0; i < nbAvailableNetwork; i++) {
			rankRSSI[i] = 254;
		}

		//on classe les reseau par force du signal
		int i, j;
		unsigned char count = 0;
		for (i = 0; i < nbAvailableNetwork; i++) {
			for (j = 0; j < nbAvailableNetwork; j++) {
				if (WiFi.RSSI(i) < WiFi.RSSI(j)) {
					count++;
				}
			}
			while (rankRSSI[count] != 254) {
				count++;
			}
			rankRSSI[count] = i;
			count = 0;

		}


		int nbSavedNetworks = this->getNbSavedCredidentials();

		for (int i = 0; i < nbSavedNetworks; i++) {
			char ssid_saved[32];
			this->getCreditentialSSID(ssid_saved, i);

			if (strcmp(WiFi.SSID(rankRSSI[rank]).c_str(), ssid_saved) == 0) {
				strcpy(ssid, ssid_saved);
				char pswd_saved[32];
				this->getCreditentialPSWD(pswd_saved, i);
				strcpy(pswd, pswd_saved);

				delete rankRSSI;
				//Serial.println(" dispo dans les reseaux sauvegardes");
				return;
			}
		}
		//Serial.println(" Pas dispo dans les reseaux sauvegardes...");

		strcpy(ssid, "");
		strcpy(pswd, "");

		delete rankRSSI;

	}
	else {
		Serial.println("not so much SSID found:");
		strcpy(ssid, "");
		strcpy(pswd, "");
	}
}

void DN_WIFIClass::handleRt() {
	if (server.hasArg("delete") && server.arg("delete") != NULL) {
		this->rmCreditential(server.arg("delete").toInt());
	}
	if (server.hasArg("tryToConnect")) {
		this->connectMain();
	}
	if (server.hasArg("foceAP")) {
		if (server.arg("foceAP") == "true") {
			this->forceAP();
		}
		else if (server.arg("foceAP") == "false") {
			this->unforceAP();

		}
	}
	if (server.hasArg("DeviceIdNewName")) {
		this->deviceId.setName(server.arg("DeviceIdNewName").c_str());
	}
	if (server.hasArg("resetId")) {
		this->deviceId.reset();
	}

	handleOtherActions();
	server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
	server.sendHeader("Pragma", "no-cache");
	server.sendHeader("Expires", "-1");
	server.setContentLength(CONTENT_LENGTH_UNKNOWN);

	server.sendContent("<h1>");
	char name[26];;
	deviceId.getName(name);
	server.sendContent(name);
	server.sendContent(" (");
	char id[26];
	deviceId.getId(id);
	server.sendContent(id);
	server.sendContent(")</h1>");
	server.sendContent("<a href=\"./\">acceuil</a>");
	server.sendContent("<a href=\"./scan\"> recharger les reseaux</a>");

	server.sendContent("<form action=\"/\" method=\"GET\">");
	server.sendContent("<input type = \"hidden\" value = \"0\"name=\"resetId\">");
	server.sendContent("<input type=\"submit\" value=\"resetId\">");
	server.sendContent("</form>");
	server.sendContent("<p>IP: ");
	server.sendContent(WiFi.localIP().toString());
	if (connectionInProgress) {
		server.sendContent(" (Connexion en cours)");

	}
	else {
		server.sendContent(" (");
		server.sendContent(ssid_current);
		server.sendContent(")");
	}
	server.sendContent("</p>");
	server.sendContent("<p>MAC: ");
	server.sendContent(WiFi.macAddress());
	server.sendContent("</p>");



	server.sendContent("<h2>liste des reseaux disponible</h2><ul>");
	int nbRes = WiFi.scanComplete();
	for (int i = 0; i < nbRes; i++) {
		server.sendContent("<li>");
		server.sendContent("SSID: ");
		server.sendContent(WiFi.SSID(i).c_str());
		server.sendContent(" | RSSI:  ");
		server.sendContent(String(WiFi.RSSI(i)));
		server.sendContent("</li>");
	}
	server.sendContent("</ul>");

	server.sendContent("<h2>liste des reseaux enregistres</h2><ul>");
	nbRes = this->getNbSavedCredidentials();

	for (int i = 0; i < nbRes; i++) {
		server.sendContent("<li>");
		char ssid[32];
		this->getCreditentialSSID(ssid, i);
		server.sendContent("<form action=\"/\" method=\"GET\">");
		server.sendContent(ssid);

		server.sendContent("<input type = \"hidden\" value = \"");
		server.sendContent(String(i));
		server.sendContent("\" name=\"delete\">");
		server.sendContent("<input type=\"submit\" value=\"Supprimer\">");
		server.sendContent("</form>");







		server.sendContent("</li>");
	}
	server.sendContent("</ul>");
	server.sendContent("<form action=\"/addCreditential\" method=\"get\">");
	server.sendContent("<label> SSID: </label>");
	server.sendContent("<input type=\"text\" name=\"ssid\" required>");
	server.sendContent("<label> PSWD: </label>");
	server.sendContent("<input type=\"text\" name=\"pswd\">");
	server.sendContent("<input type=\"submit\" value=\"Ajouter\">");
	server.sendContent("</form>");


	server.sendContent("<form action=\"/\" method=\"get\">");
	server.sendContent("<input type = \"hidden\" name = \"tryToConnect\" value = \"true\">");

	server.sendContent("<input type=\"submit\" value=\"Tenter une connection wifi\">");
	server.sendContent("</form>");

	server.sendContent("<form action=\"/\" method=\"get\">");
	server.sendContent("<input type = \"hidden\" name = \"foceAP\" value = \"true\">");

	server.sendContent("<input type=\"submit\" value=\"Forcer l'access point\">");
	server.sendContent("</form>");

	server.sendContent("<form action=\"/\" method=\"get\">");
	server.sendContent("<input type = \"hidden\" name = \"foceAP\" value = \"false\">");

	server.sendContent("<input type=\"submit\" value=\"stoper le forcage l'access point\">");
	server.sendContent("</form>");
	server.sendContent("<form action=\"/\" method=\"get\">");
	server.sendContent("<label> Name: </label>");
	server.sendContent("<input type = \"text\" name = \"DeviceIdNewName\" value = \"\">");

	server.sendContent("<input type=\"submit\" value=\"Enregistrer\">");
	server.sendContent("</form>");
	server.sendContent("<h1>AUTRES PARAMETRES</h1>");

	for (int i = 0; i < nbhandler; i++) {
		server.sendContent("<form action=\"/\" method=\"get\">");
		server.sendContent("<input type = \"hidden\" name = \"otherHandler\" value = ");
		server.sendContent(String(i));
		server.sendContent(">");
		listGenerator[i](server);
		server.sendContent("</form>");

		server.sendContent("-------------------------------------------------");

	}




	server.client().stop();


}

void DN_WIFIClass::handleScan()
{
	this->scanSSID();
	this->handleRt();
}

void DN_WIFIClass::handleAddCreditential()
{
	if (!server.hasArg("ssid") || !server.hasArg("pswd")
		|| server.arg("ssid") == NULL) {
		server.send(400, "text/plain", "400: Invalid Request");
		return;
	}
	this->addCreditential(server.arg("ssid").c_str(), server.arg("pswd").c_str());
	server.send(200, "text/plain", "Le reseau a bien etait enregistr�");

}

void DN_WIFIClass::handleOtherActions()
{
	if (server.hasArg("otherHandler")) {
		int pos = server.arg("otherHandler").toInt();
		if (pos >= 0 && pos <= nbhandler) {
			listHandler[pos](server);
		}
	}
}

void DN_WIFIClass::addWebElement(PtrFonctHandle handlePtr, PtrFonctGenerate generatePtr)
{
	if (nbhandler < 10) {

		listHandler[nbhandler] = handlePtr;
		listGenerator[nbhandler] = generatePtr;
		nbhandler++;
	}
	else {
		Serial.print("Nb max handler reach");
	}
}

void DN_WIFIClass::getIP(char* ip_)
{
	strcpy(ip_, WiFi.localIP().toString().c_str());
}

void DN_WIFIClass::getMac(char* mac_)
{
	strcpy(mac_, WiFi.macAddress().c_str());

}

bool DN_WIFIClass::isConnected()
{
	return WiFi.status() == WL_CONNECTED;
}

void DN_WIFIClass::addCreditential(const char* ssid, const char* pswd)
{

	this->rmCreditential(this->creditentialExist(ssid));
	File f = memoryManager.openFile("/wifiManager/creditential.txt", "a");
	f.printf(ssid);
	f.printf(" ");
	f.printf(pswd);
	f.printf("\n");
	f.close();


}

void DN_WIFIClass::rmCreditential(int pos)
{
	if (pos < this->getNbSavedCredidentials() && pos >= 0) {
		LittleFS.rename("/wifiManager/creditential.txt", "/wifiManager/creditential.txt.old");
		File fnew = memoryManager.openFile("/wifiManager/creditential.txt", "w");
		File fold = memoryManager.openFile("/wifiManager/creditential.txt.old", "r");
		if (!(!fnew) && !(!fold)) {
			int count = 0;
			char next;
			while (fold.available() > 0 && count < pos) {
				next = fold.read();
				fnew.print(next);

				if (next == '\n') {
					count++;
				}
			}
			next = fold.read();
			while (fold.available() > 0 && next != '\n') {
				next = fold.read();
			}
			while (fold.available() > 0) {
				next = fold.read();
				fnew.print(next);
			}
			fnew.close();
			fold.close();
			LittleFS.remove("/wifiManager/creditential.txt.old");

		}


	}
}

int DN_WIFIClass::creditentialExist(const char* ssid)
{
	unsigned char nbReseaux = this->getNbSavedCredidentials();
	for (int i = 0; i < nbReseaux; i++) {
		char ssidFile[32];
		this->getCreditentialSSID(ssidFile, i);
		if (strcmp(ssid, ssidFile) == 0) {
			return i;
		}
	}
	return -1;
}

void DN_WIFIClass::printCreditential()
{
	File f = memoryManager.openFile("/wifiManager/creditential.txt", "a+");
	if (!(!f)) {
		while (f.available()) {
			Serial.write(f.read());
		}
	}
	else {
		Serial.println("Une erreur est survenu lors de l'ouverture de la sauvegarde");
	}
	f.close();

}

void DN_WIFIClass::getCreditentialSSID(char* res, unsigned char num)
{
	if (num < this->getNbSavedCredidentials()) {
		int pos = 0;
		unsigned char count = 0;
		File f = memoryManager.openFile("/wifiManager/creditential.txt", "r");
		while (f.available() > 0 && count < num) {
			if (f.read() == '\n') {
				count++;
			}
		}
		if (count == num) {
			char next;
			if (f.available() > 0) {
				next = f.read();
			}
			while (f.available() > 0 && next != ' ') {
				res[pos] = next;
				pos++;
				next = f.read();
			}
			res[pos] = '\0';
			f.close();
			return;
		}
		f.close();
	}
	else {
		Serial.println("reseau inexistant");
	}
}

void DN_WIFIClass::getCreditentialPSWD(char* res, unsigned char num)
{
	if (num < this->getNbSavedCredidentials()) {
		int pos = 0;
		unsigned char count = 0;
		File f = memoryManager.openFile("/wifiManager/creditential.txt", "r");

		while (f.available() > 0 && count < num) {
			if (f.read() == '\n') {
				count++;
			}
		}
		if (count == num) {
			char next;

			if (f.available() > 0) {
				next = f.read();
			}
			while (f.available() > 0 && next != ' ') {
				next = f.read();
			}
			next = f.read();
			while (f.available() > 0 && next != '\n') {
				res[pos] = next;
				pos++;
				next = f.read();
			}
			res[pos] = '\0';
			f.close();

			return;
		}
		f.close();

	}
	else {
		Serial.println("reseau inexistant");
	}

}

unsigned char DN_WIFIClass::getNbSavedCredidentials()
{
	unsigned char count = 0;
	File f = memoryManager.openFile("/wifiManager/creditential.txt", "r");

	while (f.available() > 0) {
		if (f.read() == '\n') {
			count++;
		}
	}
	f.close();
	return count;
}



