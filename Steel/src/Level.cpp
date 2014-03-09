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
#include "tools/OgreUtils.h"
#include "tools/StringUtils.h"
#include "tools/JsonUtils.h"
#include "terrain/TerrainPhysicsManager.h"
#include "SelectionManager.h"
#include "models/Agent.h"
#include "models/AgentManager.h"
#include "models/BlackBoardModelManager.h"
#include "models/BTModelManager.h"
#include "models/LocationModelManager.h"
#include "models/Model.h"
#include "models/OgreModelManager.h"
#include "models/PhysicsModelManager.h"
#include "SignalManager.h"

namespace Steel
{

    const char *Level::BACKGROUND_COLOR_ATTRIBUTE = "backgroundColor";
    const char *Level::NAME_ATTRIBUTE = "name";
    const char *Level::CAMERA_ATTRIBUTE = "camera";
    const char *Level::TERRAIN_ATTRIBUTE = "terrain";
    const char *Level::AGENTS_ATTRIBUTE = "agents";
    const char *Level::MANAGERS_ATTRIBUTE = "managers";
    const char *Level::GRAVITY_ATTRIBUTE = "gravity";

    const char *Level::DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE = "$cancelDynamicFilling";

    const char *Level::MODELS_ATTRIBUTE = "models";
    const char *Level::MODEL_TYPE_ATTRIBUTE = "modelType";
    const char *Level::MODEL_PATH_ATTRIBUTE = "path";
    const char *Level::MODEL_REF_OVERRIDE_ATTRIBUTE = "overrides";

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
        mCamera = new Camera(engine, this);

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

        if(mSceneManager != nullptr)
        {
            Ogre::RenderWindow *window = mEngine->renderWindow();

            if(nullptr != mCamera)
            {
                delete mCamera;
                mCamera = nullptr;
            }

            mSceneManager->clearScene();
            mSceneManager->destroyAllCameras();
            window->removeViewport(mViewport->getZOrder());
            mViewport = nullptr;

            Ogre::Root::getSingletonPtr()->destroySceneManager(mSceneManager);
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
            Debug::error(intro)("mType ")((long int) mType)(" aka ")
            .quotes(toString(mType))(" does not exist.").endl();
            return false;
        }

        if(mid == INVALID_ID)
        {
            Debug::error(intro)("invalid model id.").endl();
            return false;
        }

