// 
// 
// 

#include "DN_Value_int.h"

DN_Value_int::DN_Value_int(unsigned char type):DN_Value_Base(type)
{
	valeur = 0;
	limit[0] = 0;
	limit[1] = 0;

}

void DN_Value_int::set(int val)
{
	if (local) {
		valeur = val;
	}
	else {
		if (externModifAllowed) {
			//envoie de la modif à l'objet (ou serveur)
		}
		else {
			Serial.print("Cette valeur ne peut pas être mdofiée ");
			Serial.println(externModifAllowed);

		}
	}
}

int DN_Value_int::get(bool* success, int timeOut/*Si la valeur est local se parametre n'est pas regardé (max 1s)*/) const
{
	if (init) {
		if (local) {
			*success = true;
			return valeur;
		}
		else {
			if (timeOut > 1000) {
				timeOut = 1000;
			}
			//get bloquant avec recuperation de la valeur sur le serveur

		}

	}
	return 0;
}

void DN_Value_int::setLimit(int min, int max)
{
	if (max < min) {
		Serial.println("max ne peut pas être plus petit que min");
	}
	limit[0] = min;
	limit[1] = max;
}
