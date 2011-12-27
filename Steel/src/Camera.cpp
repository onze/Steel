/*
 * Camera.cpp
 *
 *  Created on: 2011-05-11
 *      Author: onze
 */
#ifdef DEBUG
#include <iostream>
using namespace std;
#include <assert.h>
#endif

#include <OGRE/OgreMath.h>
#include <OGRE/OgreQuaternion.h>
#include <OGRE/OgreVector3.h>
#include <OGRE/OgreEntity.h>

#include "Camera.h"
#include "Debug.h"

namespace Steel
{

Camera::Camera(Ogre::SceneManager *sceneManager)
{
	mSceneManager = sceneManager;

	mCamera = sceneManager->createCamera("camera");
	mCamera->setPosition(0.0, 0.0, -.0);

	mCameraNode = sceneManager->getRootSceneNode()->createChildSceneNode("cameraNode");
	mCameraNode->attachObject(mCamera);

	mCameraNode->setPosition(0.0, 50.0, 200.0);
	mCameraNode->lookAt(Ogre::Vector3::ZERO, Ogre::SceneNode::TS_WORLD);
}

Camera::~Camera()
{
	mCameraNode->detachAllObjects();
	mSceneManager->destroySceneNode("cameraNode");
	mSceneManager->destroyCamera(mCamera);
}

void Camera::lookTowards(float x, float y, float roll, float factor)
{
	x *= factor;
	y *= factor;

	mCameraNode->pitch(Ogre::Degree(x), Ogre::SceneNode::TS_LOCAL);
	mCameraNode->yaw(Ogre::Degree(y), Ogre::SceneNode::TS_WORLD);
}

bool Camera::fromJson(Json::Value &root)
{
	if(root.isNull())
	{
		Debug::warning("in Camera::fromJson(): root is null !").endl();
		return false;
	}

	bool success = true;
	Json::Value value;

	value = root["position"];
	Ogre::Vector3 pos;
	if (value.isNull())
	{
		success = false;
		Debug::warning("in Camera::fromJson(): missing field 'position'.").endl();
	}
	else
	{
		pos = Ogre::StringConverter::parseVector3(value.asString());

	}
	mCameraNode->setPosition(pos);

	value = root["rotation"];
	Ogre::Quaternion rot;
	if (value.isNull())
	{
		success = false;
		Debug::warning("in Camera::fromJson(): missing field 'rotation'.").endl();
	}
	else
	{
		rot = Ogre::StringConverter::parseQuaternion(value.asString());

	}
	mCameraNode->setOrientation(rot);
	return success;
}

Json::Value Camera::toJson()
{
	Json::Value value;
	value["position"] = Ogre::StringConverter::toString(mCameraNode->getPosition());
	value["rotation"] = Ogre::StringConverter::toString(mCameraNode->getOrientation());
	return value;
}

void Camera::translate(float dx, float dy, float dz)
{
	mCameraNode->translate(mCameraNode->getOrientation() * (Ogre::Vector3(dx, dy, dz)));
}

}
