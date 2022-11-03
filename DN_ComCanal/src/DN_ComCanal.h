/*
 Name:		DN_ComEntity.h
 Created:	28/10/2022 16:02:33
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#ifndef _DN_ComEntity_h
#define _DN_ComEntity_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <ArduinoJson.h>
#include <DN_MQTT.h>
class DN_ComCanal {
public:

	//constructeur
	DN_ComCanal();
	/* Retourne True si le canal est pret à être utilisé par une nouvelle communication
	* Faux sinon 
	*/
	bool is_free();

	/*Retourne vrai si il y a bien une communication en cours avec le comNo
	  passé en parametre
	 */
	bool is_ComNo(int comNo);

	/*Retourne vrai si il y a bien une communication en cours avec le comId
	  passé en parametre
	 */
	bool is_ComId(const char* comId);

	/*
	Initialise le canal pour une communication donnée
	Renvoie faux si le canal n'est pas libre
	*/
	bool startCanal(const int comNo, const unsigned int timeOut);

	/*
	Permet de configurer le canal, doit être appelé avant d'envoyer/ lire des message
	*/
	void setupCanal(const char externalDeviceId[19], const char comId[26]);

	/*
	* Indique si un nouveau message est en attente de lecture.
	*/
	bool availableMsg();

	/*
	* Recupère le contenu du dernier message recus. Le contenue est copié dans la variable message, 
	* len est la taille max que peut stocker la chaine de caractère pointé par message. Après l'execution len est modifié et contient le longeur du message entrant
	* Si le message en attente est plus long que len la fonction retourne -1 et msg = "", len = longeur du message en attente(le message reste consideré comme non lu)
	* La fonction retounre la valeur de availableMsg (ou -1 si un message en attente plus long que len).
	* Si aucun message n'etait disponible, msg = "" et len = 1
	* Retourn -2 si le canal n'est pas configuré (ready = false)
	*/
	char getMessage(char* message, int& len);

	/*
	* Process les nouveaux message en provenance de l'objet distant. 
	* Le contenu a destination de l'objet est stocké dans inMessage et le falg newMessage est mis à 1
	*/
	void handleNewMessage(const char* IncommingMessage);

	/*
	* Envoie le message vers l'objet distant, à conditions que le canal soit pret(redy == true) et qu'aucun message n'est déja eté envoyé depuis la dernière reception
	*/
	void sendMessage(const char* message);


	void setMqttManager(DN_MQTTclass*);

	//a appelé regulièrement pour gerer les taches de fonds du canal
	void handle();
private:
	DN_MQTTclass* mqttManager;
	
	
	bool ready;//Permet de ne pas envoyer de message tant que la connection n'est pas configurée
	bool used;//inidque si le canal est utilisé
	char externalDeviceId[19];//Stock l'id de l'appareil distant (attention il peut s'agir d'une adresse mac si ce dernié n'est pas config d'ou le len)
	char comId[26];//Stock l'id de la communication actuellment en cours sur le canal
	int step;//Le nombre d'echange(reussi) qui ont déja eu lieu entre les deux objets
	int duplicationSafe;// Securité permettant d'eviter la reception de message en double.
	int comNo;//Le numero d'identification de la communication(propre a l'objet, il peut être different sur l'objet distant)
	unsigned int timeOut;//Le timeOut de la communication en cours.Correspond au temps maximum autorisé entre deux messages(communication fermée au dela)
	unsigned long lastActivity;//Stock le dernier millis() au quel la communication recus/emis un message, permet la fermeture auto avec timeOut
	
	char inMessage[500];// Contient le dernier message recus sur le canal.
	bool newMessage;// flag indiqaunt la presence d'un message non lu dans le buffer inMessage
	bool endCom;//Indique si la communication est terminée

	/*
	* Verifie la validité des champs step et duplicationSafe recus.
	* Si les valeurs ne concorde pas(reception d'un message déja recus), 
	* renvoie faux. Vraie sinon
	*/
	bool checkDuplicationSafe(const int receivedStep, const int receivedDuplicationSafe);
	
	//Verifie que la communication n'est pas expirer et agit si besoin
	void checkTimeOut();

	/*
	* Ferme definitivement la communication et libère le canal
	*/
	void closeCom();

};

#endif

