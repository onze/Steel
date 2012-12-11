/*
 * Level.h
 *
 *  Created on: 2011-05-13
 *      Author: onze
 */

#ifndef LEVEL_H_
#define LEVEL_H_

#include <map>

#include "steeltypes.h"
//#include "BTModelManager.h"
#include "tools/File.h"

namespace Steel
{
    class Agent;
    class Camera;
    class ModelManager;
    class OgreModelManager;
    class Level
    {
        private:
            Level();
            Level(Level &level);
            Level &operator=(const Level &level);
        public:
            ///@param[in] path: parent dir where the level saves itself
            Level(File path, Ogre::String name, Ogre::SceneManager *sceneManager, Camera *camera);
            virtual ~Level();

            /**
             * TODO: write this docstring.
             */
            Ogre::String addAuxiliaryResourceName(Ogre::String name);

            void deleteAgent(AgentId id);

            /**
             * read properties in the given string and set them where they should.
             */
            bool deserialize(Ogre::String &s);

            /**
             * Returns a pointer to the agent whose id's given, or NULL if there's no such agent.
             */
            Agent *getAgent(AgentId id);

            /**
             * fills the ModelId list with ids of Things that own nodes in the the given list.
             */
            void getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, std::list<ModelId> &selection);

            /**
             * returns the name of the json file that contains this level's properies.
             */
            File getSavefile();

            bool isOver();

            bool linkAgentToModel(AgentId aid, ModelType mtype, ModelId mid);

            /**
             * loads a level serialization string from a file and restore the state it represents.
             * Return true is the loading went successfully, false otherwise.
             */
            bool load();

            /**
             * Creates an empty agent and return its id. Agent can be linked to models via Agent::linkTo.
             */
            AgentId newAgent();

            /**
             * creates a new instance of Agent.
             * name: name of the mesh to use
             * pos: position of the node
             * rot: rotation of the node
             * involvesNewResources: if false (default), needed resources are assumed to be declared to Ogre::ResourceManager.
             */
            ModelId newOgreModel(Ogre::String name, Ogre::Vector3 pos = Ogre::Vector3::ZERO, Ogre::Quaternion rot =
                                     Ogre::Quaternion::IDENTITY,
                                 bool involvesNewResources = false);

            /**
             * loads behavior trees available for this  level.
             */
//	void loadBTrees();

            /**
             * save a seralization string into a file that can be loaded and read back with a call to load.
             * Return true if the saving went successfully.
             */
            bool save();
            /**
             * collects level's agents' properties and put them in a string.
             */
            void serialize(Ogre::String &s);

            //getters
            inline File path()
            {
                return mPath;
            }
            inline OgreModelManager *ogreModelMan()
            {
                return mOgreModelMan;
            }
            inline Ogre::SceneNode *levelRoot()
            {
                return mLevelRoot;
            }

            ///Return the level's model manager for the given type.
            ModelManager *modelManager(ModelType modelType);

            inline Ogre::SceneManager *sceneManager()
            {
                return mSceneManager;
            }
            
        protected:
            
            ///name used in debug output
            Ogre::String logName();

            ///level folder
            File mPath;

            ///level name (i.e. name of the folder its loads its resources from)
            Ogre::String mName;

            /**
             * Pointer to steel's global scene manager.
             */
            Ogre::SceneManager *mSceneManager;

            /**
             * root node of the level. All level-dependant entities are its children.
             */
            Ogre::SceneNode *mLevelRoot;

            /**
             * agent container.
             */
            std::map<AgentId, Agent *> mAgents;

            /**
             * responsible for OgreModel's instances.
             */
            OgreModelManager *mOgreModelMan;

            /**
             * behavior tree manager.
             */
//	BTModelManager *mBTModelMan;

            ///
            unsigned int mResGroupAux;

            Camera *mCamera;
    };

}

#endif /* LEVEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
