#include "PhysicsModelManager.h"

#include "BulletDynamics/Dynamics/btDynamicsWorld.h"

#include "Level.h"
#include <OgreModel.h>
#include <Agent.h>

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
        return std::vector<ModelId>();
    }

    ModelId PhysicsModelManager::fromSingleJson(Json::Value &models)
    {
        ModelId mid=newModel();
        return mid;
    }

    ModelId PhysicsModelManager::newModel()
    {
        ModelId mid=allocateModel();
        return mid;
    }

    bool PhysicsModelManager::onAgentLinked(AgentId aid, ModelId pmodelId)
    {
        Ogre::String intro=logName()+"::onLinked(): ";
        Agent *agent=mLevel->getAgent(aid);

        PhysicsModel *pmodel=at(pmodelId);
#ifdef DEBUG
        assert(agent->modelId(MT_PHYSICS)==pmodelId);
        assert(agent->model(MT_PHYSICS)==pmodel);
#endif
        if(NULL==pmodel)
        {
            Debug::error(intro)("agent ")(agent->id())(" has no PhysicsModel ")(pmodelId)(" to represent. Aborting.").endl();
            return false;
        }

        OgreModel *omodel=(OgreModel *)agent->model(MT_OGRE);
        if(NULL==omodel)
        {
            Debug::error(intro)("agent ")(agent->id())(" has no OgreModel to represent. Aborting.").endl();
            return false;
        }

        pmodel->init(mWorld,omodel);
        return true;
    }
}



// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
