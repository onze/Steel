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

//template Debug::DebugObject &Debug::DebugObject::log<Ogre::Real>(Ogre::Real const &);
//template Debug::DebugObject &Debug::DebugObject::log<Ogre::Radian>(Ogre::Radian const &);
//template Debug::DebugObject &Debug::DebugObject::log<Ogre::Degree>(Ogre::Degree const &);
//template Debug::DebugObject &Debug::DebugObject::log<Ogre::Vector2>(Ogre::Vector2 const &);
//template Debug::DebugObject &Debug::DebugObject::log<Ogre::Vector3>(Ogre::Vector3 const &);
//template Debug::DebugObject &Debug::DebugObject::log<Ogre::Quaternion>(Ogre::Quaternion const &);

Debug::Debug()
{
	// TODO Auto-generated constructor stub

}

Debug::~Debug()
{
	// TODO Auto-generated destructor stub
}

}
