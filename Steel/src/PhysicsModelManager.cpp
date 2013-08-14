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

    bool PhysicsModelManager::fromSingleJson(Json::Value &model, ModelId &id)
    {
        Ogre::String intro=logName()+"::fromSingleJson(): ";
        Json::Value value;
        id=newModel();

        if(!mModels[id].fromJson(model))
        {
            deallocateModel(id);
            id=INVALID_ID;
            return false;
        }
        return true;
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
        
        // stop the physics simulation in case the agent is selected
        pmodel->setSelected(agent->isSelected());
        
        return true;
    }
}



// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
