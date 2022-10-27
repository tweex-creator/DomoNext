// DN_Value_int.h

#ifndef _DN_VALUE_INT_h
#define _DN_VALUE_INT_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif



#include "DN_Value_Base.h"

class DN_Value_int: public DN_Value_Base
{
public:
	DN_Value_int(unsigned char type);

	void set(int);
	int get(bool* success, int timeOut = 0/*Si la valeur est local se parametre n'est pas regard� (max 1s)*/) const; // get bloquant(pour les value extern la r�ponse sera attendu). Si la valeur a pu �tre recuper�e la fn retourne vrai et le pointeur vers la valeur est pass� en parametre 
	void setLimit(int min, int max);
	

private:

	int valeur; 
	int limit[2]; 
};



#endif