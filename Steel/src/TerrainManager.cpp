#include "TerrainManager.h"

#include <stdexcept>
#include <limits.h>

#include <OgreTerrain.h>
#include <OgreTerrainGroup.h>
#include <OgreRoot.h>
#include <OgreImage.h>

#include <Debug.h>
#include <TerrainManagerEventListener.h>

namespace Steel
{
    TerrainManager::TerrainManager():Ogre::FrameListener(),
        mLoadingState(INIT),mListeners(std::set<TerrainManagerEventListener *>()),
        mTerrainGlobals(NULL),mTerrainGroup(NULL),mTerrainsImported(false),mPath("")
    {
    }

    TerrainManager::TerrainManager(const TerrainManager& o):
        mLoadingState(o.mLoadingState),mListeners(o.mListeners),
        mTerrainGlobals(o.mTerrainGlobals),mTerrainGroup(o.mTerrainGroup),mTerrainsImported(o.mTerrainsImported),mPath(o.mPath)
    {
    }

    TerrainManager::~TerrainManager()
    {
        destroy();
    }

    TerrainManager& TerrainManager::operator=(const TerrainManager& o)
    {
        mLoadingState=o.mLoadingState;
        mListeners=o.mListeners;
        mTerrainGlobals=o.mTerrainGlobals;
        mTerrainGroup=o.mTerrainGroup;
        mTerrainsImported=o.mTerrainsImported;
        mPath=o.mPath;
        return *this;
    }

    bool TerrainManager::operator==(const TerrainManager& other) const
    {
        return false;
    }

    void TerrainManager::addTerrainManagerEventListener(TerrainManagerEventListener *listener)
    {
        mListeners.insert(listener);
    }

    void TerrainManager::removeTerrainManagerEventListener(TerrainManagerEventListener *listener)
    {
        auto it=std::find(mListeners.begin(),mListeners.end(),listener);
        if(it!=mListeners.end())
        {
            assert(listener==*it);
            mListeners.erase(*it);
        }
    }

    void TerrainManager::yieldEvent(LoadingState state)
    {
        auto copy=std::vector<TerrainManagerEventListener *>(mListeners.begin(),mListeners.end());
        for(auto it=copy.begin(); copy.size()>0 && it!=copy.end(); ++it)
        {
            (*it)->onTerrainEvent(state);
        }
    }

    void TerrainManager::destroy()
    {
        mSceneManager->destroyLight("terrainLight");
        OGRE_DELETE mTerrainGroup;
        OGRE_DELETE mTerrainGlobals;
        mSceneManager=NULL;
    }

    void TerrainManager::init(Ogre::String resourceGroupName, File path, Ogre::SceneManager* sceneManager)
    {
        mSceneManager=sceneManager;
        mPath=path;

        mTerrainsImported=false;
        mLoadingState=INIT;
        mTerrainGlobals = OGRE_NEW Ogre::TerrainGlobalOptions();
        mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneManager, Ogre::Terrain::ALIGN_X_Z, 513, 12000.0f);

        auto rgm=Ogre::ResourceGroupManager::getSingletonPtr();
        rgm->createResourceGroup(resourceGroupName);
        rgm->addResourceLocation(path.fullPath(),"FileSystem",resourceGroupName);
        rgm->addResourceLocation(path.subfile("build").fullPath(),"FileSystem",resourceGroupName);
        mTerrainGroup->setResourceGroup(resourceGroupName);

