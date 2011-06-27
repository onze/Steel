/*
 * Level.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifdef DEBUG
#include <assert.h>
#include <iostream>
using namespace std;
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
	mThings = std::map<ThingId, Steel::Thing *>();
	mOgreModelMan=new OgreModelManager(mSceneManager,mLevelRoot);
}

Level::~Level()
{
	mOgreModelMan->clear();
	delete mOgreModelMan;
}

ThingId Level::newThing(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
{

#ifdef DEBUG
	cout << "Level::newThing():" << " meshName: " << meshName << " pos:" << pos << " rot: " << rot << endl;
	assert(mSceneManager);
	assert(mLevelRoot);
#endif

	Thing *t = new Thing(this);
	t->addModel(MT_OGRE,mOgreModelMan->newModel(meshName,pos,rot));
	mThings.insert(std::pair<ThingId, Thing *>(t->id(), t));
	return t->id();
}

Thing *Level::getThing(ThingId id)
{
	map<ThingId,Thing *>::iterator it=mThings.find(id);
	if(it==mThings.end())
		return NULL;
	return it->second;
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
