/*
 * BTManager.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: onze
 */

#include "BTModelManager.h"

namespace Steel
{

    BTModelManager::BTModelManager(Level *level,Ogre::String path) :
        _ModelManager<BTModel>(level),
        mPath(path)
    {

    }

    BTModelManager::~BTModelManager()
    {
        // TODO Auto-generated destructor stub
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
