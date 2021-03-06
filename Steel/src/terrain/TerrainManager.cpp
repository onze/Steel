#include "terrain/TerrainManager.h"

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreRoot.h>
#include <OgreImage.h>

#include "steeltypes.h"
#include "Debug.h"
#include "terrain/TerrainManagerEventListener.h"
#include "terrain/TerrainPhysicsManager.h"
// #include "terrain/TerrainMaterialGenerator.h"
#include "tools/JsonUtils.h"
#include "tools/StringUtils.h"
#include "Level.h"

namespace Steel
{

    TerrainManager::TerrainManager(): Ogre::FrameListener(),
        mLevel(nullptr), mSceneManager(nullptr), mResourceGroupName("TerrainManager-defaultResourceGroup-name"),
        mLoadingState(LoadingState::INIT), mListeners(std::set<TerrainManagerEventListener *>()),
        mTerrainGlobals(nullptr), mTerrainGroup(nullptr), mTerrainsImported(false),
        mPath(StringUtils::BLANK), mTerrainPhysicsMan(nullptr)
        //, mTerrainMaterialGenerator(nullptr)
    {
    }

    TerrainManager::TerrainManager(const TerrainManager &o):
        mLevel(o.mLevel), mSceneManager(o.mSceneManager), mResourceGroupName(o.mResourceGroupName),
        mLoadingState(o.mLoadingState), mListeners(o.mListeners),
        mTerrainGlobals(o.mTerrainGlobals), mTerrainGroup(o.mTerrainGroup), mTerrainsImported(o.mTerrainsImported),
        mPath(o.mPath), mTerrainPhysicsMan(o.mTerrainPhysicsMan)
        //, mTerrainMaterialGenerator(new TerrainMaterialGenerator(*o.mTerrainMaterialGenerator))
    {
    }

    TerrainManager::~TerrainManager()
    {
        shutdown();
    }

    TerrainManager &TerrainManager::operator=(const TerrainManager &o)
    {
        Debug::error("TerrainManager::operator=(const TerrainManager &o) no implemeted").endl().breakHere();
        return *this;
    }

    bool TerrainManager::operator==(const TerrainManager &o) const
    {
        Debug::error("TerrainManager::operator==(const TerrainManager &o) no implemeted").endl().breakHere();
        return false;
    }

    void TerrainManager::addTerrainManagerEventListener(TerrainManagerEventListener *listener)
    {
        mListeners.insert(listener);
    }

    void TerrainManager::removeTerrainManagerEventListener(TerrainManagerEventListener *listener)
    {
        auto it = std::find(mListeners.begin(), mListeners.end(), listener);

        if(it != mListeners.end())
        {
            assert(listener == *it);
            mListeners.erase(*it);
        }
    }

    void TerrainManager::yieldEvent(LoadingState state)
    {
        auto copy = std::vector<TerrainManagerEventListener *>(mListeners.begin(), mListeners.end());

        for(auto it = copy.begin(); copy.size() > 0 && it != copy.end(); ++it)
        {
            (*it)->onTerrainEvent(state);
        }
    }

    void TerrainManager::shutdown()
    {
//         if(nullptr != mTerrainMaterialGenerator)
//         {
//             delete mTerrainMaterialGenerator;
//             mTerrainMaterialGenerator = nullptr;
//         }

        if(nullptr != mTerrainPhysicsMan)
        {
            delete mTerrainPhysicsMan;
            mTerrainPhysicsMan = nullptr;
        }

        if(nullptr != mTerrainGroup)
        {
            if(mTerrainGroup->isDerivedDataUpdateInProgress())
            {
                // no way to query the channel ? hardcode it.
                Ogre::WorkQueue *wq = Ogre::Root::getSingleton().getWorkQueue();
                auto mWorkQueueChannel = wq->getChannel("Ogre/Terrain");
                wq->abortRequestsByChannel(mTerrainGroup->WORKQUEUE_LOAD_REQUEST);
                wq->removeResponseHandler(mWorkQueueChannel, mTerrainGroup);
                wq->removeRequestHandler(mWorkQueueChannel, mTerrainGroup);

                Ogre::TerrainGroup::TerrainIterator it = mTerrainGroup->getTerrainIterator();

                while(it.hasMoreElements())
                {
                    Ogre::Terrain *terrain = it.getNext()->instance;

                    if(nullptr == terrain)
                        continue;

                    if(terrain->isDerivedDataUpdateInProgress())
                    {
                        wq->abortRequestsByChannel(terrain->WORKQUEUE_DERIVED_DATA_REQUEST);
                        wq->removeResponseHandler(terrain->WORKQUEUE_DERIVED_DATA_REQUEST, terrain);
                        wq->removeRequestHandler(terrain->WORKQUEUE_DERIVED_DATA_REQUEST, terrain);
                    }

                    terrain->unload();
                }
            }

            mTerrainGroup->removeAllTerrains();
            OGRE_DELETE mTerrainGroup;
            mTerrainGroup = nullptr;
        }

        Ogre::Root::getSingletonPtr()->removeFrameListener(this);

        mTerrainGlobals = nullptr;
        mSceneManager = nullptr;
        auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();

        if(rgm->resourceGroupExists(mResourceGroupName))
        {
            rgm->destroyResourceGroup(mResourceGroupName);
            // that's my good debug value !
            mResourceGroupName = "unloaded-terrainManager (previously" + mResourceGroupName + ")";
        }
    }

