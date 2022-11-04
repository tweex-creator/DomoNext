/*
 Name:		DN_ComManager.h
 Created:	28/10/2022 15:39:52
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#ifndef _DN_ComManager_h
#define _DN_ComManager_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif
#define MAX_COM_CHANNEL 5
#include "C:\Users\Herve\source\repos\DN_Base3\DN_ComCanal\src\DN_ComCanal.h"
#include <ArduinoJson.h>
#include <DN_DeviceId.h>
#include <DN_MQTT.h>

class DN_ComManager {
public:
	//Constructeur
	DN_ComManager(DN_DeviceId& device_id, DN_MQTTclass* mqttManager);


	/* Demarre une communication, retourne le numero de la nouvelle communication
	 * (-1 en cas d'erreur).
	 *
	 * La demande de communication est envoy� au serveur distant, une fois la communication �tablie
	 * un nouveau message sera disponible avec le champ "comStatus".
	 *		- si comStatus = open -> la connection a reussi
	 *		-              = timeOut -> la connection a echou� ou a �t� perdue
	 *      -			   = close -> la connection � �t� ferm� par l'objet distant
	 * Timeout correspond au temps maximum autoris� entre deux messages(communicatio ferm�e au dela)
	 */
	int initCom(const char externalDeviceId[19], int timeOut);

	/*
	* Permet de recptioner les demandes de nouvelles connection
	* elle est appel� lorsqu'une connection dont l'id est inconnue arrive avec step == 0
	*/
	int acceptCom(const char* msg);

	/* Retourne Vrai si un nouveau message est en attente sur la communication dont le
	 * num�ro est pass� en param�tre,
	 * Faux sinon
	 */
	bool messageAvailable(const int comNo);

	/* Remplace le contenue du parametre res par le message pr�sent en entr�e dans le buffer de la communication
	 * Si aucun message n'est pr�sent res = ""
	 * len correspond a la taille maximal de res
	 *
	 * Une fois le contenue lue, le  buffer est vid�, le message ne peut donc pas �tre lue deux fois
	 * Si la com n'existe pas rien est fait
	 */
	void getMessage(const int comNo, char* res, int len);

	/* Envoie le contenue de message sur la communication et renvoie vraie
	 * Si la communication est d�ja en attente d'une reponse(un message a deja ete envoy�),
	 * le message est ignor� et la fonction retourne faux
	 * Si la com n'existe pas rien est fait et la fonciton retourne faux
	 *
	 * si closeCom == true, la communication est ferm�e de facons propre et la partie distante en est inform�e
	 */
	bool sendMessage(const int comNo, const char* message, bool closeCom);

	/*
	Retourne vrai si il y a bien une communication en cours avec le comNo
	pass� en parametre*/
	bool is_ComNo_Valid(const int comNo);

	/*
	* Routing the incomming mqtt message to the good chanel
	*/
	void handleMqtt(const char* msg);
	/*
	* Permet de gerer les taches de fond
	*/
	void handle();

private:

	/*reference vers le gestionnaire d'id de l'objet*/
	DN_DeviceId& device_id;

	//Ptr vers le gestionaire mqtt
	DN_MQTTclass* mqttManager;

	/*
	Les differents canaux de communication permettant de gerer plusieurs
	communication en m�me temps. Le nombre de communication simultann�es maximum
	est defini par MAX_COM_CHANNEL*/
	DN_ComCanal canaux[MAX_COM_CHANNEL];

	int currentComNo;


	/*
	Cr�e une nouvelle communication sur un des canaux libre,
	le num�ro de cette communication est renvoy�
	Timeout correspond au temps maximum autoris� entre deux messages(communicatio ferm�e au dela):
	Retourne -1 Si aucun channel n'est dispo
	*/
	int getNewCom(int timeOut);

	/*
	*Genere l'entete dedi�e au protocole multiCom
	*/
	void generateMultiComHeader(char header[150], const char comId[25], const int step, const int duplicationSafe, const bool endCom);


	/*
	Retourne une reference vers le canal qui contient la com dont le
	numero est pass� en parametre
	Attention a bien utilise is_ComNo_Valid() avant
	*/
	DN_ComCanal& getCom(const int comNo);


	/*
	Retourne vrai si il y a bien une communication en cours avec le comId
	pass� en parametre*/
	bool is_ComId_Valid(const char* comId);
	/*
	Retourne une reference vers le canal qui contient la com dont le
	Id est pass� en parametre
	Attention a bien utilise is_ComId_Valid() avant
	*/
	DN_ComCanal& getCom(const char* comId);



};

#endif

