#ifndef STEEL_LEVEL_H_
#define STEEL_LEVEL_H_

#include <memory>
#include <list>

#include "steeltypes.h"
//#include "BTModelManager.h"
#include "tools/File.h"
#include "TerrainManager.h"
#include "TerrainManagerEventListener.h"
#include "BTModelManager.h"

namespace Steel
{
    class Engine;
    class Agent;
    class Camera;
    class AgentManager;
    class ModelManager;
    class OgreModelManager;
    class PhysicsModelManager;
    class BlackBoardModelManager;
    class SelectionManager;

    class Level: public TerrainManagerEventListener
    {
        private:
            Level();
            Level(Level &level);
            Level &operator=(const Level &level);

        public:
            Level(Engine *engine, File path, Ogre::String name);
            virtual ~Level();

            /// Read properties in the given string and set them where they should.
            bool deserialize(Ogre::String &s);

            /// Ffills the list of AgentId with agents that own nodes in the the given list.
            void getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *>& nodes, Selection& selection);

            /// Returns the name of the json file that contains this level's properies.
            File getSavefile();

            bool isOver();

            /// Links an agent to a model.
            bool linkAgentToModel(AgentId aid, ModelType mType, ModelId mid);
            /// Triggered by an agent that linked to a model.
            bool onAgentLinkedToModel(AgentId aid, ModelType mtype, ModelId mid);

            /**
             * loads a level serialization string from a file and restore the state it represents.
             * Return true is the loading went successfully, false otherwise.
             */
            bool load();

            virtual void onTerrainEvent(TerrainManager::LoadingState state);

            /// execute a serialized command
            void processCommand(std::vector<Ogre::String> command);

            /// Allows a manager to register itself to the level
            void registerManager(ModelType type, ModelManager *manager);

            /**
             * save a seralization string into a file that can be loaded and read back with a call to load.
             * Return true if the saving went successfully.
             */
            bool save();
            /// Collects level's agents' properties and put them in a string.
            void serialize(Ogre::String &s);

            /// Main loop iteration.
            void update(float timestep);

            //getters
            inline Ogre::ColourValue backgroundColor()
            {
                return mBackgroundColor;
            }

            /// Player camera. For now there's only one camera per level.
            inline Camera *camera()
            {
                return mCamera;
            }

            inline Ogre::String name()
            {
                return mName;
            }

            inline File path()
            {
                return mPath;
            }

            inline Ogre::SceneNode *levelRoot()
            {
                return mLevelRoot;
            }

            ///Return the level's model manager for the given type.
            ModelManager *modelManager(ModelType modelType);

            inline OgreModelManager *ogreModelMan()
            {
                return mOgreModelMan;
            }

            inline PhysicsModelManager *physicsModelMan()
            {
                return mPhysicsModelMan;
            }

            inline Ogre::SceneManager *sceneManager()
            {
                return mSceneManager;
            }

            inline TerrainManager *terrainManager()
            {
                return &mTerrainMan;
            }

            inline BTModelManager *BTModelMan()
            {
                return mBTModelMan;
            }

            inline AgentManager *agentMan()
            {
                return mAgentMan;
            }

            inline SelectionManager *selectionMan()
            {
                return mSelectionMan;
            }

            inline LocationModelManager *locationMan()
            {
                return mLocationMan;
            }

        private:
            /// name used in debug output
            Ogre::String logName();

            //not owned
            Engine *mEngine;

            //owned
            /// what displays the level camera
            Ogre::Viewport *mViewport;

            /// level folder (where the level manages its own resources)
            File mPath;

            /// level name (i.e. name of the folder its loads its resources from)
            Ogre::String mName;

            /// default color
            Ogre::ColourValue mBackgroundColor;

            /// Pointer to steel's global scene manager.
            Ogre::SceneManager *mSceneManager;

            /// root node of the level. All level-dependant entities are its children.
            Ogre::SceneNode *mLevelRoot;

            /// Maps ModelType to managers
            std::map<ModelType, ModelManager *> mManagers;

            // actual managers
            AgentManager *mAgentMan;
            OgreModelManager *mOgreModelMan;
            PhysicsModelManager *mPhysicsModelMan;
            BTModelManager *mBTModelMan;
            TerrainManager mTerrainMan;
            SelectionManager *mSelectionMan;

            /// Main camera
            Camera *mCamera;

            Ogre::Light *mMainLight;
    };
}

#endif /* STEEL_LEVEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
