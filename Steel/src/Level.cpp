/*
 * Level.cpp
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifdef DEBUG
#include <assert.h>
#include <Debug.h>
#endif

#include <iostream>
#include <map>
#include <set>

#include <json/json.h>

#include <Ogre.h>

#include "Level.h"
#include "Camera.h"
#include "Agent.h"
#include "OgreModelManager.h"
#include "Model.h"
#include "tools/OgreUtils.h"
#include <tools/StringUtils.h>

namespace Steel
{

    Level::Level(File path, Ogre::String name, Ogre::SceneManager *sceneManager, Camera *camera) :
        TerrainManagerEventListener(),
        mPath(path.subfile(name)), mName(name), mSceneManager(sceneManager), mResGroupAux(0), mCamera(camera),
        mTerrainMan()
    {
        Debug::log(logName()+"()").endl();
        if(!mPath.exists())
            mPath.mkdir();
        auto resGroupMan=Ogre::ResourceGroupManager::getSingletonPtr();
        resGroupMan->addResourceLocation(mPath.fullPath(),"FileSystem",mName,true);
        resGroupMan->initialiseResourceGroup(name);

        mLevelRoot = mSceneManager->getRootSceneNode()->createChildSceneNode("LevelNode", Ogre::Vector3::ZERO);
        mAgents = std::map<AgentId, Agent *>();
        mOgreModelMan = new OgreModelManager(mSceneManager, mLevelRoot);
//	mBTModelMan = new BTModelManager(mPath);
        //mBTModelMan->loadResources();
        mSceneManager->setAmbientLight(Ogre::ColourValue::White);
        mTerrainMan.init(mName+".terrainManager",path.subfile(mName),sceneManager);

    }

    Level::~Level()
    {
        Debug::log(logName()+".~Level()").endl();
        mOgreModelMan->clear();
        delete mOgreModelMan;
        mTerrainMan.destroy();
        OgreUtils::destroyAllAttachedMovableObjects(mLevelRoot);
        OgreUtils::destroySceneNode(mLevelRoot);
        Ogre::ResourceGroupManager::getSingletonPtr()->destroyResourceGroup(mName);
    }

    void Level::deleteAgent(AgentId id)
    {
        Debug::log(logName()+".deleteAgent() id: ")(id).endl();
        std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
        if (it == mAgents.end())
            return;
        delete (*it).second;
        mAgents.erase(it);
    }

    bool Level::linkAgentToModel(AgentId aid, ModelType mtype, ModelId mid)
    {
        Agent *agent = getAgent(aid);
        if (agent == NULL)
        {
            Debug::error(logName()+".linkAgentToOgreModel(): aid ")(aid)(" does not exist.").endl();
            return false;
        }

        ModelManager *mm = modelManager(mtype);
        if (mm == NULL)
        {
            Debug::error(logName()+".linkAgentToOgreModel(): mtype ")((long int)mtype)(" aka \"");
            Debug::error(modelTypesAsString[mtype])("\" does not exist.").endl();
            return false;
        }
        //all clear
        return agent->linkToModel(mtype, mid);
    }

    Ogre::String Level::logName()
    {
        return Ogre::String("Steel::Level<"+mName+">");
    }

    File Level::getSavefile()
    {
        return mPath.subfile(mName + ".lvl");
    }

    bool Level::load()
    {
        Debug::log(logName()+".load()").indent().endl();

        File savefile = getSavefile();
        if (!savefile.exists())
        {
            Debug::warning(logName()+".load(): file does not exists: ");
            Debug::warning(savefile)(" -> Loading cancelled.").endl();
            return false;
        }
        Ogre::String s = savefile.read();
        mTerrainMan.addTerrainManagerEventListener(this);
        if (!deserialize(s))
        {
            mTerrainMan.removeTerrainManagerEventListener(this);
            Debug::warning(logName()+".load(): error deserializing saved file.");
            return false;
        }
        Debug::log(logName()+".load(): loaded ")(savefile)(" successfully.").unIndent().endl();
        return true;
    }

    bool Level::save()
    {
        Debug::log(logName()+".save():").endl();

        Ogre::String s="";
        serialize(s);

        File savefile = getSavefile();
        savefile.write(s);

        Debug::log(logName()+".save() into ")(savefile).endl();
        return true;
    }

    ModelManager *Level::modelManager(ModelType modelType)
    {
        //TODO: replace this by a lookup into a protected map of model managers.
        ModelManager *mm = NULL;
        switch (modelType)
        {
            case MT_OGRE:
                mm = mOgreModelMan;
                break;
            default:
                break;
        }
        return mm;
    }

    AgentId Level::newAgent()
    {

        Agent *t = new Agent(this);
        //	Debug::log(" with id:")(t->id()).endl();
        mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
        return t->id();
    }

    ModelId Level::newOgreModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
    {
        Debug::log(logName()+".newOgreModel(): meshName: ")(meshName)(" pos:")(pos)(" rot: ")(rot).endl();

#ifdef DEBUG
        assert(mSceneManager);
        assert(mLevelRoot);
#endif
        ModelId mid = mOgreModelMan->newModel(meshName, pos, rot);
        return mid;
    }

    void Level::onTerrainEvent(TerrainManager::LoadingState state)
    {
        if(state==mTerrainMan.READY)
            mTerrainMan.removeTerrainManagerEventListener(this);
    }

    Agent *Level::getAgent(AgentId id)
    {
        std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
        if (it == mAgents.end())
            return NULL;
        return it->second;
    }

    void Level::getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection)
    {
//         Debug::log(logName()+".getAgentsIdsFromSceneNodes()").endl();
        //retrieving models
        ModelId id;
        //	Debug::log("adding ids:");
        std::list<ModelId> models = std::list<ModelId>();
        for (std::list<Ogre::SceneNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            auto any=(*it)->getUserObjectBindings().getUserAny();
            if(any.isEmpty())
                continue;
            id = any.get<ModelId>();
            if (!mOgreModelMan->isValid(id))
                continue;
            //		Debug::log("scenenode: ")((*it)->getName())("-> model id: ")(id)(", ");
            models.push_back(id);
        }
        //	Debug::log.endl()("then mapping models to agents: ");

        //retrieving agents
        Agent *t;
        ModelId modelId;
        for (std::map<AgentId, Agent *>::iterator it_agents = mAgents.begin(); it_agents != mAgents.end(); ++it_agents)
        {
            t = (*it_agents).second;
            modelId = t->ogreModelId();
            for (std::list<ModelId>::iterator it_models = models.begin(); it_models != models.end(); ++it_models)
                if (mOgreModelMan->isValid(modelId) && modelId == (*it_models))
                    selection.push_back(t->id());
        }
        //	Debug::log("done.");
    }

    bool Level::isOver(void)
    {
        return false;
    }

    void Level::serialize(Ogre::String &s)
    {
        Debug::log(logName()+".serialise()").endl().indent();
        Json::Value root;
        root["name"] = mName;
        
        root["camera"] = mCamera->toJson();
        root["terrain"] = mTerrainMan.toJson();

        // serialise agents
        Debug::log("processing agents...").endl().indent();
        Json::Value agents;

        for (std::map<AgentId, Agent *>::iterator it_agents = mAgents.begin(); it_agents != mAgents.end(); ++it_agents)
        {
            AgentId aid = (*it_agents).first;
            Agent *agent = (*it_agents).second;
            agents[Ogre::StringConverter::toString(aid)] = agent->toJson();
        }

        root["agents"] = agents;
        Debug::log("all agents done.").unIndent().endl();

        // serialise models
        Debug::log("processing models...").endl();
        Json::Value models;

        for (ModelType modelType = (ModelType) ((int) MT_FIRST + 1); modelType != MT_LAST;
                modelType = (ModelType) ((int) modelType + 1))
        {
            Debug::log(modelType).endl();
            ModelManager *mm = modelManager(modelType);
            if (mm == NULL)
            {
                Debug::log(logName()+".serialize(): no modelManager of type ")(modelTypesAsString[modelType]).endl();
                continue;
            }
            mm->toJson(models[modelTypesAsString[modelType]]);
        }
        root["models"] = models;
        Debug::log("all models done.").unIndent().endl();

        Debug::log("serialization done").unIndent().endl();
        s = root.toStyledString();
        Debug::log(s).endl();
    }

    bool Level::deserialize(Ogre::String &s)
    {
        Debug::log(logName()+".deserialize():").endl().indent();
        Debug::log(s).endl();
        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(s, root, false);
        if (!parsingOk)
        {
            Debug::error("could not parse this:").endl();
            Debug::error(s).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return false;
        }
        Json::Value value;

        // get level info
        value = root["name"];
        assert(!value.isNull());
        // we load the right level
        assert(mName==value.asString());

        //camera
        if(!mCamera->fromJson(root["camera"]))
        {
            Debug::error(logName())(": could not deserialize camera.").endl();
            return false;
        }

        if(!mTerrainMan.fromJson(root["terrain"]))
        {
            Debug::error(logName())(": could not deserialize terrain.").endl();
            return false;
        }

        Debug::log("instanciate ALL the models ! \\o/").endl();
        Json::Value dict = root["models"];
        assert(!dict.isNull());

        for (ModelType i = (ModelType) ((int) MT_FIRST + 1); i != MT_LAST; i = (ModelType) ((int) i + 1))
        {
            Ogre::String type = modelTypesAsString[i];
            Json::Value models = dict[type];
            if (models.isNull())
            {
                Debug::log("no models for type ")(type).endl();
                continue;
            }
            ModelManager *mm = modelManager(i);
            if (mm == NULL)
            {
                Debug::warning("no modelManager for type ")(type).endl();
                continue;
            }
            mm->fromJson(models);
        }
        Debug::log("models done").endl();

        Debug::log("now instanciate ALL the agents ! \\o/").endl();
        dict = root["agents"];

        for (Json::ValueIterator it = dict.begin(); it != dict.end(); ++it)
        {
            //TODO: remap ids to lowest range
            AgentId aid = newAgent();
            assert(aid!=INVALID_ID);
            Agent *agent = getAgent(aid);
            agent->fromJson(*it);
        }
        Debug::log("agents done").endl();
        Debug::log(logName()+".deserialize(): done").unIndent().endl();
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
