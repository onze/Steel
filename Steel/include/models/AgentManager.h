#ifndef STEEL_AGENT_MANAGER_H
#define STEEL_AGENT_MANAGER_H

#include "steeltypes.h"

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

            /// Creates an empty agent and return its id. Agent can be linked to models via Agent::linkTo.
            AgentId newAgent();

            /// Returns a pointer to the agent whose id's given, or nullptr if there's no such agent.
            Agent *getAgent(AgentId id) const;

            void deleteAgent(AgentId id);
            void deleteAllAgents();
            
            /// Returns all agent ids currently in use.
            std::vector<AgentId> getAgentIds() const;
            
            /// Path building condition on source agent
            bool agentCanBePathSource(AgentId const aid) const;
            bool agentCanBePathDestination(AgentId const aid) const;
            
            bool agentHasBTPath(AgentId aid);
            bool agentCanBeAssignedBTPath(AgentId aid);
            bool assignBTPath(AgentId movableAid, AgentId pathAid);
            bool unassignBTPath(AgentId movableAid, AgentId pathAid);
            
            bool agentHasLocationPath(AgentId aid);
            
            /// Register agent as tagged
            void addTaggedAgent(Tag const &tag, AgentId const aid);
            /// Unegister agent as tagged
            void removeTaggedAgent(Tag const &tag, AgentId const aid);
            /// Retreive all agents tagged with the given tag
            const std::set< AgentId > & agentTagged(Steel::Tag tag);
            
            enum class PublicSignal : u32
            {
                agentCreated = 0
            };
            Signal getSignal(AgentManager::PublicSignal signal) const;

        private:
            /// Creates an empty agent with the given id.
            Agent *newAgent(AgentId &aid);
            /// Returns true if the id could be reserved.
            bool reserveId(AgentId id);

            // not owned
            Level *mLevel;

            // owned
            // An AgenId is whether:
            // -used: in mAgents
            // unused: in the freelest or >mNextFreeId
            /// agent container.
            std::map<AgentId, Agent *> mAgents;
            std::list<AgentId> mFreeList;
            // max(max(mAgents), max(mFreeList)): AgentId's upper bound.
            AgentId mNextFreeId;
            
            /// Tag to agent map
            std::map<Tag, std::set<AgentId>> mTagRegister;

    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
