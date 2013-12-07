
#include <json/json.h>

#include <Rocket/Controls/ElementFormControlInput.h>
#include <Rocket/Controls/ElementTabSet.h>
#include <Rocket/Controls/ElementFormControlSelect.h>
#include <Rocket/Controls/ElementFormControlTextArea.h>
#include <Rocket/Core/Event.h>
#include <Rocket/Debugger.h>
#include <Rocket/Core/Element.h>

#include "UI/Editor.h"
#include "Debug.h"
#include "tools/StringUtils.h"
#include "tools/OgreUtils.h"
#include "Level.h"
#include "Engine.h"
#include "UI/FileSystemDataSource.h"
#include "Camera.h"
#include "Agent.h"
#include "OgreModelManager.h"
#include "PhysicsModelManager.h"
#include "tools/JsonUtils.h"
#include <AgentManager.h>
#include <SelectionManager.h>
#include <LocationModel.h>

namespace Steel
{

    const Ogre::String Editor::REFERENCE_PATH_LOOKUP_TABLE_SETTING = "Editor::referencePathsLookupTable";
    const Ogre::String Editor::MENU_TAB_INDEX_SETTING = "Editor::menuTabIndex";

    const char *Editor::DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE = "$cancelDynamicFilling";

    const char *Editor::SELECTION_TAGS_INFO_BOX = "selectionTagsInfoBox";
    const char *Editor::SELECTIONS_TAG_EDIT_BOX = "selection_tags_editbox";
    const char *Editor::AGENT_TAG_ITEM_NAME = "agenttagitem";

    const char *Editor::SELECTION_PATH_INFO_BOX = "selectionPathsInfoBox";
    const char *Editor::SELECTIONS_PATH_EDIT_BOX = "selection_path_editbox";

    const char *Editor::MODELS_ATTRIBUTE = "models";
    const char *Editor::MODEL_TYPE_ATTRIBUTE = "modelType";
    const char *Editor::MODEL_PATH_ATTRIBUTE = "path";
    const char *Editor::MODEL_REF_OVERRIDE_ATTRIBUTE = "overrides";

    Editor::Editor(): UIPanel("Editor", "data/ui/current/editor/editor.rml"),
        mEngine(nullptr), mUI(nullptr), mInputMan(nullptr), mFSResources(nullptr),
        mDataDir(), mBrush(), mDebugEvents(false), mIsDraggingFromMenu(false),
        mReferencePathsLookupTable()
    {
#ifdef DEBUG
        mAutoReload = true;
#endif
    }

    Editor::Editor(const Editor &other)
    {
        throw std::runtime_error("Editor::Editor(const Editor& other): Not Implemented");
    }

    Editor::~Editor()
    {
        shutdown();

        if(nullptr != mFSResources)
        {
            delete mFSResources;
            mFSResources = nullptr;
        }
    }

    Editor &Editor::operator=(const Editor &other)
    {
        throw std::runtime_error("Editor::operator=(const Editor& other): Not Implemented");
        return *this;
    }

    void Editor::loadConfig(ConfigFile const &config)
    {
        mBrush.loadConfig(config);
        setupReferencePathsLookupTable(config.getSetting(Editor::REFERENCE_PATH_LOOKUP_TABLE_SETTING));
    }

    void Editor::saveConfig(ConfigFile &config) const
    {
        mBrush.saveConfig(config);
        saveMenuTabIndexSetting(config);
    }

    void Editor::saveMenuTabIndexSetting(ConfigFile &config) const
    {
        // save state
        if(nullptr != mDocument)
        {
            auto elem = (Rocket::Controls::ElementTabSet *) mDocument->GetElementById("editor_tabset");

            if(nullptr != elem)
            {
                int tabNo = elem->GetActiveTab();
                config.setSetting(Editor::MENU_TAB_INDEX_SETTING, Ogre::StringConverter::toString(tabNo));
            }
        }
    }

    void Editor::shutdown()
    {
        mBrush.shutdown();

        if(nullptr != mEngine->level())
        {
            mEngine->level()->selectionMan()->removeListener(this);
        }

        mEngine->removeEngineEventListener(this);
        UIPanel::shutdown();
    }

    void Editor::init(unsigned int width, unsigned int height)
    {
        init(width, height, nullptr, nullptr);
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI *ui)
    {
        Debug::log("Editor::init()").endl();

        if(nullptr != engine)
        {
            mEngine = engine;
            mInputMan = engine->inputMan();
        }

        if(nullptr != ui)
            mUI = ui;

        mDataDir = mUI->dataDir().subfile("editor").fullPath();
        auto resGroupMan = Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI", true);
        //resGroupMan->addResourceLocation(mDataDir.subfile("images").fullPath(), "FileSystem", "UI",true);
        //resGroupMan->declareResource("inode-directory.png","Texture","UI");

        setupReferencePathsLookupTable(mEngine->config().getSetting(Editor::REFERENCE_PATH_LOOKUP_TABLE_SETTING));
        mFSResources = new FileSystemDataSource("resources", mEngine->resourcesDir());
        UIPanel::init(width, height);
        mFSResources->localizeDatagridBody(mDocument);
        auto elem = (Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");

        if(elem != nullptr)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
            // elem->AddEventListener("submit",this);
        }

        mBrush.init(mEngine, this, mInputMan);

        mEngine->addEngineEventListener(this);

        if(nullptr != mEngine->level())
            mEngine->level()->selectionMan()->addListener(this);
    }

