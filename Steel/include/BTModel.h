#ifndef STEEL_BTMODEL_H_
#define STEEL_BTMODEL_H_

#include <stack>
#include <json/json.h>

#include "BT/btnodetypes.h"
#include "BT/BTStateStream.h"

#include "Model.h"
#include "BlackBoardModel.h"
#include "Level.h"
#include "BlackBoardModelManager.h"

namespace Steel
{
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
    public:
        static const char *SHAPE_NAME_ATTRIBUTE;

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

        static ModelType modelType()
        {
            return MT_BT;
        };

        /// Deserialize itself from the given Json object. For internal use only, see BTModelManager::buildFromFile.
        virtual bool fromJson(Json::Value &node);

        /// Serialize itself into the given Json object
        virtual void toJson(Json::Value &node);

        /// Sets the current shape to the given one.
        bool switchShapeTo(BTShapeStream *shapeStream);

        /// Runs the tree until its end or a node yields RUNNING.
        void update(float timestep);
        virtual void cleanup();

        /// Sets the model's current path
        void setPath(Ogre::String const &name);
        Ogre::String path();
        bool hasPath();

        void setSelected(bool flag);

        void pause() {mPaused = true;}
        void unpause() {mPaused = false;}
        bool paused() {return mPaused;}
        void kill(){mKilled=true;}
        
        void setBlackboardModelId(ModelId mid);
        
        ///////////////////////////////////////////////////////
        // BlackBoardModel shortcuts
        void setVariable(Ogre::String const &name, Ogre::String const &value);
        void setVariable(Ogre::String const &name, AgentId const &value);
        Ogre::String getStringVariable(Ogre::String const &name, Ogre::String const &defaultValue=Ogre::StringUtil::BLANK);
        AgentId getAgentIdVariable(const Ogre::String &name, const Steel::AgentId &defaultValue = INVALID_ID);
        
        inline Level *level(){return mLevel;}
        
    protected:
        /**
         * Name of the string variable containing the name of the path the model currently 
         * follows. This is the name the default path following BTree looks for.
         */
        static const Ogre::String CURRENT_PATH_NAME_VARIABLE;
        
        // not owned
        /// BLackboard the model is using as memory
        ModelId mBlackBoardModelId;
        Level *mLevel;
        
        // owned
        /// See SHAPE_NAME_ATTRIBUTE
        Ogre::String mShapeName;

        /// states, aligned with the shape stream.
        BTStateStream mStateStream;

        /// Index of the currently running node, within the StateStream.
        BTStateIndex mCurrentStateIndex;
        std::stack<BTStateIndex> mStatesStack;
        
        /// Volatile state (not serialized). Can be undone.
        bool mPaused;
        
        /// Volatile state (not serialized) Cannot be undone.
        bool mKilled;
    };

}
#endif /* STEEL_BTMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