    void TerrainManager::init(Level *level, Ogre::String resourceGroupName, File path, Ogre::SceneManager *sceneManager)
    {
        mSceneManager = sceneManager;
        mPath = path;
        mResourceGroupName = resourceGroupName;
        mLevel = level;

        mTerrainsImported = false;
        mLoadingState = LoadingState::INIT;

        mTerrainGlobals = Ogre::TerrainGlobalOptions::getSingletonPtr();


//         if(nullptr == mTerrainMaterialGenerator)
//         {
//             Ogre::String mDefaultTerrainMaterialName("triPlanarMaterial1");
//             mTerrainMaterialGenerator = new TerrainMaterialGenerator(mDefaultTerrainMaterialName);
//         }

        if(nullptr == mTerrainGlobals)
            mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();

        mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneManager,
                        Ogre::Terrain::ALIGN_X_Z,
                        DEFAULT_TERRAIN_SIZE,
                        DEFAULT_WORLD_SIZE);
        auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();
        rgm->createResourceGroup(resourceGroupName);
        rgm->addResourceLocation(path.fullPath(), "FileSystem", resourceGroupName);
        rgm->addResourceLocation(path.subfile("build").fullPath(), "FileSystem", resourceGroupName);
        mTerrainGroup->setResourceGroup(resourceGroupName);

        File buildPath = mPath.subfile("build");

        if(!buildPath.exists())
            buildPath.mkdir();

        mTerrainGroup->setFilenameConvention(buildPath.subfile("terrain").fullPath(), Ogre::String("terrain"));
        mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);

        mTerrainPhysicsMan = new TerrainPhysicsManager(this);

        // default terrain
