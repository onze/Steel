#ifndef STEEL_LEVEL_H_
#define STEEL_LEVEL_H_

#include <memory>

#include "steeltypes.h"
#include "tools/ConfigFile.h"
#include "terrain/TerrainManager.h"
#include "terrain/TerrainManagerEventListener.h"

namespace Steel
{
    class Engine;
    class Agent;
    class Camera;
    class AgentManager;
    class ModelManager;
    class OgreModelManager;
    class PhysicsModelManager;
    class LocationModelManager;
    class BTModelManager;
    class BlackBoardModelManager;
    class SelectionManager;

    class Level: public TerrainManagerEventListener
    {
    private:
        Level();
        Level(Level &level);
        Level &operator=(const Level &level);

        static const char *BACKGROUND_COLOR_ATTRIBUTE;
        static const char *NAME_ATTRIBUTE;
        static const char *CAMERA_ATTRIBUTE;
        static const char *TERRAIN_ATTRIBUTE;
        static const char *AGENTS_ATTRIBUTE;
        static const char *MANAGERS_ATTRIBUTE;
        static const char *GRAVITY_ATTRIBUTE;

        /// Model resource parsing
        static const char *MODELS_ATTRIBUTE;
        static const char *MODEL_TYPE_ATTRIBUTE;
        static const char *MODEL_PATH_ATTRIBUTE;
        static const char *MODEL_REF_OVERRIDE_ATTRIBUTE;

    public:
        Level(Engine *engine, File path, Ogre::String name);
        virtual ~Level();

        /// Read properties in the given string and set them where they should.
        bool deserialize(Ogre::String &s);

        /// Ffills the list of AgentId with agents that own nodes in the the given list.
        void getAgentsIdsFromSceneNodes(std::list<Ogre::SceneNode *> &nodes, Selection &selection);

        /// Returns the name of the json file that contains this level's properies.
        File getSavefile();

        void loadConfig(ConfigFile const &config);

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
         * Saves a seralization string into a file that can be loaded and read back with a call to load.
         * Return true if the saving went successfully.
         */
        bool save();
        /// Collects level's agents' properties and put them in a string.
        void serialize(Ogre::String &s);

        /// Main loop iteration.
        void update(float timestep);

        /// Finds the agent owning the first OgreModel under the mouse, and returns its id.
        AgentId agentIdUnderMouse();
        /// Finds the agent owning the first OgreModel under the mouse, and returns its model of the given type.
        ModelId modelIdUnderMouse(ModelType mType);

        Ogre::Vector3 getDropTargetPosition();
        Ogre::Quaternion getDropTargetRotation();
        Ogre::Vector2 getSlotDropPosition();
        
        //////////////////////////////////////////////////////
        // edition
        /**
         * Preprocess a resource file (fills dynamic values), and instanciate its description if all requirements are satisfied.
         * - if the file (absolute path or relative to the data directory) content contains an aid, the resource will be attached to the pointed agent (created if needed).
         * - if the aid argument is not INVALID_ID, the resource will be attached to the pointed agent (created if needed).
         * The aid given as argument has precedence over the one in the serialization.
         * This agentId to which the resource is attached is eventually returned.
         */
        bool instanciateResource(File const &file, AgentId &aid);
        /// Forward call to instanciateResource(File& file, AgentId &aid), and discards the aid.
        bool instanciateResource(File const &file);

        /**
         * Instanciate a model from its serialization.
         * If aid is valid (!=INVALID_ID), the model is attached to that agent. Otherwise, a new agent is created,
         * aid is set to its id, and then the model is attached to it.
         * Returns false if a stopping problem occured.
         */
        bool loadModelFromSerialization(Json::Value &root, AgentId &aid);

        /// Instanciate one or many models from a serialization, and returns the AgentId of the agent that controls it.
        bool loadModelsFromSerializations(Json::Value &root, AgentId &aid);

        /// Instanciate models from a .model_refs file content.
        bool loadModelsReferencesFromSerializations(Json::Value &root, Steel::AgentId &aid);

        /**
         * reads an incomplete terrain slot file from the data folder, fills the incomplete parts (i.e.: terrain position),
         * and instanciate it.
         */
        bool loadTerrainSlotFromSerialization(Json::Value &root);

        //////////////////////////////////////////////////////
        //getters
        inline Ogre::ColourValue backgroundColor() const {return mBackgroundColor;}

        /// Player camera. For now there's only one camera per level.
        inline Camera *camera() const {return mCamera;}

        inline Ogre::String name() const {return mName;}
        inline File path() const {return mPath;}
        inline Ogre::SceneNode *levelRoot() const {return mLevelRoot;}
        inline Ogre::Vector3 gravity() const {return mGravity;}

        ///Return the level's model manager for the given type.
        ModelManager *modelManager(ModelType modelType);

        inline OgreModelManager *const ogreModelMan() const {return mOgreModelMan;}
        inline PhysicsModelManager *const physicsModelMan() const {return mPhysicsModelMan;}
        inline Ogre::SceneManager *const sceneManager() const {return mSceneManager;}
        inline TerrainManager *terrainManager() {return &mTerrainMan;}
        inline BTModelManager *const BTModelMan() const {return mBTModelMan;}
        inline AgentManager *const agentMan() const {return mAgentMan;}
        inline SelectionManager *const selectionMan() const {return mSelectionMan;}
        inline LocationModelManager *const locationModelMan() const {return mLocationModelMan;}
        inline BlackBoardModelManager *const blackBoardModelMan() const {return mBlackBoardModelManagerMan;}

        inline Engine *const engine() const {return mEngine;}
        inline Ogre::Viewport *viewport() const {return mViewport;}
        
        enum class PublicSignal : u32
        {
            loaded = 0
        };
        Signal getSignal(Level::PublicSignal signal) const;
    private:
        /// name used in debug output
        Ogre::String logName() const;

        /// If found and set to true in an object of a model serialization, values of the object skip their way through dynamicFillSerialization.
        static const char *DF_CANCEL_DYNAMIC_FILLING_ATTRIBUTE;
        /**
         * Fills dynamic fields with values of dynamic queries. Optional aid helps filling dynamic
         * fields involving the owner agent. This method is recursive.
         */
        bool dynamicFillSerialization(Json::Value &node, Steel::AgentId aid = INVALID_ID);

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
        LocationModelManager *mLocationModelMan;
        BlackBoardModelManager *mBlackBoardModelManagerMan;

        /// Main camera
        Camera *mCamera;

        Ogre::Light *mMainLight;
        /// See GRAVITY_ATTRIBUTE
        Ogre::Vector3 mGravity;
    };
}

#endif /* STEEL_LEVEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

