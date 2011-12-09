/*
 * OgreModelManager.cpp
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */
#include <exception>
#include <iostream>

#include <OGRE/OgreEntity.h>

#include "Debug.h"
#include "OgreModelManager.h"

namespace Steel
{

OgreModelManager::OgreModelManager() :
		_ModelManager<OgreModel>(), mSceneManager(NULL), mLevelRoot(NULL)
{

}

OgreModelManager::OgreModelManager(	Ogre::SceneManager *sceneManager,
									Ogre::SceneNode *levelRoot) :
		_ModelManager<OgreModel>(), mSceneManager(sceneManager), mLevelRoot(levelRoot)
{

}

OgreModelManager::~OgreModelManager()
{
	// TODO Auto-generated destructor stub
}

void OgreModelManager::loadModels(Json::Value models)
{
	for (Json::ValueIterator it = models.begin(); it != models.end(); ++it)
	{
		//TODO: implement id remapping, so that we stay in a low range
//		ModelId key = it.key().asInt64();
		Json::Value value = *it;
		//get value for init
		Ogre::String meshName = value["entityMeshName"].asString();
		Ogre::Vector3 pos =
				Ogre::StringConverter::parseVector3(value["pos"].asString());
		Ogre::Quaternion rot =
				Ogre::StringConverter::parseQuaternion(value["rot"].asString());
		ModelId id = newModel(meshName, pos, rot);
		//get values for load
		at(id)->fromJson(value);
	}
}

ModelId OgreModelManager::newModel(	Ogre::String meshName,
									Ogre::Vector3 pos,
									Ogre::Quaternion rot)
{
	ModelId id = allocateModel();
	at(id)->init(meshName, pos, rot, mLevelRoot, mSceneManager);
	at(id)->setNodeAny(Ogre::Any(id));
	return id;
}

}

