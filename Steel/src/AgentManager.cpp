

#include "AgentManager.h"
#include "Agent.h"
#include "Level.h"
#include <LocationModel.h>

namespace Steel
{

    AgentManager::AgentManager(Level *level): mLevel(level), mAgents(std::map<AgentId, Agent *>()), mFreeList(std::list<AgentId>()), mNextFreeId(0)
    {

    }

    AgentManager::~AgentManager()
    {
        deleteAllAgents();
        mLevel = nullptr;
        mAgents.clear();
    }

    AgentId AgentManager::getFreeAgentId()
    {
        if(mFreeList.size())
        {
            AgentId id = mFreeList.front();
            mFreeList.pop_front();
            return id;
        }

        return mNextFreeId++;
    }

    void AgentManager::makeSureIdCantBeTaken(AgentId id)
    {
        mFreeList.remove(id);

        if(id >= mNextFreeId)
        {
            mNextFreeId = id + 1;
        }
    }

    bool AgentManager::isIdFree(AgentId id) const
    {
        return std::find(mFreeList.begin(), mFreeList.end(), id) != mFreeList.end() || id >= mNextFreeId;
    }

    AgentId AgentManager::newAgent()
    {
        Agent *t = new Agent(getFreeAgentId(), mLevel);
        mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
        return t->id();
    }

    Agent *AgentManager::newAgent(AgentId &id)
    {
        Agent *t = nullptr;

        // check is not already taken
        if(isIdFree(id))
        {
            makeSureIdCantBeTaken(id);
            t = new Agent(id, mLevel);
            mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
        }

        return t;
    }

    Agent *AgentManager::getAgent(Steel::AgentId id) const
    {
        const std::map<AgentId, Agent *>::const_iterator it = mAgents.find(id);

        if(it == mAgents.end())
            return nullptr;

        return it->second;
    }

    void AgentManager::deleteAgent(AgentId id)
    {
        Debug::log("AgentManager::deleteAgent() id: ")(id).endl();
        std::map<AgentId, Agent *>::iterator it = mAgents.find(id);

        if(it == mAgents.end())
            return;

        delete(*it).second;
        mAgents.erase(it);
        mFreeList.push_back(id);
    }

    void AgentManager::deleteAllAgents()
    {
        while(mAgents.size())
        {
            delete mAgents.begin()->second;
            mAgents.erase(mAgents.begin());
        }

        mFreeList.clear();
        mNextFreeId = 0;
    }

    bool AgentManager::agentCanBePathSource(AgentId const aid) const
    {
        Agent *agent = getAgent(aid);
        return nullptr == agent ? false : true;
    }

    bool AgentManager::agentCanBePathDestination(AgentId const aid) const
    {
        Agent *agent = getAgent(aid);
        return nullptr == agent ? false : true;
    }

    bool AgentManager::agentHasBTPath(AgentId aid)
    {
        Agent *agent = getAgent(aid);
        return nullptr == agent ? false : agent->hasBTPath();
    }
    
    bool AgentManager::agentHasLocationPath(AgentId aid)
    {
        Agent *agent = getAgent(aid);
        return nullptr == agent ? false : agent->hasLocationPath();
    }

    bool AgentManager::agentCanBeAssignedBTPath(AgentId aid)
    {
        return !agentHasBTPath(aid);
    }

    bool AgentManager::assignBTPath(AgentId movableAid, AgentId pathAid)
    {
        Agent *movableAgent;

        if(nullptr == (movableAgent = getAgent(movableAid)))
            return false;
        
        return movableAgent->followNewPath(pathAid);
    }

    bool AgentManager::unassignBTPath(AgentId movableAid, AgentId pathAid)
    {
        
        Agent *movableAgent;
        
        if(nullptr == (movableAgent = getAgent(movableAid)))
            return true;
        
        return movableAgent->stopFollowingPath(pathAid);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
