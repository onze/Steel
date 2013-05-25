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
    bool Debug::isInit=false;
    Ogre::String Debug::DebugObject::sIndentString=Ogre::String("    ");

    Debug::Debug()
    {
        // TODO Auto-generated constructor stub

    }

    Debug::~Debug()
    {
        // TODO Auto-generated destructor stub
    }
    
    void Debug::ignoreNextErrorMessage()
    {
        Debug::warning("next error can be safely ignored").endl();
    }
    
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