        File buildPath=mPath.subfile("build");
        if(!buildPath.exists())
            buildPath.mkdir();
        mTerrainGroup->setFilenameConvention(buildPath.subfile("terrain").fullPath(), Ogre::String("terrain"));
        mTerrainGroup->setOrigin(Ogre::Vector3::ZERO);
    }

    Json::Value TerrainManager::toJson()
    {
        Json::Value root;

        // general terrain data
        Ogre::Light *light=mSceneManager->getLight("terrainLight");
        root["lightDir"]=Json::Value(Ogre::StringConverter::toString(light->getDirection()));
        root["diffuseColor"]=Json::Value(Ogre::StringConverter::toString(light->getDiffuseColour()));
        root["specularColor"]=Json::Value(Ogre::StringConverter::toString(light->getSpecularColour()));

        // actual terrains
        Ogre::TerrainGroup::TerrainIterator it = mTerrainGroup->getTerrainIterator();
        while(it.hasMoreElements())
        {
            Ogre::TerrainGroup::TerrainSlot *slot=it.getNext();
            Json::Value terrainValue;
            terrainValue["slot"]["x"]=Json::Value(Ogre::StringConverter::toString(slot->x));
            terrainValue["slot"]["y"]=Json::Value(Ogre::StringConverter::toString(slot->y));

            Ogre::String heightmapPath="";
            saveTerrainHeightmapAs(slot->x,slot->y,slot->instance,heightmapPath);
            terrainValue["heightmapPath"]=heightmapPath;

            terrainValue["position"]=Json::Value(Ogre::StringConverter::toString(slot->instance->getPosition()));


//             Ogre::Terrain* terrain = slot->instance;
            root["terrainSlots"].append(terrainValue);
        }
        return root;
    }

    void TerrainManager::saveTerrainHeightmapAs(long int x, long int y, Ogre::Terrain *instance,Ogre::String &heightmapPath)
    {
        // get data
        float *heights_init=instance->getHeightData();
        size_t side=instance->getSize();

        // put it in the png greyscale format
        char *heights=new char[side*side];
        for(size_t i=0; i<side*side; ++i)heights[i]=static_cast<char>(heights_init[i]);
        Ogre::DataStreamPtr streamPtr(OGRE_NEW Ogre::MemoryDataStream(heights,side*side));

        // make it an image for easy saving
        Ogre::Image img;
        img.loadRawData(streamPtr,side,side,1,Ogre::PixelFormat::PF_L8);
        Ogre::String filename="heightmap_"+Ogre::StringConverter::toString(x)+"_"+Ogre::StringConverter::toString(y)+".png";
        heightmapPath=mPath.subfile(filename).fullPath();
        img.save(heightmapPath);
//         Debug::log("TerrainManager::saveTerrainHeightmapAs(): ")(Ogre::PixelUtil::getFormatName(img.getFormat())).endl();
    }

    float *TerrainManager::loadTerrainHeightmapFrom(Ogre::String filepath)
    {
        Ogre::Image img;
        img.load(filepath, mTerrainGroup->getResourceGroup());
//         Debug::log("TerrainManager::loadTerrainHeightmapFrom(): ")(Ogre::PixelUtil::getFormatName(img.getFormat())).endl();

        Ogre::uchar *img_data=img.getData();
        size_t size=img.getWidth()*img.getHeight();
        float *heights=new float[size];
        for(size_t i=0; i<size; ++i)
            heights[i]=static_cast<float>(img_data[i]);

        return heights;
    }

    bool TerrainManager::fromJson(Json::Value &root)
    {
        Ogre::String intro="TerrainManager::fromJSon(): ";
        Json::Value value;

//         Ogre::Vector3 lightDir(.2f, -.5f, .3f);
        Ogre::Vector3 lightDir(0.55, -0.3, 0.75);
        value=root["lightDir"];
        if(value.isNull())
            Debug::warning(intro)("key 'lightDir' is null. Using default.").endl();
        else
            lightDir=Ogre::StringConverter::parseVector3(value.asString());

        //TODO: move light management to a dedicated LightManager
        Ogre::ColourValue diffuseColor=Ogre::ColourValue::White;
        value=root["diffuseColor"];
        if(value.isNull())
            Debug::warning(intro)("key 'diffuseColor' is null. Using default.").endl();
        else
            diffuseColor=Ogre::StringConverter::parseColourValue(value.asString());

        Ogre::ColourValue specularColor(0.4, 0.4, 0.4);
        value=root["specularColor"];
        if(value.isNull())
            Debug::warning(intro)("key 'specularColor' is null. Using default.").endl();
        else
            specularColor=Ogre::StringConverter::parseColourValue(value.asString());

        // terrains
        std::list<TerrainSlotData> terrainSlots;
        Json::Value terrainSlotsValue=root["terrainSlots"];
        bool valid=false;
        if(value.isNull())
            Debug::warning(intro)("key 'terrainSlots' is null.").endl();
        else if(!terrainSlotsValue.isConvertibleTo(Json::arrayValue))
            Debug::warning(intro)("key 'terrainSlots' can't be converted to an array.").endl();
        else
        {
            Json::ValueIterator it = terrainSlotsValue.begin();
            for (; it != terrainSlotsValue.end(); ++it)
            {
                TerrainSlotData terrainSlot;
                Json::Value terrainSlotValue=*it;
                terrainSlot.slot_x=Ogre::StringConverter::parseLong(terrainSlotValue["slot"]["x"].asString(),terrainSlot.slot_x);
                terrainSlot.slot_y=Ogre::StringConverter::parseLong(terrainSlotValue["slot"]["y"].asString(),terrainSlot.slot_y);
                terrainSlot.heightmapPath=terrainSlotValue["heightmapPath"].asString().c_str();
                terrainSlot.position=Ogre::StringConverter::parseVector3(terrainSlotValue["position"].asString().c_str(),Ogre::Vector3::ZERO);
                if(terrainSlot.isValid())
                    terrainSlots.push_back(terrainSlot);
                else
                {
                    Debug::error("invalid terrainSlot:").endl()(terrainSlotValue.toStyledString()).endl();
                    break;
                }
            }
            // keep last in this block
            if(it==terrainSlotsValue.end())
                valid=true;
        }
        if(!valid)
        {
            Debug::warning("key 'terrainSlots':").endl()(root["terrainSlots"].toStyledString()).endl();
        }

        build(lightDir, diffuseColor,specularColor,terrainSlots);
        return true;
    }

    void TerrainManager::build(Ogre::Vector3 lightDir,
                               Ogre::ColourValue diffuseColor,
                               Ogre::ColourValue specularColor,
                               std::list<TerrainSlotData> &terrainSlots)
    {
        mTerrainsImported=false;
        mLoadingState=INIT;

        Ogre::Root::getSingletonPtr()->addFrameListener(this);

        // setup directional light for the terrain manager
        // TODO: encapsulate into a lightManager (need light models ?)
        lightDir.normalise();
        // keep name in sync with destructor/shutdown
        Ogre::Light* light = mSceneManager->createLight("terrainLight");
        light->setType(Ogre::Light::LT_DIRECTIONAL);
        light->setDirection(lightDir);
        light->setDiffuseColour(diffuseColor);
        light->setSpecularColour(specularColor);
        configureTerrainDefaults(light);

        bool harcoded=false;
        if(harcoded)
        {
            TerrainSlotData tsd;
            for (long x = 0; x <= 0; ++x)
            {
                for (long y = 0; y <= 0; ++y)
                {
                    tsd.slot_x=x;
                    tsd.slot_y=y;
                    tsd.heightmapPath="default_terrain.png";
                    defineTerrain(tsd);
                }
            }
        }
        else
        {
            for(auto it=terrainSlots.begin(); it!=terrainSlots.end(); ++it)
                defineTerrain(*it);
        }

        // sync load since we want everything in place when we start
        mTerrainGroup->loadAllTerrains(true);

        if (mTerrainsImported)
        {
            Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
            while(ti.hasMoreElements())
            {
                Ogre::Terrain* t = ti.getNext()->instance;
                initBlendMaps(t);
            }
        }
        mTerrainGroup->freeTemporaryResources();
    }

    void TerrainManager::defineTerrain(TerrainSlotData &terrainSlotData)
    {
        long x=terrainSlotData.slot_x;
        long y=terrainSlotData.slot_y;
        Ogre::String filename = mTerrainGroup->generateFilename(x, y);
        if (Ogre::ResourceGroupManager::getSingleton().resourceExists(mTerrainGroup->getResourceGroup(), filename))
        {
            mTerrainGroup->defineTerrain(x, y);
        }
        else
        {
            Ogre::Terrain::ImportData idata;
            idata.inputFloat=loadTerrainHeightmapFrom(terrainSlotData.heightmapPath);
            idata.pos=terrainSlotData.position;
            idata.layerDeclaration=mTerrainGroup->getDefaultImportSettings().layerDeclaration;
            idata.layerList=mTerrainGroup->getDefaultImportSettings().layerList;
            mTerrainGroup->defineTerrain(x, y, &idata);
            mTerrainsImported = true;
        }
    }

    void TerrainManager::configureTerrainDefaults(Ogre::Light *light)
    {
        // MaxPixelError decides how precise our terrain is going to be.
        // A lower number will mean a more accurate terrain, at the cost of performance (because of more vertices).
        mTerrainGlobals->setMaxPixelError(8);
        // CompositeMapDistance decides how far the Ogre terrain will render the lightmapped terrain
        mTerrainGlobals->setCompositeMapDistance(3000);

        // Important to set these so that the terrain knows what to use for derived (non-realtime) data
        mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
        mTerrainGlobals->setCompositeMapAmbient(mSceneManager->getAmbientLight());
        mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

        // Configure default import settings for if we use imported image
        Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
        defaultimp.terrainSize = 513;
        defaultimp.worldSize = 12000.0f;
        defaultimp.inputScale = 600.f; // due terrain.png is 8 bpp
        defaultimp.minBatchSize = 33;
        defaultimp.maxBatchSize = 65;

        // textures
        // we set the number of terrain texture layers to 3
        defaultimp.layerList.resize(3);
        defaultimp.layerList[0].worldSize = 100;
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.png");
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.png");
        defaultimp.layerList[1].worldSize = 30;
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.png");
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.png");
        defaultimp.layerList[2].worldSize = 200;
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.png");
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.png");
    }

    void TerrainManager::initBlendMaps(Ogre::Terrain* terrain)
    {
        Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
        Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
        Ogre::Real minHeight0 = 70;
        Ogre::Real fadeDist0 = 40;
        Ogre::Real minHeight1 = 70;
        Ogre::Real fadeDist1 = 15;
        float* pBlend0 = blendMap0->getBlendPointer();
        float* pBlend1 = blendMap1->getBlendPointer();
        for (Ogre::uint16 y = 0; y < terrain->getLayerBlendMapSize(); ++y)
        {
            for (Ogre::uint16 x = 0; x < terrain->getLayerBlendMapSize(); ++x)
            {
                Ogre::Real tx, ty;

                blendMap0->convertImageToTerrainSpace(x, y, &tx, &ty);
                Ogre::Real height = terrain->getHeightAtTerrainPosition(tx, ty);
                Ogre::Real val = (height - minHeight0) / fadeDist0;
                val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
                *pBlend0++ = val;

                val = (height - minHeight1) / fadeDist1;
                val = Ogre::Math::Clamp(val, (Ogre::Real)0, (Ogre::Real)1);
                *pBlend1++ = val;
            }
        }
        blendMap0->dirty();
        blendMap1->dirty();
        blendMap0->update();
        blendMap1->update();
    }

    bool TerrainManager::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        if (mTerrainGroup->isDerivedDataUpdateInProgress())
        {
            if (mTerrainsImported)
            {
                if(mLoadingState!=BUILDING)
                {
                    mLoadingState=BUILDING;
                    yieldEvent(mLoadingState);
                }
            }
            else
            {
                if(mLoadingState!=TEXTURING)
                {
                    mLoadingState=TEXTURING;
                    yieldEvent(mLoadingState);
                }
            }
        }
        else
        {
            if (mTerrainsImported)
            {
                if(mLoadingState!=SAVING)
                {
                    mLoadingState=SAVING;
                    yieldEvent(mLoadingState);
//                     mTerrainGroup->saveAllTerrains(true);
                }
                else
                {
                    mTerrainsImported = false;
                    Ogre::Root::getSingletonPtr()->removeFrameListener(this);
                    mLoadingState=READY;
                    yieldEvent(mLoadingState);
                }
            }
        }
        // continue rendering
        return true;
    }

    Ogre::TerrainGroup::RayResult TerrainManager::intersectRay(const Ogre::Ray& ray)
    {
        Ogre::TerrainGroup::RayResult result(false,NULL,Ogre::Vector3::ZERO);
        if(mTerrainGroup!=NULL)
        {
            result=mTerrainGroup->rayIntersects(ray);
            result.terrain=NULL;
        }
        return result;
    }

    TerrainManager::TerrainSlotData::TerrainSlotData():
        slot_x(LONG_MAX),slot_y(LONG_MAX),heightmapPath("default_terrain.png")
    {}
    TerrainManager::TerrainSlotData::~TerrainSlotData() {}
    bool TerrainManager::TerrainSlotData::isValid()
    {
        bool valid=true;
        valid&=slot_x!=LONG_MAX;
        valid&=slot_y!=LONG_MAX;
        valid&=File(heightmapPath).exists();
        return valid;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
