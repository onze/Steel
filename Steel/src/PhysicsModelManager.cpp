#include "PhysicsModelManager.h"

#include "BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "Level.h"
#include <OgreModel.h>
#include <Agent.h>
#include <OgreModelManager.h>

namespace Steel
{
    PhysicsModelManager::PhysicsModelManager(Level * level,btDynamicsWorld* world):
        _ModelManager<PhysicsModel>(level),
        mWorld(NULL)
    {
        mWorld=world;
    }

    PhysicsModelManager::~PhysicsModelManager()
    {
        mWorld=NULL;
    }

    std::vector<ModelId> PhysicsModelManager::fromJson(Json::Value &models)
    {
        Debug::log(logName()+"::fromJson()")(models).endl();
        std::vector<ModelId> ids;
        for (Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            //TODO: implement id remapping, so that we stay in a low id range
            Json::Value value = *it;
            ids.push_back(fromSingleJson(value));
        }
        return ids;
    }

    ModelId PhysicsModelManager::fromSingleJson(Json::Value &model)
    {
        Ogre::String intro=logName()+"::fromSingleJson(): ";
        Json::Value value;
        ModelId mid=newModel();
        
        if(!mModels[mid].fromJson(model))
        {
            mid=INVALID_ID;
        }
        
        return mid;
    }

    ModelId PhysicsModelManager::newModel()
    {
        ModelId mid=allocateModel();
        return mid;
    }
    
    //p* as in physics*
    bool PhysicsModelManager::onAgentLinkedToModel(Agent *agent, ModelId pmid)
    {
        Ogre::String intro=logName()+"::onLinked(): ";

        PhysicsModel *pmodel=at(pmid);
#ifdef DEBUG
        assert(agent->modelId(MT_PHYSICS)==pmid);
        assert(agent->model(MT_PHYSICS)==pmodel);
#endif
        
        ModelId omid=agent->ogreModelId();
        if(INVALID_ID==omid)
        {
            Debug::error(intro)("Invalid OgreModel id for agent ")(agent->id()).endl();
            return false;
        }
        OgreModel *omodel=mLevel->ogreModelMan()->at(omid);
        pmodel->init(mWorld,omodel);
        
        return true;
    }
}



// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
