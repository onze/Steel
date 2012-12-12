/*
 * Debug.cpp
 *
 *  Created on: 2011-06-21
 *      Author: onze
 */

#include "../include/Debug.h"

#include <Ogre.h>

namespace Steel
{

Debug::DebugObject Debug::log;
Debug::DebugObject Debug::warning;
Debug::DebugObject Debug::error;
Ogre::String Debug::DebugObject::sIndentString=Ogre::String("    ");

Debug::Debug()
{
	// TODO Auto-generated constructor stub

}

Debug::~Debug()
{
	// TODO Auto-generated destructor stub
}

}
