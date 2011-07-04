/*
 * OgreModel.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "OgreModel.h"
#include "Debug.h"

namespace Steel
{

OgreModel::OgreModel(Ogre::SceneNode *sceneNode,Ogre::Entity *entity) :
	Model(), mSceneNode(sceneNode),mEntity(entity)
{
//	Debug::log("OgreModel::OgreModel()").endl();
}

OgreModel::OgreModel(const OgreModel &m)
{
//	Debug::log("OgreModel::OgreModel(const OgreModel &m)").endl();
	Model::operator=(m);
	(*this) = m;
}

OgreModel &OgreModel::operator=(const OgreModel &m)
{
	Model::operator =(m);
//	Debug::log("OgreModel::operator=(const OgreModel &m)").endl();
	mSceneNode = m.mSceneNode;
	return *this;
}

OgreModel::~OgreModel()
{
//	Debug::log("OgreModel::~OgreModel()").endl();
}

void OgreModel::cleanup()
{
	Debug::log("OgreModel::cleanup()").endl();

	mSceneNode->removeAndDestroyAllChildren();
	mSceneNode->detachAllObjects();
}

Ogre::Vector3 OgreModel::position()
{
	Debug::log("my pos: ")(mSceneNode->getPosition()).endl();
	return mSceneNode->getPosition();
}

void OgreModel::rotate(Ogre::Vector3 &rotation)
{
	mSceneNode->rotate(Ogre::Vector3::UNIT_X,Ogre::Degree(rotation.x),Ogre::Node::TS_WORLD);
	mSceneNode->rotate(Ogre::Vector3::UNIT_Y,Ogre::Degree(rotation.y),Ogre::Node::TS_WORLD);
	mSceneNode->rotate(Ogre::Vector3::UNIT_Z,Ogre::Degree(rotation.z),Ogre::Node::TS_WORLD);
}

Ogre::Quaternion OgreModel::rotation()
{
	return mSceneNode->getOrientation();
}

void OgreModel::setPosition(Ogre::Vector3 pos)
{
	Debug::log("my pos: ")(mSceneNode->getPosition()).endl();
	mSceneNode->setPosition(pos);
}

void OgreModel::setSelected(bool selected)
{

#ifdef DEBUG
	mSceneNode->showBoundingBox(selected);
#endif

}

void OgreModel::translate(Ogre::Vector3 t)
{
	mSceneNode->translate(t);
}

}
