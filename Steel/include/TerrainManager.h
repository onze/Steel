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

            /// holds all sensitive terrain data between deserialization and instanciation
            /// (because JSon stuff are better confined in {from/to}Json methods)
            class TerrainSlotData
            {
                public:
                    TerrainSlotData();
                    ~TerrainSlotData();
                    bool isValid();
                    long int slot_x;
                    long int slot_y;
                    Ogre::String heightmapPath;
                    Ogre::Vector3 position;
            };

        public:
            enum LoadingState {INIT=0,BUILDING,TEXTURING,SAVING,READY};

            TerrainManager();
            TerrainManager(const TerrainManager& other);
            virtual ~TerrainManager();
            virtual TerrainManager& operator=(const TerrainManager& other);
            virtual bool operator==(const TerrainManager& other) const;

            /// initial setup
            void init(Ogre::String resourceGroupName, File path, Ogre::SceneManager* sceneManager);
            /// clears the terrain and associated structures. (automatically called upon deletion).
            void destroy();

            void addTerrainManagerEventListener(TerrainManagerEventListener *listener);
            void removeTerrainManagerEventListener(TerrainManagerEventListener *listener);

            /// called by Ogre once per frame 
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);
            
            /** return the coordinate at which the given ray intersect the terrain.
             * the pointer of Terrain is always NULL, since this is implementation specific.
             */
            Ogre::TerrainGroup::RayResult intersectRay(const Ogre::Ray& ray);

            /// generates the terrain
            void build(Ogre::Vector3 lightDir,
                       Ogre::ColourValue diffuseColor,
                       Ogre::ColourValue specularColor,
                       std::list< Steel::TerrainManager::TerrainSlotData >& terrainSlots);

            bool fromJson(Json::Value &value);
            Json::Value toJson();
            float *loadTerrainHeightmapFrom(Ogre::String filepath);
            void saveTerrainHeightmapAs(long int x, long int y, Ogre::Terrain *instance,Ogre::String &heightmapPath);

            void yieldEvent(LoadingState state);

        protected:
            /// prepares ogre and the terrain, at startup
            void configureTerrainDefaults(Ogre::Light* light);
            void defineTerrain(Steel::TerrainManager::TerrainSlotData& terrainSlotData);
            void initBlendMaps(Ogre::Terrain* terrain);

            // not owned
            Ogre::SceneManager *mSceneManager;

            // owned
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
