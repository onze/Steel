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

//         // get pointer to target's ogre model
//         ModelId omid=INVALID_ID;
//         OgreModel *omodel=NULL;
//         value=model["targetOgreModel"];
//         if(value.isNull())
//         {
//             Debug::error(intro)("Invalid field 'targetOgreModel' (skipped).").endl();
//             return INVALID_ID;
//         }
// 
//         omid=Ogre::StringConverter::parseUnsignedLong(value.asString(),INVALID_ID);
//         if(INVALID_ID==omid)
//         {
//             Debug::error(intro)("Invalid OgreModel id for content:").endl();
//             Debug::error(model).endl()(". Aborted.").endl();
//             return INVALID_ID;
//         }
// 
//         omodel=mLevel->ogreModelMan()->at(omid);
//         if(NULL==omodel)
//         {
//             Debug::error(intro)("Could not find targetOgreModel's OgreModel's pointer with id ")(omid).endl();
//             Debug::error("for model:").endl()(model).endl()("Aborting.").endl();
//             return INVALID_ID;
//         }

//         if(!mModels[mid].fromJson(model,mWorld,omid,omodel))
        if(!mModels[mid].fromJson(model))
        {
            releaseModel(mid);
            mid=INVALID_ID;
        }
//         if(!mLevel->linkAgentToModel(aid,MT_PHYSICS,mid))
//         {
//             releaseModel(mid);
//             Debug::error(intro)("could not link agent ")(aid)(" to PhysicsModel ")(mid)(". Model released. Aborted.").endl();
//             return INVALID_ID;
//         }
        return mid;
    }

    ModelId PhysicsModelManager::newModel()
    {
        ModelId mid=allocateModel();
        return mid;
    }
    
    //p* as in physics*
    bool PhysicsModelManager::onAgentLinkedToModel(AgentId aid, ModelId pmid)
    {
        Ogre::String intro=logName()+"::onLinked(): ";
        Agent *agent=mLevel->getAgent(aid);
        if(NULL==agent)
        {
            Debug::error(intro)("Can't find agent ")(aid).endl();
            return false;
        }

        PhysicsModel *pmodel=at(pmid);
#ifdef DEBUG
        assert(agent->modelId(MT_PHYSICS)==pmid);
        assert(agent->model(MT_PHYSICS)==pmodel);
#endif
        
        ModelId omid=agent->ogreModelId();
        if(INVALID_ID==omid)
        {
            Debug::error(intro)("Invalid OgreModel id for agent ")(aid).endl();
            return false;
        }
        OgreModel *omodel=mLevel->ogreModelMan()->at(omid);
        pmodel->init(mWorld,omodel);
        
        return true;
    }
}



// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
