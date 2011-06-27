/*
 * OgreModel.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "../include/OgreModel.h"

namespace Steel
{

OgreModel::OgreModel(Ogre::SceneNode *sceneNode) :
	Model(), mSceneNode(sceneNode)
{
}

OgreModel::OgreModel(const OgreModel &m)
{
	(*this)=m;
}

OgreModel &OgreModel::operator=(const OgreModel &m)
{
	mSceneNode=m.mSceneNode;
	return *this;
}

OgreModel::~OgreModel()
{
}

Ogre::Vector3 OgreModel::position()
{
	return mSceneNode->getPosition();
}

void OgreModel::setSelected(bool selected)
{

#ifdef DEBUG
	mSceneNode->showBoundingBox(selected);
#endif

}

}
