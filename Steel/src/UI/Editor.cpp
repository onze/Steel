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

namespace Steel
{

    const Ogre::String Editor::REFERENCE_PATH_LOOKUP_TABLE = "Editor::referencePathsLookupTable";
    const Ogre::String Editor::MENU_TAB_INDEX_SETTING = "Editor::menuTabIndex";

    Editor::Editor():UIPanel("Editor", "data/ui/current/editor/editor.rml"),
        mEngine(NULL), mUI(NULL), mInputMan(NULL), mFSResources(NULL),
        mDataDir(), mBrush(), mDebugEvents(false), mIsDraggingFromMenu(false),
        mReferencePathsLookupTable(std::map<Ogre::String, Ogre::String>())
    {
#ifdef DEBUG
        mAutoReload=true;
#endif
    }

    Editor::Editor(const Editor& other)
    {
        throw std::runtime_error("Editor::Editor(const Editor& other): Not Implemented");
    }

    Editor::~Editor()
    {
        shutdown();
        if (NULL != mFSResources)
        {
            delete mFSResources;
            mFSResources = NULL;
        }
    }

    Editor& Editor::operator=(const Editor& other)
    {
        throw std::runtime_error("Editor::operator=(const Editor& other): Not Implemented");
        return *this;
    }

    void Editor::shutdown()
    {
        UIPanel::shutdown();
        mBrush.shutdown();

        // when this panel is reloaded, it calls Editor::shutdown but UIPanel::init,
        // so those 4 pointers aren't set back to meaningful values -> donbt delete them
//         if (NULL != mFSResources)
//         {
//             delete mFSResources;
//             mFSResources = NULL;
//         }
//         mEngine = NULL;
//         mUI = NULL;
//         mInputMan = NULL;
    }

    void Editor::loadConfig(ConfigFile const &config)
    {
        mBrush.loadConfig(config);
    }

    void Editor::saveConfig(ConfigFile &config) const
    {
        mBrush.saveConfig(config);
        saveMenuTabIndexSetting(config);
    }

    void Editor::saveMenuTabIndexSetting(ConfigFile &config) const
    {
        // save state
        if(NULL!=mDocument)
        {
            auto elem = (Rocket::Controls::ElementTabSet *) mDocument->GetElementById("editor_tabset");
            if (NULL != elem)
            {
                int tabNo=elem->GetActiveTab();
                config.setSetting(Editor::MENU_TAB_INDEX_SETTING, Ogre::StringConverter::toString(tabNo));
            }
        }
    }

