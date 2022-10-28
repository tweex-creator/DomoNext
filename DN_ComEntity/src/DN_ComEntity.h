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

class DN_ComCanal {
public:
	/* Retourne True si le canal est pret à être utilisé par une nouvelle communication
	* Faux sinon 
	*/
	bool is_free();

	/*Retourne vrai si il y a bien une communication en cours avec le comNo
	  passé en parametre
	 */
	bool is_ComNo(int comNo);

	/*
	Initialise le canal pour une communication donnée
	Renvoie faux si le canal n'est pas libre
	*/
	bool startCanal(int comNo, unsigned int timeOut);

private:
	bool free;//Stock si le canal est actuellement utilisé par une communication

	char externalDeviceId[19];//Stock l'id de l'appareil distant (attention il peut s'agir d'une adresse mac si ce dernié n'est pas config d'ou le len)
	char comId[26];//Stock l'id de la communication actuellment en cours sur le canal
	int comNo;//Le numero d'identification de la communication(propre a l'objet, il peut être different sur l'objet distant)
	unsigned int timeOut;//Le timeOut de la communication en cours.Correspond au temps maximum autorisé entre deux messages(communication fermée au dela)
	unsigned long lastActivity;//Stock le dernier millis() au quel la communication recus/emis un message, permet la fermeture auto avec timeOut
};

#endif

