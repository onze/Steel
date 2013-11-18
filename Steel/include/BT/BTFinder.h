#ifndef STEEL_BTFINDER_H_
#define STEEL_BTFINDER_H_

#include <steeltypes.h>
#include <BT/btnodetypes.h>
#include <BT/BTNode.h>

namespace Steel
{
    /**
     * BTFinder looks for a spec(ification) and if the spec is matched,
     * saves the result in the BTModel under the specified dest(ination).
     */
    class BTFinder: public BTNode
    {
    public:
        /// How to find the target.
        static const char *SEARCH_STRATEGY_ATTRIBUTE;
        ///// Possible value for SEARCH_STRATEGY_ATTRIBUTE. -> taken from enum parsing
        //static const char *NEXT_IN_PATH_SEARCH_STRATEGY_ATTRIBUTE_VALUE;

        /// Variable under which to save the value.
        static const char *TARGET_AGENT_ID_ATTRIBUTE;
        
        /// Name of the path to search in. Relevant with NextLocationInPath strategy.
        static const char *SOURCE_PATH_ATTRIBUTE;
        /// Possible value for SOURCE_PATH_ATTRIBUTE. Makes the node look into the agent's current path.
        static const char *CURRENT_PATH_SOURCE_PATH_ATTRIBUTE_VALUE;

        inline static BTShapeTokenType tokenType()
        {
            return BTFinderToken;
        }

        BTFinder(BTShapeToken const &token);
        BTFinder(BTFinder const &o);
        virtual ~BTFinder();
        void run(BTModel *btModel, float timestep);

    private:
        /// How the arget will be looked for
        enum class SearchStrategy : int
        {
            None = 0,
            NextLocationInPath,
        };
        SearchStrategy parseSearchStrategy(Ogre::String value);
        void setSearchStrategyFunction(SearchStrategy s);
        
        /// See BTNode::parseNodeContent
        bool parseNodeContent(Json::Value &root);
        
        AgentId noneStrategyFindFn(BTModel *btModel);
        AgentId nextLocationInPathStrategyFindFn(BTModel *btModel);

        // not owned
        //owned
        SearchStrategy mSearchStrategy;
        /// Strategy function. See BTFinder::SEARCH_STRATEGY_ATTRIBUTE
        std::function<AgentId(BTModel *btModel)> mSearchStrategyFn;
        
        /// see BTFinder::TARGET_AGENT_ID_ATTRIBUTE
        Ogre::String mTargetAgentIdVariable;
        
        /////////////////
        // specific to SearchStrategy::NextLocationInPath
        /// see BTFinder::NEXT_IN_PATH_STRAT_SOURCE_PATH_ATTRIBUTE
        LocationPathName mSourcePath;
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
