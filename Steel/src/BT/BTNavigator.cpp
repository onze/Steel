#include <assert.h>
#include <iostream>

#include <Ogre.h>

#include "Debug.h"
#include "Engine.h"
#include "Level.h"
#include "BT/BTNavigator.h"
#include "models/BTModel.h"
#include "models/AgentManager.h"
#include "models/Agent.h"
#include "tools/JsonUtils.h"
#include "tools/DynamicLines.h"

namespace Steel
{

    const char *BTNavigator::TARGET_AGENT_ATTRIBUTE = "targetAgent";
    const char *BTNavigator::SPEED_ATTRIBUTE = "speed";
    const char *BTNavigator::LOOK_AT_TARGET = "lookAtTarget";

    const char *BTNavigator::TARGET_AGENT_ID_VARIABLE_ATTRIBUTE = "targetAgentIdVariable";


    BTNavigator::BTNavigator(const Steel::BTShapeToken &token) : BTNode(token),
        mTargetAgentStrategy(TargetAgentStrategy::None), mTargetAgentStrategyFn(nullptr),
        mTargetAgentIdVariable(StringUtils::BLANK), mTargetAgent(INVALID_ID),
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
        mTargetAgentStrategy = parseTargetAgentStrategy(JsonUtils::asString(root[BTNavigator::TARGET_AGENT_ATTRIBUTE], StringUtils::BLANK));
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
        Agent *agent = btModel->level()->agentMan()->getAgent(btModel->ownerAgent());

        if(nullptr == agent)
        {
            Debug::error(STEEL_METH_INTRO, "owner agent ", btModel->ownerAgent(), " does not exists. Aborting.").endl();
            mState = BTNodeState::FAILURE;
            return;
        }

        switch(mState)
        {
            case BTNodeState::READY:
                mTargetAgent = mTargetAgentStrategyFn(btModel);
                mState = BTNodeState::RUNNING;
                mPreviousPosition = agent->position() + agent->rotation() * Ogre::Vector3::UNIT_SCALE;

                if(0 && btModel->debug() && btModel->level()->engine()->stats().frameCount > 100)
                    initDebugLines(btModel->level()->levelRoot());

            case BTNodeState::RUNNING:
            {
                Agent *targetAgent;

                if(nullptr == (targetAgent = btModel->level()->agentMan()->getAgent(mTargetAgent)))
                {
                    Debug::error(STEEL_METH_INTRO, "targetAgent ", mTargetAgent, " does not exists. Aborting.").endl();
                    mState = BTNodeState::FAILURE;
                    return;
                }

                // move
                auto position = agent->position();
                auto targetPos = targetAgent->position();
                auto velocity = agent->velocity();
                Ogre::Vector3 direction = (targetPos - position).normalisedCopy();
                
                if(velocity.squaredLength() < mSpeed * mSpeed)
                    agent->applyCentralImpulse(direction * mSpeed * timestep);

                // target reached ?
                if(agent->position().squaredDistance(targetPos) < velocity.squaredLength())
                {
                    mState = BTNodeState::SUCCESS;

                    if(btModel->debug())
                        deleteDebugLines();
                }
                else
                {
                    // rotate
                    if(mLookAtTarget)
                    {
                        Ogre::Quaternion rotation = agent->rotation();
                        Ogre::Vector3 srcDir = rotation * Ogre::Vector3::UNIT_Z;
//                         srcDir.y=0;
                        srcDir.normalise();

                        agent->applyTorqueImpulse(srcDir.crossProduct(direction) * timestep);

//                         Ogre::Quaternion dstRotation = srcDir.getRotationTo(direction);
//                         agent->setBodyRotation(dstRotation*rotation);
                    }

                    if(btModel->debug())
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
                            mDebugMoveLine->addPoint(position + velocity);
                            mDebugMoveLine->update();
                        }
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

