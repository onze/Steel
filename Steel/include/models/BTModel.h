#ifndef STEEL_BTMODEL_H_
#define STEEL_BTMODEL_H_

#include "BT/btnodetypes.h"
#include "BT/BTStateStream.h"

#include "Model.h"

namespace Steel
{

    class UnitTestExecutionContext;
    class BlackBoardModel;
    class UnitTestExecutionContext;
    class Level;
    class BTModelManager;
    /**
     * Instances of this class hold agent-specific data related to their BTree.
     * One can see them as a blackboard on which
     * BTNode subclasses can write stuff other nodes could need.
     * It is like a short&long term memory for an agent.
     * An agent's BTModel is also responsible for running the Btree it holds.
     */
    class BTModel: public Model
    {
        DECLARE_STEEL_MODEL(BTModel, ModelType::BT);

    public:
        /// Name of the BT this model is running
        static const char *SHAPE_NAME_ATTRIBUTE;
        /// Currently running state
        static const char *CURRENT_STATE_INDEX_ATTRIBUTE;
        /// Stack of current run
        static const char *STATES_STACK_ATTRIBUTE;


        BTModel();
        BTModel(const BTModel &m);
        BTModel &operator=(const BTModel &m);
        virtual ~BTModel();

        /**
         * Initialize a BTModel from a shapestream:
         * - creates the states stream matching the shape stream
         * - do other wonders
         */
        bool init(BTModelManager *manager, BTShapeStream *shapeStream);

        /// Deserialize itself from the given Json object. For internal use only, see BTModelManager::buildFromFile.
        bool fromJson(Json::Value const &node);

        /// Serialize itself into the given Json object
        void toJson(Json::Value &node);

        /// Sets the current shape to the given one.
        bool switchShapeTo(BTShapeStream *shapeStream);

        /// Runs the tree until its end or a node yields RUNNING.
        void update(float timestep);
        void cleanup();

        /// Sets the model's current path
        void setPath(Ogre::String const &name);
        void unsetPath();
        Ogre::String path();
        bool hasPath();

        void setSelected(bool flag);

        void pause() {mPaused = true;}
        void unpause() {mPaused = false;}
        bool paused() {return mPaused;}
        void kill() {mKilled = true;}

        void setBlackboardModelId(ModelId mid);

        ///////////////////////////////////////////////////////
        // BlackBoardModel shortcuts
        void setVariable(Ogre::String const &name, Ogre::String const &value);
        void setVariable(Ogre::String const &name, AgentId const &value);
        void unsetVariable(Ogre::String const &name);
        Ogre::String getStringVariable(Ogre::String const &name, Ogre::String const &defaultValue = StringUtils::BLANK);
        AgentId getAgentIdVariable(const Ogre::String &name, const Steel::AgentId &defaultValue = INVALID_ID);

        inline Level *level() const {return mLevel;}
        Ogre::String shapeName();

        /// Sets wich agent owns this model
        inline AgentId ownerAgent() const {return mOwnerAgent;}
        inline void setOwnerAgent(AgentId aid) {mOwnerAgent = aid;}

        inline bool debug() const {return mDebug;}
        inline void setDebug(bool flag) {mDebug = flag;}

        BTStateStream stateStream()const {return mStateStream;}

    protected:
        /**
         * Name of the string variable containing the name of the path the model currently
         * follows. This is the name the default path following BTree looks for.
         */
        static const Ogre::String CURRENT_PATH_NAME_VARIABLE;
        /// Link the owner agent with a newly created blackboard model, and returns a pointer to it, or nullptr.
        BlackBoardModel *getOwnerAgentBlackboard();

        // not owned
        AgentId mOwnerAgent;
        /// BLackboard the model is using as memory
        ModelId mBlackBoardModelId;
        Level *mLevel;

        // owned
        /// states, aligned with the shape stream.
        BTStateStream mStateStream;

        /// Index of the currently running node, within the StateStream.
        BTStateIndex mCurrentStateIndex;
        std::list<BTStateIndex> mStatesStack;

        /// Volatile state (not serialized). Can be undone.
        bool mPaused;

        /// Volatile state (not serialized). Cannot be undone.
        bool mKilled;

        /// Can be set to true to display debug information. Also used by BT nodes.
        bool mDebug;
    };

    bool utest_BTrees(UnitTestExecutionContext const *context);
}
#endif /* STEEL_BTMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

