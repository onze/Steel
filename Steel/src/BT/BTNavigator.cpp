#include <assert.h>
#include <iostream>

#include <OgreQuaternion.h>
#include <OgreSceneNode.h>

#include "BT/BTNavigator.h"
#include <Debug.h>
#include <tools/JsonUtils.h>
#include <tools/DynamicLines.h>
#include <BTModel.h>
#include <AgentManager.h>
#include <Agent.h>

namespace Steel
{

    const char *BTNavigator::TARGET_AGENT_ATTRIBUTE = "targetAgent";
    const char *BTNavigator::SPEED_ATTRIBUTE = "speed";
    const char *BTNavigator::LOOK_AT_TARGET = "lookAtTarget";

    const char *BTNavigator::TARGET_AGENT_ID_VARIABLE_ATTRIBUTE = "targetAgentIdVariable";


    BTNavigator::BTNavigator(const Steel::BTShapeToken &token) : BTNode(token),
        mTargetAgentStrategy(TargetAgentStrategy::None), mTargetAgentStrategyFn(nullptr),
        mTargetAgentIdVariable(Ogre::StringUtil::BLANK), mTargetAgent(INVALID_ID),
        mSpeed(.0f),
        mDebugTargetLine(nullptr), mDebugMoveLine(nullptr)
    {
        setTargetAgentStrategyFunction(mTargetAgentStrategy);
    }

    BTNavigator::BTNavigator(BTNavigator const &o): BTNode(o),
        mTargetAgentStrategy(o.mTargetAgentStrategy), mTargetAgentStrategyFn(nullptr),
        mTargetAgentIdVariable(o.mTargetAgentIdVariable), mTargetAgent(o.mTargetAgent),
        mSpeed(o.mSpeed), mDebugTargetLine(o.mDebugTargetLine), mDebugMoveLine(o.mDebugMoveLine)
    {
        setTargetAgentStrategyFunction(mTargetAgentStrategy);
    }

    BTNavigator::~BTNavigator()
    {
        deleteDebugLines();
    }

    void BTNavigator::deleteDebugLines()
    {
        if(nullptr != mDebugTargetLine)
        {
            mDebugTargetLine->clear();

            if(nullptr != mDebugTargetLine->getParentSceneNode())
                mDebugTargetLine->getParentSceneNode()->detachObject(mDebugTargetLine);

            delete mDebugTargetLine;
            mDebugTargetLine = nullptr;
        }

        if(nullptr != mDebugMoveLine)
        {
            mDebugMoveLine->clear();

            if(nullptr != mDebugMoveLine->getParentSceneNode())
                mDebugMoveLine->getParentSceneNode()->detachObject(mDebugMoveLine);

            delete mDebugMoveLine;
            mDebugMoveLine = nullptr;
        }
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

        mLookAtTarget = JsonUtils::asBool(root[BTNavigator::LOOK_AT_TARGET], false);

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

    void BTNavigator::initDebugLines(Ogre::SceneNode *parentNode)
    {
        if(nullptr == mDebugTargetLine)
        {
            mDebugTargetLine = new DynamicLines();
            mDebugTargetLine->setColor(Ogre::ColourValue::White);
            parentNode->attachObject(mDebugTargetLine);
        }

        if(nullptr == mDebugMoveLine)
        {
            mDebugMoveLine = new DynamicLines();
            mDebugMoveLine->setColor(Ogre::ColourValue::Red);
            parentNode->attachObject(mDebugMoveLine);
        }
    }

    void BTNavigator::run(BTModel *btModel, float timestep)
    {
        static Ogre::String intro = "in BTNavigator::run(): ";

        Agent *agent = btModel->level()->agentMan()->getAgent(btModel->ownerAgent());

        if(nullptr == agent)
        {
            Debug::error(intro)("owner agent ")(btModel->ownerAgent())(" does not exists. Aborting.").endl();
            mState = BTNodeState::FAILURE;
            return;
        }

        switch(mState)
        {
            case BTNodeState::READY:
                mTargetAgent = mTargetAgentStrategyFn(btModel);
                mState = BTNodeState::RUNNING;
                mPreviousPosition = agent->position() + agent->rotation() * Ogre::Vector3::UNIT_SCALE;

                if(btModel->debug())
                    initDebugLines(btModel->level()->levelRoot());

            case BTNodeState::RUNNING:
            {
                Agent *targetAgent;

                if(nullptr == (targetAgent = btModel->level()->agentMan()->getAgent(mTargetAgent)))
                {
                    Debug::error(intro)("targetAgent ")(mTargetAgent)(" does not exists. Aborting.").endl();
                    mState = BTNodeState::FAILURE;
                    return;
                }

                // move
                //WITH FORCES DUMBASS
                auto position = agent->position();
                auto targetPos = targetAgent->position();
                Ogre::Vector3 direction = ((position - mPreviousPosition).normalisedCopy() + (targetPos - position).normalisedCopy()) / 2.f;

//                 agent->move(direction * mSpeed);
                if(agent->velocity().length() < mSpeed)
                    agent->applyCentralImpulse(direction * mSpeed * timestep);

                // rotate
                if(mLookAtTarget)
                {
//                     if(mPreviousPosition != position)
                    {
//                         Ogre::Vector3 direction = (position - mPreviousPosition).normalisedCopy();
//                         agent->rotate(direction.getRotationTo(directionToTarget, Ogre::Vector3::UNIT_Y));
                        //agent->rotate((agent->rotation()*Ogre::Vector3::UNIT_SCALE).getRotationTo(direction, Ogre::Vector3::UNIT_Y));
                    }
                }

                // target reached ?
                if(agent->position().squaredDistance(targetPos) < .10)
                {
                    mState = BTNodeState::SUCCESS;

                    if(btModel->debug())
                        deleteDebugLines();
                }
                else if(btModel->debug())
                {
                    if(nullptr != mDebugTargetLine)
                    {
                        mDebugTargetLine->clear();
                        mDebugTargetLine->addPoint(position);
                        mDebugTargetLine->addPoint(targetPos);
                        mDebugTargetLine->update();
                    }

                    if(nullptr != mDebugMoveLine)
                    {
                        mDebugMoveLine->clear();
                        mDebugMoveLine->addPoint(position);
                        mDebugMoveLine->addPoint(position + direction * mSpeed * timestep);
                        mDebugMoveLine->update();
                    }
                }


                mPreviousPosition = position;
            }
            break;

            default:
                break;
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
