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
	ModelManager<OgreModel> (), mSceneManager(NULL), mLevelRoot(NULL)
{

}

OgreModelManager::OgreModelManager(Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot) :
	ModelManager<OgreModel> (), mSceneManager(sceneManager), mLevelRoot(levelRoot)
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

	ModelId id = (ModelId) mModels.size()+1;
	sceneNode->setUserAny(Ogre::Any(id));

	mModels.insert(std::pair<ModelId, OgreModel>(id, OgreModel(sceneNode)));
	ModelMapIterator it=mModels.find(id);

	it->second.incRef();
	return id;
}

void OgreModelManager::getThingsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection)
{
	Debug::log("OgreModelManager::getThingsIdsFromSceneNodes()").endl();
	ModelId id;
	for (std::list<Ogre::SceneNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
	{
		id = Ogre::any_cast<ModelId>((*it)->getUserAny());
		if (!isValid(id))
			continue;
		selection.push_back(id);
	}
}

}

