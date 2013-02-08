/*
 * OgreModel.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "OgreModel.h"
#include "Debug.h"
#include <tools/StringUtils.h>
#include <tools/OgreUtils.h>

namespace Steel
{

    OgreModel::OgreModel() :
        Model(),
        mSceneNode(NULL), mEntity(NULL),mSceneManager(NULL)
    {

    }
    void OgreModel::init(Ogre::String meshName,
                         Ogre::Vector3 pos,
                         Ogre::Quaternion rot,
                         Ogre::SceneNode *levelRoot,
                         Ogre::SceneManager *sceneManager)
    {
        mSceneManager=sceneManager;
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
        mSceneManager=m.mSceneManager;
        return *this;
    }

    OgreModel::~OgreModel()
    {
//	Debug::log("OgreModel::~OgreModel()").endl();
    }

    void OgreModel::cleanup()
    {
//         Debug::log("OgreModel::cleanup()").endl();
        if (mEntity != NULL)
        {
            mEntity->detachFromParent();
            mSceneManager->destroyEntity(mEntity);
            mEntity = NULL;
        }
        if (mSceneNode != NULL)
        {
            OgreUtils::destroySceneNode(mSceneNode);
            mSceneNode = NULL;
        }
    }

    Ogre::Vector3 OgreModel::position()
    {
        return mSceneNode->getPosition();
    }

    ModelType OgreModel::modelType()
    {
        return MT_OGRE;
    }

    void OgreModel::rotate(Ogre::Vector3 &rotation)
    {
        mSceneNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Degree(rotation.x), Ogre::Node::TS_WORLD);
        mSceneNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Degree(rotation.y), Ogre::Node::TS_WORLD);
        mSceneNode->rotate(Ogre::Vector3::UNIT_Z, Ogre::Degree(rotation.z), Ogre::Node::TS_WORLD);
    }
    
    void OgreModel::rotate(Ogre::Quaternion &q)
    {
        mSceneNode->rotate(q,Ogre::Node::TS_WORLD);
    }

    Ogre::Quaternion OgreModel::rotation()
    {
        return mSceneNode->getOrientation();
    }

    void OgreModel::setNodeAny(Ogre::Any any)
    {
        mSceneNode->getUserObjectBindings().setUserAny(any);
    }

    void OgreModel::move(Ogre::Vector3 const &dpos)
    {
        mSceneNode->translate(dpos);
    }

    void OgreModel::setPosition(Ogre::Vector3 const &pos)
    {
        mSceneNode->setPosition(pos);
    }

    void OgreModel::setRotation(Ogre::Quaternion const &rot)
    {
        mSceneNode->setOrientation(rot);
    }

    void OgreModel::setSelected(bool selected)
    {
#ifdef DEBUG
        mSceneNode->showBoundingBox(selected);
#endif
    }

    void OgreModel::toJson(Json::Value &node)
    {
        if (mSceneNode == NULL)
        {
            Debug::error("OgreModel::toJson() called while mSceneNode is NULL !");
            return;
        }
        //TODO: use abbreviated keys for release
        node["position"] = StringUtils::toJson(mSceneNode->getPosition());
        node["rotation"] = StringUtils::toJson(mSceneNode->getOrientation());
        node["entityMeshName"] = Json::Value(mEntity->getMesh()->getName());
    }

    bool OgreModel::fromJson(Json::Value &node, Ogre::SceneNode *levelRoot, Ogre::SceneManager *sceneManager)
    {
        Ogre::String intro="in OgreModel::fromJson(): ";
        // data to gather
        Ogre::String meshName;
        Ogre::Vector3 pos;
        Ogre::Quaternion rot;

        Json::Value value;
        bool allWasFine = true;

        // gather it
        value = node["position"];
        if (value.isNull() && !(allWasFine = false))
            Debug::error(intro)("invalid field 'position' (skipped).").endl();
        else
            pos = Ogre::StringConverter::parseVector3(value.asString());

        value = node["rotation"];
        if (value.isNull() && !(allWasFine = false))
            Debug::error(intro)("invalid field 'rotation' (skipped).").endl();
        else
            rot = Ogre::StringConverter::parseQuaternion(value.asString());

        value = node["entityMeshName"];
        if (value.isNull() && !(allWasFine = false))
            Debug::error(intro)("invalid field 'entityMeshName' (skipped).").endl();
        else
            meshName = Ogre::String(value.asString());

        if (!allWasFine)
        {
            Debug::error("json was:").endl()(node.toStyledString()).endl();
            Debug::error("deserialisation aborted.").endl();
            return false;
        }

        // now whether we have minor changes (and we apply them directly), or major ones (cleanup, then init).
        // lets start with major ones
        if (mEntity == NULL || meshName != mEntity->getMesh()->getName())
        {
            // make sure we've been called with all arguments, because they're all needed now
            if (levelRoot == NULL || sceneManager == NULL)
            {
                Debug::error(intro)("a new mesh is required, but sceneManager or levelRoot are NULL.");
                Debug::error(" Aborting.").endl();
                return false;
            }
            // make sure the new meshName is valid
            if (meshName.length()==0 || !Ogre::ResourceGroupManager::getSingletonPtr()->resourceExistsInAnyGroup(meshName))
            {
                Debug::error(intro)("new mesh name is not valid:")(meshName).endl();
                return false;
            }
            Ogre::Any any = mSceneNode->getUserAny();
            cleanup();
            init(meshName, pos, rot, levelRoot, sceneManager);
            setNodeAny(any);
        }
        else
        {
            mSceneNode->setPosition(pos);
            mSceneNode->setOrientation(rot);
        }
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