    void Editor::onLevelSet(Level *level)
    {
        level->selectionMan()->addListener(this);
    }

    void Editor::onLevelUnset(Level *level)
    {
        level->selectionMan()->removeListener(this);
    }

    AgentId Editor::agentIdUnderMouse()
    {
        Selection selection;
        mEngine->pickAgents(selection, mInputMan->mousePos().x, mInputMan->mousePos().y);

        AgentId aid = INVALID_ID;

        if(selection.size() > 0)
            aid = selection.front();

        return aid;
    }

    ModelId Editor::modelIdUnderMouse(ModelType mType)
    {
        static const Ogre::String intro = "in Editor::ModelIdUnderMouse(mType=" + modelTypesAsString[mType] + "): ";

        ModelId mid = INVALID_ID;
        AgentId aid = agentIdUnderMouse();

        if(INVALID_ID == aid)
            return INVALID_ID;

        Agent *agent = mEngine->level()->agentMan()->getAgent(aid);

        if(nullptr == agent)
            Debug::error(intro)("can't find agent ")(aid).endl();
        else
            mid = agent->modelId(mType);

        return mid;
    }

    bool Editor::dynamicFillSerialization(Json::Value &node, AgentId aid)
    {
        static const Ogre::String intro = "in Editor::dynamicFillSerialization(): ";

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
            if(node.isMember(Editor::DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE) &&
                    JsonUtils::asBool(node[Editor::DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE], false))
                return true;

            // dict:: process each value
            for(auto it = node.begin(); it != node.end(); ++it)
            {
                Ogre::String mamberName = it.memberName();

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
                auto agent = mEngine->level()->agentMan()->getAgent(aid);

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
                        ModelId mid_um = modelIdUnderMouse(MT_OGRE);

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

    Ogre::Vector3 Editor::getDropTargetPosition()
    {
        if(nullptr == mEngine || nullptr == mEngine->level())
            return Ogre::Vector3::ZERO;

        return mEngine->level()->camera()->dropTargetPosition();
    }

    Ogre::Quaternion Editor::getDropTargetRotation()
    {
        if(nullptr == mEngine || nullptr == mEngine->level())
            return Ogre::Quaternion::ZERO;

        return mEngine->level()->camera()->dropTargetRotation();
    }

    Ogre::Vector2 Editor::getSlotDropPosition()
    {
        if(nullptr == mEngine || nullptr == mEngine->level())
            return Ogre::Vector2::ZERO;

        auto cam = mEngine->level()->camera();

        auto mousPos = mInputMan->mousePos();
        auto camRay = cam->cam()->getCameraToViewportRay(mousPos.x / static_cast<float>(mWidth),
                      mousPos.y / static_cast<float>(mHeight));

// get terrain under the cam
        Ogre::Terrain *terrain = nullptr;
        auto camPos = cam->camNode()->convertLocalToWorldPosition(Ogre::Vector3::ZERO);
        float height = mEngine->level()->terrainManager()->terrainGroup()->getHeightAtWorldPosition(camPos, &terrain);

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
            Debug::warning("Camera::slotDropPosition(): can't drop above horizontal camera plan !").endl();
            return Ogre::Vector2(FLT_MAX, FLT_MAX);
        }

        Ogre::Vector3 slotWorldPos = camRay.getPoint(result.second);

        long int x = 0, y = 0;
        mEngine->level()->terrainManager()->terrainGroup()->convertWorldPositionToTerrainSlot(slotWorldPos, &x, &y);
        return Ogre::Vector2(static_cast<float>(x), static_cast<float>(y));
    }

    bool Editor::hitTest(int x, int y, Rocket::Core::String childId)
    {
        Rocket::Core::Element *elem = mDocument;

        if(elem != nullptr)
        {
            if((elem = elem->GetElementById(childId)) != nullptr)
            {
                const Rocket::Core::Vector2f &tl = elem->GetAbsoluteOffset(Rocket::Core::Box::PADDING);
                int left = tl.x;
                int top = tl.y;
                const Rocket::Core::Vector2f &box = elem->GetBox(Rocket::Core::Box::BORDER).GetSize(
                                                        Rocket::Core::Box::BORDER);
                int right = left + box.x;
                int bottom = top + box.y;

                if(x >= left && y >= top && x <= right && y <= bottom)
                    return true;
            }
        }

        return false;
    }

    bool Editor::instanciateResource(File &file)
    {
        AgentId aid = INVALID_ID;
        return instanciateResource(file, aid);
    }

    bool Editor::instanciateResource(File &file, AgentId &aid)
    {
        static Ogre::String intro = "Editor::instanciateResource(" + file.fullPath() + "): ";

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
            {
                Level *level = mEngine->level();

                if(nullptr == level)
                {
                    Debug::error(intro)("no level to create agent.").endl();
                }

                aid = mEngine->level()->agentMan()->newAgent();
            }
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

    bool Editor::loadModelsReferencesFromSerializations(Json::Value &root, Steel::AgentId &aid)
    {
        Ogre::String intro = "Editor::loadModelsReferencesFromSerializations(): ";

        Level *level = mEngine->level();

        if(level == nullptr)
        {
            Debug::error(intro)("no level to instanciate models.").endl();
            return false;
        }

        if(!root.isMember(Editor::MODELS_ATTRIBUTE))
        {
            Debug::error(intro)("member ").quotes(Editor::MODELS_ATTRIBUTE)(" not found. Aborting.").endl();
            return false;
        }

        Json::Value models = root[Editor::MODELS_ATTRIBUTE];

        if(models.isNull())
        {
            Debug::warning(intro).quotes(Editor::MODELS_ATTRIBUTE)(" attribute is null. Aborting.");
            return false;
        }

        if(!models.isArray())
        {
            Debug::warning(intro).quotes(Editor::MODELS_ATTRIBUTE)(" attribute is not an array. Aborting.");
            return false;
        }

        bool revertAgent = false;

        for(Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            Json::Value refNode = *it, modelNode;

            // if path is there, it's a reference
            bool hasPathMember = refNode.isMember(Editor::MODEL_PATH_ATTRIBUTE);

            if(hasPathMember)
            {
                Json::Value pathValue = refNode[Editor::MODEL_PATH_ATTRIBUTE];

                if(!pathValue.isString())
                {
                    Debug::warning(intro)("Could not read reference path ").quotes(pathValue)(" as string. Aborting").endl();
                    revertAgent = true;
                    break;
                }

                // load the referee
                Ogre::String path = Ogre::StringUtil::BLANK;
                resolveReferencePaths(pathValue.asString(), path);
                File file = mEngine->dataDir().subfile(path);

                if(!file.readInto(modelNode, false))
                {
                    Debug::warning(intro)("Could not read reference path \"")(path)("\". Aborting.");
                    revertAgent = true;
                    break;
                }

                // possibly add attributes overrides
                if(refNode.isMember(Editor::MODEL_REF_OVERRIDE_ATTRIBUTE))
                {
                    Json::Value overrides = refNode[Editor::MODEL_REF_OVERRIDE_ATTRIBUTE];

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

                Json::Value value = modelNode[Editor::MODEL_TYPE_ATTRIBUTE];

                if(value.isNull() || !value.isString() || value.asString() != modelTypesAsString[MT_OGRE])
                {
                    Debug::error(intro)("serialization is not starting with a model of type MT_OGRE. Aborted.").endl();
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
                mEngine->level()->agentMan()->deleteAgent(aid);
                aid = INVALID_ID;
            }

            return false;
        }

        return true;
    }

    void Editor::setupReferencePathsLookupTable(Ogre::String const &source)
    {
        Ogre::String intro = "Editor::setupReferencePaths(): ";
        Ogre::String outro = "Loading resource may not be possible.";
        Json::Reader reader;
        Json::Value root;

        if(!reader.parse(source.c_str(), root))
        {
            Debug::error(intro)("Could not parse lookup table:")(reader.getFormatedErrorMessages()).endl();
            Debug::error("Source was: ")(source).endl()(outro).endl();
            return;
        }

        if(!root.isObject() || root.isNull())
        {
            Debug::error(intro)("Table is invalid: ")(root).endl()(outro).endl();
            return;
        }

        mReferencePathsLookupTable.clear();
        Json::Value::Members members = root.getMemberNames();

        for(Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
        {
            Json::Value value = root[*it];

            if(!value.isString())
            {
                Debug::warning(intro)("invalid value type for key\"")(*it)("\": ")(value.toStyledString())(outro).endl();
                continue;
            }

            Ogre::String s0 = *it;
            Ogre::String s1 = value.asString();
            mReferencePathsLookupTable.insert(std::pair<Ogre::String, Ogre::String>(s0, s1));
        }
    }

    void Editor::resolveReferencePaths(Ogre::String const &src, Ogre::String &dst)
    {
        dst.assign(src);

        while(true)
        {
            Ogre::String save(dst);

            for(auto it = mReferencePathsLookupTable.begin(); it != mReferencePathsLookupTable.end(); ++it)
            {
                Ogre::String what = (*it).first;
                Ogre::String withWhat = (*it).second;
                dst.assign(Ogre::StringUtil::replaceAll(dst, what, withWhat));
            }

            if(save == dst)
                break;
        }
    }

    bool Editor::loadModelsFromSerializations(Json::Value &root, Steel::AgentId &aid)
    {
        Ogre::String intro = "Editor::loadModelsFromSerializations(): ";

        Level *level = mEngine->level();

        if(level == nullptr)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return false;
        }

        if(!root.isObject())
        {
            Debug::error(intro)("Expecting object content. Aborting.").endl();
            return false;
        }

        if(!root.isMember(Editor::MODELS_ATTRIBUTE))
        {
            Debug::error(intro)("member ").quotes(Editor::MODELS_ATTRIBUTE)(" not found. Aborting.").endl();
            return false;
        }


        Json::Value node = root[Editor::MODELS_ATTRIBUTE];

        if(!node.isArray() || node.isNull())
        {
            Debug::warning(intro).quotes(Editor::MODELS_ATTRIBUTE)(" attribute is not valid (expecting a non-empty array).").endl();
            return false;
        }

        // instanciate all models
        for(Json::ValueIterator it = node.begin(); it != node.end(); ++it)
        {
            Json::Value modelNode = *it;

            if(INVALID_ID == aid)
            {
                Json::Value value = modelNode[Editor::MODEL_TYPE_ATTRIBUTE];

                if(value.isNull() || value.asCString() != modelTypesAsString[MT_OGRE])
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

    bool Editor::loadModelFromSerialization(Json::Value &root, Steel::AgentId &aid)
    {
        Debug::log("Editor::loadModelFromSerialization(")(root)(")").endl();
        Ogre::String intro = "Editor::loadModelFromSerialization(): ";

        Level *level = mEngine->level();

        if(level == nullptr)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return false;
        }

        if(!root.isObject())
        {
            Debug::error("expecting a json object. Aborting.").endl();
            return false;
        }

        Json::Value value = root[Editor::MODEL_TYPE_ATTRIBUTE];

        if(value.isNull())
        {
            Debug::error("serialization is missing a ").quotes(Editor::MODEL_TYPE_ATTRIBUTE)(" value. Aborted.").endl();
            return false;
        }

        Ogre::String modelTypeString = value.asString();

        // this is used to know it the agent should be deleted upon failure of the method
        bool fresh_aid = INVALID_ID == aid;

        if(fresh_aid)
        {
            aid = mEngine->level()->agentMan()->newAgent();

            if(INVALID_ID == aid)
            {
                Debug::error(intro)("could not create an agent to link the model to. Aborted.").endl();
                return false;
            }

            Debug::log(intro)("created agent ")(aid).endl();
        }

        // ask the right manager to load this model
        ModelType modelType = MT_FIRST;
        auto it = std::find(modelTypesAsString.begin(), modelTypesAsString.end(), modelTypeString);

        if(modelTypesAsString.end() == it)
        {
            Debug::warning(intro)("Unknown model type: ")(modelTypeString).endl();
            return false;
        }
        else
            modelType = (ModelType) std::distance(modelTypesAsString.begin(), it);

        // check if the agent is already linked to such a model
        Agent *agent = mEngine->level()->agentMan()->getAgent(aid);

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
        auto manager = level->modelManager(modelType);

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

        if(!level->linkAgentToModel(aid, modelType, mid))
        {
            level->modelManager(modelType)->decRef(mid);
            Debug::error(intro)("could not link agent ")(aid)(" to  ")(modelTypeString)(" Model ")
            (mid)(". Model released. Aborted.").endl();

            if(fresh_aid)
                mEngine->level()->agentMan()->deleteAgent(aid);

            return false;
        }
        else
            Debug::log("new ")(modelTypeString)(" Model with id ")(mid)(" linked to agent ")(aid).endl();

        //TODO add visual notification in the UI
        return true;
    }

    AgentId Editor::instanciateFromMeshFile(File &meshFile, Ogre::Vector3 &pos, Ogre::Quaternion &rot)
    {
        Ogre::String intro = "Editor::instanciateFromMeshFile(";
        Debug::log(intro)(meshFile)(" pos=")(pos)(" rot=")(rot).endl();
        Level *level = mEngine->level();

        if(level == nullptr)
        {
            Debug::warning(intro + "): no level to instanciate stuff in.").endl();
            return INVALID_ID;
        }

//      Ogre::Quaternion r = mEngine->camera()->camNode()->getOrientation();
        Steel::ModelId mid = level->ogreModelMan()->newModel(meshFile.fileName(), pos, rot);
        AgentId aid = mEngine->level()->agentMan()->newAgent();

        if(!level->linkAgentToModel(aid, MT_OGRE, mid))
        {
            Debug::error(intro + "): could not level->linkAgentToModel(")(aid)(", MT_OGRE, ")(mid)(")").endl();
            return INVALID_ID;
        }

        return mid;
    }

    bool Editor::loadTerrainSlotFromSerialization(Json::Value &root)
    {
        Ogre::String intro = "Editor::loadTerrainSlotFromSerialization()";

        auto level = mEngine->level();

        if(nullptr == level)
        {
            Debug::error(intro)("no level loaded. Aborted.").endl();
            return false;
        }

        if(root["slotPosition"].isNull())
        {
            Debug::error(intro)("can't find key \"slotPosition\". Aborted.").endl();
            return false;
        }

        TerrainManager::TerrainSlotData slot;
        level->terrainManager()->terrainSlotFromJson(root, slot);

        if(!slot.isValid())
        {
            Debug::error(intro)("TerrainManager::TerrainSlotData is not valid. Serialized string was:");
            Debug::error(root.toStyledString()).endl()("Aborted.").endl();
            return false;
        }

        level->terrainManager()->addTerrainSlot(slot);
        return true;
    }

    bool Editor::keyPressed(Input::Code key, Input::Event const &evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI->getKeyIdentifier(key);
        mContext->ProcessKeyDown(keyIdentifier, mUI->getKeyModifierState());

        if(evt.text >= 32)
        {
            mContext->ProcessTextInput((Rocket::Core::word) evt.text);
        }
        else if(keyIdentifier == Rocket::Core::Input::KI_RETURN)
        {
            mContext->ProcessTextInput((Rocket::Core::word) '\n');
        }

        return true;
    }

    bool Editor::keyReleased(Input::Code key, Input::Event const &evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI->getKeyIdentifier(key);
        mContext->ProcessKeyUp(keyIdentifier, mUI->getKeyModifierState());

        SelectionManager *selectionMan = mEngine->level()->selectionMan();
        Level *level = mEngine->level();

        switch(key)
        {
            case Input::Code::KC_H:
                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::TERRAFORM);

                break;

            case Input::Code::KC_L:
                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::LINK);

                break;

            case Input::Code::KC_R:
                if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                    reloadContent();

                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::ROTATE);

                break;

            case Input::Code::KC_S:
                if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                {
                    if(level != nullptr)
                        level->save();

                    mEngine->saveConfig(mEngine->config());
                }

                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::SCALE);

                break;

            case Input::Code::KC_T:
                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    mBrush.setMode(EditorBrush::TRANSLATE);

                break;

            case Input::Code::KC_DELETE:
            {
                auto child = mDocument->GetElementById("editor");

                if(nullptr == child || !child->IsPseudoClassSet("focus"))
                {
                    selectionMan->deleteSelection();
                }
            }
            break;

            case Input::Code::KC_F5:
                processCommand("engine.register.ui.reload");
                break;

            default:
                break;
        }

        // numeric key pressed: handles memoing (keys: 1,2,3,...,0)
        if(nullptr != mEngine && key >= Input::Code::KC_1 && key <= Input::Code::KC_0)
        {
            // OIS::KC_0 is the highest; the modulo makes it 0
            int memoKey = ((int)key - (int)Input::Code::KC_1 + 1) % 10;
            Ogre::String memo = Ogre::StringConverter::toString(memoKey);

            if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                selectionMan->saveSelectionToMemo(memo);
            else
                selectionMan->selectMemo(memo);
        }

        return true;
    }

    bool Editor::mousePressed(Input::Code button, Input::Event const &evt)
    {
        if(!hitTest(evt.position.x, evt.position.y, "menu"))
        {
            mBrush.mousePressed(button, evt);
        }

        return true;
    }

    bool Editor::mouseReleased(Input::Code button, Input::Event const &evt)
    {
        mBrush.mouseReleased(button, evt);

        if(button == Input::Code::MC_RIGHT)
            mFSResources->expandRows();

        return true;
    }

    bool Editor::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        bool hoveringMenu = hitTest(position.x, position.y, "menu");

        if((!mIsDraggingFromMenu && !hoveringMenu) || (mBrush.isDragging() || mBrush.isInContiniousMode() || mBrush.isSelecting()))
        {
            mBrush.mouseMoved(position, evt);
            Rocket::Core::Element *elem;

            elem = mDocument->GetElementById("editor_terrabrush_intensity");

            if(elem != nullptr)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.intensity()).c_str());

            elem = mDocument->GetElementById("editor_terrabrush_radius");

            if(elem != nullptr)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.radius()).c_str());
        }
        else
        {
            mContext->ProcessMouseMove(position.x, position.y, mUI->getKeyModifierState());
        }

        return true;
    }

    void Editor::onFileChangeEvent(File file)
    {
        Debug::log("Editor::onFileChangeEvent(): ")(file.fullPath()).endl();
        UIPanel::onFileChangeEvent(file);
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);
    }

    void Editor::onShow()
    {
        Ogre::String intro = "Editor::onShow(): ";
        mBrush.onShow();

        // (re)load state
        // active menu tab
        auto elem = (Rocket::Controls::ElementTabSet *) mDocument->GetElementById("editor_tabset");

        if(nullptr != elem)
        {
            int tabNo = mEngine->config().getSettingAsInt(Editor::MENU_TAB_INDEX_SETTING, 0);
            elem->SetActiveTab(tabNo);
        }

        // ## reconnect to document
        // data sources
        mFSResources->refresh(mDocument);

        // events
        if(nullptr != mDocument)
        {
            mDocument->AddEventListener("click", this);
            mDocument->AddEventListener("dragstart", this);
            mDocument->AddEventListener("dragdrop", this);
            mDocument->AddEventListener("change", this);
            mDocument->AddEventListener("submit", this);
        }

        // debugger
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);

        // set brush shape
        auto select_form = static_cast<Rocket::Controls::ElementFormControlSelect *>(mDocument->GetElementById("editor_select_terrabrush_shape"));

        if(select_form != nullptr and select_form->GetNumOptions() > 0)
        {
            int index = select_form->GetSelection();

            if(index < 0)
                select_form->SetSelection(0);
            else
                processCommand(Ogre::String("editorbrush.terrabrush.distribution.") + select_form->GetOption(index)->GetValue().CString());
        }

        refreshSelectionTagsWidget();
        refreshSelectionPathWidget();
    }

    void Editor::onHide()
    {
        if(nullptr != mDocument)
        {
            mDocument->RemoveEventListener("click", this);
            mDocument->RemoveEventListener("dragdrop", this);
            mDocument->RemoveEventListener("change", this);
            mDocument->RemoveEventListener("submit", this);
            saveMenuTabIndexSetting(mEngine->config());
        }

        mBrush.onHide();
    }

    void Editor::onSelectionChanged(Selection &selection)
    {
        refreshSelectionTagsWidget();
        refreshSelectionPathWidget();
    }

    void Editor::refreshSelectionPathWidget()
    {
        static const Ogre::String intro = "in Editor::refreshSelectionPathWidget(): ";

        if(nullptr == mEngine->level())
            return;

        auto level = mEngine->level();
        auto selectionMan = level->selectionMan();

        Rocket::Core::String innerRML = "";

        if(selectionMan->selection().size() == 0)
        {
            innerRML = "empty selection";
        }
        else if(selectionMan->selection().size() > 1)
        {
            innerRML = "single selection only";
        }
        else
        {
            Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());

            if(nullptr == agent)
                innerRML = "no path found in selection";
            else
            {
                LocationModel *model = agent->locationModel();

                if(nullptr == model || !model->hasPath())
                    innerRML = "no path found in selection";
                else
                {
                    innerRML = model->path().c_str();
                }
            }
        }

        if(nullptr == mDocument)
            return;

        Rocket::Core::Element *elem = mDocument->GetElementById(Editor::SELECTION_PATH_INFO_BOX);

        if(nullptr == elem)
        {
            Debug::error(intro)("child ").quotes(Editor::SELECTION_PATH_INFO_BOX)("not found. Aborting.").endl();
            return;
        }

        elem->SetInnerRML(innerRML);
    }

    void Editor::refreshSelectionTagsWidget()
    {
        if(nullptr == mEngine->level())
            return;

        auto allTags = mEngine->level()->selectionMan()->tagsUnion();
        auto allTagsStrings = TagManager::instance().fromTags(allTags);
        populateSelectionTagsWidget(allTagsStrings);
    }

    void Editor::populateSelectionTagsWidget(std::list<Ogre::String> tags)
    {
        static const Ogre::String intro = "in Editor::populateSelectionTagWidget(): ";

        if(nullptr == mDocument)
            return;

        Rocket::Core::Element *elem = mDocument->GetElementById(Editor::SELECTION_TAGS_INFO_BOX);

        if(nullptr == elem)
        {
            Debug::error(intro)("child with id ").quotes(Editor::SELECTION_TAGS_INFO_BOX)(" not found. Aborting.").endl();
            return;
        }

        // Emtpy it
        while(nullptr != elem->GetFirstChild())
        {
            Rocket::Core::Element *child = elem->GetFirstChild();
            elem->RemoveChild(child);
        }

        // Repopulate it
        for(auto const & it : tags)
        {
            Rocket::Core::Element *child = mDocument->CreateElement(Editor::AGENT_TAG_ITEM_NAME);
            decorateSelectionTagWidgetItem(child, it.c_str());
            elem->AppendChild(child);
        }
    }

    void Editor::decorateSelectionTagWidgetItem(Rocket::Core::Element *item, Ogre::String const &tagName)
    {
        // default tag button: "tagName [x]"
        Ogre::String rml = tagName + "<p style=\"margin-left:3px;\" onclick=\"selection.tag.unset.$tagName\">[x]</p>";

        // try getting the closing button from the data templates
        Rocket::Core::Element *elemTemplate = mDocument->GetElementById("SelectionTagWidget_AgentItem_Btn");

        if(nullptr != elemTemplate && elemTemplate->GetNumChildren() > 0)
        {
            rml = tagName + elemTemplate->GetInnerRML().CString();
        }

        item->SetInnerRML(Ogre::StringUtil::replaceAll(rml, "$tagName", tagName).c_str());
    }

    void Editor::ProcessEvent(Rocket::Core::Event &event)
    {
        if(!isVisible())
            return;

// create the command
        Rocket::Core::Element *elem = nullptr;

// in case of drag&drop, elem points to the element being dragged
        if(event == "dragdrop")
        {
            // ok in stable, not in dev
            //             elem= static_cast<Rocket::Core::Element *>(event.GetParameter< void * >("drag_element", nullptr));
            Rocket::Core::ElementReference *ref = static_cast<Rocket::Core::ElementReference *>(event.GetParameter<void *>(
                    "drag_element", nullptr));
            elem = **ref;
            mIsDraggingFromMenu = false;
        }
        else if(event == "dragstart")
            mIsDraggingFromMenu = true;
        else
        {
            elem = event.GetTargetElement();
            mIsDraggingFromMenu = false;
        }

        if(elem == nullptr)
        {
            if(mDebugEvents)
                Debug::log("Editor::ProcessEvent(): no target element for event of type ")(event.GetType()).endl();

            return;
        }

        auto etype = event.GetType();
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + etype, "NoValue").CString();

        if(event_value == "NoValue")
        {
            if(elem->GetId() != "")
            {
                if(mDebugEvents)
                {
                    Debug::warning("Editor::ProcessEvent(): no event_value for event of type ")
                    (event.GetType())(" with elem of id ")(elem->GetId()).endl();
                }

                return;
            }
        }

        if(etype == "change")
            processChangeEvent(event, elem);
        else if(etype == "click")
            processClickEvent(event, elem);
        else if(etype == "dragdrop")
            processDragDropEvent(event, elem);
        else if(etype == "submit")
            processSubmitEvent(event, elem);
        else
            Debug::log("Editor::ProcessEvent(): unknown event ot type:")(event.GetType())(" and value: ")(event_value).endl();

        return;
    }

    void Editor::processSubmitEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Debug::log("Editor::processSubmitEvent(): type:")(event.GetType());
        Debug::log(" and value: ")(elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString());
        Debug::log.endl();
    }

    void Editor::processClickEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = event_value;

        if(rawCommand == "engine.set_level")
        {
            auto inputField = (Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");

            if(inputField == nullptr)
            {
                Debug::error("Editor::processClickEvent(): can't find level name input field with id=\"level_name\". Aborted.").endl();
                return;
            }

            Ogre::String levelName = inputField->GetValue().CString();

            if(levelName == "")
            {
                Debug::error("Editor::processClickEvent(): can't create a level with not name. Aborted.").endl();
                return;
            }

            rawCommand += "." + levelName;
        }
        else if(rawCommand == "selection.tag.set")
        {
            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                Debug::log("in Editor::processClickEvent(): ")("rawCommand: ").quotes(rawCommand)
                (", text input is empty. Not setting the empty tag.").endl();
                return;
            }
        }
        else if(Ogre::StringUtil::startsWith(rawCommand, "selection.tag.unset"))
        {
            // valid command, nothing to add
        }
        else if(rawCommand == "selection.path.set")
        {
            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_PATH_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                Debug::log("in Editor::processClickEvent(): ")("rawCommand: ").quotes(rawCommand)
                (", text input is empty. Not setting the empty tag.").endl();
                return;
            }
        }
        else if(Ogre::StringUtil::startsWith(rawCommand, "selection.path.unset."))
        {
            // valid command, nothing to add
        }

        //         Debug::log("Editor::processClickEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if(rawCommand.size())
            processCommand(rawCommand);
    }

    void Editor::processChangeEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = event_value;

        if(rawCommand == "editorbrush.terrabrush.distribution")
        {
            Rocket::Controls::ElementFormControlSelect *form = static_cast<Rocket::Controls::ElementFormControlSelect *>(elem);
            auto optionId = form->GetSelection();

            if(optionId > -1)
            {
                rawCommand += ".";
                rawCommand += form->GetValue().CString();
            }
            else
                return;
        }
        else if(rawCommand == "selection.tag.set")
        {
            bool linebreak = event.GetParameter<bool>("linebreak", false);

            if(!linebreak)
                return;

            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                if(mDebugEvents)
                {
                    Debug::log("in Editor::processChangeEvent(): ")
                    ("rawCommand: ").quotes(rawCommand)
                    (", event_value: ").quotes(event_value)
                    (" does not end with a new line. Skipping.").endl();
                }

                return;
            }
        }
        else if(rawCommand == "selection.path.set")
        {
            bool linebreak = event.GetParameter<bool>("linebreak", false);

            if(!linebreak)
                return;

            Ogre::String content = getFormControlInputValue(Editor::SELECTIONS_PATH_EDIT_BOX);

            if(content.size() > 0)
            {
                rawCommand += ".";
                rawCommand += content;
            }
            else
            {
                if(mDebugEvents)
                {
                    Debug::log("in Editor::processChangeEvent(): ")
                    ("rawCommand: ").quotes(rawCommand)
                    (", event_value: ").quotes(event_value)
                    (" does not end with a new line. Skipping.").endl();
                }

                return;
            }
        }

        //         Debug::log("Editor::processChangeEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if(rawCommand.size())
            processCommand(rawCommand);
    }

    void Editor::processDragDropEvent(Rocket::Core::Event &event, Rocket::Core::Element *elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = "";

        if(elem->GetId() == mFSResources->GetDataSourceName())
        {
            rawCommand = "instanciate.";
            File file = File(event_value);

            if(!file.exists())
            {
                Debug::error("Editor::ProcessEvent(): file not found: ")(file).endl();
                return;
            }

            rawCommand += file.fullPath();
        }
        else
        {
            Debug::warning("Editor::ProcessDragDropEvent() unknown element source for event of type: ")(event.GetType());
            Debug::warning(" value: ")(event_value).endl();
            return;
        }

        //         Debug::log("Editor::processDragDropEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if(rawCommand.size())
            processCommand(rawCommand);
    }

    bool Editor::processCommand(Ogre::String rawCommand)
    {
        if("NoValue" == rawCommand)
            return true;

        Debug::log("Editor::processCommand(raw=")(rawCommand)(")").endl();
        return processCommand(StringUtils::split(std::string(rawCommand), std::string(".")));
    }

    bool Editor::processCommand(std::vector<Ogre::String> command)
    {
        Ogre::String intro = "in Editor::processCommand(): ";

        auto level = mEngine->level();
        auto selectionMan = level->selectionMan();

        // dispatch the command to the right subprocessing function
        if(command[0] == "editor")
        {
            command.erase(command.begin());

            if(command.size() == 0)
                Debug::warning(intro)("no command given.").endl();
            else if(command[0] == "hide")
                mEngine->stopEditMode();
            else if(command[0] == "options")
            {
                command.erase(command.begin());
                return processOptionCommand(command);
            }
            else
            {
                Debug::warning(intro)("unkown command: ")(command).endl();
                return false;
            }
        }
        else if(command[0] == "editorbrush")
        {
            command.erase(command.begin());
            return mBrush.processCommand(command);
        }
        else if(command[0] == "engine")
        {
            command.erase(command.begin());
            return mEngine->processCommand(command);
        }
        else if(command[0] == "instanciate")
        {
            intro += "instanciate: ";
            command.erase(command.begin());

            if(command.size() == 0)
            {
                Debug::error(intro)("command \"")(command);
                Debug::error("\" contains no file !").endl();
                return false;
            }

            File file(StringUtils::join(command, "."));

            if(file.exists())
            {
                return instanciateResource(file);
            }
            else
            {
                Debug::warning(intro)("file \"")(file)("\" not found for command \"");
                Debug::warning(command)("\". Aborted.").endl();
                return false;
            }
        }
        else if(command[0] == "selection")
        {
            command.erase(command.begin());// selection

            if(command[0] == "tag")
            {
                if(command.size() < 2)
                {
                    Debug::error(intro)("command ").quotes(command)(" is incomplete").endl();
                    return false;
                }

                command.erase(command.begin());// tag

                if(command[0] == "set")
                {
                    command.erase(command.begin());// set

                    if(command.size() == 0)
                    {
                        Debug::error(intro)("no tag to set").endl();
                        return false;
                    }

                    if(nullptr != level && selectionMan->hasSelection())
                    {
                        selectionMan->tagSelection(TagManager::instance().toTag(StringUtils::join(command, ".")));
                        refreshSelectionTagsWidget();
                        auto form = setFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX, "");

                        if(nullptr != form)
                            form->Focus();
                    }
                    else
                        return false;
                }
                else if(command[0] == "unset")
                {

                    command.erase(command.begin());// unset

                    if(command.size() == 0)
                    {
                        Debug::error(intro)("no tag to unset").endl();
                        return false;
                    }

                    if(nullptr != mEngine->level() && mEngine->level()->selectionMan()->hasSelection())
                    {
                        mEngine->level()->selectionMan()->untagSelection(TagManager::instance().toTag(StringUtils::join(command, ".")));
                        refreshSelectionTagsWidget();
                        auto form = setFormControlInputValue(Editor::SELECTIONS_TAG_EDIT_BOX, "");

                        if(nullptr != form)
                            form->Focus();

                        return true;
                    }
                    else
                        return false;
                }
                else
                {
                    Debug::warning(intro)("unknown command: \"")(command)("\".").endl();
                    return false;
                }
            }
            else if(command[0] == "path")
            {
                if(command.size() < 2)
                {
                    Debug::error(intro)("command \"")(command);
                    Debug::error("\" is incomplete").endl();
                    return false;
                }

                command.erase(command.begin());// path

                if(command[0] == "set")
                {
                    command.erase(command.begin());// set

                    if(command.size() == 0)
                    {
                        Debug::error(intro)("no path to set").endl();
                        return false;
                    }

                    if(!selectionMan->hasSelection() || selectionMan->selection().size() > 1)
                    {
                        Debug::error(intro)("need to select 1 model linked to a LocationModel.").endl();
                        return false;
                    }

                    Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());

                    if(nullptr == agent)
                        return false;

                    agent->setLocationPath(StringUtils::join(command, "."));
                    refreshSelectionPathWidget();
                }
                else if(command[0] == "unset")
                {
                    command.erase(command.begin());// unset
                    Agent *agent = level->agentMan()->getAgent(selectionMan->selection().front());

                    if(nullptr == agent)
                        return false;

                    agent->unsetLocationPath();
                    refreshSelectionPathWidget();
                }
                else
                {
                    Debug::warning(intro)("unknown path command: \"")(command)("\".").endl();
                    return false;
                }
            }
            else
            {
                Debug::warning(intro)("unknown selection command: \"")(command)("\".").endl();
                return false;
            }
        }
        else
        {
            Debug::warning(intro)("unknown editor command: \"")(command)("\".").endl();
            return false;
        }

        return true;
    }

    Ogre::String Editor::getFormControlInputValue(Ogre::String elementId)
    {
        if(nullptr == mDocument)
            return "";

        Rocket::Core::Element *elem = mDocument->GetElementById(elementId.c_str());

        if(nullptr == elem)
            return "";

        // try to assert we're actually using a form
        Ogre::String tagName = elem->GetTagName().CString();

        if("input" != tagName && "select" != tagName)
        {
            Debug::error("in Editor::getFormControlInputValue(): trying to use elem with id ").quotes(elementId)
            (" and tagname ").quotes(tagName)(" as Rocket form. This would probably result in a segfault. Aborting.").endl();
            return "";
        }

        Rocket::Controls::ElementFormControlInput *form = static_cast<Rocket::Controls::ElementFormControlInput *>(elem);
        Ogre::String content = form->GetValue().CString();
        return content;
    }

    Rocket::Controls::ElementFormControlInput *Editor::setFormControlInputValue(Ogre::String elementId, Ogre::String value)
    {
        if(nullptr == mDocument)
            return nullptr;

        Rocket::Core::Element *elem = mDocument->GetElementById(elementId.c_str());

        if(nullptr == elem)
            return nullptr;

        // try to assert we're actually using a form
        Ogre::String tagName = elem->GetTagName().CString();

        if("input" != tagName && "select" != tagName)
        {
            Debug::error("in Editor::setFormControlInputValue(): trying to use elem with id ").quotes(elementId)
            (" and tagname ").quotes(tagName)(" as Rocket form. This would probably result in a segfault. Aborting.").endl();
            return nullptr;
        }

        Rocket::Controls::ElementFormControlInput *form = static_cast<Rocket::Controls::ElementFormControlInput *>(elem);
        Rocket::Core::String _value = value.c_str();
        form->SetValue(_value);
        return form;
    }

    bool Editor::processOptionCommand(std::vector<Ogre::String> command)
    {
        if(command.size() == 0)
            return false;

        if(command[0] == "resourceGroupsInfos")
            OgreUtils::resourceGroupsInfos();
        else if(command[0] == "switch_debug_events")
        {
            mDebugEvents = !mDebugEvents;
            Debug::log("flag DebugEvent ")(mDebugEvents ? "activated" : "deactivated").endl();
        }
        else
        {
            Debug::warning("Editor::processOptionCommand(): unknown command: ")(command).endl();
            return false;
        }

        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

