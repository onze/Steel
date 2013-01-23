#include "PhysicsModelManager.h"

namespace Steel
{
    PhysicsModelManager::PhysicsModelManager():_ModelManager<PhysicsModel>(),
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

    ModelId PhysicsModelManager::newModel()
    {
        return INVALID_ID;
    }

}



// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
