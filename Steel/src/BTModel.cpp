
#include <OgreString.h>

#include "BTModel.h"
#include "BT/BTNode.h"
#include <tools/JsonUtils.h>
#include <BTModelManager.h>
#include <Level.h>
#include <BlackBoardModelManager.h>
#include <LocationModel.h>
#include <AgentManager.h>
#include <Agent.h>

namespace Steel
{
    const char *BTModel::SHAPE_NAME_ATTRIBUTE = "rootPath";
    const char *BTModel::CURRENT_STATE_INDEX_ATTRIBUTE = "currentStateIndex";
    const char *BTModel::STATES_STACK_ATTRIBUTE = "statesStack";

    const Ogre::String BTModel::CURRENT_PATH_NAME_VARIABLE = "__currentPath";

    BTModel::BTModel():
        mOwnerAgent(INVALID_ID), mBlackBoardModelId(INVALID_ID), mLevel(nullptr),
        mStateStream(), mCurrentStateIndex(0), mStatesStack(),
        mPaused(false), mKilled(false), mDebug(false)
    {
    }

    BTModel::BTModel(const BTModel &o):
        mOwnerAgent(o.mOwnerAgent), mBlackBoardModelId(o.mBlackBoardModelId), mLevel(o.mLevel),
        mStateStream(o.mStateStream), mCurrentStateIndex(o.mCurrentStateIndex), mStatesStack(o.mStatesStack),
        mPaused(o.mPaused), mKilled(o.mKilled), mDebug(o.mDebug)
    {
    }

    BTModel::~BTModel()
    {
    }

    BTModel &BTModel::operator=(const BTModel &o)
    {
        if(this == &o)
            return *this;

        if(!isFree())
            cleanup();

        if(!o.isFree())
        {
            mOwnerAgent = o.mOwnerAgent;
            mBlackBoardModelId = o.mBlackBoardModelId;
            mLevel = o.mLevel;
            mStateStream = o.mStateStream;
            mCurrentStateIndex = o.mCurrentStateIndex;
            mStatesStack = o.mStatesStack;
            mPaused = o.mPaused;
            mKilled = o.mKilled;
        }

        return *this;
    }

    bool BTModel::init(Steel::BTModelManager *manager, Steel::BTShapeStream *shapeStream)
    {
        mPaused = mKilled = false;
        mLevel = manager->level();
        mBlackBoardModelId = INVALID_ID;
        return switchShapeTo(shapeStream);
    }

    bool BTModel::switchShapeTo(BTShapeStream *shapeStream)
    {
        mStateStream.clear();
        mCurrentStateIndex = 0;
        mStatesStack.clear();

        if(!mStateStream.init(shapeStream))
        {
            Debug::error("BTModel::switchShapeTo(): can't init stateStream from shapeStream: ")(shapeStream).endl();
            return false;
        }

        return true;
    }

    bool BTModel::fromJson(Json::Value &node)
    {
        mCurrentStateIndex = JsonUtils::asUnsignedLong(BTModel::CURRENT_STATE_INDEX_ATTRIBUTE, 0);
        mStatesStack = JsonUtils::asUnsignedLongList(BTModel::STATES_STACK_ATTRIBUTE);
        return true;
    }

    void BTModel::toJson(Json::Value &node)
    {
        static const Ogre::String intro = "in BTModel::toJson(): ";
        BTShapeStream *st = mStateStream.shapeStream();

        if(nullptr == st)
        {
            Debug::error(intro)("stateStream's has no shapeStream !").endl();
            node[BTModel::SHAPE_NAME_ATTRIBUTE] = Json::Value::null;
        }
        else
            node[BTModel::SHAPE_NAME_ATTRIBUTE] = JsonUtils::toJson(shapeName());

        if(INVALID_ID != mCurrentStateIndex)
            node[BTModel::CURRENT_STATE_INDEX_ATTRIBUTE] = JsonUtils::toJson(mCurrentStateIndex);

        if(mStatesStack.size() > 0)
            node[BTModel::STATES_STACK_ATTRIBUTE] = JsonUtils::toJson(mStatesStack);
    }

    void BTModel::cleanup()
    {
        mStateStream.clear();
        Model::cleanup();
    }

