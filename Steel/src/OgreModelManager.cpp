/*
 * OgreModelManager.cpp
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */
#include <exception>
#include <iostream>

#include <OgreEntity.h>

#include "Debug.h"
#include "OgreModelManager.h"

namespace Steel
{

OgreModelManager::OgreModelManager() :
	_ModelManager<OgreModel> (), mSceneManager(NULL), mLevelRoot(NULL)
{

}

OgreModelManager::OgreModelManager(Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot) :
	_ModelManager<OgreModel> (), mSceneManager(sceneManager), mLevelRoot(levelRoot)
{

}

OgreModelManager::~OgreModelManager()
{
	// TODO Auto-generated destructor stub
}

ModelId OgreModelManager::newModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
{
	Ogre::Entity *entity = mSceneManager->createEntity(meshName);
	Ogre::SceneNode* sceneNode = mLevelRoot->createChildSceneNode(pos, rot);
	sceneNode->attachObject(entity);

	ModelId id=insertModel(OgreModel(sceneNode,entity));
	Debug::log("OgreModelManager::newModel() id:")(id)(" sceneNode name: ")(sceneNode->getName()).endl();
	sceneNode->setUserAny(Ogre::Any(id));
	return id;
}



}