//         Ogre::ColourValue ambient=Ogre::ColourValue::White;
//         Ogre::Vector3 lightDir(0.55, -0.3, 0.75);
//         Ogre::ColourValue diffuseColor=Ogre::ColourValue::White;
//         Ogre::ColourValue specularColor(0.4, 0.4, 0.4);
//         std::list<TerrainSlotData> terrainSlots;
//         terrainSlots.push_back(TerrainSlotData(0L,0L));
//         build(ambient,lightDir,diffuseColor,specularColor,terrainSlots);
    }

    Json::Value TerrainManager::toJson()
    {
        Json::Value root;

        // general level settings
        // "levelLight", as defined in Level::Level
        Ogre::Light *light = mSceneManager->getLight("levelLight");
        root["ambientLight"] = JsonUtils::toJson(mSceneManager->getAmbientLight());
        root["lightDir"] = JsonUtils::toJson(light->getDirection());
        root["diffuseColor"] = JsonUtils::toJson(light->getDiffuseColour());
        root["specularColor"] = JsonUtils::toJson(light->getSpecularColour());

        // defaults terrain data
        Json::Value defaultsValue;
        Ogre::Terrain::ImportData &defaults = mTerrainGroup->getDefaultImportSettings();
        defaultsValue["terrainSize"] = JsonUtils::toJson(defaults.terrainSize);
        defaultsValue["worldSize"] = JsonUtils::toJson(defaults.worldSize);

        for(auto it = defaults.layerList.begin(); it != defaults.layerList.end(); ++it)
        {
            Ogre::Terrain::LayerInstance layer = *it;
            Json::Value layerValue;
            layerValue["worldSize"] = layer.worldSize;

            for(auto it_textureNames = layer.textureNames.begin(); it_textureNames != layer.textureNames.end(); ++it_textureNames)
                layerValue["textureNames"].append(Json::Value((*it_textureNames).c_str()));

            defaultsValue["layerList"].append(layerValue);
        }

        root["defaultTerrain"] = defaultsValue;

        // actual terrains
        Ogre::TerrainGroup::TerrainIterator it = mTerrainGroup->getTerrainIterator();

        while(it.hasMoreElements())
        {
            Ogre::TerrainGroup::TerrainSlot *slot = it.getNext();
            Ogre::Terrain *terrain = slot->instance;

            if(nullptr == terrain)
            {
                // all slots should have a terrain attached to them.
                Debug::error("TerrainManager::toJson(): slot ")(slot->x)("x")(slot->y)(
                    " has no terrain defined ! Skipping slot.").endl();
                continue;
            }

            Json::Value terrainValue;
            terrainValue["slotPosition"] = JsonUtils::toJson(Ogre::Vector2(static_cast<float>(slot->x), static_cast<float>(slot->y)));

            Ogre::String heightmapPath = StringUtils::BLANK;
            saveTerrainHeightmapAs(slot->x, slot->y, terrain, heightmapPath);
            terrainValue["heightmapPath"] = heightmapPath;

            terrainValue["size"] = JsonUtils::toJson(terrain->getSize());
            terrainValue["worldSize"] = JsonUtils::toJson(terrain->getWorldSize());

            serializeLayerList(terrain, terrainValue["layerList"]);

            root["terrainSlots"].append(terrainValue);
        }

        return root;
    }

    void TerrainManager::saveTerrainHeightmapAs(long int x, long int y, Ogre::Terrain *instance,
            Ogre::String &heightmapPath)
    {
        // get data
        float *heights_init = instance->getHeightData();
        size_t side = instance->getSize();

        // heightmap is 16bpp greyscale
        short *heights = new short[side * side];

        for(size_t i = 0; i < side * side; ++i)
            heights[i] = static_cast<short>(heights_init[i]);

        Ogre::DataStreamPtr streamPtr(OGRE_NEW Ogre::MemoryDataStream(heights, side * side * sizeof(short)));

        // make it an image for easy saving
        Ogre::Image img;
        img.loadRawData(streamPtr, side, side, 1, Ogre::PixelFormat::PF_SHORT_L);
        Ogre::String filename = "heightmap_" + Ogre::StringConverter::toString(x) + "_" + Ogre::StringConverter::toString(y) + ".png";
        heightmapPath = mPath.subfile(filename).fullPath();
        img.save(heightmapPath);
    }

    float *TerrainManager::loadTerrainHeightmapFrom(Ogre::String filepath, int size)
    {
        Ogre::String intro = "TerrainManager::loadTerrainHeightmapFrom(" + filepath + "): ";
        Ogre::Image img;
        short *img_data = 0;
        int resolution = size * size;
        bool use_file = File(filepath).exists();

        if(use_file)
        {
            img.load(filepath, mTerrainGroup->getResourceGroup());
            resolution = img.getWidth() * img.getHeight();
            Debug::log(intro)("format is ")(Ogre::PixelUtil::getFormatName(img.getFormat()));
            Debug::log(" depth: ")(img.getDepth());
            Debug::log(" resolution: ")(img.getWidth())("x")(img.getHeight()).endl();
            img_data = reinterpret_cast<short *>(img.getData());
        }
        else
        {
            if(filepath != StringUtils::BLANK)
                Debug::error(intro)("file defined but not found, using height 0.").endl();

            img_data = new short[resolution];
            std::fill(img_data, img_data + resolution, 1);
        }

        //float *heights = new float[resolution];
        // created here and given to ogre. yerk
        float *heights = OGRE_ALLOC_T(float, resolution, Ogre::MEMCATEGORY_GEOMETRY);

        for(int i = 0; i < resolution; ++i)
            heights[i] = static_cast<float>(img_data[i]);

        if(!use_file)
            delete img_data;

        return heights;
    }

    void TerrainManager::serializeLayerList(Ogre::Terrain *terrain, Json::Value &layerListValue)
    {
        for(unsigned int layerIndex = 0; layerIndex < terrain->getLayerCount(); ++layerIndex)
        {
            Json::Value layerValue;
            layerValue["worldSize"] = terrain->getLayerWorldSize(layerIndex);
            Ogre::TerrainLayerSamplerList samplers = terrain->getLayerDeclaration().samplers;

            for(unsigned int samplerIndex = 0; samplerIndex < samplers.size(); ++samplerIndex)
                layerValue["textureNames"].append(JsonUtils::toJson(terrain->getLayerTextureName(layerIndex, samplerIndex)));

            layerListValue.append(layerValue);
        }
    }

    bool TerrainManager::deserializeLayerList(const Json::Value &layersValue, Ogre::Terrain::LayerInstanceList &layerList)
    {
        if(layersValue.isNull())
            Debug::warning(STEEL_METH_INTRO, "layersValue is null !").endl();

        if(!layersValue.isConvertibleTo(Json::arrayValue))
        {
            Debug::warning(STEEL_METH_INTRO, "can't convert layersValue to array. Using default.").endl();
            return false;
        }
        else
        {
            Json::Value value;

            for(Json::ArrayIndex layerIndex = 0; layersValue.isValidIndex(layerIndex); ++layerIndex)
            {
                Json::Value layerValue = layersValue.get(layerIndex, Json::nullValue);

                value = layerValue["worldSize"];
                Ogre::Real worldSize = -1.f;

                if(value.isNull())
                {
                    Debug::warning(STEEL_METH_INTRO, "key 'layerList[", layerIndex, "].worldSize' is null. Skipping.").endl();
                    continue;
                }

                worldSize = value.asFloat();

                if(-1 == worldSize)
                {
                    Debug::warning(STEEL_METH_INTRO, "could not parse key 'layerList[", layerIndex, "].worldSize' (", value, "). Skipping.").endl();
                    continue;
                }

                Json::Value textureNamesValue = layerValue["textureNames"];
                std::vector<Ogre::String> textureNames;

                if(textureNamesValue.isNull())
                {
                    Debug::warning(STEEL_METH_INTRO, "key 'layerList[", layerIndex, "].textureNames' is null. Skipping.").endl();
                    continue;
                }

                if(!textureNamesValue.isConvertibleTo(Json::arrayValue))
                {
                    Debug::warning(STEEL_METH_INTRO, "key 'layerList[", layerIndex, "].textureNames' is not an Array (", textureNamesValue, "). Skipping.").endl();
                    continue;
                }

                Json::ValueIterator it_textureNames = textureNamesValue.begin();

                for(; it_textureNames != textureNamesValue.end(); ++it_textureNames)
                {
                    textureNames.push_back((*it_textureNames).asString());
                }

                Ogre::Terrain::LayerInstance instance;
                instance.worldSize = worldSize;
                instance.textureNames.assign(textureNames.begin(), textureNames.end());
                layerList.push_back(instance);

                // hardcoded
                if(0)
                {
                    layerList[0].textureNames.resize(3);
                    layerList[0].worldSize = 10;
                    layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.png");
                    layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.png");

                    layerList[1].worldSize = 3;
                    layerList[1].textureNames.push_back("grass_green-01_diffusespecular.png");
                    layerList[1].textureNames.push_back("grass_green-01_normalheight.png");

                    layerList[2].worldSize = 20;
                    layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.png");
                    layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.png");
                }
            }
        }

        // add default layer if still none has been defined so far
        if(layerList.size() == 0)
        {
            Ogre::String defaultTexturePath = "default_terrain_texture.png";
            Debug::log(STEEL_METH_INTRO, "adding default layer ").quotes(defaultTexturePath)(" to layerList.").endl();
            Ogre::Terrain::LayerInstance instance;
            instance.worldSize = 2;
            instance.textureNames.push_back(defaultTexturePath);
            layerList.push_back(instance);
        }

        return true;
    }

    bool TerrainManager::fromJson(Json::Value &root)
    {
        Ogre::String intro = "TerrainManager::fromJSon(): ";
        Json::Value value;

        //         Ogre::Vector3 lightDir(.2f, -.5f, .3f);

        Ogre::ColourValue ambient = Ogre::ColourValue::White;
        value = root["ambientLight"];

        if(!value.isNull())
            ambient = Ogre::StringConverter::parseColourValue(value.asString());

        mSceneManager->setAmbientLight(ambient);

        Ogre::Vector3 lightDir(0.55, -0.3, 0.75);
        value = root["lightDir"];

        if(value.isNull())
            Debug::warning(intro)("key 'lightDir' is null. Using default.").endl();
        else
            lightDir = Ogre::StringConverter::parseVector3(value.asString());

        //TODO: move light management to a dedicated LightManager
        Ogre::ColourValue diffuseColor = Ogre::ColourValue::White;
        value = root["diffuseColor"];

        if(value.isNull())
            Debug::warning(intro)("key 'diffuseColor' is null. Using default.").endl();
        else
            diffuseColor = Ogre::StringConverter::parseColourValue(value.asString());

        Ogre::ColourValue specularColor(0.4, 0.4, 0.4);
        value = root["specularColor"];

        if(value.isNull())
            Debug::warning(intro)("key 'specularColor' is null. Using default.").endl();
        else
            specularColor = Ogre::StringConverter::parseColourValue(value.asString());

        Ogre::Terrain::ImportData defaultImp;
        defaultImp.terrainSize = TerrainManager::DEFAULT_TERRAIN_SIZE;
        defaultImp.worldSize = TerrainManager::DEFAULT_WORLD_SIZE;
        defaultImp.maxBatchSize = 65;
        defaultImp.minBatchSize = 33;
        defaultImp.inputScale = 1.f;
        defaultImp.inputBias = .0f;
        Json::Value defaultTerrain = root["defaultTerrain"];

        if(defaultTerrain.isNull())
            Debug::warning(intro)("key 'defaultTerrain' is null. Using default.").endl();
        else
        {
            value = defaultTerrain["terrainSize"];

            if(value.isNull())
                Debug::warning(intro)("key 'defaultTerrain.terrainSize' is null. Using default.").endl();
            else
                defaultImp.terrainSize = Ogre::StringConverter::parseInt(value.asString(), -1);

            if(-1 == defaultImp.terrainSize)
            {
                Debug::warning(intro)("Could not parse key 'defaultTerrain.terrainSize' (")(value)("). Using default.").endl();
                defaultImp.terrainSize = TerrainManager::DEFAULT_TERRAIN_SIZE;
            }

            value = defaultTerrain["worldSize"];

            if(value.isNull())
                Debug::warning(intro)("key 'defaultTerrain.worldSize' is null. Using default.").endl();
            else
                defaultImp.worldSize = Ogre::StringConverter::parseInt(value.asString(), -1);

            if(-1 == defaultImp.worldSize)
            {
                Debug::warning(intro)("Could not parse key 'defaultTerrain.worldSize' (")(value)("). Using default.").endl();
                defaultImp.worldSize = TerrainManager::DEFAULT_WORLD_SIZE;
            }

            defaultImp.layerList.clear();
            deserializeLayerList(defaultTerrain["layerList"], defaultImp.layerList);
        }

        // terrains
        std::list<TerrainSlotData> terrainSlots;
        Json::Value terrainSlotsValue = root["terrainSlots"];
        bool valid = false;

        if(value.isNull())
            Debug::warning(intro)("key 'terrainSlots' is null.").endl();
        else if(!terrainSlotsValue.isConvertibleTo(Json::arrayValue))
            Debug::warning(intro)("key 'terrainSlots' can't be converted to an array.").endl();
        else
        {
            Json::ValueIterator it = terrainSlotsValue.begin();

            for(; it != terrainSlotsValue.end(); ++it)
            {
                Json::Value terrainSlotValue = *it;
                TerrainSlotData terrainSlot;
                terrainSlotFromJson(terrainSlotValue, terrainSlot);

                if(terrainSlot.isValid())
                    terrainSlots.push_back(terrainSlot);
                else
                {
                    Debug::error("invalid terrainSlot:").endl()(terrainSlotValue.toStyledString()).endl();
                    break;
                }
            }

            // keep last in this block
            if(it == terrainSlotsValue.end())
                valid = true;
        }

        if(!valid)
        {
            Debug::warning("key 'terrainSlots':").endl()(root["terrainSlots"].toStyledString()).endl();
        }

        build(ambient, lightDir, diffuseColor, specularColor, defaultImp, terrainSlots);
        return true;
    }

    void TerrainManager::addTerrainSlot(TerrainManager::TerrainSlotData slot)
    {
        defineTerrain(slot);
        updateTerrains();
    }

    bool TerrainManager::terrainSlotFromJson(Json::Value &terrainSlotValue,
            TerrainManager::TerrainSlotData &terrainSlot)
    {
        Ogre::Vector2 slotPosition = Ogre::Vector2::ZERO;

        if(!terrainSlotValue["slotPosition"].isNull())
            slotPosition = JsonUtils::asVector2(terrainSlotValue["slotPosition"]);
        else
        {
            Debug::error(STEEL_METH_INTRO, "key ").quotes("slotPosition")(" not found in resource. Aborting. Serialization:").endl()(terrainSlotValue).endl();
            return false;
        }

        terrainSlot.slot_x = static_cast<long int>(slotPosition.x);
        terrainSlot.slot_y = static_cast<long int>(slotPosition.y);
        terrainSlot.heightmapPath = terrainSlotValue["heightmapPath"].asString().c_str();

        terrainSlot.size = Ogre::StringConverter::parseInt(terrainSlotValue["terrainSize"].asString().c_str());

        if(terrainSlot.size == 0)
            terrainSlot.size = TerrainManager::DEFAULT_TERRAIN_SIZE;

        terrainSlot.worldSize = Ogre::StringConverter::parseReal(terrainSlotValue["worldSize"].asString().c_str());

        if(terrainSlot.worldSize == .0f)
            terrainSlot.worldSize = TerrainManager::DEFAULT_WORLD_SIZE;

        deserializeLayerList(terrainSlotValue["layerList"], terrainSlot.layerList);
        return true;
    }

    void TerrainManager::build(Ogre::ColourValue ambient, Ogre::Vector3 lightDir,
                               Ogre::ColourValue diffuseColor, Ogre::ColourValue specularColor,
                               Ogre::Terrain::ImportData defaultImp, std::list<TerrainSlotData> &terrainSlots)
    {

        mLoadingState = LoadingState::INIT;

        // setup directional light for the terrain manager
        // TODO: encapsulate into a lightManager (need light models ?)
        lightDir.normalise();
        // keep name in sync with destructor/shutdown
        mSceneManager->setAmbientLight(ambient);
        // "levelLight", as defined in Level::Level
        Ogre::Light *light = mSceneManager->getLight("levelLight");
        light->setType(Ogre::Light::LT_DIRECTIONAL);
        light->setDirection(lightDir);
        light->setDiffuseColour(diffuseColor);
        light->setSpecularColour(specularColor);
        configureTerrainDefaults(light, defaultImp);

        if(terrainSlots.size() == 0)
            Debug::warning("TerrainManager::build(): terrainSlots has size 0. Level has no terrain !").endl();
        else
            for(auto it = terrainSlots.begin(); it != terrainSlots.end(); ++it)
                defineTerrain(*it);

        updateTerrains();
    }

    void TerrainManager::defineTerrain(TerrainSlotData &terrainSlotData)
    {
        long x = terrainSlotData.slot_x;
        long y = terrainSlotData.slot_y;
        Ogre::String filename = mTerrainGroup->generateFilename(x, y);

        if(Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
        {
            mTerrainGroup->defineTerrain(x, y);
        }
        else
        {
            Ogre::Terrain::ImportData idata;
            idata.inputFloat = loadTerrainHeightmapFrom(terrainSlotData.heightmapPath, terrainSlotData.size);
            // hence the OGRE_ALLOC_T allocation
            idata.deleteInputData = true;

            idata.inputBias = 0.f;
            idata.terrainSize = terrainSlotData.size;
            idata.worldSize = terrainSlotData.worldSize;
            // as for now, we only use defaults for those values
            idata.layerDeclaration = mTerrainGroup->getDefaultImportSettings().layerDeclaration;
            idata.layerList.assign(terrainSlotData.layerList.begin(), terrainSlotData.layerList.end());

            if(mTerrainGroup->getTerrain(x, y) != nullptr)
                mTerrainGroup->removeTerrain(x, y);

            mTerrainGroup->defineTerrain(x, y, &idata);
            mTerrainsImported = true;
        }
    }

    void TerrainManager::configureTerrainDefaults(Ogre::Light *light, Ogre::Terrain::ImportData &newDefault)
    {
        // MaxPixelError decides how precise our terrain is going to be.
        // A lower number will mean a more accurate terrain, at the cost of performance (because of more vertices).
        mTerrainGlobals->setMaxPixelError(8);
        // CompositeMapDistance decides how far the Ogre terrain will render the lightmapped terrain
        mTerrainGlobals->setCompositeMapDistance(500);

        // Important to set these so that the terrain knows what to use for derived (non-realtime) data
        mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
        mTerrainGlobals->setCompositeMapAmbient(mSceneManager->getAmbientLight());
        mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

        // custom material
//         mTerrainGlobals->setDefaultMaterialGenerator(Ogre::TerrainMaterialGeneratorPtr(mTerrainMaterialGenerator));

        // Configure default import settings for if we use imported image
        Ogre::Terrain::ImportData &defaultimp = mTerrainGroup->getDefaultImportSettings();
        defaultimp.terrainSize = newDefault.terrainSize;
        // 1 unit == 1 meter
        defaultimp.worldSize = newDefault.worldSize;
        // due terrain.png is 16 bpp (a float)
        defaultimp.inputScale = newDefault.inputScale;
        defaultimp.minBatchSize = newDefault.minBatchSize;
        defaultimp.maxBatchSize = newDefault.maxBatchSize;

        // textures
        defaultimp.layerList.clear();
        defaultimp.layerList.assign(newDefault.layerList.begin(), newDefault.layerList.end());
    }

    void TerrainManager::update(float timestep)
    {
        mTerrainPhysicsMan->update(timestep);
    }

    void TerrainManager::updateTerrains()
    {
        mLoadingState = LoadingState::INIT;
        Ogre::Root::getSingletonPtr()->addFrameListener(this);
        // sync load since we want everything in place when we start
        mTerrainGroup->loadAllTerrains(true);

        if(mTerrainsImported)
        {
            Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();

            while(ti.hasMoreElements())
            {
                Ogre::Terrain *terrain = ti.getNext()->instance;
                updateBlendMaps(terrain);

                if(nullptr == mTerrainPhysicsMan->getTerrainFor(terrain))
                    mTerrainPhysicsMan->createTerrainFor(terrain);
            }
        }

        mTerrainGroup->freeTemporaryResources();
    }

    void TerrainManager::updateBlendMaps(Ogre::Terrain *terrain)
    {
        if(terrain->getLayerCount() < 3)
        {
            // steel terrain
//             Ogre::MaterialPtr mat = terrain->getCompositeMapMaterial();
//             Ogre::Pass *pass = mat->getTechnique(0)->getPass(0);
//             Ogre::Pass *pass = mat->getTechnique(0)->createPass();
//             Ogre::TextureUnitState *tus = pass->createTextureUnitState();
//             tus->setTextureName(mat->getTechnique(0)->getPass(0)->getTextureUnitState(0)->getTextureName());
//             pass->setVertexProgram("triPlanarMaterial1_vs");
//             pass->setFragmentProgram("triPlanarMaterial1_ps");
//             mat->_dirtyState();
        }
        else
        {
            // prebaked blended terrains

            //TODO: load this through serialization (no editing included, but fast reload would be nice)
            Ogre::TerrainLayerBlendMap *blendMap0 = terrain->getLayerBlendMap(1);
            Ogre::TerrainLayerBlendMap *blendMap1 = terrain->getLayerBlendMap(2);
            Ogre::Real minHeight0 = 15;
            Ogre::Real fadeDist0 = 10;
            Ogre::Real minHeight1 = 15;
            Ogre::Real fadeDist1 = 0;
            float *pBlend0 = blendMap0->getBlendPointer();
            float *pBlend1 = blendMap1->getBlendPointer();

            for(Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
            {
                for(Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
                {
                    Ogre::Real tx, ty;

                    blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
                    Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
                    Ogre::Real val = (height - minHeight0) / fadeDist0;
                    val = Ogre::Math::Clamp(val, (Ogre::Real) 0, (Ogre::Real) 1);
                    *pBlend0++ = val;

                    val = (height - minHeight1) / fadeDist1;
                    val = Ogre::Math::Clamp(val, (Ogre::Real) 0, (Ogre::Real) 1);
                    *pBlend1++ = val;
                }
            }

            blendMap0->dirty();
            blendMap1->dirty();
            blendMap0->update();
            blendMap1->update();
        }
    }

    void TerrainManager::updateHeightmap(Ogre::Terrain *terrain)
    {
        mTerrainPhysicsMan->updateHeightmap(terrain);
    }

    bool TerrainManager::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        bool verbose = false;
        bool updateInProgress = mTerrainGroup->isDerivedDataUpdateInProgress();

        switch(mLoadingState)
        {
            case LoadingState::INIT:
                if(updateInProgress)
                {
                    if(mTerrainsImported)
                    {
                        mLoadingState = LoadingState::BUILDING;

                        if(verbose)
                            Debug::log("TerrainManager now in BUILDING state.").endl();

                        yieldEvent(mLoadingState);
                    }
                    else
                    {
                        // done importing
                        mLoadingState = LoadingState::TEXTURING;

                        if(verbose)
                            Debug::log("TerrainManager now in TEXTURING state.").endl();

                        yieldEvent(mLoadingState);
                    }
                }
                else
                    mLoadingState = LoadingState::SAVING;

                break;

            case LoadingState::BUILDING:
                // geometry operations are synchronous, no need to wait.
                mLoadingState = LoadingState::TEXTURING;

                if(verbose)
                    Debug::log("TerrainManager now in TEXTURING state.").endl();

                yieldEvent(mLoadingState);
                break;

            case LoadingState::TEXTURING:
                if(!updateInProgress)
                {
                    //done texturing
                    mLoadingState = LoadingState::SAVING;

                    if(verbose)
                        Debug::log("TerrainManager now in SAVING state.").endl();

                    yieldEvent(mLoadingState);
                }

                break;

            case LoadingState::SAVING:
                // TODO:keep fast caching, add invalidate feature
                //mTerrainGroup->saveAllTerrains(true);
                mLoadingState = LoadingState::READY;

                if(verbose)
                    Debug::log("TerrainManager now in READY state.").endl();

                yieldEvent(mLoadingState);
                break;

            case LoadingState::READY:
                Ogre::Root::getSingletonPtr()->removeFrameListener(this);
                break;

            default:
                Debug::error("hit default case in TerrainManager::frameRenderingQueued()").endl();
        }

        // continue rendering
        return true;
    }

    Ogre::TerrainGroup::RayResult TerrainManager::intersectRay(const Ogre::Ray &ray)
    {
        Ogre::TerrainGroup::RayResult result(false, nullptr, Ogre::Vector3::ZERO);

        if(mTerrainGroup != nullptr)
        {
            result = mTerrainGroup->rayIntersects(ray);
            result.terrain = nullptr;
        }

        return result;
    }

    Ogre::TerrainGroup::TerrainList TerrainManager::raiseTerrainAt(Ogre::Vector3 terraCenter,
            Ogre::Real intensity,
            Ogre::Real radius,
            TerrainManager::RaiseMode rmode,
            TerrainManager::RaiseShape rshape)
    {
        std::set<Ogre::Vector2> tIds;
        //         Debug::log("TerrainManager::raiseTerrainAt(terraCenter=")(terraCenter)(", intensity=")(intensity)(", radius=")(radius)(")").endl();

        // retrieve a list of all slots involved in the operation. This is done by getting all slots
        // colliding with the terraforming shape's bounding box
        auto diff = Ogre::Vector3::UNIT_SCALE * radius;
        Ogre::AxisAlignedBox box(terraCenter - diff, terraCenter + diff);
        Ogre::TerrainGroup::TerrainList terrains;
        mTerrainGroup->boxIntersects(box, &terrains);

        // now, for each of them, get all vertices in the sphere and move them up/down.
        // for reference, we work as much as possible in local coordinates
        Ogre::Vector2 terraPos(terraCenter.x, terraCenter.z);

        for(auto it = terrains.begin(); it != terrains.end(); ++it)
        {
            Ogre::Terrain *terrain = *it;
            Ogre::uint16 size = terrain->getSize();

            Ogre::Vector3 localTerraCenter3;
            terrain->getTerrainPosition(terraCenter, &localTerraCenter3);

            // distance between 2 tiles ? (tile \in terrain \in terrainGroup)

            Ogre::Vector2 localTerraCenter2 = Ogre::Vector2(localTerraCenter3.x, localTerraCenter3.y) * size;

            Ogre::Vector3 local_radius3;
            terrain->getTerrainPosition(Ogre::Vector3(radius * size, .0f, .0f), &local_radius3);
            Ogre::Real local_radius = local_radius3.x;

            //The list of floats wil be interpreted such that the first row in the array equates to the bottom row of vertices
            float *heights = terrain->getHeightData();
            int minx = size, maxx = 0, minz = size, maxz = 0;

            for(int i = 0; i < size * size; ++i)
            {
                int x = i % size, z = i / size;
                Ogre::Vector2 vertexPos = Ogre::Vector2(x, z);
                auto dist = vertexPos.distance(localTerraCenter2);

                if(dist < local_radius)
                {
                    minx = minx < x ? minx : x;
                    maxx = maxx > x ? maxx : x;
                    minz = minz < z ? minz : z;
                    maxz = maxz > z ? maxz : z;
                    float ratio;

                    switch(rshape)
                    {
                        case RaiseShape::ROUND:
                            // 1 at the center, 0 at max dist
                            ratio = std::cos(3.14 / 2. * dist / local_radius);

                            if(rmode == RaiseMode::RELATIVE)
                                heights[i] += intensity * ratio;
                            else if(rmode == RaiseMode::ABSOLUTE)
                                heights[i] += ((terraCenter.y + intensity * ratio) - heights[i]) / (25.f - intensity > .9f ? 25.f - intensity : .9f);

                            break;

                        case RaiseShape::SINH:
                            //
                            ratio = 1.f - std::sinh(dist / local_radius);

                            if(rmode == RaiseMode::RELATIVE)
                                heights[i] += intensity * ratio;
                            else if(rmode == RaiseMode::ABSOLUTE)
                                heights[i] += ((terraCenter.y + intensity * ratio) - heights[i]) / (25.f - intensity > .9f ? 25.f - intensity : .9f);

                            break;

                        case RaiseShape::TRANGULAR:
                            // 1 at the center, 0 at max dist
                            ratio = 1.f - dist / local_radius;

                            if(rmode == RaiseMode::RELATIVE)
                                heights[i] += intensity * ratio;
                            else if(rmode == RaiseMode::ABSOLUTE)
                                heights[i] += ((terraCenter.y + intensity * ratio) - heights[i]) / (25.f - intensity > .9f ? 25.f - intensity : .9f);

                            break;

                        case RaiseShape::UNIFORM:
                        default:
                            if(rmode == RaiseMode::RELATIVE)
                                heights[i] += intensity;
                            else if(rmode == RaiseMode::ABSOLUTE)
                                heights[i] += (terraCenter.y - heights[i]) / (25.f - intensity > .9f ? 25.f - intensity : .9f);

                            break;
                    }

                    heights[i] = heights[i] < TerrainManager::MIN_TERRAIN_HEIGHT ? TerrainManager::MIN_TERRAIN_HEIGHT : heights[i];
                    heights[i] = heights[i] > TerrainManager::MAX_TERRAIN_HEIGHT ? TerrainManager::MAX_TERRAIN_HEIGHT : heights[i];
                }
            }

            if(minx < maxx && minz < maxz)
            {
                terrain->dirtyRect(Ogre::Rect(minx, minz, maxx, maxz));
                terrain->update(false);

                if(LoadingState::BUILDING != mLoadingState)
                {
                    mLoadingState = LoadingState::INIT;
                    mTerrainsImported = true;
                    Ogre::Root::getSingletonPtr()->addFrameListener(this);
                }

                mTerrainPhysicsMan->updateHeightmap(terrain);
            }
        }

        return terrains;
    }

    TerrainManager::TerrainSlotData::TerrainSlotData():
        slot_x(LONG_MAX), slot_y(LONG_MAX),
        heightmapPath(StringUtils::BLANK), size(TerrainManager::DEFAULT_TERRAIN_SIZE),
        worldSize(TerrainManager::DEFAULT_WORLD_SIZE), layerList(Ogre::Terrain::LayerInstanceList())
    {
    }

    TerrainManager::TerrainSlotData::TerrainSlotData(long x, long y):
        slot_x(x), slot_y(y),
        heightmapPath(StringUtils::BLANK), size(TerrainManager::DEFAULT_TERRAIN_SIZE),
        worldSize(TerrainManager::DEFAULT_WORLD_SIZE), layerList(Ogre::Terrain::LayerInstanceList())
    {
    }

    TerrainManager::TerrainSlotData::TerrainSlotData(const TerrainManager::TerrainSlotData &o):
        slot_x(o.slot_x), slot_y(o.slot_y),
        heightmapPath(o.heightmapPath), size(o.size),
        worldSize(o.worldSize), layerList(o.layerList)
    {
    }

    TerrainManager::TerrainSlotData::~TerrainSlotData()
    {
    }

    TerrainManager::TerrainSlotData &TerrainManager::TerrainSlotData::operator=(const TerrainManager::TerrainSlotData &o)
    {
        slot_x = o.slot_x;
        slot_y = o.slot_y;
        heightmapPath = o.heightmapPath;
        size = o.size;
        worldSize = o.worldSize;
        layerList.assign(o.layerList.begin(), o.layerList.end());
        return *this;
    }

    bool TerrainManager::TerrainSlotData::isValid()
    {
        bool valid = true;
        valid &= slot_x != LONG_MAX;
        valid &= slot_y != LONG_MAX;
        valid &= File(heightmapPath).exists() || heightmapPath == StringUtils::BLANK;
        valid &= size > 3;
        // minimal terrain size is 9m**2, just like a student's room.
        valid &= worldSize > 3;
        valid &= layerList.size() > 0;
        return valid;
    }

    bool TerrainManager::hasLoadedTerrains() const
    {
        auto it = mTerrainGroup->getTerrainIterator();
        return it.begin() != it.end();
    }


}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

