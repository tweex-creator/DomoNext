/*
 Name:		DN_Value_int.h
 Created:	27/10/2022 11:39:51
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#ifndef _DN_Value_int_h
#define _DN_Value_int_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

#include <DN_Value.h>

class DN_Value_int : public DN_Value
{
public:
	DN_Value_int(unsigned char type);

	void set(int);
	int get(bool* success, int timeOut = 0/*Si la valeur est local se parametre n'est pas regardé (max 1s)*/) const; // get bloquant(pour les value extern la réponse sera attendu). Si la valeur a pu être recuperée la fn retourne vrai et le pointeur vers la valeur est passé en parametre 
	void setLimit(int min, int max);


private:

	int valeur;
	int limit[2];
};

#endif

