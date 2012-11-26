/*
 * BTManager.cpp
 *
 *  Created on: Jan 18, 2012
 *      Author: onze
 */

#include "BTModelManager.h"

namespace Steel
{

BTModelManager::BTModelManager() :
		_ModelManager<BTModel>(), mPath("")
{
}

BTModelManager::BTModelManager(Ogre::String path) :
		_ModelManager<BTModel>(), mPath(path)
{

}

BTModelManager::~BTModelManager()
{
	// TODO Auto-generated destructor stub
}

} /* namespace Steel */
