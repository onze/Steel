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

bool OgreModelManager::fromJson(Json::Value &models)
{
	Debug::log("OgreModelManager::fromJson()").endl();
	Debug::log(Ogre::String(models.toStyledString())).endl();
	for (Json::ValueIterator it = models.begin(); it != models.end(); ++it)
	{
		//TODO: implement id remapping, so that we stay in a low id range
		//TODO: put fool proof conditions under #ifdef DEBUG
		Json::Value value = *it;
		//get value for init
		Ogre::String meshName = value["entityMeshName"].asString();

		Ogre::Vector3 pos =
				Ogre::StringConverter::parseVector3(value["position"].asString());
		Ogre::Quaternion rot =
				Ogre::StringConverter::parseQuaternion(value["rotation"].asString());
		ModelId id = newModel(meshName, pos, rot);
		//get values for load
		//incRef(id);
		mModels[id].fromJson(value);
	}
	return true;
}

ModelId OgreModelManager::newModel(	Ogre::String meshName,
									Ogre::Vector3 pos,
									Ogre::Quaternion rot)
{
	ModelId id = allocateModel();
	mModels[id].init(meshName, pos, rot, mLevelRoot, mSceneManager);
	mModels[id].setNodeAny(Ogre::Any(id));
	return id;
}

}

