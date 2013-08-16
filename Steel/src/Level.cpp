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

#include <Engine.h>
#include "Level.h"
#include "Camera.h"
#include "Agent.h"
#include "Model.h"
#include "OgreModelManager.h"
#include "tools/OgreUtils.h"
#include "tools/StringUtils.h"
#include "tools/JsonUtils.h"
#include "PhysicsModelManager.h"
#include "TerrainPhysicsManager.h"
#include "AgentManager.h"
#include "SelectionManager.h"

namespace Steel
{

    Level::Level(Engine *engine, File path, Ogre::String name) : TerrainManagerEventListener(),
        mEngine(engine), mViewport(NULL), mPath(path.subfile(name)), mName(name),
        mBackgroundColor(Ogre::ColourValue::Black), mSceneManager(NULL), mLevelRoot(NULL),
        mAgentMan(NULL), mOgreModelMan(NULL), mPhysicsModelMan(NULL), mBTModelMan(NULL), mTerrainMan(), mSelectionMan(NULL),
        mCamera(NULL), mMainLight(NULL)
    {
        Debug::log(logName() + "()").endl();
        if (!mPath.exists())
            mPath.mkdir();
        auto resGroupMan = Ogre::ResourceGroupManager::getSingletonPtr();
        resGroupMan->addResourceLocation(mPath.fullPath(), "FileSystem", mName, true);
        resGroupMan->initialiseResourceGroup(name);

        mSceneManager = Ogre::Root::getSingletonPtr()->createSceneManager(Ogre::ST_GENERIC, logName() + "_sceneManager");
        // mTerrainManager search it under this same name in TerrainManager::toJson() and TerrainManager::build()
        mMainLight = mSceneManager->createLight("levelLight");

        // Create the camera
        mCamera = new Camera(this);

        // reuse viewport if possible, entire window
        Ogre::RenderWindow *window = mEngine->renderWindow();
        int zOrder = -1;
        while (window->hasViewportWithZOrder(++zOrder))
            ;
        mViewport = window->addViewport(mCamera->cam(), zOrder);
        mViewport->setBackgroundColour(mBackgroundColor);

        // Alter the camera aspect ratio to match the viewport
        Ogre::Real aspectRatio = Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight());
        mCamera->cam()->setAspectRatio(aspectRatio);

        mLevelRoot = mSceneManager->getRootSceneNode()->createChildSceneNode("levelNode", Ogre::Vector3::ZERO);
        mTerrainMan.init(mName + ".terrainManager", path.subfile(mName), mSceneManager);
        
        mSelectionMan = new SelectionManager(this);
        mAgentMan=new AgentManager(this);
        mOgreModelMan = new OgreModelManager(this, mSceneManager, mLevelRoot);
        mPhysicsModelMan = new PhysicsModelManager(this, mTerrainMan.terrainPhysicsMan()->world());
        mBTModelMan = new BTModelManager(this, mEngine->rawResourcesDir().subfile("BT"));
        mSelectionMan = new SelectionManager(this);
        
