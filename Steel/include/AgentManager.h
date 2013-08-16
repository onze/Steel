#ifndef STEEL_AGENT_MANAGER_H
#define STEEL_AGENT_MANAGER_H

#include <map>

#include "steeltypes.h"
#include "Level.h"

namespace Steel
{
    class Agent;
    class Level;
    
    /**
     * Agents are local to a Leveljust like.
     */
    class AgentManager
    {
        friend Level;
        public:
            AgentManager(Level *level);
            ~AgentManager();
            
            AgentId getFreeAgentId();
            bool isIdFree(AgentId id) const; 
            void makeSureIdCantBeTaken(AgentId id);

            /// Creates an empty agent and return its id. Agent can be linked to models via Agent::linkTo.
            AgentId newAgent();
            
            /// Returns a pointer to the agent whose id's given, or NULL if there's no such agent.
            Agent *getAgent(AgentId id);
            
            void deleteAgent(AgentId id);
            void deleteAllAgents();
            
        private:
            /// Creates an empty agent with the given id.
            Agent *newAgent(AgentId &aid);
            
            // not owned
            Level *mLevel;
            
            // owned
            /// agent container.
            std::map<AgentId, Agent *> mAgents;
            std::list<AgentId> mFreeList;
            AgentId mNextFreeId;
            
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
