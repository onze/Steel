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
#include <Level.h>

namespace Steel
{   
    OgreModelManager::OgreModelManager(Level *level,Ogre::SceneManager *sceneManager, Ogre::SceneNode *levelRoot) :
    _ModelManager<OgreModel>(level),
        mSceneManager(sceneManager), mLevelRoot(levelRoot)
    {
        // defining default material
        Ogre::MaterialManager *mm=Ogre::MaterialManager::getSingletonPtr();
        if(!mm->resourceExists(OgreModel::MISSING_MATERIAL_NAME))
        {
            Ogre::MaterialPtr mat=mm->create(OgreModel::MISSING_MATERIAL_NAME,level->name(),true);
            Ogre::Technique *tech=mat->getTechnique(0);
            Ogre::Pass *pass=tech->getPass(0);
            pass->setIlluminationStage(Ogre::IlluminationStage::IS_AMBIENT);
            
            pass->setColourWriteEnabled(true);
            pass->setDepthCheckEnabled(true);
            pass->setDepthWriteEnabled(false);
            
            pass->setLightingEnabled(true);
            pass->setSceneBlending(Ogre::SceneBlendType::SBT_TRANSPARENT_ALPHA);
            
            pass->setAmbient(Ogre::ColourValue::White);
            pass->setDiffuse(1.,1.,1.,.75);
            pass->setSpecular(Ogre::ColourValue::White);
            
        }
    }

    OgreModelManager::~OgreModelManager()
    {
        // TODO Auto-generated destructor stub
    }

    bool OgreModelManager::fromSingleJson(Json::Value &model, ModelId &id)
    {
        id = allocateModel();
        int loadingOk=mModels[id].fromJson(model, mLevelRoot, mSceneManager, mLevel->name());
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
        if(!mModels[id].init(meshName, pos, rot, Ogre::Vector3::UNIT_SCALE, mLevelRoot, mSceneManager,mLevel->name()))
            id=INVALID_ID;
        return id;
    }
    
    bool OgreModelManager::onAgentLinkedToModel(Agent *agent, ModelId id)
    {
        mModels[id].setNodeAny(agent->id());
        return true;
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

