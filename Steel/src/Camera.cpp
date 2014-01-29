/*
 * Camera.cpp
 *
 *  Created on: 2011-05-11
 *      Author: onze
 */

#include <assert.h>
#include <float.h>

#include <OgreMath.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreEntity.h>
#include <OgreRoot.h>

#include "Camera.h"
#include "Level.h"
#include "Debug.h"
#include <tools/JsonUtils.h>
#include <AgentManager.h>
#include <Agent.h>
#include <Engine.h>
#include <OgreModel.h>

namespace Steel
{

    Camera::Camera(Engine *engine, Level *level): EngineEventListener()
    {
        mEngine = engine;
        mLevel = level;
        mSceneManager = mLevel->sceneManager();

        mCamera = mSceneManager->createCamera("mainCamera");
        mCamera->setPosition(0.0, 0.0, -.0);

        mCameraNode = mSceneManager->getRootSceneNode()->createChildSceneNode("mainCameraNode");
        mCameraNode->attachObject(mCamera);

        mCameraNode->setPosition(0.0, 0.0, 0.0);
        mCameraNode->lookAt(Ogre::Vector3::NEGATIVE_UNIT_Z, Ogre::SceneNode::TS_WORLD);
        mCameraNode->setInitialState();

        mCamera->setNearClipDistance(.01);
        mCamera->setFarClipDistance(500);
        auto rs = Ogre::Root::getSingletonPtr()->getRenderSystem();

        if(rs->getCapabilities()->hasCapability(Ogre::RSC_INFINITE_FAR_PLANE))
        {
            mCamera->setFarClipDistance(0);   // enable infinite far clip distance if we can
        }

        mEngine->addEngineEventListener(this);
    }

    Camera::~Camera()
    {
        mEngine->removeEngineEventListener(this);
        mCameraNode->detachAllObjects();
        mSceneManager->destroyCamera(mCamera);
        mLevel = nullptr;
        mEngine = nullptr;
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

        if(value.isNull())
            Debug::warning("in Camera::fromJson(): missing field 'position'.").endl();
        else
            pos = Ogre::StringConverter::parseVector3(value.asString());

        mCameraNode->setPosition(pos);

        value = root["rotation"];
        Ogre::Quaternion rot;

        if(value.isNull())
            Debug::warning("in Camera::fromJson(): missing field 'rotation'.").endl();
        else
            rot = Ogre::StringConverter::parseQuaternion(value.asString());

        mCameraNode->setOrientation(rot);

        return true;
    }

    Json::Value Camera::toJson()
    {
        Json::Value value;
        value["position"] = JsonUtils::toJson(mCameraNode->getPosition());
        value["rotation"] = JsonUtils::toJson(mCameraNode->getOrientation());
        return value;
    }

    void Camera::attachToAgent(AgentId aid)
    {
        mAgentAttachedTo = aid;
        mSceneManager->getRootSceneNode()->removeChild(mCameraNode);

        mLevel->agentMan()->getAgent(aid)->ogreModel()->sceneNode()->addChild(mCameraNode);
        mCameraNode->setPosition(Ogre::Vector3::ZERO);
        mCameraNode->setOrientation(Ogre::Quaternion(Ogre::Degree(0), Ogre::Vector3::NEGATIVE_UNIT_Z));
    }

    void Camera::detachFromAgent()
    {
        mAgentAttachedTo = INVALID_ID;
        auto parent = mCameraNode->getParentSceneNode();
        parent->removeChild(mCameraNode);

        mSceneManager->getRootSceneNode()->addChild(mCameraNode);
        mCameraNode->resetToInitialState();
        mCameraNode->setPosition(parent->getPosition());
    }

    void Camera::translate(float dx, float dy, float dz, float speed)
    {
        mCameraNode->translate(speed * (mCameraNode->getOrientation() * Ogre::Vector3(dx, dy, dz).normalisedCopy()));
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

    Ogre::Vector2 Camera::screenPosition(const Ogre::Vector3 &worldPosition)
    {
        Ogre::Vector3 screenPosition = mCamera->getProjectionMatrix() * mCamera->getViewMatrix() * worldPosition;
        return Ogre::Vector2(0.5f + 0.5f * screenPosition.x, 0.5f - 0.5f * screenPosition.y);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
