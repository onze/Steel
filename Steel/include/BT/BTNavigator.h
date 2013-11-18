#ifndef STEEL_BTNAVIGATOR_H_
#define STEEL_BTNAVIGATOR_H_

#include <OgreVector3.h>

#include <BT/btnodetypes.h>
#include <BT/BTNode.h>
#include <steeltypes.h>

namespace Ogre
{
    class SceneNode;
}

namespace Steel
{
    class BTModel;
    class DynamicLines;
    class BTNavigator: public BTNode
    {
        /// Where to look for a target
        static const char *TARGET_AGENT_ATTRIBUTE;

        /// the name of the variable that contains the target AgentId
        static const char *TARGET_AGENT_ID_VARIABLE_ATTRIBUTE;

        /// The speed at which to move the agent
        static const char *SPEED_ATTRIBUTE;
        
        /// Orient the OgreModel toward the current target.
        static const char *LOOK_AT_TARGET;

    public:
        inline static BTShapeTokenType tokenType()
        {
            return BTNavigatorToken;
        }
        
        BTNavigator(BTShapeToken const &token);
        BTNavigator(BTNavigator const &o);
        virtual ~BTNavigator();
        void run(BTModel *btModel, float timestep);

    protected:
        enum class TargetAgentStrategy : int
        {
            None = 0,
            /// The target AgentId will be searched for in the agent's blackboard
            FromVariable
        };
        /// See BTNode::parseNodeContent
        bool parseNodeContent(Json::Value &root);
        TargetAgentStrategy parseTargetAgentStrategy(Ogre::String value);
        void setTargetAgentStrategyFunction(TargetAgentStrategy s);
        
        AgentId fromVariableStrategyTargetAgentFn(BTModel *btModel);
        AgentId noneStrategyTargetAgentFn(BTModel *btModel);
        
        void initDebugLines(Ogre::SceneNode *parentNode);
        void deleteDebugLines();

        // not owned
        // owned
        TargetAgentStrategy mTargetAgentStrategy;
        /// Strategy function. See BTFinder::TARGET_AGENT_ATTRIBUTE
        std::function<AgentId(BTModel *btModel)> mTargetAgentStrategyFn;
        
        Ogre::String mTargetAgentIdVariable;
        AgentId mTargetAgent;
        Ogre::Real mSpeed;
        bool mLookAtTarget;
        /// Agent position last frame.
        Ogre::Vector3 mPreviousPosition;
        
        /// points to the current target
        DynamicLines *mDebugTargetLine;
        /// represent the speed and direction of the model
        DynamicLines *mDebugMoveLine;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
