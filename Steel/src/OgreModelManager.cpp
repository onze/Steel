/*
 * OgreModelManager.cpp
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */
#include <exception>
#include <iostream>

#include <OgreEntity.h>

#include "Debug.h"
#include "OgreModelManager.h"
#include "tools/OgreUtils.h"
#include <Agent.h>

namespace Steel
{
    OgreModelManager::OgreModelManager(Level *level,Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot) :
        _ModelManager<OgreModel>(level),
        mSceneManager(sceneManager), mLevelRoot(levelRoot)
    {

    }

    OgreModelManager::~OgreModelManager()
    {
        // TODO Auto-generated destructor stub
    }

    bool OgreModelManager::fromSingleJson(Json::Value &model, ModelId &id)
    {
        //TODO: add safety
        // get values for init
        Ogre::String meshName = model["entityMeshName"].asString();
        // TODO: preparse all meshes, batch declare them, do a single group initialization
        // TODO: use the level's resourceGroup name
        Ogre::ResourceGroupManager::getSingleton ().declareResource(meshName, "FileSystem", "Steel");
        Ogre::ResourceGroupManager::getSingleton ().initialiseResourceGroup("Steel");

        Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(model["position"].asString());
        Ogre::Quaternion rot = Ogre::StringConverter::parseQuaternion(model["rotation"].asString());
        id = newModel(meshName, pos, rot);
        int loadingOk=mModels[id].fromJson(model, mLevelRoot, mSceneManager);
        //TODO discard, quarantine, repair ?
        if(!loadingOk)
        {
            deallocateModel(id);
            id=INVALID_ID;
            return false;
        }
        return true;
    }

    ModelId OgreModelManager::newModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
    {
        ModelId id = allocateModel();
        mModels[id].init(meshName, pos, rot, Ogre::Vector3::UNIT_SCALE, mLevelRoot, mSceneManager);
        return id;
    }
    
    bool OgreModelManager::onAgentLinkedToModel(Agent *agent, ModelId id)
    {
        mModels[id].setNodeAny(agent->id());
        return true;
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

