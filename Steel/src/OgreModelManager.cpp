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

    std::vector<ModelId> OgreModelManager::fromJson(Json::Value &models)
    {
        Debug::log(logName()+"::fromJson()")(models).endl();
        std::vector<ModelId> ids;
        for (Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            //TODO: implement id remapping, so that we stay in a low id range
            //TODO: put fool proof conditions under #ifdef DEBUG
            Json::Value value = *it;
            ids.push_back(fromSingleJson(value));
        }
        return ids;
    }

    ModelId OgreModelManager::fromSingleJson(Json::Value &model)
    {
        // get values for init
        Ogre::String meshName = model["entityMeshName"].asString();
        // TODO: preparse all meshes, batch declare them, do a single group initialization
        // TODO: use the level's resourceGroup name
        Ogre::ResourceGroupManager::getSingleton ().declareResource(meshName, "FileSystem", "Steel");
        Ogre::ResourceGroupManager::getSingleton ().initialiseResourceGroup("Steel");

        Ogre::Vector3 pos = Ogre::StringConverter::parseVector3(model["position"].asString());
        Ogre::Quaternion rot = Ogre::StringConverter::parseQuaternion(model["rotation"].asString());
        ModelId id = newModel(meshName, pos, rot);
        //get values for load
        //incRef(id);
        int loadingOk=mModels[id].fromJson(model, mLevelRoot, mSceneManager);
        //TODO discard, quarantine, repair ?
        if(!loadingOk)
        {
            releaseModel(id);
            id=INVALID_ID;
        }
        return id;
    }

    ModelId OgreModelManager::newModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
    {
        ModelId id = allocateModel();
        mModels[id].init(meshName, pos, rot, mLevelRoot, mSceneManager);
        mModels[id].setNodeAny(Ogre::Any(id));
        return id;
    }

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

