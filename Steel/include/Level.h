/*
 * Level.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <map>
#include <memory>

#include "steeltypes.h"
//#include "BTModelManager.h"
#include "tools/File.h"
#include "TerrainManager.h"
#include "TerrainManagerEventListener.h"

namespace Steel
{
    class Engine;
    class Agent;
    class Camera;
    class ModelManager;
    class OgreModelManager;
    class PhysicsModelManager;

    class Level:public TerrainManagerEventListener
    {
        private:
            Level();
            Level(Level &level);
            Level &operator=(const Level &level);
        public:
            
            ///@param[in] path: parent dir where the level saves itself
            Level(Engine *engine, File path, Ogre::String name);
            virtual ~Level();

            void deleteAgent(AgentId id);

            /// Read properties in the given string and set them where they should.
            bool deserialize(Ogre::String &s);

            /// Returns a pointer to the agent whose id's given, or NULL if there's no such agent.
             Agent *getAgent(AgentId id);

            /// Ffills the list of AgentId with agents that own nodes in the the given list.
            void getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<AgentId> &selection);

            /// Returns the name of the json file that contains this level's properies.
            File getSavefile();
            
            bool isOver();

            /// Links an agent to a model.
            bool linkAgentToModel(AgentId aid,ModelType mType,ModelId mid);
            /// Triggered by an agent that linked to a model.
            bool onAgentLinkedToModel(AgentId aid, ModelType mtype, ModelId mid);

            /**
             * loads a level serialization string from a file and restore the state it represents.
             * Return true is the loading went successfully, false otherwise.
             */
            bool load();

            /// Creates an empty agent and return its id. Agent can be linked to models via Agent::linkTo.
             AgentId newAgent();

            /**
             * creates a new instance of Agent.
             * name: name of the mesh to use
             * pos: position of the node
             * rot: rotation of the node
             * involvesNewResources: if false (default), needed resources are assumed to be declared to Ogre::ResourceManager.
             */
            ModelId newOgreModel(Ogre::String name,
                                 Ogre::Vector3 pos = Ogre::Vector3::ZERO,
                                 Ogre::Quaternion rot = Ogre::Quaternion::IDENTITY);
            
//             ModelId newPhysicsModel()=0;

            virtual void onTerrainEvent(TerrainManager::LoadingState state);

            /// execute a serialized command
            void processCommand(std::vector<Ogre::String> command);

            /// Loads behavior trees available for this  level.
//	void loadBTrees();

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


        protected:
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

            /// agent container.
            std::map<AgentId, Agent *> mAgents;

            /// responsible for OgreModel's instances.
            OgreModelManager *mOgreModelMan;
            
            /// responsible for PhysicsModel's instances.
            PhysicsModelManager *mPhysicsModelMan;

            /// behavior tree manager.
//	BTModelManager *mBTModelMan;

            /// main camera
            Camera *mCamera;

            /// eases terrain manipulation
            TerrainManager mTerrainMan;
            
            Ogre::Light *mMainLight;
    };
}

#endif /* LEVEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