        mSceneManager->setAmbientLight(Ogre::ColourValue::White);
    }

    Level::~Level()
    {
        Debug::log(logName() + ".~Level()").endl();

        mPhysicsModelMan->clear();
        delete mPhysicsModelMan;
        mPhysicsModelMan = NULL;
        mTerrainMan.shutdown();

        mOgreModelMan->clear();
        delete mOgreModelMan;
        mOgreModelMan = NULL;

        mBTModelMan->clear();
        delete mBTModelMan;
        mBTModelMan = NULL;

        Ogre::RenderWindow *window = mEngine->renderWindow();
        if (mSceneManager != NULL)
        {
            delete mCamera;
            mSceneManager->clearScene();
            mSceneManager->destroyAllCameras();
            window->removeViewport(mViewport->getZOrder());
            Ogre::Root::getSingletonPtr()->destroySceneManager(mSceneManager);
            mViewport = NULL;
            mCamera = NULL;
            mSceneManager = NULL;
        }
        Ogre::ResourceGroupManager::getSingletonPtr()->destroyResourceGroup(mName);
        mViewport = NULL;
    }

    bool Level::linkAgentToModel(AgentId aid, ModelType mType, ModelId mid)
    {
        Ogre::String intro = "Level::linkAgentToModel(): ";
        if (aid == INVALID_ID)
        {
            Debug::error(intro)("aid ")(aid)(" does not exist.").endl();
            return false;
        }
        Agent *agent = mAgentMan->getAgent(aid);

        ModelManager *mm = modelManager(mType);
        if (mm == NULL)
        {
            Debug::error(intro)("mType ")((long int) mType)(" aka \"");
            Debug::error(modelTypesAsString[mType])("\" does not exist.").endl();
            return false;
        }

        if (mid == INVALID_ID)
        {
            Debug::error(intro)("invalid model id.").endl();
            return false;
        }

        if (!agent->linkToModel(mType, mid))
        {
            Debug::error(intro)("linkage agent ")(aid)("<->model<")(modelTypesAsString[mType])(">");
            Debug::error(mid)(" failed.").endl();
            return false;
        }
        return true;
    }

    Ogre::String Level::logName()
    {
        return Ogre::String("Steel::Level<" + mName + ">");
    }

    File Level::getSavefile()
    {
        return mPath.subfile(mName + ".lvl");
    }

    bool Level::load()
    {
        Debug::log(logName() + ".load()").indent().endl();

        File savefile = getSavefile();
        if (!savefile.exists())
        {
            Debug::warning(logName() + ".load(): file does not exists: ");
            Debug::warning(savefile)(" -> Loading cancelled.").endl();
            return false;
        }
        Ogre::String s = savefile.read();
        mTerrainMan.addTerrainManagerEventListener(this);
        if (!deserialize(s))
        {
            mAgentMan->deleteAllAgents();
            mBTModelMan->clear();
            mPhysicsModelMan->clear();
            mOgreModelMan->clear();
            
            mTerrainMan.removeTerrainManagerEventListener(this);
            Debug::warning(logName() + ".load(): error deserializing saved file.");
            return false;
        }
        Debug::log(logName() + ".load(): loaded ")(savefile)(" successfully.").unIndent().endl();
        return true;
    }

    bool Level::save()
    {
        Debug::log(logName() + ".save():").endl();

        Ogre::String s = "";
        serialize(s);

        File savefile = getSavefile();
        savefile.write(s, File::OM_OVERWRITE);

        Debug::log(logName() + ".save() into ")(savefile).endl();
        return true;
    }

    ModelManager *Level::modelManager(ModelType modelType)
    {
        //TODO: replace this by a lookup into a protected map of model managers ?
        ModelManager *mm = NULL;
        switch (modelType)
        {
            case MT_OGRE:
                mm = mOgreModelMan;
                break;
            case MT_PHYSICS:
                mm = mPhysicsModelMan;
                break;
            case MT_BT:
                mm = mBTModelMan;
                break;
            default:
                Debug::error("Level::modelManager(): unknown modelType ")(modelType);
                if (MT_FIRST < modelType && modelType < MT_LAST)
                    Debug::error(" (")(modelTypesAsString[modelType])(")");
                else
                    Debug::error(" (with no string representation.)");
                Debug::error.endl();
                break;
        }
        return mm;
    }

    ModelId Level::newOgreModel(Ogre::String meshName, Ogre::Vector3 pos, Ogre::Quaternion rot)
    {
        Debug::log(logName() + ".newOgreModel(): meshName: ")(meshName)(" pos:")(pos)(" rot: ")(rot).endl();

#ifdef DEBUG
        assert(mSceneManager);
        assert(mLevelRoot);
#endif
        ModelId mid = mOgreModelMan->newModel(meshName, pos, rot);
        return mid;
    }

    void Level::onTerrainEvent(TerrainManager::LoadingState state)
    {
        if (state == mTerrainMan.READY)
            mTerrainMan.removeTerrainManagerEventListener(this);
    }

    void Level::getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection)
    {
        //retrieving models
        ModelId id;
        std::list<ModelId> models = std::list<ModelId>();
        for (std::list<Ogre::SceneNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            auto any = (*it)->getUserObjectBindings().getUserAny();
            if (any.isEmpty())
                continue;
            id = any.get<ModelId>();
            if (!mOgreModelMan->isValid(id))
                continue;
            models.push_back(id);
        }

        //retrieving agents
        Agent *t;
        ModelId modelId;
        // TODO: remove direct access
        for (std::map<AgentId, Agent *>::iterator it_agents = mAgentMan->mAgents.begin(); it_agents != mAgentMan->mAgents.end(); ++it_agents)
        {
            t = (*it_agents).second;
            modelId = t->ogreModelId();
            for (std::list<ModelId>::iterator it_models = models.begin(); it_models != models.end(); ++it_models)
                if (mOgreModelMan->isValid(modelId) && modelId == (*it_models))
                    selection.push_back(t->id());
        }
    }

    bool Level::isOver(void)
    {
        return false;
    }

    void Level::processCommand(std::vector<Ogre::String> command)
    {
        Ogre::String intro = "Level::processCommand(" + StringUtils::join(command, ".") + ")";
        if (command.size() == 0)
            return;
        if (command[0] == "load")
            load();
        else if (command[0] == "save")
            save();
        else if (command.size() > 1 && command[0] == "PhysicsTerrainManager" && command[1] == "switch_debug_draw")
            mTerrainMan.terrainPhysicsMan()->setDebugDraw(!mTerrainMan.terrainPhysicsMan()->getDebugDraw());
        else if (command[0] == "delete")
            Debug::error(intro)("to be implemented: level deletion").endl();
        else
            Debug::log("Level::processLevelCommand(): unknown command: ")(command).endl();
    }

    void Level::serialize(Ogre::String &s)
    {
        Debug::log(logName() + ".serialise()").endl().indent();
        Json::Value root;
        root["name"] = mName;

        root["backgroundColor"] = JsonUtils::toJson(mBackgroundColor);

        root["camera"] = mCamera->toJson();
        root["terrain"] = mTerrainMan.toJson();

        // serialise agents
        Debug::log("processing agents...").endl().indent();
        Json::Value agents;
        for (std::map<AgentId, Agent *>::iterator it_agents = mAgentMan->mAgents.begin(); it_agents != mAgentMan->mAgents.end(); ++it_agents)
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
            ModelManager *mm = modelManager(modelType);
            if (mm == NULL)
            {
                Debug::warning(logName() + ".serialize(): no modelManager of type ")(modelTypesAsString[modelType]).endl();
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
        Debug::log(logName() + ".deserialize():").endl().indent();
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
        if(value.isNull())
        {
            Debug::error("level name is null. Aborting.").endl();
            return false;
        }
        if(mName!=value.asString())
        {
            Debug::error("level name ").quotes(mName)(" does not match loaded data's ").quotes(value.asString())(" . Aborting.").endl();
            return false;
        }

        value = root["backgroundColor"];
        if (value.isNull())
            Debug::warning(logName() + ": key \"background\" is null. Using default.").endl();
        else
            mBackgroundColor = Ogre::StringConverter::parseColourValue(value.asString(), Ogre::ColourValue::White);
        if (Ogre::ColourValue::White == mBackgroundColor)
            Debug::warning(logName() + ": could no parse key \"background\". Using default.").endl();
        mBackgroundColor = Ogre::ColourValue::Black;

        //camera
        if (!mCamera->fromJson(root["camera"]))
        {
            Debug::error(logName())(": could not deserialize camera.").endl();
            return false;
        }

        if (!mTerrainMan.fromJson(root["terrain"]))
        {
            Debug::error(logName())(": could not deserialize terrain.").endl();
            return false;
        }

        Debug::log("instanciate ALL the models ! \\o/").endl();
        Json::Value dict = root["models"];
        if (dict.isNull())
        {
            Debug::warning("no models, really ?").endl();
        }
        else
        {

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
        }

        Debug::log("now instanciate ALL the agents ! \\o/").endl();
        dict = root["agents"];

        for (Json::ValueIterator it = dict.begin(); it != dict.end(); ++it)
        {
            AgentId aid=Ogre::StringConverter::parseUnsignedLong(it.key().asString());
            assert(aid!=INVALID_ID);
            if(!mAgentMan->isIdFree(aid))
            {
                Debug::error("AgentId ")(aid)(" could not be used: id is not free.").endl();
                return false;
            }
            Agent *agent = mAgentMan->newAgent(aid);
            assert(NULL!=agent);
            agent->fromJson(*it);
            if (agent->modelsIds().size() == 0)
            {
                Debug::warning("deleting agent ")(aid)(" with 0 models.").endl();
                mAgentMan->deleteAgent(aid);
            }
        }
        Debug::log("agents done").endl();
        Debug::log(logName() + ".deserialize(): done").unIndent().endl();
        return true;
    }

    void Level::update(float timestep)
    {
        mTerrainMan.update(timestep);
        // assume mTerrainMan's btWorld has been updated
        mPhysicsModelMan->update(timestep);
        SignalManager::instance().fireEmittedSignals();
        // actually needed ?
        //mOgreModelMan.update(timestep);
        mBTModelMan->update(timestep);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
