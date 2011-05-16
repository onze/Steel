/*
 * Camera.cpp
 *
 *  Created on: 2011-05-11
 *      Author: onze
 */
#include <iostream>
using namespace std;
#include <assert.h>

#include <OgreMath.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreEntity.h>

#include "Camera.h"

namespace Steel
{

Camera::Camera() :
	mCamera(0), mCameraNode(0), mSceneManager(0)
{
}

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

Camera::Camera(const Camera &camera)
{
	assert(false);
}

Camera::~Camera()
{
	mCameraNode->detachAllObjects();
	mSceneManager->destroySceneNode("cameraNode");
}

Camera &Camera::operator=(const Camera &camera)
{
	assert(false);
	return *this;
}

void Camera::lookTowards(float x, float y, float roll, float factor)
{
	x *= factor;
	y *= factor;

	mCameraNode->pitch(Ogre::Degree(x),Ogre::SceneNode::TS_LOCAL);
	mCameraNode->yaw(Ogre::Degree(y),Ogre::SceneNode::TS_WORLD);
}

void Camera::translate(float dx,float dy,float dz)
{
	mCameraNode->translate(mCameraNode->getOrientation()*(Ogre::Vector3(dx,dy,dz)));
}

}
