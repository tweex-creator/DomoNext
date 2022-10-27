/*
 Name:		DN_Value.cpp
 Created:	27/10/2022 11:41:02
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_Value.h"



DN_Value::DN_Value(unsigned char type)
{
	this->type = type;
}

DN_Value::~DN_Value()
{
}

void DN_Value::setupLocalValue(char name[10], char valueId[3], bool externModifAllowed, bool offlineModifAllowed)
{
	strcpy(this->name, name);
	strcpy(this->valueId, valueId);
	this->externModifAllowed = externModifAllowed;
	this->offlineModifAllowed = offlineModifAllowed;
	init = true;
	local = true;
}

void DN_Value::setuExternlValue(char objId[3], char valueId[3])
{
	strcpy(this->objId, objId);
	strcpy(this->valueId, valueId);
	this->externModifAllowed = false;// par securité, sera mis a la bonne valeur lors de la synchronisation avec le serveur
	this->offlineModifAllowed = false;
	init = false;
	local = true;
}

unsigned char DN_Value::getType()
{
	return type;
}

void DN_Value::getName(char* res)
{
	strcpy(res, name);
}

void DN_Value::getValueId(char* res)
{
	strcpy(res, valueId);

}

void DN_Value::getObjId(char* res)
{
	strcpy(res, objId);

}
