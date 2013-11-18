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
#include "LocationModelManager.h"
#include "BlackBoardModelManager.h"
#include "BTModelManager.h"

namespace Steel
{
    const char *Level::BACKGROUND_COLOR_ATTRIBUTE = "backgroundColor";
    const char *Level::NAME_ATTRIBUTE = "name";
    const char *Level::CAMERA_ATTRIBUTE = "camera";
    const char *Level::TERRAIN_ATTRIBUTE = "terrain";
    const char *Level::AGENTS_ATTRIBUTE = "agents";
    const char *Level::MANAGERS_ATTRIBUTE = "managers";
    const char *Level::GRAVITY_ATTRIBUTE = "gravity";

    Level::Level(Engine *engine, File path, Ogre::String name) : TerrainManagerEventListener(),
        mEngine(engine), mViewport(nullptr), mPath(path.subfile(name)), mName(name),
        mBackgroundColor(Ogre::ColourValue::Black), mSceneManager(nullptr), mLevelRoot(nullptr),
        mManagers(std::map<ModelType, ModelManager *>()), mAgentMan(nullptr), mOgreModelMan(nullptr),
        mPhysicsModelMan(nullptr), mBTModelMan(nullptr), mTerrainMan(), mSelectionMan(nullptr), mLocationModelMan(nullptr),
        mBlackBoardModelManagerMan(nullptr),
        mCamera(nullptr), mMainLight(nullptr),
        mGravity(Ogre::Vector3::ZERO)
    {
        Debug::log(logName() + "()").endl();

        if(!mPath.exists())
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

        while(window->hasViewportWithZOrder(++zOrder));

        mViewport = window->addViewport(mCamera->cam(), zOrder);
        mViewport->setBackgroundColour(mBackgroundColor);

        // Alter the camera aspect ratio to match the viewport
        Ogre::Real aspectRatio = Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight());
        mCamera->cam()->setAspectRatio(aspectRatio);

        mLevelRoot = mSceneManager->getRootSceneNode()->createChildSceneNode("levelNode", Ogre::Vector3::ZERO);
        mTerrainMan.init(this, mName + ".terrainManager", path.subfile(mName), mSceneManager);

        mSelectionMan = new SelectionManager(this);
        mAgentMan = new AgentManager(this);
        mOgreModelMan = new OgreModelManager(this, mSceneManager, mLevelRoot);
        mPhysicsModelMan = new PhysicsModelManager(this, mTerrainMan.terrainPhysicsMan()->world());
        mBTModelMan = new BTModelManager(this, mEngine->rawResourcesDir().subfile("BT"));
        mLocationModelMan = new LocationModelManager(this);
        mBlackBoardModelManagerMan = new BlackBoardModelManager(this);

