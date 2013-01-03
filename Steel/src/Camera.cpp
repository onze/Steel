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

#include <OgreMath.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreEntity.h>
#include <OgreRoot.h>

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
        mCameraNode->lookAt(Ogre::Vector3::NEGATIVE_UNIT_Z, Ogre::SceneNode::TS_WORLD);

        mCamera->setNearClipDistance(.01);
        mCamera->setFarClipDistance(500);
        auto rs=Ogre::Root::getSingletonPtr()->getRenderSystem();
        if (rs->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
        {
            mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
        }
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
        
        mCameraNode->yaw(Ogre::Degree(x), Ogre::SceneNode::TS_WORLD);
        mCameraNode->pitch(Ogre::Degree(y), Ogre::SceneNode::TS_LOCAL);
    }

    bool Camera::fromJson(Json::Value &root)
    {
        if(root.isNull())
        {
            Debug::warning("in Camera::fromJson(): root is null !").endl();
            return false;
        }

        Json::Value value;

        value = root["position"];
        Ogre::Vector3 pos;
        if (value.isNull())
            Debug::warning("in Camera::fromJson(): missing field 'position'.").endl();
        else
            pos = Ogre::StringConverter::parseVector3(value.asString());
        mCameraNode->setPosition(pos);

        value = root["rotation"];
        Ogre::Quaternion rot;
        if (value.isNull())
            Debug::warning("in Camera::fromJson(): missing field 'rotation'.").endl();
        else
            rot = Ogre::StringConverter::parseQuaternion(value.asString());
        mCameraNode->setOrientation(rot);
        
        return true;
    }

    Json::Value Camera::toJson()
    {
        Json::Value value;
        value["position"] = Ogre::StringConverter::toString(mCameraNode->getPosition());
        value["rotation"] = Ogre::StringConverter::toString(mCameraNode->getOrientation());
        return value;
    }

    void Camera::translate(float dx, float dy, float dz, float speed)
    {
        mCameraNode->translate(speed*(mCameraNode->getOrientation() * Ogre::Vector3(dx, dy, dz).normalisedCopy()));
    }

    Ogre::Vector3 Camera::dropTargetPosition()
    {
        //TODO: test with different delta values
        Ogre::Vector3 delta(.0, -1., -10.);
        Ogre::Vector3 pos = mCameraNode->getPosition();
        pos += mCameraNode->getOrientation() * delta;
        return pos;
    }

    Ogre::Quaternion Camera::dropTargetRotation()
    {
        return mCameraNode->getOrientation();
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
