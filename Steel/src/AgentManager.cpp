
#include "AgentManager.h"
#include "Agent.h"
#include "Level.h"

namespace Steel
{

    AgentManager::AgentManager(Level *level):mLevel(level),mAgents(std::map<AgentId, Agent *>()),mFreeList(std::list<AgentId>()),mNextFreeId(0)
    {

    }

    AgentManager::~AgentManager()
    {
        deleteAllAgents();
        mLevel=NULL;
        mAgents.clear();
    }

    AgentId AgentManager::getFreeAgentId()
    {
        if(mFreeList.size())
        {
            AgentId id=mFreeList.front();
            mFreeList.pop_front();
            return id;
        }
        return mNextFreeId++;
    }

    AgentId AgentManager::newAgent()
    {
        Agent *t = new Agent(getFreeAgentId(),mLevel);
        mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
        return t->id();
    }

    void AgentManager::makeSureIdCantBeTaken(AgentId id)
    {
        mFreeList.remove(id);
        if(id >= mNextFreeId)
        {
            mNextFreeId=id+1;
        }
    }

    bool AgentManager::isIdFree(AgentId id) const
    {
        return std::find(mFreeList.begin(), mFreeList.end(),id)!=mFreeList.end() || id>=mNextFreeId;
    }

    Agent *AgentManager::newAgent(AgentId &id)
    {
        Agent *t = NULL;
        // check is not already taken
        if(isIdFree(id))
        {
            makeSureIdCantBeTaken(id);
            t = new Agent(id, mLevel);
        }
        return t;
    }

    Agent *AgentManager::getAgent(AgentId id)
    {
        std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
        if (it == mAgents.end())
            return NULL;
        return it->second;
    }

    void AgentManager::deleteAgent(AgentId id)
    {
        Debug::log("AgentManager::deleteAgent() id: ")(id).endl();
        std::map<AgentId, Agent *>::iterator it = mAgents.find(id);
        if (it == mAgents.end())
            return;
        delete (*it).second;
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
        mNextFreeId=0;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
