#include "models/PhysicsModelManager.h"

#include <bullet/BulletDynamics/Dynamics/btDynamicsWorld.h>
#include <bullet/BulletCollision/CollisionDispatch/btGhostObject.h>
#include <bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.h>
// #include "BulletDynamics/Dynamics/btDynamicsWorld.h"
// #include "BulletCollision/CollisionDispatch/btGhostObject.h"

#include "Debug.h"
#include "Level.h"
#include "models/Agent.h"
#include "models/OgreModel.h"
#include "models/OgreModelManager.h"


extern ContactAddedCallback gContactAddedCallback;
extern ContactDestroyedCallback gContactDestroyedCallback;

namespace Steel
{
    PhysicsModelManager::PhysicsModelManager(Level *level, btDynamicsWorld *world): _ModelManager<PhysicsModel>(level),
        mWorld(nullptr), mbulletGhostPairCallback(nullptr)
    {
        mWorld = world;

        // allows for ghost functionality (hitbox/triggers)
        mbulletGhostPairCallback = new btGhostPairCallback();
        mWorld->getPairCache()->setInternalGhostPairCallback(mbulletGhostPairCallback);
    }

    PhysicsModelManager::~PhysicsModelManager()
    {
        STEEL_DELETE(mbulletGhostPairCallback);

        if(nullptr != mWorld)
            mWorld = nullptr;
    }

    ModelId PhysicsModelManager::newModel()
    {
        ModelId mid = allocateModel();
        return mid;
    }

    //p* as in physics*
    bool PhysicsModelManager::onAgentLinkedToModel(Agent *agent, ModelId pmid)
    {
        Ogre::String intro = logName() + "::onLinked(): ";

        PhysicsModel *pmodel = at(pmid);
#ifdef DEBUG
        assert(agent->modelId(ModelType::PHYSICS) == pmid);
        assert(agent->model(ModelType::PHYSICS) == pmodel);
#endif

        ModelId omid = agent->ogreModelId();

        if(INVALID_ID == omid)
        {
            Debug::error(intro)("Invalid OgreModel id for agent ")(agent->id()).endl();
            return false;
        }

        OgreModel *omodel = mLevel->ogreModelMan()->at(omid);
        pmodel->init(mWorld, omodel);
        pmodel->setUserPointer(agent);

        // stop the physics simulation in case the agent is selected
        pmodel->setSelected(agent->isSelected());

        return true;
    }

    void PhysicsModelManager::update(float timestep)
    {
        // TODO: OPT: use an update list, updated upon model init/cleanup ?
        for(auto & model : mModels)
        {
            if(model.refCount() > 0)
                model.update(timestep, this);
        }
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