        if(!agent->linkToModel(mType, mid))
        {
            Debug::error(intro)("linkage agent ")(aid)("<->model<")(toString(mType))(">");
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

    void Level::loadConfig(ConfigFile const &config)
    {

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

            for(ModelType mt = ModelType::FIRST; mt < ModelType::LAST; mt = (ModelType)(((unsigned long)mt) + 1L))
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

        mTerrainMan.terrainPhysicsMan()->setWorldGravity(mGravity);

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
            Debug::error("in Level::registerManager(): a manager for type ").quotes(toString(type))
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
        if(TerrainManager::LoadingState::READY == state)
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

        // list of models to save
        std::map<ModelType, std::list<ModelId>> persistentModels;

        for(std::map<AgentId, Agent *>::iterator it_agents = mAgentMan->mAgents.begin(); it_agents != mAgentMan->mAgents.end(); ++it_agents)
        {
            AgentId aid = (*it_agents).first;
            Agent *agent = (*it_agents).second;

            if(!agent->isPersistent())
                continue;

            agents[Ogre::StringConverter::toString(aid)] = agent->toJson();

            for(auto const & it : agent->modelsIds())
                persistentModels.emplace(it.first, std::list<ModelId>()).first->second.push_back(it.second);

        }

        root[Level::AGENTS_ATTRIBUTE] = agents;
        Debug::log("all agents done.").unIndent().endl();

        // serialise models
        Debug::log("processing models...").endl();
        Json::Value models;

        for(ModelType modelType = (ModelType)((int) ModelType::FIRST + 1);
                modelType != ModelType::LAST;
                modelType = (ModelType)((int) modelType + 1))
        {
            ModelManager *mm = modelManager(modelType);

            if(mm == nullptr)
            {
                Debug::warning(logName() + ".serialize(): no modelManager of type ")(toString(modelType)).endl();
                continue;
            }

            std::list<ModelId> modelsIds = persistentModels.emplace(modelType, std::list<ModelId>()).first->second;
            mm->toJson(models[toString(modelType)], modelsIds);
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
            for(ModelType i = (ModelType)((unsigned long) ModelType::FIRST + 1); i != ModelType::LAST; i = (ModelType)((unsigned long) i + 1))
            {
                Ogre::String type = toString(i);
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

            Agent *agent = mAgentMan->newAgent(aid);

            if(nullptr == agent)
            {
                Debug::error("could not create agent ")(aid)(".").endl().breakHere();
                return false;
            }

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

    bool Level::instanciateResource(File const &file)
    {
        AgentId aid = INVALID_ID;
        return instanciateResource(file, aid);
    }

    bool Level::instanciateResource(File const &_file, AgentId &aid)
    {
        File file;

        if(_file.isPathAbsolute())
            file = _file;
        else
            file = mEngine->dataDir().subfile(_file.fullPath());

        // file is now absolute

        static Ogre::String intro = "Level::instanciateResource(" + file.fullPath() + "): ";

        if(!file.exists())
        {
            Debug::error(intro)("file not found: ")(file).endl();
            return false;
        }

        Ogre::String content = file.read();

        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(content, root, false);

        if(!parsingOk)
        {
            Debug::error(intro)("could not parse this:").endl();
            Debug::error(content).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return false;
        }

        if(!dynamicFillSerialization(root))
        {
            Debug::error(intro)("could not find all values. Aborted.").endl();
            return false;
        }

        if(INVALID_ID == aid)
        {
            // will end up pointing to the agent owning all created models
            if(!root.isArray() && root.isMember(Agent::ID_ATTRIBUTE))
                aid = JsonUtils::asUnsignedLong(root[Agent::ID_ATTRIBUTE], INVALID_ID);
            else
                aid = agentMan()->newAgent();
        }

        assert(INVALID_ID != aid);

        Ogre::String instancitationType = file.extension();

        if(instancitationType == "model")
            return loadModelFromSerialization(root, aid);
        else if(instancitationType == "models")
            return loadModelsFromSerializations(root, aid);
        else if(instancitationType == "model_refs")
            return loadModelsReferencesFromSerializations(root, aid);
        else if(instancitationType == "terrain_slot")
            return loadTerrainSlotFromSerialization(root);
        else
        {
            Debug::warning(intro)("instanciation type unknown: ")(instancitationType).endl();
            return false;
        }

        return true;
    }

    bool Level::loadModelFromSerialization(Json::Value &root, Steel::AgentId &aid)
    {
        Debug::log("Level::loadModelFromSerialization(")(root)(")").endl();
        Ogre::String intro = "Level::loadModelFromSerialization(): ";

        if(!root.isObject())
        {
            Debug::error("expecting a json object. Aborting.").endl();
            return false;
        }

        Json::Value value = root[Level::MODEL_TYPE_ATTRIBUTE];

        if(value.isNull())
        {
            Debug::error("serialization is missing a ").quotes(Level::MODEL_TYPE_ATTRIBUTE)(" value. Aborted.").endl();
            return false;
        }

        Ogre::String modelTypeString = value.asString();

        // this is used to know it the agent should be deleted upon failure of the method
        bool fresh_aid = INVALID_ID == aid;

        if(fresh_aid)
        {
            aid = agentMan()->newAgent();

            if(INVALID_ID == aid)
            {
                Debug::error(intro)("could not create an agent to link the model to. Aborted.").endl();
                return false;
            }

            Debug::log(intro)("created agent ")(aid).endl();
        }

        // ask the right manager to load this model
        ModelType modelType = toModelType(modelTypeString);

        if(ModelType::LAST == modelType)
        {
            Debug::warning(intro)("Unknown model type: ")(modelTypeString).endl();
            return false;
        }

        // check if the agent is already linked to such a model
        Agent *agent = agentMan()->getAgent(aid);

        if(nullptr == agent)
        {
            Debug::log(intro)("could not retrieve agent for id ")(aid).endl();
            return false;
        }

        if(INVALID_ID != agent->modelId(modelType))
        {
            Debug::error(intro)("cannot create a second ")(modelTypeString)(" Model to agent ")
            (aid)(". Skipping ")(modelTypeString)(" Model instanciation.").endl();
            return true; // skipped, not aborted
        }

        // try to instanciate the model
        intro.append("in ").append(modelTypeString).append(" type: ");
        auto manager = modelManager(modelType);

        if(nullptr == manager)
        {
            Debug::error(intro)("could not find proper manager for modelType ")
            (modelTypeString)(". ")("Aborting.").endl();
            return false;
        }

        ModelId mid = INVALID_ID;

        if(!manager->fromSingleJson(root, mid) || INVALID_ID == mid)
        {
            Debug::error(intro)("could not build model from serialization:")
            (root)("Aborting.").endl();
            return false;
        }

        if(!linkAgentToModel(aid, modelType, mid))
        {
            manager->decRef(mid);
            Debug::error(intro)("could not link agent ")(aid)(" to  ")(modelTypeString)(" Model ")
            (mid)(". Model released. Aborted.").endl();

            if(fresh_aid)
                agentMan()->deleteAgent(aid);

            return false;
        }
        else
            Debug::log("new ")(modelTypeString)(" Model with id ")(mid)(" linked to agent ")(aid).endl();

        //TODO add visual notification in the UI
        return true;
    }

    AgentId Level::agentIdUnderMouse()
    {
        Selection selection;
        mEngine->pickAgents(selection, mEngine->inputMan()->mousePos().x, mEngine->inputMan()->mousePos().y);

        AgentId aid = INVALID_ID;

        if(selection.size() > 0)
            aid = selection.front();

        return aid;
    }

    ModelId Level::modelIdUnderMouse(ModelType mType)
    {
        static const Ogre::String intro = "in Level::ModelIdUnderMouse(mType=" + toString(mType) + "): ";

        ModelId mid = INVALID_ID;
        AgentId aid = agentIdUnderMouse();

        if(INVALID_ID == aid)
            return INVALID_ID;

        Agent *agent = agentMan()->getAgent(aid);

        if(nullptr == agent)
            Debug::error(intro)("can't find agent ")(aid).endl();
        else
            mid = agent->modelId(mType);

        return mid;
    }

    Ogre::Vector3 Level::getDropTargetPosition()
    {
        return camera()->dropTargetPosition();
    }

    Ogre::Quaternion Level::getDropTargetRotation()
    {
        return camera()->dropTargetRotation();
    }

    Ogre::Vector2 Level::getSlotDropPosition()
    {
        auto cam = camera();

        auto mousPos = mEngine->inputMan()->mousePos();
        auto camRay = cam->cam()->getCameraToViewportRay(mousPos.x / static_cast<float>(mEngine->renderWindow()->getWidth()),
                      mousPos.y / static_cast<float>(mEngine->renderWindow()->getHeight()));

        // get terrain under the cam
        Ogre::Terrain *terrain = nullptr;
        auto camPos = cam->camNode()->convertLocalToWorldPosition(Ogre::Vector3::ZERO);
        float height = mTerrainMan.terrainGroup()->getHeightAtWorldPosition(camPos, &terrain);

        Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0.f);

        if(nullptr != terrain)
        {
            // cam is above a terrain, we use its height as base for the plane
            plane.d += height;
        }

        // find where the drop point is
        auto result = camRay.intersects(plane);

        if(!result.first)
        {
            Debug::warning("Level::slotDropPosition(): can't drop above horizontal camera plan !").endl();
            return Ogre::Vector2(FLT_MAX, FLT_MAX);
        }

        Ogre::Vector3 slotWorldPos = camRay.getPoint(result.second);

        long int x = 0, y = 0;
        mTerrainMan.terrainGroup()->convertWorldPositionToTerrainSlot(slotWorldPos, &x, &y);
        return Ogre::Vector2(static_cast<float>(x), static_cast<float>(y));
    }

    bool Level::dynamicFillSerialization(Json::Value &node, AgentId aid)
    {
        static const Ogre::String intro = "in Level::dynamicFillSerialization(): ";

        if(node.isArray())
        {
            // array: provcess each value
            for(unsigned i = 0; i < node.size(); ++i)
            {
                if(!dynamicFillSerialization(node[i]))
                    return false;
            }
        }
        else if(node.isObject())
        {
            if(node.isMember(Level::DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE) && JsonUtils::asBool(node[Level::DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE], false))
                return true;

            // dict:: process each value
            for(auto it = node.begin(); it != node.end(); ++it)
            {
                if(!dynamicFillSerialization(*it, aid))
                    return false;
            }
        }
        else if(node.isString())
        {
            Ogre::String svalue, new_svalue;
            svalue = new_svalue = node.asString();

            if(svalue.at(0) != '$')
                new_svalue = svalue;
            else
            {
                auto agent = agentMan()->getAgent(aid);

                if(svalue == "$agentOgreModel")
                {
                    if(nullptr == agent)
                        new_svalue = Ogre::StringConverter::toString(INVALID_ID);
                    else
                        new_svalue = Ogre::StringConverter::toString(agent->ogreModelId());
                }
                else if(svalue == "$agentPhysicsModel")
                {
                    if(nullptr == agent)
                        new_svalue = Ogre::StringConverter::toString(INVALID_ID);
                    else
                        new_svalue = Ogre::StringConverter::toString(agent->physicsModelId());
                }
                else if(svalue == "$agentUnderMouse")
                {
                    if(nullptr == agent)
                    {
                        AgentId aid_um = agentIdUnderMouse();

                        if(INVALID_ID == aid_um)
                        {
                            Debug::error(intro)("no agent under mouse.").endl();
                            return false;
                        }

                        new_svalue = Ogre::StringConverter::toString(aid_um);
                    }
                    else
                        new_svalue = Ogre::StringConverter::toString((unsigned long) aid);
                }
                else if(svalue == "$dropTargetPosition")
                    new_svalue = Ogre::StringConverter::toString(getDropTargetPosition());
                else if(svalue == "$dropTargetRotation")
                    new_svalue = Ogre::StringConverter::toString(getDropTargetRotation());
                else if(svalue == "$ogreModelUnderMouse")
                {
                    if(nullptr == agent)
                    {
                        ModelId mid_um = modelIdUnderMouse(ModelType::OGRE);

                        if(INVALID_ID == mid_um)
                        {
                            Debug::error(intro)("no OgreModel under mouse.").endl();
                            return false;
                        }

                        new_svalue = Ogre::StringConverter::toString(mid_um);
                    }
                    else
                        new_svalue = Ogre::StringConverter::toString(agent->ogreModelId());
                }
                else if(svalue == "$slotDropPosition")
                {
                    auto slotPosition = getSlotDropPosition();

                    // drops farther than 5km away are forbidden
                    if(slotPosition.length() > 5000)
                    {
                        Debug::error(intro)("slot drop position is invalid (>5k units away):")(slotPosition)(". Aborted.").endl();
                        return false;
                    }

                    new_svalue = Ogre::StringConverter::toString(slotPosition);
                }
            }

            node = new_svalue.c_str();
        }

        return true;
    }

    bool Level::loadModelsFromSerializations(Json::Value &root, Steel::AgentId &aid)
    {
        Ogre::String intro = "Level::loadModelsFromSerializations(): ";

        if(!root.isObject())
        {
            Debug::error(intro)("Expecting object content. Aborting.").endl();
            return false;
        }

        if(!root.isMember(Level::MODELS_ATTRIBUTE))
        {
            Debug::error(intro)("member ").quotes(Level::MODELS_ATTRIBUTE)(" not found. Aborting.").endl();
            return false;
        }

        Json::Value node = root[Level::MODELS_ATTRIBUTE];

        if(!node.isArray() || node.isNull())
        {
            Debug::warning(intro).quotes(Level::MODELS_ATTRIBUTE)(" attribute is not valid (expecting a non-empty array).").endl();
            return false;
        }

        // instanciate all models
        for(Json::ValueIterator it = node.begin(); it != node.end(); ++it)
        {
            Json::Value modelNode = *it;

            if(INVALID_ID == aid)
            {
                Json::Value value = modelNode[Level::MODEL_TYPE_ATTRIBUTE];

                if(value.isNull() || value.asCString() != toString(ModelType::OGRE))
                {
                    Debug::error("serialization is not starting with an OgreModel as modelType. Aborted.").endl();
                    return false;
                }
            }

            if(!loadModelFromSerialization(modelNode, aid))
            {
                Debug::error(intro)("could not load models. Aborting.").endl();
                return false;
            }
        }

        return true;
    }

    bool Level::loadModelsReferencesFromSerializations(Json::Value &root, Steel::AgentId &aid)
    {
        Ogre::String intro = "Level::loadModelsReferencesFromSerializations(): ";

        if(!root.isMember(Level::MODELS_ATTRIBUTE))
        {
            Debug::error(intro)("member ").quotes(Level::MODELS_ATTRIBUTE)(" not found. Aborting.").endl();
            return false;
        }

        Json::Value models = root[Level::MODELS_ATTRIBUTE];

        if(models.isNull())
        {
            Debug::warning(intro).quotes(Level::MODELS_ATTRIBUTE)(" attribute is null. Aborting.");
            return false;
        }

        if(!models.isArray())
        {
            Debug::warning(intro).quotes(Level::MODELS_ATTRIBUTE)(" attribute is not an array. Aborting.");
            return false;
        }

        bool revertAgent = false;

        for(Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            Json::Value refNode = *it, modelNode;

            // if path is there, it's a reference
            bool hasPathMember = refNode.isMember(Level::MODEL_PATH_ATTRIBUTE);

            if(hasPathMember)
            {
                Json::Value pathValue = refNode[Level::MODEL_PATH_ATTRIBUTE];

                if(!pathValue.isString())
                {
                    Debug::warning(intro)("Could not read reference path ").quotes(pathValue)(" as string. Aborting").endl();
                    revertAgent = true;
                    break;
                }

                // load the referee
                Ogre::String path = Ogre::StringUtil::BLANK;
                mEngine->resolveReferencePaths(pathValue.asString(), path);
                File file = mEngine->dataDir().subfile(path);

                if(!file.readInto(modelNode, false))
                {
                    Debug::warning(intro)("Could not read reference path \"")(path)("\". Aborting.");
                    revertAgent = true;
                    break;
                }

                // possibly add attributes overrides
                if(refNode.isMember(Level::MODEL_REF_OVERRIDE_ATTRIBUTE))
                {
                    Json::Value overrides = refNode[Level::MODEL_REF_OVERRIDE_ATTRIBUTE];

                    if(!overrides.isObject())
                    {
                        Debug::error(intro)("invalid overrides:")(overrides)(". Was expecting an object. Aborting").endl();
                        revertAgent = true;
                        break;
                    }

                    // actual overrides
                    JsonUtils::updateObjectWithOverrides(overrides, modelNode);
                }

            }

            if(INVALID_ID == aid)
            {
                if(!modelNode.isObject())
                {
                    Debug::error(intro)("serialization is not a json object. Aborting.").endl();
                    revertAgent = true;
                    break;
                }

                Json::Value value = modelNode[Level::MODEL_TYPE_ATTRIBUTE];

                if(value.isNull() || !value.isString() || value.asString() != toString(ModelType::OGRE))
                {
                    Debug::error(intro)("serialization is not starting with a model of type ModelType::OGRE. Aborted.").endl();
                    revertAgent = true;
                    break;
                }
            }

            // should be done here too, as we may have newly loaded data
            if(!dynamicFillSerialization(modelNode, aid))
            {
                revertAgent = true;
                break;
            }

            if(!loadModelFromSerialization(modelNode, aid))
            {
                Debug::error(intro)("could not load models. Aborting.").endl();
                revertAgent = true;
                break;
            }
        }

        if(revertAgent)
        {
            if(INVALID_ID != aid)
            {
                agentMan()->deleteAgent(aid);
                aid = INVALID_ID;
            }

            return false;
        }

        return true;
    }

    bool Level::loadTerrainSlotFromSerialization(Json::Value &root)
    {
        Ogre::String intro = "Level::loadTerrainSlotFromSerialization(): ";

        if(root["slotPosition"].isNull())
        {
            Debug::error(intro)("can't find key \"slotPosition\". Aborted.").endl();
            return false;
        }

        TerrainManager::TerrainSlotData slot;
        mTerrainMan.terrainSlotFromJson(root, slot);

        if(!slot.isValid())
        {
            Debug::error(intro)("TerrainManager::TerrainSlotData is not valid. Serialized string was:");
            Debug::error(root.toStyledString()).endl()("Aborted.").endl();
            return false;
        }

        mTerrainMan.addTerrainSlot(slot);
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
