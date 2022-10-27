// DN_Value_Base.h

#ifndef _DN_VALUE_BASE_h
#define _DN_VALUE_BASE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif


class DN_Value_Base
{
public:
	DN_Value_Base(unsigned char type);
	~DN_Value_Base();
	void setupLocalValue(char name[10], char valueId[3], bool externModifAllowed, bool offlineModifAllowed);
	void setuExternlValue(char objId[3], char valueId[3]);

	unsigned char getType();
	void getName(char*);
	void getValueId(char*);
	void getObjId(char*);

protected:
	bool externModifAllowed; //Indique si la valeur peut être modifiée par depuis l'exterieur.  True: le serveur et autres objets peuvent modifier la valeur. False: la valeur ne peut être modifiée que par l'objet local.
	bool offlineModifAllowed;//Indique si la valeur peut être modifiée par depuis l'exterieur lorsque l'objet est hors ligne.  
	
	char name[10]; //Nom de la value
	char valueId[3];//L'id de la valeur(il doit être unique au sein des valeur comprises dans l'objet)
	char objId[3]; //Utile uniquement si local = false.Contient l'id de l'objet où se trouve la valeur

	unsigned char type;//Le type de la value (0:int, 1:float, 2:bool, 3:char, 4:char*)
	bool local;//indque si la valeur est local ou si elle est a recuperer sur un autre objet
	bool init;//indique si la valeur est prète à être utilisée

	unsigned char updateRate; //0: La valeur n'est jamais envoyé au serveur sans qu'elle n'ai été explicitement demandée, 1: La valeur est envoyé au serveur à chaque changement, 2: La valeur est envoyé au serveur au moins tous les  maxInterval

};



#endif