    void Editor::init(unsigned int width, unsigned int height, Engine *engine, UI *ui, InputManager *inputMan)
    {
        Debug::log("Editor::init()").endl();

        mDataDir = ui->dataDir().subfile("editor").fullPath();
        mEngine = engine;
        mUI = ui;
        mInputMan = inputMan;

        auto resGroupMan = Ogre::ResourceGroupManager::getSingletonPtr();
        // true is for recursive search. Add to this resources.cfg
        resGroupMan->addResourceLocation(mDataDir.fullPath(), "FileSystem", "UI", true);
        //resGroupMan->addResourceLocation(mDataDir.subfile("images").fullPath(), "FileSystem", "UI",true);
        //resGroupMan->declareResource("inode-directory.png","Texture","UI");

        setupReferencePathsLookupTable(mEngine->config().getSetting(Editor::REFERENCE_PATH_LOOKUP_TABLE));
        mFSResources = new FileSystemDataSource("resources", engine->rootDir().subfile("data").subfile("resources"));
        UIPanel::init(width, height);
        mFSResources->localizeDatagridBody(mDocument);
        auto elem = (Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");
        if (elem != NULL)
        {
            elem->SetValue("MyLevel");
            // does not work for some reason
            //             elem->AddEventListener("submit",this);
        }
        mBrush.init(engine, this, mInputMan);
    }

    AgentId Editor::agentIdUnderMouse()
    {
        Selection selection;
        mEngine->pickAgents(selection, mInputMan->mousePos().x, mInputMan->mousePos().y);

        AgentId aid = INVALID_ID;
        if (selection.size() > 0)
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

        Agent *agent = mEngine->level()->getAgent(aid);
        if (NULL == agent)
            Debug::error(intro)("can't find agent ")(aid).endl();
        else
            mid = agent->modelId(mType);
        return mid;
    }

    bool Editor::dynamicFillSerialization(Json::Value& node, AgentId aid)
    {
        static const Ogre::String intro = "in Editor::dynamicFillSerialization(): ";

        if (node.isArray())
        {
            // array: provcess each value
            for (unsigned i = 0; i < node.size(); ++i)
            {
                if (!dynamicFillSerialization(node[i]))
                    return false;
            }
        }
        else if (node.isObject())
        {
            // dict:: process each value
            for (auto it = node.begin(); it != node.end(); ++it)
            {
                if (!dynamicFillSerialization(*it, aid))
                    return false;
            }
        }
        else
        {
            Ogre::String svalue, new_svalue;
            svalue = new_svalue = node.asString();


            if (svalue.at(0) != '$')
                new_svalue = svalue;
            else
            {
                auto agent = mEngine->level()->getAgent(aid);

                if(svalue == "$agentOgreModel")
                {
                    if(NULL==agent)
                        new_svalue=Ogre::StringConverter::toString(INVALID_ID);
                    else
                        new_svalue = Ogre::StringConverter::toString(agent->ogreModelId());
                }
                else if(svalue == "$agentPhysicsModel")
                {
                    if(NULL==agent)
                        new_svalue=Ogre::StringConverter::toString(INVALID_ID);
                    else
                        new_svalue = Ogre::StringConverter::toString(agent->physicsModelId());
                }
                else if(svalue == "$agentUnderMouse")
                {
                    if(NULL==agent)
                    {
                        AgentId aid_um=agentIdUnderMouse();
                        if(INVALID_ID == aid_um)
                        {
                            Debug::error(intro)("no agent under mouse.").endl();
                            return false;
                        }
                        new_svalue=Ogre::StringConverter::toString(aid_um);
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
                    if(NULL==agent)
                    {
                        ModelId mid_um=modelIdUnderMouse(MT_OGRE);
                        if(INVALID_ID == mid_um)
                        {
                            Debug::error(intro)("no OgreModel under mouse.").endl();
                            return false;
                        }
                        new_svalue=Ogre::StringConverter::toString(mid_um);
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
                        Debug::error(intro)("slot drop position is invalid (>10km away):")(slotPosition)(". Aborted.").endl();
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
        if (NULL == mEngine || NULL == mEngine->level())
            return Ogre::Vector3::ZERO;
        return mEngine->level()->camera()->dropTargetPosition();
    }

    Ogre::Quaternion Editor::getDropTargetRotation()
    {
        if (NULL == mEngine || NULL == mEngine->level())
            return Ogre::Quaternion::ZERO;
        return mEngine->level()->camera()->dropTargetRotation();
    }

    Ogre::Vector2 Editor::getSlotDropPosition()
    {
        if (NULL == mEngine || NULL == mEngine->level())
            return Ogre::Vector2::ZERO;

        auto cam = mEngine->level()->camera();

        auto mousPos = mInputMan->mousePos();
        auto camRay = cam->cam()->getCameraToViewportRay(mousPos.x / static_cast<float>(mWidth),
                      mousPos.y / static_cast<float>(mHeight));

// get terrain under the cam
        Ogre::Terrain *terrain = NULL;
        auto camPos = cam->camNode()->convertLocalToWorldPosition(Ogre::Vector3::ZERO);
        float height = mEngine->level()->terrainManager()->terrainGroup()->getHeightAtWorldPosition(camPos, &terrain);

        Ogre::Plane plane(Ogre::Vector3::UNIT_Y, 0.f);
        if (NULL != terrain)
        {
            // cam is above a terrain, we use its height as base for the plane
            plane.d += height;
        }

// find where the drop point is
        auto result = camRay.intersects(plane);
        if (!result.first)
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
        if (elem != NULL)
        {
            if ((elem = elem->GetElementById(childId)) != NULL)
            {
                const Rocket::Core::Vector2f &tl = elem->GetAbsoluteOffset(Rocket::Core::Box::PADDING);
                int left = tl.x;
                int top = tl.y;
                const Rocket::Core::Vector2f &box = elem->GetBox(Rocket::Core::Box::BORDER).GetSize(
                                                        Rocket::Core::Box::BORDER);
                int right = left + box.x;
                int bottom = top + box.y;
                if (x >= left && y >= top && x <= right && y <= bottom)
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
        if (!file.exists())
        {
            Debug::error(intro)("file not found: ")(file).endl();
            return false;
        }
        Ogre::String content = file.read();

        Json::Reader reader;
        Json::Value root;
        bool parsingOk = reader.parse(content, root, false);
        if (!parsingOk)
        {
            Debug::error(intro)("could not parse this:").endl();
            Debug::error(content).endl();
            Debug::error(reader.getFormattedErrorMessages()).endl();
            return false;
        }

        if (!dynamicFillSerialization(root))
        {
            Debug::error(intro)("could not find all values. Aborted.").endl();
            return false;
        }

        if (INVALID_ID == aid)
        {
            // will end up pointing to the agent owning all created models
            Ogre::String aid_s = "";
            if (!root.isArray() && !root["aid"].isNull())
            {
                aid_s = root["aid"].asCString();
                aid = Ogre::StringConverter::parseUnsignedLong(aid_s, INVALID_ID);
            }
            else
            {
                Level *level=mEngine->level();
                if(NULL==level)
                {
                    Debug::error(intro)("no level to create agent.").endl();
                }
                aid=level->newAgent();
            }
        }

        Ogre::String instancitationType = file.extension();
        if (instancitationType == "model")
            return loadModelFromSerialization(root, aid);
        else if (instancitationType == "models")
            return loadModelsFromSerializations(root, aid);
        else if (instancitationType == "model_refs")
            return loadModelsReferencesFromSerializations(root, aid);
        else if (instancitationType == "terrain_slot")
            return loadTerrainSlotFromSerialization(root);
        else
        {
            Debug::warning(intro)("instanciation type unknown: ")(instancitationType).endl();
            return false;
        }
        return true;
    }

    bool Editor::loadModelsReferencesFromSerializations(Json::Value& root, Steel::AgentId& aid)
    {
        Ogre::String intro = "Editor::loadModelsReferencesFromSerializations(): ";

        Level *level = mEngine->level();
        if (level == NULL)
        {
            Debug::error(intro)("no level to instanciate models.").endl();
            return false;
        }

        Json::Value models = root["models"];
        if (models.isNull())
        {
            Debug::warning(intro)("\"models\' attribute is null. Aborting.");
            return false;
        }
        if (!models.isArray())
        {
            Debug::warning(intro)("\"models\' attribute is not an array. Aborting.");
            return false;
        }

        bool revertAgent = false;
        for (Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            Json::Value refNode = *it, modelNode;

            // if path is there, it's a reference
            bool hasPathMember=refNode.isMember("path");
            if (hasPathMember)
            {
                Json::Value pathValue = refNode["path"];
                if (!pathValue.isString())
                {
                    Debug::warning(intro)("Could not read reference path \"")
                    (refNode["path"].toStyledString())("\" as string. Aborting").endl();
                    revertAgent = true;
                    break;
                }
                // load the referee
                Ogre::String path = "";
                resolveReferencePaths(pathValue.asString(), path);
                File file = mEngine->resourcesDir().subfile(path);
                if (!file.readInto(modelNode, false))
                {
                    Debug::warning(intro)("Could not read reference path \"")(path)("\". Aborting.");
                    revertAgent = true;
                    break;
                }

                // possibly add attributes overrides
                if (refNode.isMember("overrides"))
                {
                    Json::Value overrides = refNode["overrides"];
                    if (!overrides.isObject())
                    {
                        Debug::error(intro)("invalid overrides:")(overrides)(". Aborting").endl();
                        revertAgent = true;
                        break;
                    }
                    // actual overrides
                    JsonUtils::updateObjectWithOverrides(overrides, modelNode);
                }

            }

            if (INVALID_ID == aid)
            {
                Json::Value value = modelNode["modelType"];
                if (value.isNull() || !value.isString() || value.asString() != modelTypesAsString[MT_OGRE])
                {
                    Debug::error(intro)("serialization is not starting with a model of type MT_OGRE. Aborted.").endl();
                    revertAgent = true;
                    break;
                }
            }

            // should be done here too, as we may have newly loaded data
            if (!dynamicFillSerialization(modelNode, aid))
            {
                revertAgent = true;
                break;
            }

            if (!loadModelFromSerialization(modelNode, aid))
            {
                Debug::error(intro)("could not load models. Aborting.").endl();
                revertAgent = true;
                break;
            }

//        Ogre::String path;
//        resolveReferencePaths(value.asString(), path);
//        path = mEngine->resourcesDir().subfile(path).fullPath();
//        File file(path);
//        if (!instanciateResource(file, aid))
//        {
//            Debug::error(intro)("Could not resolve path \"")(path)("\". Skipping reference.").endl();
//            continue;
//        }
        }

        if (revertAgent)
        {
            if (INVALID_ID != aid)
            {
                level->deleteAgent(aid);
                aid = INVALID_ID;
            }
            return false;
        }
        return true;
    }

    void Editor::setupReferencePathsLookupTable(Ogre::String const& source)
    {
        Ogre::String intro = "Editor::setupReferencePaths(): ";
        Ogre::String outro = "Loading resource may not be possible.";
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(source.c_str(), root))
        {
            Debug::error(intro)("Could not parse lookup table:")(reader.getFormatedErrorMessages()).endl();
            Debug::error("Source was: ")(source).endl()(outro).endl();
            return;
        }

        if (!root.isObject() || root.isNull())
        {
            Debug::error(intro)("Table is invalid: ")(root).endl()(outro).endl();
            return;
        }

        mReferencePathsLookupTable.clear();
        Json::Value::Members members = root.getMemberNames();
        for (Json::Value::Members::iterator it = members.begin(); it != members.end(); ++it)
        {
            Json::Value value = root[*it];
            if (!value.isString())
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
        while (true)
        {
            Ogre::String save(dst);
            for (auto it = mReferencePathsLookupTable.begin(); it != mReferencePathsLookupTable.end(); ++it)
            {
                Ogre::String what = (*it).first;
                Ogre::String withWhat = (*it).second;
                dst.assign(Ogre::StringUtil::replaceAll(dst, what, withWhat));
            }
            if (save == dst)
                break;
        }
    }

    bool Editor::loadModelsFromSerializations(Json::Value& root, Steel::AgentId& aid)
    {
        Ogre::String intro = "Editor::loadModelsFromSerializations(): ";

        Level *level = mEngine->level();
        if (level == NULL)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return false;
        }

        Json::Value node = root["models"];
        if (!node.isArray() || node.isNull())
        {
            Debug::warning(intro)("\"models\" attribute is not valid (expecting a non-empty array).").endl();
            return false;
        }

        // instanciate all models
        for (Json::ValueIterator it = node.begin(); it != node.end(); ++it)
        {
            Json::Value modelNode = *it;

            if (INVALID_ID == aid)
            {
                Json::Value value = modelNode["modelType"];
                if (value.isNull() || value.asCString() != modelTypesAsString[MT_OGRE])
                {
                    Debug::error("serialization is not starting with an OgreModel as modelType. Aborted.").endl();
                    return false;
                }
            }

            if (!loadModelFromSerialization(modelNode, aid))
            {
                Debug::error(intro)("could not load models. Aborting.").endl();
                return false;
            }
        }
        return true;
    }

    bool Editor::loadModelFromSerialization(Json::Value& root, Steel::AgentId& aid)
    {
        Debug::log("Editor::loadModelFromSerialization(")(root)(")").endl();
        Ogre::String intro = "Editor::loadModelFromSerialization(): ";

        Level *level = mEngine->level();
        if (level == NULL)
        {
            Debug::error(intro)("no level to instanciate stuff in.").endl();
            return false;
        }

        Json::Value value = root["modelType"];
        if (value.isNull())
        {
            Debug::error("serialization is missing a \"modelType\" value. Aborted.").endl();
            return false;
        }

        Ogre::String modelTypeString = value.asString();

        // this is used to know it the agent should be deleted upon failure of the method
        bool fresh_aid = INVALID_ID == aid;
        if (fresh_aid)
        {
            //TODO #OPT: Agent *agent=level->newAgent(aid);
            aid = level->newAgent();
            if (INVALID_ID == aid)
            {
                Debug::error(intro)("could not create an agent to link the model to. Aborted.").endl();
                return false;
            }
            Debug::log(intro)("created agent ")(aid).endl();
        }

        // ask the right manager to load this model
        ModelType modelType = MT_FIRST;
        if (modelTypeString == "MT_OGRE")
            modelType = MT_OGRE;
        else if (modelTypeString == "MT_BT")
            modelType = MT_BT;
        else if (modelTypeString == "MT_PHYSICS")
            modelType = MT_PHYSICS;
        else
        {
            Debug::warning(intro)("Unknown model type: ")(modelTypeString).endl();
            return false;
        }

        // check if the agent is already linked to such a model
        Agent *agent=level->getAgent(aid);
        if(NULL==agent)
        {
            Debug::log(intro)("could not retrieve agent for id ")(aid).endl();
            return false;
        }
        if (INVALID_ID != agent->modelId(modelType))
        {
            Debug::error(intro)("cannot create a second ")(modelTypeString)(" Model to agent ")
            (aid)(". Skipping ")(modelTypeString)(" Model instanciation.").endl();
            return true; // skipped, not aborted
        }

        // try to instanciate the model
        intro.append("in ").append(modelTypeString).append(" type: ");
        auto manager = level->modelManager(modelType);
        if (NULL == manager)
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

        if (!level->linkAgentToModel(aid, modelType, mid))
        {
            level->modelManager(modelType)->decRef(mid);
            Debug::error(intro)("could not link agent ")(aid)(" to  ")(modelTypeString)(" Model ")
            (mid)(". Model released. Aborted.").endl();
            if (fresh_aid)
                level->deleteAgent(aid);
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
        if (level == NULL)
        {
            Debug::warning(intro + "): no level to instanciate stuff in.").endl();
            return INVALID_ID;
        }
//      Ogre::Quaternion r = mEngine->camera()->camNode()->getOrientation();
        Steel::ModelId mid = level->newOgreModel(meshFile.fileName(), pos, rot);
        AgentId aid = level->newAgent();
        if (!level->linkAgentToModel(aid, MT_OGRE, mid))
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
        if (NULL == level)
        {
            Debug::error(intro)("no level loaded. Aborted.").endl();
            return false;
        }

        if (root["slotPosition"].isNull())
        {
            Debug::error(intro)("can't find key \"slotPosition\". Aborted.").endl();
            return false;
        }

        TerrainManager::TerrainSlotData slot;
        level->terrainManager()->terrainSlotFromJson(root, slot);
        if (!slot.isValid())
        {
            Debug::error(intro)("TerrainManager::TerrainSlotData is not valid. Serialized string was:");
            Debug::error(root.toStyledString()).endl()("Aborted.").endl();
            return false;
        }

        level->terrainManager()->addTerrainSlot(slot);
        return true;
    }

    bool Editor::keyPressed(const OIS::KeyEvent& evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI->keyIdentifiers()[evt.key];
        mContext->ProcessKeyDown(keyIdentifier, mUI->getKeyModifierState());

        if (evt.text >= 32)
        {
            mContext->ProcessTextInput((Rocket::Core::word) evt.text);
        }
        else if (keyIdentifier == Rocket::Core::Input::KI_RETURN)
        {
            mContext->ProcessTextInput((Rocket::Core::word) '\n');
        }
        return true;
    }

    bool Editor::keyReleased(const OIS::KeyEvent& evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mUI->keyIdentifiers()[evt.key];
        mContext->ProcessKeyUp(keyIdentifier, mUI->getKeyModifierState());
        Level *level = mEngine->level();

        switch (evt.key)
        {
            case OIS::KC_H:
                mBrush.setMode(EditorBrush::TERRAFORM);
                break;
            case OIS::KC_R:
                if (mInputMan->isKeyDown(OIS::KC_LCONTROL))
                    reloadContent();
                else
                    mBrush.setMode(EditorBrush::ROTATE);
                break;
            case OIS::KC_S:
                if (mInputMan->isKeyDown(OIS::KC_LCONTROL))
                {
                    if (level != NULL)
                        level->save();
                    mEngine->saveConfig(mEngine->config());
                }
                else
                    mBrush.setMode(EditorBrush::SCALE);
                break;
            case OIS::KC_T:
                mBrush.setMode(EditorBrush::TRANSLATE);
                break;
            case OIS::KC_DELETE:
                mEngine->selectionMan().deleteSelection();
                break;
            default:
                break;
        }

        // numeric key pressed: handles tagging (keys: 1,2,3,...,0)
        if (NULL != mEngine && evt.key >= OIS::KC_1 && evt.key <= OIS::KC_0)
        {
            // OIS::KC_0 is the highest; the modulo makes it 0
            int tagKey = (evt.key - OIS::KC_1 + 1) % 10;
            Ogre::String sKey = Ogre::StringConverter::toString(tagKey);
            if (mInputMan->isKeyDown(OIS::KC_LCONTROL))
                mEngine->selectionMan().setSelectionTag(sKey);
            else
                mEngine->selectionMan().setTaggedSelection(sKey);
        }
        return true;
    }

    bool Editor::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        if (!hitTest(evt.state.X.abs, evt.state.Y.abs, "menu"))
            mBrush.mousePressed(evt, id);
        return true;
    }

    bool Editor::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        mBrush.mouseReleased(evt, id);
        if (id == OIS::MB_Right)
            mFSResources->expandRows();
        return true;
    }

    bool Editor::mouseMoved(const OIS::MouseEvent& evt)
    {
        bool hoveringMenu=hitTest(evt.state.X.abs, evt.state.Y.abs, "menu");
        if (!mIsDraggingFromMenu && !hoveringMenu)
        {
            mBrush.mouseMoved(evt);
            Rocket::Core::Element *elem;

            elem = mDocument->GetElementById("editor_terrabrush_intensity");
            if (elem != NULL)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.intensity()).c_str());

            elem = mDocument->GetElementById("editor_terrabrush_radius");
            if (elem != NULL)
                elem->SetInnerRML(Ogre::StringConverter::toString(mBrush.radius()).c_str());
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
        Ogre::String intro="Editor::onShow(): ";
        mBrush.onShow();
        // (re)load state
        // active menu tab
        auto elem = (Rocket::Controls::ElementTabSet *) mDocument->GetElementById("editor_tabset");
        if (NULL!=elem)
        {
            int tabNo=mEngine->config().getSettingAsInt(Editor::MENU_TAB_INDEX_SETTING, 0);
            elem->SetActiveTab(tabNo);
        }


        // ## reconnect to document

        // data sources
        mFSResources->refresh(mDocument);

        // events
        mDocument->AddEventListener("click", this);
        mDocument->AddEventListener("dragstart", this);
        mDocument->AddEventListener("dragdrop", this);
        mDocument->AddEventListener("change", this);
        mDocument->AddEventListener("submit", this);

        // form settings (can't be set through rml apparently)
        auto selection_serialization = static_cast<Rocket::Controls::ElementFormControlTextArea *>(mDocument->GetElementById("selection_serialization"));
        if(NULL!=selection_serialization)
        {
            // uh ? dk. Let's say 20 pix per character...
            //int n=elem->GetClientWidth()/20;
            selection_serialization->SetNumColumns(100);
            selection_serialization->SetNumRows(30);
        }
        else
        {
            Debug::warning(intro)("cannot find Rocket::Controls::ElementFormControlTextArea with id \"selection_serialization\".");
            Debug::warning("Editing models is not gonna be possible.").endl();
        }

        // debugger
        Rocket::Debugger::SetContext(mContext);
        Rocket::Debugger::SetVisible(true);

        // set brush shape
        auto select_form = static_cast<Rocket::Controls::ElementFormControlSelect *>(mDocument->GetElementById("editor_select_terrabrush_shape"));
        if (select_form != NULL and select_form->GetNumOptions() > 0)
        {
            int index = select_form->GetSelection();
            if (index < 0)
                select_form->SetSelection(0);
            else
                processCommand(Ogre::String("editorbrush.terrabrush.distribution.")+ select_form->GetOption(index)->GetValue().CString());
        }
    }

    void Editor::onHide()
    {
        if (mDocument)
        {
            mDocument->RemoveEventListener("click", this);
            mDocument->RemoveEventListener("dragdrop", this);
            mDocument->RemoveEventListener("change", this);
            mDocument->RemoveEventListener("submit", this);
            saveMenuTabIndexSetting(mEngine->config());
        }
        mBrush.onHide();
    }

    void Editor::ProcessEvent(Rocket::Core::Event &event)
    {
        if (!isVisible())
            return;
// create the command
        Rocket::Core::Element *elem = NULL;
// in case of drag&drop, elem points to the element being dragged
        if (event == "dragdrop")
        {
            // ok in stable, not in dev
            //             elem= static_cast<Rocket::Core::Element *>(event.GetParameter< void * >("drag_element", NULL));
            Rocket::Core::ElementReference *ref = static_cast<Rocket::Core::ElementReference *>(event.GetParameter<void *>(
                    "drag_element", NULL));
            elem = **ref;
            mIsDraggingFromMenu = false;
        }
        else if (event == "dragstart")
            mIsDraggingFromMenu = true;
        else
            elem = event.GetTargetElement();

        if (elem == NULL)
        {
            if (mDebugEvents)
                Debug::log("Editor::ProcessEvent(): no target element for event of type ")(event.GetType()).endl();
            return;
        }

        auto etype = event.GetType();
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + etype, "NoValue").CString();
        if (event_value == "NoValue")
        {
            if(elem->GetId() != "")
            {
                if (mDebugEvents)
                {
                    Debug::warning("Editor::ProcessEvent(): no event_value for event of type ");
                    Debug::warning(event.GetType())(" with elem of id ")(elem->GetId()).endl();
                }
                return;
            }
        }

        if(etype=="change")
            processChangeEvent(event,elem);
        else if(etype=="click")
            processClickEvent(event,elem);
        else if(etype=="dragdrop")
            processDragDropEvent(event,elem);
        else if(etype=="submit")
            processSubmitEvent(event,elem);
        else
            Debug::log("Editor::ProcessEvent(): unknown event ot type:")(event.GetType())(" and value: ")(event_value).endl();
        return;
    }

    void Editor::processSubmitEvent(Rocket::Core::Event& event, Rocket::Core::Element* elem)
    {
        Debug::log("Editor::processSubmitEvent(): type:")(event.GetType());
        Debug::log(" and value: ")(elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString());
        Debug::log.endl();
    }

    void Editor::processClickEvent(Rocket::Core::Event& event, Rocket::Core::Element* elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = event_value;
        if (rawCommand == "engine.set_level")
        {
            auto inputField = (Rocket::Controls::ElementFormControlInput *) mDocument->GetElementById("level_name");
            if (inputField == NULL)
            {
                Debug::error(
                    "Editor::ProcessEvent(): can't find level name input field with id=\"level_name\". Aborted.").endl();
                return;
            }
            Ogre::String levelName = inputField->GetValue().CString();
            if (levelName == "")
            {
                Debug::error("Editor::ProcessEvent(): can't create a level with not name. Aborted.").endl();
                return;
            }
            rawCommand += "." + levelName;
        }
        //         Debug::log("Editor::processClickEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if (rawCommand.size())
            processCommand(rawCommand);
    }

    void Editor::processChangeEvent(Rocket::Core::Event& event, Rocket::Core::Element* elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = event_value;
        if (rawCommand == "editorbrush.terrabrush.distribution")
        {
            Rocket::Controls::ElementFormControlSelect *form = static_cast<Rocket::Controls::ElementFormControlSelect *>(elem);
            auto optionId = form->GetSelection();
            if (optionId > -1)
            {
                rawCommand += ".";
                rawCommand += form->GetValue().CString();
            }
            else
                return;
        }
        //         Debug::log("Editor::processChangeEvent() event type:")(event.GetType())(" rawCommand:")(rawCommand).endl();
        if (rawCommand.size())
            processCommand(rawCommand);
    }

    void Editor::processDragDropEvent(Rocket::Core::Event& event, Rocket::Core::Element* elem)
    {
        Ogre::String event_value = elem->GetAttribute<Rocket::Core::String>("on" + event.GetType(), "NoValue").CString();
        Ogre::String rawCommand = "";
        if (elem->GetId() == mFSResources->GetDataSourceName())
        {
            rawCommand = "instanciate.";
            File file = File(event_value);
            if (!file.exists())
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
        if (rawCommand.size())
            processCommand(rawCommand);
    }

    void Editor::processCommand(Ogre::String rawCommand)
    {
        if("NoValue" == rawCommand)
            return;
        Debug::log("Editor::processCommand(raw=")(rawCommand)(")").endl();
        processCommand(StringUtils::split(std::string(rawCommand), std::string(".")));
    }

    void Editor::processCommand(std::vector<Ogre::String> command)
    {
        Ogre::String intro = "in Editor::processCommand(): ";
// dispatch the command to the right subprocessing function
        if (command[0] == "editor")
        {
            command.erase(command.begin());
            if (command[0] == "hide")
                mEngine->stopEditMode();
        }
        else if (command[0] == "editorbrush")
        {
            command.erase(command.begin());
            mBrush.processCommand(command);
        }
        else if (command[0] == "engine")
        {
            command.erase(command.begin());
            mEngine->processCommand(command);
        }
        else if (command[0] == "instanciate")
        {
            intro += "instanciate: ";
            command.erase(command.begin());
            if (command.size() < 1)
            {
                Debug::error(intro)("command \"")(command);
                Debug::error("\" contains no file !").endl();
                return;
            }
            File file(StringUtils::join(command, "."));
            if (file.exists())
            {
                instanciateResource(file);
            }
            else
            {
                Debug::warning(intro)("file \"")(file)("\" not found for command \"");
                Debug::warning(command)("\". Aborted.").endl();
            }
        }
        else if (command[0] == "options")
        {
            intro += "options: ";
            command.erase(command.begin());
            if (command.size() == 0)
                Debug::warning(intro)("no option given.").endl();
            else
                processOptionCommand(command);
        }
        else if (command[0] == "resourceGroupsInfos")
            OgreUtils::resourceGroupsInfos();
        else
            Debug::warning(intro)("unknown command: \"")(command)("\".").endl();
    }

    void Editor::processOptionCommand(std::vector<Ogre::String> command)
    {
        if (command.size() == 0)
            return;
        if (command[0] == "switch_debug_events")
        {
            mDebugEvents = !mDebugEvents;
            Debug::log("flag DebugEvent ")(mDebugEvents ? "activated" : "deactivated").endl();
        }
        else
            Debug::warning("Editor::processOptionCommand(): unknown command: ")(command).endl();

    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

