
#include <assert.h>
#include <iostream>
#include <json/value.h>

#include "BT/BTFinder.h"
#include <Debug.h>
#include <tools/JsonUtils.h>
#include <BTModel.h>
#include <LocationModelManager.h>
#include <AgentManager.h>
#include <Agent.h>
#include <Level.h>

namespace Steel
{
    const char *BTFinder::SEARCH_STRATEGY_ATTRIBUTE = "searchStrategy";

    const char *BTFinder::TARGET_AGENT_ID_ATTRIBUTE = "targetVariable";

    const char *BTFinder::SOURCE_PATH_ATTRIBUTE = "sourcePath";
    const char *BTFinder::CURRENT_PATH_SOURCE_PATH_ATTRIBUTE_VALUE = "$current";

    BTFinder::BTFinder(const Steel::BTShapeToken &token) : BTNode(token),
        mSearchStrategy(SearchStrategy::None), mSearchStrategyFn(nullptr), mTargetAgentIdVariable(Ogre::StringUtil::BLANK), mSourcePath(Ogre::StringUtil::BLANK)
    {
        setSearchStrategyFunction(mSearchStrategy);
    }

    BTFinder::BTFinder(BTFinder const &o): BTNode(o),
        mSearchStrategy(o.mSearchStrategy), mSearchStrategyFn(nullptr), mTargetAgentIdVariable(o.mTargetAgentIdVariable), mSourcePath(o.mSourcePath)
    {
        setSearchStrategyFunction(mSearchStrategy);
    }

    BTFinder::~BTFinder()
    {
    }

    bool BTFinder::parseNodeContent(Json::Value &root)
    {
        mSearchStrategy = parseSearchStrategy(JsonUtils::asString(root[BTFinder::SEARCH_STRATEGY_ATTRIBUTE], Ogre::StringUtil::BLANK));
        setSearchStrategyFunction(mSearchStrategy);

        switch(mSearchStrategy)
        {
            case SearchStrategy::NextLocationInPath:
                mSourcePath = JsonUtils::asString(root[BTFinder::SOURCE_PATH_ATTRIBUTE], Ogre::StringUtil::BLANK);
                break;

            default:
                break;
        }

        mTargetAgentIdVariable = JsonUtils::asString(root[BTFinder::TARGET_AGENT_ID_ATTRIBUTE], Ogre::StringUtil::BLANK);
        return true;
    }

    BTFinder::SearchStrategy BTFinder::parseSearchStrategy(Ogre::String value)
    {
        // regular sucky enum parsing
        if("nextLocationInPath" == value)return SearchStrategy::NextLocationInPath;

        if("none" != value)
            Debug::warning("BTFinder::parseSearchStrategy(): unknown value ").quotes(value).endl();

        return SearchStrategy::None;
    }

    void BTFinder::setSearchStrategyFunction(SearchStrategy s)
    {
        switch(mSearchStrategy)
        {
            case SearchStrategy::NextLocationInPath:
                mSearchStrategyFn = std::bind(&BTFinder::nextLocationInPathStrategyFindFn, this, std::placeholders::_1);
                break;

            case SearchStrategy::None:
                mSearchStrategyFn = std::bind(&BTFinder::noneStrategyFindFn, this, std::placeholders::_1);
                break;
        }
    }

    AgentId BTFinder::noneStrategyFindFn(BTModel *btModel)
    {
        return INVALID_ID;
    }

    AgentId BTFinder::nextLocationInPathStrategyFindFn(BTModel *btModel)
    {
        static Ogre::String intro = "in BTFinder::nextLocationInPathStrategyFindFn(): ";

        auto locMan = btModel->level()->locationModelMan();

        if(nullptr == locMan)
        {
            Debug::error(intro)("Level has no locationModelManager. Cannot process to path lookup. Aborting.").endl();
            return INVALID_ID;
        }

        AgentId currentLocationAgentId = btModel->getAgentIdVariable(mTargetAgentIdVariable);
        AgentId nextLocationAgentId = INVALID_ID;

        // previously saved result: get the next location
        if(INVALID_ID != currentLocationAgentId)
        {
            Agent *currentLocationAgent = btModel->level()->agentMan()->getAgent(currentLocationAgentId);

            if(nullptr == currentLocationAgent)
            {
                nextLocationAgentId = INVALID_ID;
            }
            else
            {
                LocationModel const *const lModel = currentLocationAgent->locationModel();

                if(nullptr != lModel)
                {
                    nextLocationAgentId = *(lModel->destinations().begin());
                }
            }
        }

        // no previously saved result: get the agent's path source
        if(INVALID_ID == nextLocationAgentId)
        {
            LocationPathName sourcePath = mSourcePath;

            if(BTFinder::CURRENT_PATH_SOURCE_PATH_ATTRIBUTE_VALUE == mSourcePath)
            {
                sourcePath = btModel->path();
            }

            if(LocationModel::EMPTY_PATH == sourcePath)
            {
                Debug::error(intro)("agent ")(btModel->ownerAgent())("'s btModel ")
                (btModel->level()->agentMan()->getAgent(btModel->ownerAgent())->btModelId())
                (" with shape ").quotes(btModel->shapeName())(" has no path. Aborting.").endl();
                btModel->kill();
                return INVALID_ID;
            }

            // get path root
            nextLocationAgentId = locMan->pathRoot(sourcePath);
        }

        return nextLocationAgentId;
    }

    void BTFinder::run(BTModel *btModel, float timestep)
    {
        AgentId aid = mSearchStrategyFn(btModel);

        if(INVALID_ID == aid)
        {
            mState = BTNodeState::FAILURE;
        }
        else
        {
            btModel->setVariable(mTargetAgentIdVariable, aid);
            mState = BTNodeState::SUCCESS;
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
