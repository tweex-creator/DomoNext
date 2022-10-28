/*
 Name:		DN_ComEntity.cpp
 Created:	28/10/2022 16:02:33
 Author:	Herve
 Editor:	http://www.visualmicro.com
*/

#include "DN_ComEntity.h"

bool DN_ComCanal::is_free()
{
	return free;
}

bool DN_ComCanal::is_ComNo(int comNo)
{
	return (this->comNo == comNo && !is_free());
}

bool DN_ComCanal::startCanal(int comNo, unsigned int timeOut)
{
	if (!is_free) return false;
	this->free = false;
	this->comNo = comNo;
	this->timeOut = timeOut;
	this->lastActivity = millis();
	return true;
}