    void BTModel::update(float timestep)
    {
        bool debug = false;

        static const Ogre::String intro = "in BTModel::update(): ";

        if(mPaused || mKilled)
            return;

        BTStateIndex prevStateIndex = mCurrentStateIndex;
        BTStateIndex newIndex = mCurrentStateIndex;
        BTNode *node = nullptr;
        BTNode *parent = nullptr;
        BTNodeState nodeState = BTNodeState::SUCCESS;
        BTShapeToken token;

        while(true)
        {
            if(nodeState != BTNodeState::READY)
            {
                prevStateIndex = mCurrentStateIndex;
                token = mStateStream.tokenAt(mCurrentStateIndex);

                if(BTUnknownToken == token.type)
                {
                    Debug::error(intro)("node #")(prevStateIndex)(" with token ")(mStateStream.tokenAt(prevStateIndex))
                    ("yielded an unknown token type as index ")(mCurrentStateIndex)
                    (". Node:")(node).endl()("Aborting BTNode update.").endl();
                    kill();
                    return;
                }

                node = mStateStream.stateAt(mCurrentStateIndex);
            }

            nodeState = node->state();

            switch(nodeState)
            {
                case BTNodeState::READY:
                    if(debug)
                        Debug::log("running ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();

                    node->run(this, timestep);
                    continue;

                case BTNodeState::SKIPT_TO:
                    newIndex = node->nodeSkippedTo();

                    if(node->begin() == newIndex)
                    {
                        Debug::error(intro)("node #")(mCurrentStateIndex)(" with token ")(token)
                        (" has SKIPT_TO state pointing to itself. To run again, it should keep the READY state.").endl()
                        ("Aborting tree run.");
                        mStatesStack.clear();
                        mCurrentStateIndex = 0;
                        return;
                    }

                    if(debug)
                        Debug::log("skipping to  #")(newIndex).endl();

                    // visiting a child
                    if(newIndex < node->end())
                    {
                        mStatesStack.push_back(mCurrentStateIndex);
                        mCurrentStateIndex = newIndex;
                        continue;
                    }
                    else
                    {
                        // node tries to skip to not a child: assume success
                        newIndex = node->end();
                        nodeState = BTNodeState::SUCCESS;
                    }

                case BTNodeState::FAILURE:
                case BTNodeState::SUCCESS:

                    if(debug)
                        Debug::log(nodeState == BTNodeState::SUCCESS ? "SUCCESS" : "FAILURE")(" from ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();

                    // root's success
                    if(0 == mStatesStack.size())
                    {
                        mCurrentStateIndex = 0;
                        node->onParentNotified();
                        return;
                    }

                    parent = mStateStream.stateAt(mStatesStack.back());
                    parent->childReturned(node, nodeState);
                    node->onParentNotified();
                    // give control back to parent
                    mCurrentStateIndex = mStatesStack.back();
                    mStatesStack.pop_back();
                    continue;

                case BTNodeState::RUNNING:
                    if(debug)
                        Debug::log("keeping running ")(BTShapeTokenTypeAsString[token.type])(" #")(token.begin).endl();

                    node->run(this, timestep);
                    // stop execution for this frame.
                    return;

                default:
                    Debug::error(intro)("node #")(mCurrentStateIndex)(" with token ")(token)
                    ("yielded an unknown state.")(". Node:")(node).endl()("Aborting BTNode update.").endl();
                    return;
            }
        }
    }

    void BTModel::setSelected(bool flag)
    {
        mPaused = flag;
    }

    void BTModel::setBlackboardModelId(ModelId mid)
    {
        mBlackBoardModelId = mid;
    }
    
    void BTModel::setPath(Ogre::String const &name)
    {
        if(LocationModel::EMPTY_PATH == name)
            return;
        
        setVariable(BTModel::CURRENT_PATH_NAME_VARIABLE, name);
    }
    
    void BTModel::unsetPath()
    {
        unsetVariable(BTModel::CURRENT_PATH_NAME_VARIABLE);
    }

    Ogre::String BTModel::path()
    {
        return getStringVariable(BTModel::CURRENT_PATH_NAME_VARIABLE, LocationModel::EMPTY_PATH);
    }

    bool BTModel::hasPath()
    {
        return LocationModel::EMPTY_PATH == path();
    }

    void BTModel::setVariable(Ogre::String const &name, Ogre::String const &value)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);

        if(nullptr == bbModel)
        {
            bbModel = getOwnerAgentABlackboard();

            if(nullptr == bbModel)
            {
                Debug::error("in BTModel::setVariable(): could not get ownerAgent ")(mOwnerAgent)(" a blackboard model. Aborting.").endl();
                kill();
                return;
            }
        }

        bbModel->setVariable(name, value);
    }

    BlackBoardModel *BTModel::getOwnerAgentABlackboard()
    {
        Agent *agent = mLevel->agentMan()->getAgent(mOwnerAgent);

        if(nullptr == agent)
            return nullptr;

        BlackBoardModel *bbModel = agent->blackBoardModel();

        if(nullptr == bbModel)
        {
            ModelId bbMid = mLevel->blackBoardModelMan()->newModel();
            agent->linkToModel(MT_BLACKBOARD, bbMid);
            bbModel = agent->blackBoardModel();
        }

        return bbModel;
    }

    void BTModel::setVariable(Ogre::String const &name, AgentId const &value)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);

        if(nullptr == bbModel)
        {
            bbModel = getOwnerAgentABlackboard();

            if(nullptr == bbModel)
            {
                Debug::error("in BTModel::setVariable(): could not get ownerAgent ")(mOwnerAgent)(" a blackboard model. Aborting.").endl();
                kill();
                return;
            }
        }

        bbModel->setVariable(name, value);
    }

    Ogre::String BTModel::getStringVariable(Ogre::String const &name, Ogre::String const &defaultValue/*=Ogre::StringUtil::BLANK*/)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);

        if(nullptr == bbModel)
            return defaultValue;

        return bbModel->getStringVariable(name);
    }

    AgentId BTModel::getAgentIdVariable(Ogre::String const &name, AgentId const &defaultValue/*=INVALID_ID*/)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);

        if(nullptr == bbModel)
            return defaultValue;

        return bbModel->getAgentIdVariable(name);
    }
    
    void BTModel::unsetVariable(Ogre::String const &name)
    {
        BlackBoardModel *bbModel = mLevel->blackBoardModelMan()->at(mBlackBoardModelId);
        
        if(nullptr != bbModel)
            bbModel->unsetVariable(name);
    }

} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
