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
#include <DN_ComEntity.h>

class DN_ComManager {
public:
	//Constructeur
	DN_ComManager();


	/* Demarre une communication, retourne le numero de la nouvelle communication
	 * (-1 en cas d'erreur). 
	 * 
	 * La demande de communication est envoyé au serveur distant, une fois la communication établie
	 * un nouveau message sera disponible avec le champ "comStatus".
	 *		- si comStatus = open -> la connection a reussi
	 *		-              = timeOut -> la connection a echoué ou a été perdue
	 *      -			   = close -> la connection à été fermé par l'objet distant
	 * Timeout correspond au temps maximum autorisé entre deux messages(communicatio fermée au dela)
	 */
	int initCom(const char externalDeviceId[19], const unsigned int timeOut);

	/* Retourne Vrai si un nouveau message est en attente sur la communication dont le 
	 * numéro est passé en paramètre,
	 * Faux sinon
	 */
	bool messageAvailable(const int comNo);

	/* Remplace le contenue du parametre res par le message présent en entrée dans le buffer de la communication
	 * Si aucun message n'est présent res = ""
	 * len correspond a la taille maximal de res
	 * 
	 * Une fois le contenue lue, le  buffer est vidé, le message ne peut donc pas être lue deux fois
	 * Si la com n'existe pas rien est fait
	 */
	void getMessage(const int comNo, char* res, const int len);

	/* Envoie le contenue de message sur la communication et renvoie vraie
	 * Si la communication est déja en attente d'une reponse(un message a deja ete envoyé),
	 * le message est ignoré et la fonction retourne faux
	 * Si la com n'existe pas rien est fait et la fonciton retourne faux
	 * 
	 * si closeCom == true, la communication est fermée de facons propre et la partie distante en est informée
	 */
	bool sendMessage(const int comNo, const char* message, bool closeCom);


private:
	/*
	Les differents canaux de communication permettant de gerer plusieurs
	communication en même temps. Le nombre de communication simultannées maximum
	est defini par MAX_COM_CHANNEL*/
	DN_ComCanal canaux[MAX_COM_CHANNEL];

	int currentComNo;


	/*
	Crée une nouvelle communication sur un des canaux libre, 
	le numéro de cette communication est renvoyé
	Timeout correspond au temps maximum autorisé entre deux messages(communicatio fermée au dela):
	Retourne -1 Si aucun channel n'est dispo
	*/
	int getNewCom(int timeOut);

	/*
	*Genere l'entete dediée au protocole multiCom 
	*/
	void generateMultiComHeader(char header[150], char comId[25], int step, int duplicationSafe, bool endCom, bool sucess = true);

	/*
	Retourne vrai si il y a bien une communication en cours avec le comNo
	passé en parametre*/
	bool is_ComNo_Valid(int comNo);

	/*
	Retourne une reference vers le canal qui contient la com dont le
	numero est passé en parametre
	Attention a bien utilise is_ComNo_Valid() avant
	*/
	DN_ComCanal& getCom(int comNo);
};

#endif