        mSceneManager->setAmbientLight(Ogre::ColourValue::White);
    }

    Level::~Level()
    {
        Debug::log(logName() + ".~Level()").endl();

        mBlackBoardModelManagerMan->clear();
        delete mBlackBoardModelManagerMan;
        mBlackBoardModelManagerMan = nullptr;

        mBTModelMan->clear();
        delete mBTModelMan;
        mBTModelMan = nullptr;

        mPhysicsModelMan->clear();
        delete mPhysicsModelMan;
        mPhysicsModelMan = nullptr;
        mTerrainMan.shutdown();

        mLocationModelMan->clear();
        delete mLocationModelMan;
        mLocationModelMan = nullptr;

        mOgreModelMan->clear();
        delete mOgreModelMan;
        mOgreModelMan = nullptr;

        Ogre::RenderWindow *window = mEngine->renderWindow();

        if(mSceneManager != nullptr)
        {
            delete mCamera;
            mSceneManager->clearScene();
            mSceneManager->destroyAllCameras();
            window->removeViewport(mViewport->getZOrder());
            Ogre::Root::getSingletonPtr()->destroySceneManager(mSceneManager);
            mViewport = nullptr;
            mCamera = nullptr;
            mSceneManager = nullptr;
        }

        Ogre::ResourceGroupManager::getSingletonPtr()->destroyResourceGroup(mName);
        mViewport = nullptr;
    }

    bool Level::linkAgentToModel(AgentId aid, ModelType mType, ModelId mid)
    {
        Ogre::String intro = "Level::linkAgentToModel(): ";

        if(aid == INVALID_ID)
        {
            Debug::error(intro)("aid ")(aid)(" does not exist.").endl();
            return false;
        }

        Agent *agent = mAgentMan->getAgent(aid);

        ModelManager *mm = modelManager(mType);

        if(mm == nullptr)
        {
            Debug::error(intro)("mType ")((long int) mType)(" aka \"");
            Debug::error(modelTypesAsString[mType])("\" does not exist.").endl();
            return false;
        }

        if(mid == INVALID_ID)
        {
            Debug::error(intro)("invalid model id.").endl();
            return false;
        }

        if(!agent->linkToModel(mType, mid))
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

        if(!savefile.exists())
        {
            Debug::warning(logName() + ".load(): file does not exists: ");
            Debug::warning(savefile)(" -> Loading cancelled.").endl();
            return false;
        }

        Ogre::String s = savefile.read();
        mTerrainMan.addTerrainManagerEventListener(this);

        if(!deserialize(s))
        {
            mAgentMan->deleteAllAgents();

            for(ModelType mt = MT_FIRST; mt < MT_LAST; mt = (ModelType)(((unsigned long)mt) + 1L))
            {
                ModelManager *mm = modelManager(mt);

                if(nullptr == mm)
                    continue;

                mm->clear();
            }

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

        Ogre::String s = Ogre::StringUtil::BLANK;
        serialize(s);

        File savefile = getSavefile();
        savefile.write(s, File::OM_OVERWRITE);

        Debug::log(logName() + ".save() into ")(savefile).endl();
        return true;
    }

    void Level::registerManager(ModelType type, ModelManager *_manager)
    {
        if(nullptr != modelManager(type))
        {
            Debug::error("in Level::registerManager(): a manager for type ").quotes(modelTypesAsString[type])
            (" already exists ! Aborting.").endl();
            return;
        }

        mManagers.insert( {type, _manager});
    }

    ModelManager *Level::modelManager(ModelType modelType)
    {
        ModelManager *mm = nullptr;
        auto it = mManagers.find(modelType);

        if(mManagers.end() != it)
        {
            mm = it->second;
        }

        return mm;
    }

    void Level::onTerrainEvent(TerrainManager::LoadingState state)
    {
        if(state == mTerrainMan.READY)
            mTerrainMan.removeTerrainManagerEventListener(this);
    }

    void Level::getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, Selection &selection)
    {
        static const Ogre::String intro = "in Level::getAgentsIdsFromSceneNodes(): ";

        for(std::list<Ogre::SceneNode *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            auto any = (*it)->getUserObjectBindings().getUserAny();

            if(any.isEmpty())
                continue;

            AgentId aid = any.get<AgentId>();

            if(mAgentMan->isIdFree(aid))
            {
                Debug::warning(intro)("found sceneNode with invalid agentId ")(aid)(". Deleting it.").endl();
                OgreUtils::destroySceneNode(*it);
            }
            else
            {
                selection.push_back(aid);
            }
        }
    }

