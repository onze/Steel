#include <assert.h>
#include <iostream>

#include "BT/BTNavigator.h"
#include <Debug.h>
#include <tools/JsonUtils.h>
#include <BTModel.h>
#include <AgentManager.h>
#include <Agent.h>

namespace Steel
{

    const char *BTNavigator::TARGET_AGENT_ATTRIBUTE = "targetAgent";

    const char *BTNavigator::TARGET_AGENT_ID_VARIABLE_ATTRIBUTE = "targetAgentIdVariable";

    const char *BTNavigator::SPEED_ATTRIBUTE = "speed";

    BTNavigator::BTNavigator(const Steel::BTShapeToken &token) : BTNode(token),
        mTargetAgentStrategy(TargetAgentStrategy::None), mTargetAgentStrategyFn(nullptr),
        mTargetAgentIdVariable(Ogre::StringUtil::BLANK), mTargetAgent(INVALID_ID),
        mSpeed(.0f)
    {
        setTargetAgentStrategyFunction(mTargetAgentStrategy);
    }

    BTNavigator::BTNavigator(BTNavigator const &o): BTNode(o),
        mTargetAgentStrategy(o.mTargetAgentStrategy), mTargetAgentStrategyFn(nullptr),
        mTargetAgentIdVariable(o.mTargetAgentIdVariable), mTargetAgent(o.mTargetAgent),
        mSpeed(o.mSpeed)
    {
        setTargetAgentStrategyFunction(mTargetAgentStrategy);
    }

    BTNavigator::~BTNavigator()
    {
    }

    bool BTNavigator::parseNodeContent(Json::Value &root)
    {
        mTargetAgentStrategy = parseTargetAgentStrategy(JsonUtils::asString(root[BTNavigator::TARGET_AGENT_ATTRIBUTE], Ogre::StringUtil::BLANK));
        setTargetAgentStrategyFunction(mTargetAgentStrategy);

        switch(mTargetAgentStrategy)
        {
            case TargetAgentStrategy::FromVariable:
                mTargetAgentIdVariable = JsonUtils::asString(root[BTNavigator::TARGET_AGENT_ID_VARIABLE_ATTRIBUTE]);
                break;

            default:
                break;
        }

        mSpeed = JsonUtils::asFloat(root[BTNavigator::SPEED_ATTRIBUTE]);

        return true;
    }

    BTNavigator::TargetAgentStrategy BTNavigator::parseTargetAgentStrategy(Ogre::String value)
    {
        // regular sucky enum parsing
        if("$fromVariable" == value)return TargetAgentStrategy::FromVariable;

        if("none" != value)
            Debug::warning("BTNavigator::parseSearchStrategy(): unknown value ").quotes(value).endl();

        return TargetAgentStrategy::None;
    }

    void BTNavigator::setTargetAgentStrategyFunction(TargetAgentStrategy s)
    {
        switch(s)
        {
            case TargetAgentStrategy::FromVariable:
                mTargetAgentStrategyFn = std::bind(&BTNavigator::fromVariableStrategyTargetAgentFn, this, std::placeholders::_1);
                break;

            default:
                mTargetAgentStrategyFn = std::bind(&BTNavigator::noneStrategyTargetAgentFn, this, std::placeholders::_1);
                break;
        }
    }

    AgentId BTNavigator::fromVariableStrategyTargetAgentFn(BTModel *btModel)
    {
        return btModel->getAgentIdVariable(mTargetAgentIdVariable);
    }

    AgentId BTNavigator::noneStrategyTargetAgentFn(BTModel *btModel)
    {
        return INVALID_ID;
    }

    void BTNavigator::run(BTModel *btModel, float timestep)
    {
        static Ogre::String intro = "in BTNavigator::run(): ";

        switch(mState)
        {
            case BTNodeState::READY:
                mTargetAgent = mTargetAgentStrategyFn(btModel);
                mState = BTNodeState::RUNNING;

            case BTNodeState::RUNNING:
            {
                Agent *targetAgent, *agent;

                if(nullptr == (targetAgent = btModel->level()->agentMan()->getAgent(mTargetAgent)))
                {
                    Debug::error(intro)("targetAgent ")(mTargetAgent)(" does not exists. Aborting.").endl();
                    mState = BTNodeState::FAILURE;
                    return;
                }

                if(nullptr == (agent = btModel->level()->agentMan()->getAgent(btModel->ownerAgent())))
                {
                    Debug::error(intro)("owner agent ")(btModel->ownerAgent())(" does not exists. Aborting.").endl();
                    mState = BTNodeState::FAILURE;
                    return;
                }

                // move
                auto targetPos = targetAgent->position();
                Ogre::Vector3 direction = (targetPos - agent->position()).normalisedCopy();
                agent->move(direction * mSpeed);

                // target reached ?
                if(agent->position().squaredDistance(targetPos) < mSpeed * mSpeed)
                {
                    mState = BTNodeState::SUCCESS;
                }
            }
            break;

            default:
                break;
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
