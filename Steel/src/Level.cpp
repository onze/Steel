/*
 * Level.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifdef DEBUG
#include <assert.h>
#include <Debug.h>
#endif

#include <Ogre.h>

#include "Level.h"
#include "Model.h"

namespace Steel
{

Ogre::String Level::sPath = "levels/";

Level::Level(Ogre::String name, Ogre::SceneManager *sceneManager) :
	mName(name), mSceneManager(sceneManager)
{
	Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(	Level::path() + mName + "/meshes",
																		"FileSystem",
																		mName,
																		false);
	mLevelRoot = mSceneManager->getRootSceneNode()->createChildSceneNode("LevelNode", Ogre::Vector3(0, 0, 0));
	mThings = std::map<ThingId, Thing *>();
	mOgreModelMan = new OgreModelManager(mSceneManager, mLevelRoot);
}

Level::~Level()
{
	mOgreModelMan->clear();
	delete mOgreModelMan;
}

void Level::deleteThing(ThingId id)
{
	Debug::log("Level::deleteThing() id: ")( id ).endl();
	std::map<ThingId, Thing *>::iterator it = mThings.find(id);
	if (it == mThings.end())
		return;
	delete (*it).second;
	mThings.erase(it);
}

ModelManager *Level::modelManager(ModelType modelType)
{
	ModelManager *mm = NULL;
	switch (modelType)
	{
		case MT_OGRE:
			mm = mOgreModelMan;
			break;
	}
	return mm;
}

ThingId Level::newThing(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
{
	Debug::log("Level::newThing(): meshName: ")(meshName)(" pos:")(pos)(" rot: ")(rot);

#ifdef DEBUG
	assert(mSceneManager);
	assert(mLevelRoot);
#endif

	Thing *t = new Thing(this);
	Debug::log(" with id:")(t->id()).endl();
	t->addModel(MT_OGRE, mOgreModelMan->newModel(meshName, pos, rot));
	mThings.insert(std::pair<ThingId, Thing *>(t->id(), t));
	return t->id();
}

Thing *Level::getThing(ThingId id)
{
	std::map<ThingId, Thing *>::iterator it = mThings.find(id);
	if (it == mThings.end())
		return NULL;
	return it->second;
}

void Level::getThingsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection)
{
	Debug::log("Level::getThingsIdsFromSceneNodes()").endl();
	//retrieving models
	ModelId id;
//	Debug::log("adding ids:");
	std::list<ModelId> models = std::list<ModelId>();
	for (std::list<Ogre::SceneNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
	{
		id = Ogre::any_cast<ModelId>((*it)->getUserAny());
		if (!mOgreModelMan->isValid(id))
			continue;
//		Debug::log("scenenode: ")((*it)->getName())("-> model id: ")(id)(", ");
		models.push_back(id);
	}
//	Debug::log.endl()("then mapping models to things: ");

	//retrieving things
	Thing *t;
	ModelId modelId;
	for (std::map<ThingId, Thing *>::iterator it_things = mThings.begin(); it_things != mThings.end(); ++it_things)
	{
		t=(*it_things).second;
		modelId=t->ogreModelId();
		for (std::list<ModelId>::iterator it_models = models.begin(); it_models != models.end(); ++it_models)
			if(mOgreModelMan->isValid(modelId) && modelId==(*it_models))
				selection.push_back(t->id());
	}
//	Debug::log("done.");
}

bool Level::isOver(void)
{
	return false;
}

bool Level::unload(void)
{
	Ogre::ResourceGroupManager::getSingletonPtr()->removeResourceLocation(Level::path() + mName + "/mesh", mName);
	return true;
}

}
