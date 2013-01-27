#include "PhysicsModelManager.h"

#include "Level.h"
#include <OgreModel.h>
#include <Agent.h>
#include <BtOgreGP.h>
#include <BtOgrePG.h>

namespace Steel
{
    PhysicsModelManager::PhysicsModelManager(Level * level):_ModelManager<PhysicsModel>(level),
        mWorld(NULL),mSolver(NULL),mDispatcher(NULL),mCollisionConfig(NULL),mBroadphase(NULL)
    {

    }

    PhysicsModelManager::~PhysicsModelManager()
    {
        if(NULL!=mWorld)
        {
            delete mWorld;
            delete mSolver;
            delete mDispatcher;
            delete mCollisionConfig;
            delete mBroadphase;
        }
    }

    void PhysicsModelManager::init()
    {
        mBroadphase = new btAxisSweep3(btVector3(-10000,-10000,-10000), btVector3(10000,10000,10000), 1024);
        mCollisionConfig = new btDefaultCollisionConfiguration();
        mDispatcher = new btCollisionDispatcher(mCollisionConfig);
        mSolver = new btSequentialImpulseConstraintSolver();

        mWorld = new btDiscreteDynamicsWorld(mDispatcher, mBroadphase, mSolver, mCollisionConfig);
        mWorld->setGravity(btVector3(0,-9.8,0));
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

    void PhysicsModelManager::update(float timestep)
    {
        mWorld->stepSimulation(timestep,7);
    }
}



// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