//     void Level::getAgentFromModelIds(std::list<>)
//     {
//         //retrieving agents
//         Agent *t;
//         ModelId modelId;
//         // TODO: remove direct access
//         for (std::map<AgentId, Agent *>::iterator it_agents = mAgentMan->mAgents.begin(); it_agents != mAgentMan->mAgents.end(); ++it_agents)
//         {
//             t = (*it_agents).second;
//             modelId = t->ogreModelId();
//             for (std::list<ModelId>::iterator it_models = models.begin(); it_models != models.end(); ++it_models)
//                 if (mOgreModelMan->isValid(modelId) && modelId == (*it_models))
//                     selection.push_back(t->id());
//         }
//     }

    bool Level::isOver(void)
    {
        return false;
    }

    void Level::processCommand(std::vector<Ogre::String> command)
    {
        Ogre::String intro = "Level::processCommand(" + StringUtils::join(command, ".") + ")";

        if(command.size() == 0)
            return;

        if(command[0] == "load")
            load();
        else if(command[0] == "save")
            save();
        else if(command.size() > 1 && command[0] == "PhysicsTerrainManager" && command[1] == "switch_debug_draw")
            mTerrainMan.terrainPhysicsMan()->setDebugDraw(!mTerrainMan.terrainPhysicsMan()->getDebugDraw());
        else if(command[0] == "delete")
            Debug::error(intro)("to be implemented: level deletion").endl();
        else
            Debug::log("Level::processLevelCommand(): unknown command: ")(command).endl();
    }

    void Level::serialize(Ogre::String &s)
    {
        Debug::log(logName() + ".serialise()").endl().indent();
        Json::Value root;
        root[Level::NAME_ATTRIBUTE] = mName;

        root[Level::BACKGROUND_COLOR_ATTRIBUTE] = JsonUtils::toJson(mBackgroundColor);

        root[Level::CAMERA_ATTRIBUTE] = mCamera->toJson();
        root[Level::TERRAIN_ATTRIBUTE] = mTerrainMan.toJson();

        // serialise agents
        Debug::log("processing agents...").endl().indent();
        Json::Value agents;

        for(std::map<AgentId, Agent *>::iterator it_agents = mAgentMan->mAgents.begin(); it_agents != mAgentMan->mAgents.end(); ++it_agents)
        {
            AgentId aid = (*it_agents).first;
            Agent *agent = (*it_agents).second;
            agents[Ogre::StringConverter::toString(aid)] = agent->toJson();
        }

        root[Level::AGENTS_ATTRIBUTE] = agents;
        Debug::log("all agents done.").unIndent().endl();

        // serialise models
        Debug::log("processing models...").endl();
        Json::Value models;

        for(ModelType modelType = (ModelType)((int) MT_FIRST + 1);
                modelType != MT_LAST;
                modelType = (ModelType)((int) modelType + 1))
        {
            ModelManager *mm = modelManager(modelType);

            if(mm == nullptr)
            {
                Debug::warning(logName() + ".serialize(): no modelManager of type ")(modelTypesAsString[modelType]).endl();
                continue;
            }

            mm->toJson(models[modelTypesAsString[modelType]]);
        }

        root[Level::MANAGERS_ATTRIBUTE] = models;
        root[Level::GRAVITY_ATTRIBUTE] = JsonUtils::toJson(mGravity);
        
        Debug::log("all models done.").unIndent().endl();

        Debug::log("serialization done").unIndent().endl();
        s = root.toStyledString();
//         Debug::log(s).endl();
    }

    bool Level::deserialize(Ogre::String &s)
    {
        Debug::log(logName() + ".deserialize():").endl().indent();
//         Debug::log(s).endl();
        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(s, root, false);

        if(!parsingOk)
        {
            Debug::error("could not parse this:").endl();
            Debug::error(s).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return false;
        }

        Json::Value value;

        // get level info
        value = root[Level::NAME_ATTRIBUTE];

        if(value.isNull())
        {
            Debug::error("level name is null. Aborting.").endl();
            return false;
        }

        if(mName != value.asString())
        {
            Debug::error("level name ").quotes(mName)(" does not match loaded data's ").quotes(value.asString())(" . Aborting.").endl();
            return false;
        }
        
        mGravity = JsonUtils::asVector3(root[Level::GRAVITY_ATTRIBUTE]);

        value = root[Level::BACKGROUND_COLOR_ATTRIBUTE];

        if(value.isNull())
            Debug::warning(logName() + ": key \"background\" is null. Using default.").endl();
        else
            mBackgroundColor = Ogre::StringConverter::parseColourValue(value.asString(), Ogre::ColourValue::White);

        if(Ogre::ColourValue::White == mBackgroundColor)
            Debug::warning(logName() + ": could no parse key \"background\". Using default.").endl();

        mBackgroundColor = Ogre::ColourValue::Black;

        //camera
        if(!mCamera->fromJson(root[Level::CAMERA_ATTRIBUTE]))
        {
            Debug::error(logName())(": could not deserialize camera.").endl();
            return false;
        }

        if(!mTerrainMan.fromJson(root[Level::TERRAIN_ATTRIBUTE]))
        {
            Debug::error(logName())(": could not deserialize terrain.").endl();
            return false;
        }

        Debug::log("instanciate ALL the models ! \\o/").endl();
        Json::Value dict = root[Level::MANAGERS_ATTRIBUTE];

        if(dict.isNull())
        {
            Debug::warning("no models, really ?").endl();
        }
        else
        {
            for(ModelType i = (ModelType)((unsigned long) MT_FIRST + 1); i != MT_LAST; i = (ModelType)((unsigned long) i + 1))
            {
                Ogre::String type = modelTypesAsString[i];
                Json::Value models = dict[type];

                if(models.isNull())
                {
                    Debug::log("no models for type ")(type).endl();
                    continue;
                }

                ModelManager *mm = modelManager(i);

                if(mm == nullptr)
                {
                    Debug::warning("no modelManager for type ")(type).endl();
                    continue;
                }

                mm->fromJson(models);
            }

            Debug::log("models done").endl();
        }

        Debug::log("now instanciate ALL the agents ! \\o/").endl();
        dict = root[Level::AGENTS_ATTRIBUTE];

        for(Json::ValueIterator it = dict.begin(); it != dict.end(); ++it)
        {
            AgentId aid = Ogre::StringConverter::parseUnsignedLong(it.key().asString(), INVALID_ID);
            assert(aid != INVALID_ID);

            if(!mAgentMan->isIdFree(aid))
            {
                Debug::error("AgentId ")(aid)(" could not be used: id is not free.").endl();
                return false;
            }

            Agent *agent = mAgentMan->newAgent(aid);
            assert(nullptr != agent);
            agent->fromJson(*it);

            if(agent->modelsIds().size() == 0)
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
        // actually needed ?
        //mOgreModelMan.update(timestep);
        //mLocationModelMan.update(timestep);
        SignalManager::instance().fireEmittedSignals();
        mBTModelMan->update(timestep);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
