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

#include "Camera.h"

namespace Steel
{

Camera::Camera()
{
	mMode = Camera::FREE;
	mCamera = NULL;
}

Camera::Camera(Ogre::SceneManager *sceneManager)
{
	mSceneManager = sceneManager;
	mMode = Camera::FREE;

	mCamera = sceneManager->createCamera("camera");
	mCamera->setPosition(0.0, 0.0, 0.0);

	mCameraNode = mSceneManager->getRootSceneNode()->createChildSceneNode("cameraNode");
	mCameraNode->attachObject(mCamera);
	mCameraNode->setPosition(0.0, 50.0, -200.0);

	mTarget = mSceneManager->getRootSceneNode()->createChildSceneNode("cameraTarget");
	mTarget->setPosition(0.0, 0.0, 0.0);
}

Camera::Camera(const Camera &camera)
{
	assert(false);
}

Camera::~Camera()
{
	mCameraNode->detachAllObjects();
	mSceneManager->destroySceneNode("cameraNode");
	mSceneManager->destroySceneNode("cameraTarget");
}

Camera &Camera::operator=(const Camera &camera)
{
	assert(false);
	return *this;
}

void Camera::pitchAroundTarget(float delta)
{
	//rotation on the zx plane
	Ogre::Quaternion q(Ogre::Radian(delta), Ogre::Vector3::UNIT_Y);
	q.normalise();
	//vector perpendicular to the 'plane vector' q and the vector going from the target to the camera
	Ogre::Vector3 axis=(q*Ogre::Vector3::UNIT_SCALE).crossProduct(mCameraNode->getPosition()-mTarget->getPosition());
	axis.normalise();
	//get a rotation around that
	Ogre::Quaternion b(Ogre::Radian(delta), axis);
	//make sure we have no roll
	b.y=Ogre::Real(0);
	b.normalise();
	mCameraNode->setPosition(b*mCameraNode->getPosition());
}

void Camera::rotateAroundTarget(float delta)
{
	Ogre::Quaternion q(Ogre::Radian(delta), Ogre::Vector3::UNIT_Y);
	q.normalise();
	mCameraNode->setPosition(q*mCameraNode->getPosition());
}

void Camera::setMode(Camera::Mode mode)
{
	mMode = mode;
	switch (mode)
	{
		case Camera::TARGET:
			mCameraNode->setAutoTracking(true, mTarget);
			mCameraNode->setFixedYawAxis(true);
			break;
		case Camera::FREE:
			mCamera->setAutoTracking(false);
			break;
	}
}

}
