// 
// 
// 

#include "DN_Value_Base.h"

DN_Value_Base::DN_Value_Base(unsigned char type)
{
	this->type = type;
}

DN_Value_Base::~DN_Value_Base()
{
}

void DN_Value_Base::setupLocalValue(char name[10], char valueId[3], bool externModifAllowed, bool offlineModifAllowed)
{
	strcpy(this->name, name);
	strcpy(this->valueId, valueId);
	this->externModifAllowed = externModifAllowed;
	this->offlineModifAllowed = offlineModifAllowed;
	init = true;
	local = true;
}

void DN_Value_Base::setuExternlValue(char objId[3], char valueId[3])
{
	strcpy(this->objId, objId);
	strcpy(this->valueId, valueId);
	this->externModifAllowed = false;// par securité, sera mis a la bonne valeur lors de la synchronisation avec le serveur
	this->offlineModifAllowed = false;
	init = false;
	local = true;
}

unsigned char DN_Value_Base::getType()
{
	return type;
}

void DN_Value_Base::getName(char* res)
{
	strcpy(res, name);
}

void DN_Value_Base::getValueId(char* res)
{
	strcpy(res, valueId);

}

void DN_Value_Base::getObjId(char* res)
{
	strcpy(res, objId);

}
