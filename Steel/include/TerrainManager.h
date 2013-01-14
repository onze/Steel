#ifndef TERRAINMANAGER_H
#define TERRAINMANAGER_H

#include <json/value.h>

#include <OgreLight.h>
#include <OgreImage.h>
#include <OgreFrameListener.h>
#include <OgreTerrainGroup.h>

#include "tools/File.h"

namespace Ogre
{
    class TerrainGlobalOptions;
    class Terrain;
    class TerrainGroup;
    class SceneManager;
}

namespace Steel
{
    class TerrainManagerEventListener;
    class TerrainManager:public Ogre::FrameListener
    {
        protected:
            friend class TerrainManagerEventListener;
        public:
            /// holds all sensitive terrain data between deserialization and instanciation
            /// (because JSon stuff are better confined in {from/to}Json methods)
            class TerrainSlotData
            {
                public:
                    TerrainSlotData();
                    TerrainSlotData(long x,long y);
                    TerrainSlotData(const TerrainSlotData &o);
                    virtual ~TerrainSlotData();
                    TerrainSlotData &operator=(const TerrainSlotData &o);
                    bool isValid();

                    long int slot_x;
                    long int slot_y;
                    Ogre::String heightmapPath;
                    int size;
                    Ogre::Real worldSize;
                    Ogre::Terrain::LayerInstanceList layerList;
            };

        public:
            /// terrain size in vertices (should be 2^n+1)
            static const int DEFAULT_TERRAIN_SIZE=513;
            /// terrain size in world units
            static const int DEFAULT_WORLD_SIZE=500.f;
            /// max height of the terrain
            static const int MAX_TERRAIN_HEIGHT=500.f;
            /// min height of the terrain
            static const int MIN_TERRAIN_HEIGHT=-500.f;
            enum LoadingState {INIT=0,BUILDING,TEXTURING,SAVING,READY};

            TerrainManager();
            TerrainManager(const TerrainManager& other);
            virtual ~TerrainManager();
            virtual TerrainManager& operator=(const TerrainManager& other);
            virtual bool operator==(const TerrainManager& other) const;

            /// adds and instanciate a slot.
            void addTerrainSlot(TerrainManager::TerrainSlotData slot);

            /// takes a terrain and serialize its layers into the given json object.
            void serializeLayerList(Ogre::Terrain* terrain, Json::Value& terrainValue);
            /// takes a layer serialization and parse it inot the given layer list.
            void deserializeLayerList(const Json::Value &layerListValue, Ogre::Terrain::LayerInstanceList &layerList);

            /// Initial setup method
            void init(Ogre::String resourceGroupName, File path, Ogre::SceneManager* sceneManager);
            /// Clears the terrain and all associated structures. (automatically called upon deletion).
            void shutdown();

            void addTerrainManagerEventListener(TerrainManagerEventListener *listener);
            void removeTerrainManagerEventListener(TerrainManagerEventListener *listener);

            /// Called by Ogre once per frame
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);

            /** return the coordinate at which the given ray intersect the terrain.
             * the pointer of Terrain is always NULL, since this is implementation specific.
             */
            Ogre::TerrainGroup::RayResult intersectRay(const Ogre::Ray& ray);

            /// generates the terrain
            void build(Ogre::ColourValue,
                       Ogre::Vector3 lightDir,
                       Ogre::ColourValue diffuseColor,
                       Ogre::ColourValue specularColor,
                       Ogre::Terrain::ImportData defaultImp,
                       std::list< Steel::TerrainManager::TerrainSlotData >& terrainSlots);

            bool fromJson(Json::Value &value);
            Json::Value toJson();
            float *loadTerrainHeightmapFrom(Ogre::String filepath,int size);
            void saveTerrainHeightmapAs(long int x, long int y, Ogre::Terrain *instance,Ogre::String &heightmapPath);

            enum RaiseMode {ABSOLUTE=0,RELATIVE};
            enum RaiseShape {UNIFORM=0,ROUND,SINH, TRANGULAR};
            /** Raise all terrain vertices in an round shaped area centered at the given position (terraCenter) (world coords)
             * and of the given radius, by a decreasing value starting at the given value at the center, and reaching 0
             * at radius.
             * Returns slot coordinaes of terrains that were modified in the process.
             */
            Ogre::TerrainGroup::TerrainList raiseTerrainAt(Ogre::Vector3 terraCenter,
                    Ogre::Real intensity,
                    Ogre::Real radius,
                    RaiseMode rmode=ABSOLUTE,
                    RaiseShape rshape=UNIFORM);

            /// take a terrain slot seralization and return its deserialized version
            TerrainSlotData terrainSlotFromJson(Json::Value &terrainSlotValue,TerrainManager::TerrainSlotData &terrainSlot);

            /// recompute blendmaps according to rules
            void updateBlendMaps(Ogre::Terrain* terrain);

            // getters
            inline Ogre::TerrainGroup *terrainGroup()
            {
                return mTerrainGroup;
            }

        protected:

            /// update a terrain slot.
            void defineTerrain(Steel::TerrainManager::TerrainSlotData& terrainSlotData);

            /// updates internal structures if needed.
            void update();
            /// sets default terrain settings
            void configureTerrainDefaults(Ogre::Light* light, Ogre::Terrain::ImportData& newDefault);
            /// calls event listeners' callback methods.
            void yieldEvent(LoadingState state);

            // not owned
            Ogre::SceneManager *mSceneManager;

            // owned
            Ogre::String mResourceGroupName;
            LoadingState mLoadingState;
            std::set<TerrainManagerEventListener *> mListeners;
            Ogre::TerrainGlobalOptions *mTerrainGlobals;
            Ogre::TerrainGroup *mTerrainGroup;
            bool mTerrainsImported;
            File mPath;
    };
}
#endif // TERRAINMANAGER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
