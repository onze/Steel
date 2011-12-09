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

OgreModel::OgreModel() :
		Model()
{

}
void OgreModel::init(	Ogre::String meshName,
						Ogre::Vector3 pos,
						Ogre::Quaternion rot,
						Ogre::SceneNode *levelRoot,
						Ogre::SceneManager *sceneManager)
{
	mEntity = sceneManager->createEntity(meshName);
	mSceneNode = levelRoot->createChildSceneNode(pos, rot);
	mSceneNode->attachObject(mEntity);
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
	mEntity = m.mEntity;
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
	return mSceneNode->getPosition();
}

void OgreModel::rotate(Ogre::Vector3 &rotation)
{
	mSceneNode->rotate(	Ogre::Vector3::UNIT_X,
						Ogre::Degree(rotation.x),
						Ogre::Node::TS_WORLD);
	mSceneNode->rotate(	Ogre::Vector3::UNIT_Y,
						Ogre::Degree(rotation.y),
						Ogre::Node::TS_WORLD);
	mSceneNode->rotate(	Ogre::Vector3::UNIT_Z,
						Ogre::Degree(rotation.z),
						Ogre::Node::TS_WORLD);
}

Ogre::Quaternion OgreModel::rotation()
{
	return mSceneNode->getOrientation();
}

void OgreModel::setNodeAny(Ogre::Any any)
{
	mSceneNode->setUserAny(any);
}

void OgreModel::setPosition(Ogre::Vector3 pos)
{
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

void OgreModel::toJson(Json::Value &object)
{
	if(mSceneNode==NULL)
	{
		Debug::error("OgreModel::toJson() called while mSceneNode is NULL !");
		Debug::log("OgreModel::toJson() called while mSceneNode is NULL !");
		return;
	}
	//TODO: use abreviated keys for release
	object["position"] =
			Json::Value(Ogre::StringConverter::toString(mSceneNode->getPosition()));
	object["rotation"] =
			Json::Value(Ogre::StringConverter::toString(mSceneNode->getOrientation()));
	object["entityMeshName"] = Json::Value(mEntity->getMesh()->getName());
}

void OgreModel::fromJson(Json::Value &object)
{
	mSceneNode->setPosition(Ogre::StringConverter::parseVector3(object["position"].asString()));
	mSceneNode->setOrientation(Ogre::StringConverter::parseQuaternion(object["rotation"].asString()));
	object["entityMeshName"] = Json::Value(mEntity->getMesh()->getName());
}

}
