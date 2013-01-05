#include "TerrainManager.h"

#include <stdexcept>
#include <limits.h>
#include <float.h>

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
        mTerrainGroup = OGRE_NEW Ogre::TerrainGroup(mSceneManager,
                        Ogre::Terrain::ALIGN_X_Z,
                        DEFAULT_TERRAIN_SIZE,
                        DEFAULT_WORLD_SIZE);

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

        mSceneManager->createLight("terrainLight");

        // default terrain
        Ogre::Vector3 lightDir(0.55, -0.3, 0.75);
        Ogre::ColourValue diffuseColor=Ogre::ColourValue::White;
        Ogre::ColourValue specularColor(0.4, 0.4, 0.4);
        std::list<TerrainSlotData> terrainSlots;
        terrainSlots.push_back(TerrainSlotData(0L,0L));
        build(lightDir,diffuseColor,specularColor,terrainSlots);
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
            terrainValue["size"]=Json::Value(Ogre::StringConverter::toString(slot->instance->getSize()));
            terrainValue["worldSize"]=Json::Value(Ogre::StringConverter::toString(slot->instance->getWorldSize()));

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

        // heightmap is 16bpp greyscale
        short *heights=new short[side*side];
        for(size_t i=0; i<side*side; ++i)
            heights[i]=static_cast<short>(heights_init[i]);
        Ogre::DataStreamPtr streamPtr(OGRE_NEW Ogre::MemoryDataStream(heights,side*side*sizeof(short)));

        // make it an image for easy saving
        Ogre::Image img;
        img.loadRawData(streamPtr,side,side,1,Ogre::PixelFormat::PF_SHORT_L);
        Ogre::String filename="heightmap_"+Ogre::StringConverter::toString(x)+"_"+Ogre::StringConverter::toString(y)+".png";
        heightmapPath=mPath.subfile(filename).fullPath();
        img.save(heightmapPath);
    }

    float *TerrainManager::loadTerrainHeightmapFrom(Ogre::String filepath,int size)
    {
        Debug::log("TerrainManager::loadTerrainHeightmapFrom(")(filepath)("): ");
        Ogre::Image img;
        short *img_data;
        int resolution=size*size;
        bool use_file=File(filepath).exists();
        if(use_file)
        {
            img.load(filepath, mTerrainGroup->getResourceGroup());
            resolution=img.getWidth()*img.getHeight();
            Debug::log("format is ")(Ogre::PixelUtil::getFormatName(img.getFormat()));
            Debug::log(" depth: ")(img.getDepth());
            Debug::log(" resolution: ")(img.getWidth())("x")(img.getHeight()).endl();
            img_data=reinterpret_cast<short *>(img.getData());
        }
        else
        {
            if(filepath=="")
                Debug::log("file not found, using 0 ").endl();
            else
                Debug::warning("file not found, using 0 ").endl();
            img_data=new short[resolution];
            std::fill(img_data, img_data+resolution, 0);
        }

        float *heights=new float[resolution];
        for(int i=0; i<resolution; ++i)
            heights[i]=static_cast<float>(img_data[i]);

        if(!use_file)
            delete img_data;
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
                terrainSlot.position=Ogre::StringConverter::parseVector3(terrainSlotValue["position"].asString().c_str());

                terrainSlot.size=Ogre::StringConverter::parseInt(terrainSlotValue["size"].asString().c_str());
                if(terrainSlot.size==0)
                    terrainSlot.size=TerrainManager::DEFAULT_TERRAIN_SIZE;

                terrainSlot.worldSize=Ogre::StringConverter::parseReal(terrainSlotValue["worldSize"].asString().c_str());
                if(terrainSlot.worldSize==.0f)
                    terrainSlot.worldSize=TerrainManager::DEFAULT_WORLD_SIZE;

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
        Ogre::Light* light = mSceneManager->getLight("terrainLight");
        light->setType(Ogre::Light::LT_DIRECTIONAL);
        light->setDirection(lightDir);
        light->setDiffuseColour(diffuseColor);
        light->setSpecularColour(specularColor);
        configureTerrainDefaults(light);

        if(terrainSlots.size()==0)
            Debug::warning("TerrainManager::build(): terrainSlots has size 0. Level has no terrain !").endl();
        else
            for(auto it=terrainSlots.begin(); it!=terrainSlots.end(); ++it)
                defineTerrain(*it);

        // sync load since we want everything in place when we start
        mTerrainGroup->loadAllTerrains(true);

        if (mTerrainsImported)
        {
            Ogre::TerrainGroup::TerrainIterator ti = mTerrainGroup->getTerrainIterator();
            while(ti.hasMoreElements())
            {
                Ogre::Terrain* t = ti.getNext()->instance;
                updateBlendMaps(t);
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
            idata.inputFloat=loadTerrainHeightmapFrom(terrainSlotData.heightmapPath,terrainSlotData.size);
            idata.pos=terrainSlotData.position;
            idata.inputBias=0.f;
            idata.terrainSize=terrainSlotData.size;
            idata.worldSize=terrainSlotData.worldSize;
            // as for now, we only use defaults for those values
            idata.layerDeclaration=mTerrainGroup->getDefaultImportSettings().layerDeclaration;
            idata.layerList=mTerrainGroup->getDefaultImportSettings().layerList;
            if(mTerrainGroup->getTerrain(x,y)!=NULL)
                mTerrainGroup->removeTerrain(x,y);
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
        mTerrainGlobals->setCompositeMapDistance(500);

        // Important to set these so that the terrain knows what to use for derived (non-realtime) data
        mTerrainGlobals->setLightMapDirection(light->getDerivedDirection());
        mTerrainGlobals->setCompositeMapAmbient(mSceneManager->getAmbientLight());
        mTerrainGlobals->setCompositeMapDiffuse(light->getDiffuseColour());

        // Configure default import settings for if we use imported image
        Ogre::Terrain::ImportData& defaultimp = mTerrainGroup->getDefaultImportSettings();
        defaultimp.terrainSize = TerrainManager::DEFAULT_TERRAIN_SIZE;
        // 1 unit == 1 meter
        defaultimp.worldSize = TerrainManager::DEFAULT_WORLD_SIZE;
        // due terrain.png is 16 bpp (a float)
        defaultimp.inputScale = 1.f;
        defaultimp.minBatchSize = 33;
        defaultimp.maxBatchSize = 65;

        // textures
        // TODO load this from serialization (no editing included)
        // we set the number of terrain texture layers to 3
        defaultimp.layerList.resize(3);
        defaultimp.layerList[0].worldSize = 10;
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_diffusespecular.png");
        defaultimp.layerList[0].textureNames.push_back("dirt_grayrocky_normalheight.png");
        defaultimp.layerList[1].worldSize = 3;
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_diffusespecular.png");
        defaultimp.layerList[1].textureNames.push_back("grass_green-01_normalheight.png");
        defaultimp.layerList[2].worldSize = 20;
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_diffusespecular.png");
        defaultimp.layerList[2].textureNames.push_back("growth_weirdfungus-03_normalheight.png");
    }

    void TerrainManager::updateBlendMaps(Ogre::Terrain* terrain)
    {
        //TODO: load this through serialization (no editing included, but fast reload would be nice)
        Ogre::TerrainLayerBlendMap* blendMap0 = terrain->getLayerBlendMap(1);
        Ogre::TerrainLayerBlendMap* blendMap1 = terrain->getLayerBlendMap(2);
        Ogre::Real minHeight0 = 15;
        Ogre::Real fadeDist0 = 10;
        Ogre::Real minHeight1 = 15;
        Ogre::Real fadeDist1 = 0;
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
        auto diff=Ogre::Vector3::UNIT_SCALE*radius;
        Ogre::AxisAlignedBox box(terraCenter-diff,terraCenter+diff);
        Ogre::TerrainGroup::TerrainList terrains;
        mTerrainGroup->boxIntersects(box,&terrains);

        // now, for each of them, get all vertices in the sphere and move them
        // for reference, we work as much as possible in world coordinates
        Ogre::Vector2 terraPos(terraCenter.x,terraCenter.z);
        for(auto it=terrains.begin(); it!=terrains.end(); ++it)
        {
            Ogre::Terrain *terrain=*it;
            Ogre::uint16 size=terrain->getSize();

            Ogre::Vector3 localTerraCenter3;
            terrain->getTerrainPosition(terraCenter,&localTerraCenter3);

            // distance between 2 tiles ? (tile\interrain\terrainGroup)

            Ogre::Vector2 localTerraCenter2=Ogre::Vector2(localTerraCenter3.x,localTerraCenter3.y)*size;

            Ogre::Vector3 local_radius3;
            terrain->getTerrainPosition(Ogre::Vector3(radius*size,.0f,.0f),&local_radius3);
            Ogre::Real local_radius=local_radius3.x;


            //The list of floats wil be interpreted such that the first row in the array equates to the bottom row of vertices
            float *heights=terrain->getHeightData();
            int minx=size,maxx=0,minz=size,maxz=0;
            for(int i=0; i<size*size; ++i)
            {
                int x=i%size,z=i/size;
                Ogre::Vector2 vertexPos=Ogre::Vector2(x,z);
                auto dist=vertexPos.distance(localTerraCenter2);
                if(dist<local_radius)
                {
                    minx=minx<x?minx:x;
                    maxx=maxx>x?maxx:x;
                    minz=minz<z?minz:z;
                    maxz=maxz>z?maxz:z;
                    float ratio;
                    switch(rshape)
                    {
                        case RaiseShape::ROUND:
                            // 1 at the center, 0 at max dist
                            ratio=std::cos(3.14/2.*dist/local_radius);
                            if(rmode==RELATIVE)
                                heights[i]+=intensity*ratio;
                            else if(rmode==ABSOLUTE)
                                heights[i]+=((terraCenter.y+intensity*ratio)-heights[i])/(25.f-intensity>.9f?25.f-intensity:.9f);
                            break;
                        case RaiseShape::SINH:
                            //ratio=(1.f-std::exp(dist)/(1+std::exp(local_radius)));
                            ratio=1.f-std::sinh(dist/local_radius);
                            if(rmode==RELATIVE)
                                heights[i]+=intensity*ratio;
                            else if(rmode==ABSOLUTE)
                                heights[i]+=((terraCenter.y+intensity*ratio)-heights[i])/(25.f-intensity>.9f?25.f-intensity:.9f);
                            break;
                        case RaiseShape::UNIFORM:
                        default:
                            if(rmode==RELATIVE)
                                heights[i]+=intensity;
                            else if(rmode==ABSOLUTE)
                                heights[i]+=(terraCenter.y-heights[i])/(25.f-intensity>.9f?25.f-intensity:.9f);
                            break;
                    }
                    heights[i]=heights[i]<TerrainManager::MIN_TERRAIN_HEIGHT?TerrainManager::MIN_TERRAIN_HEIGHT:heights[i];
                    heights[i]=heights[i]>TerrainManager::MAX_TERRAIN_HEIGHT?TerrainManager::MAX_TERRAIN_HEIGHT:heights[i];
                }
            }
            if(minx<maxx && minz<maxz)
            {
                Ogre::Vector3 v0=terrain->convertPosition(Ogre::Terrain::LOCAL_SPACE,
                                 Ogre::Vector3(minx,.0f,minz),
                                 Ogre::Terrain::WORLD_SPACE);
                Ogre::Vector3 v1=terrain->convertPosition(Ogre::Terrain::LOCAL_SPACE,
                                 Ogre::Vector3(maxx,.0f,maxz),
                                 Ogre::Terrain::WORLD_SPACE);
                if(v0.x<v1.x && v0.z<v1.z)
                    terrain->dirtyRect(Ogre::Rect(v0.x,v0.z,v1.x,v1.z));
                else
                    terrain->dirty();
                terrain->updateGeometry();
            }
        }
        return terrains;
    }

    TerrainManager::TerrainSlotData::TerrainSlotData():
        slot_x(LONG_MAX),slot_y(LONG_MAX),heightmapPath(""),
        position(Ogre::Vector3::ZERO),
        size(TerrainManager::DEFAULT_TERRAIN_SIZE),worldSize(TerrainManager::DEFAULT_WORLD_SIZE)
    {}
    TerrainManager::TerrainSlotData::TerrainSlotData(long x, long y):
        slot_x(x),slot_y(y),heightmapPath(""),
        position(Ogre::Vector3::ZERO),
        size(TerrainManager::DEFAULT_TERRAIN_SIZE),worldSize(TerrainManager::DEFAULT_WORLD_SIZE)
    {}
    TerrainManager::TerrainSlotData::~TerrainSlotData() {}
    bool TerrainManager::TerrainSlotData::isValid()
    {
        bool valid=true;
        valid&=slot_x!=LONG_MAX;
        valid&=slot_y!=LONG_MAX;
        valid&=File(heightmapPath).exists() || heightmapPath=="" ;
        valid&=size>3;
        // minimal terrain size is 9m**2, just like a student's room.
        valid&=worldSize>3;
        return valid;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

