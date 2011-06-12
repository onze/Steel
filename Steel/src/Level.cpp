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

#include "../include/Level.h"

namespace Steel
{

Ogre::String Level::sPath = "levels/";

Level::Level(Ogre::String name, Ogre::SceneManager *sceneManager) :
	mName(name), mSceneManager(sceneManager)
{
	Ogre::ResourceGroupManager::getSingletonPtr()->addResourceLocation(	Level::path() + mName + "/inanimate",
																		"FileSystem",
																		mName,
																		false);
	mLevelRoot = mSceneManager->getRootSceneNode()->createChildSceneNode("LevelNode", Ogre::Vector3(0, 0, 0));
	mInanimates = std::vector<Inanimate *>();
	mInanimates.push_back(NULL);
}

Level::~Level()
{
	unload();
}

unsigned long Level::createInanimate(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
{

#ifdef DEBUG
	cout << "Level::createInanimate():" << " meshName: " << meshName << " pos:" << pos << " rot: " << rot << endl;
	assert(mSceneManager);
	assert(mLevelRoot);
#endif

	Ogre::Entity *entity = mSceneManager->createEntity(meshName);
	Ogre::SceneNode* sceneNode = mLevelRoot->createChildSceneNode(pos, rot);
	sceneNode->attachObject(entity);

	Inanimate *i = new Inanimate(sceneNode);
	mInanimates.push_back(i);
	return mInanimates.size()-1;
}


Inanimate *Level::getInanimate(unsigned long id)
{
	if(id>=(unsigned long)mInanimates.size())
		return NULL;
	return mInanimates[id];
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
