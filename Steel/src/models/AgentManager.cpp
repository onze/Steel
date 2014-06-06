

#include "models/AgentManager.h"
#include "models/Agent.h"
#include "Level.h"
#include "models/LocationModel.h"
#include "Debug.h"
#include "SignalManager.h"

namespace Steel
{

    AgentManager::AgentManager(Level *level): mLevel(level),
        mAgents(), mFreeList(), mNextFreeId(0),
        mTagRegister()
    {
        Agent::staticInit();
    }

    AgentManager::~AgentManager()
    {
        deleteAllAgents();
        mLevel = nullptr;
        mAgents.clear();
        mTagRegister.clear();
    }

    std::vector<AgentId> AgentManager::getAgentIds() const
    {
        std::vector<AgentId> ids;
        ids.reserve(mAgents.size());

        for(auto const & pair : mAgents)
            ids.push_back(pair.first);

        return ids;
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

    bool AgentManager::isIdFree(AgentId id) const
    {
        return id >= mNextFreeId || (std::find(mFreeList.begin(), mFreeList.end(), id) != mFreeList.end() && mAgents.end() == mAgents.find(id));
    }

    AgentId AgentManager::newAgent()
    {
        Agent *t = new Agent(getFreeAgentId(), mLevel);
        mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
//         Debug::log("new agent with id ")(t->id()).endl();
        SignalManager::instance().emit(getSignal(PublicSignal::agentCreated));
        return t->id();
    }

    Agent *AgentManager::newAgent(AgentId &id)
    {
        Agent *t = nullptr;

        // check is not already taken
        if(reserveId(id))
        {
            t = new Agent(id, mLevel);
            mAgents.insert(std::pair<AgentId, Agent *>(t->id(), t));
            Debug::log("new agent with id ")(t->id()).endl();
            SignalManager::instance().emit(getSignal(PublicSignal::agentCreated));
        }

        return t;
    }

    bool AgentManager::reserveId(AgentId id)
    {
        // easy case: the id was never used
        if(id >= mNextFreeId)
        {
            // expand freelist, increase upper bound
            for(AgentId i = mNextFreeId; i != id; ++i)
                mFreeList.push_back(i);

            mNextFreeId = id + 1;
            return true;
        }

        // now the id is whether in the free list or taken.
        auto it_freelist = std::find(mFreeList.begin(), mFreeList.end(), id);

        // in freelist
        if(mFreeList.end() != it_freelist)
        {
            mFreeList.erase(it_freelist);
            return true;
        }

#ifdef DEBUG

        // making sure it actually is a taken agent.
        if(mAgents.end() != mAgents.find(id))
            Debug::error("AgentManager::reserveId: id ")(id)(" not out of bound nor in freelist, yet not taken.").endl().breakHere();

#endif
        return false;
    }

    Agent *AgentManager::getAgent(Steel::AgentId id) const
    {
        const std::map<AgentId, Agent *>::const_iterator it = mAgents.find(id);
        return  it == mAgents.end() ? nullptr : it->second;
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

    void AgentManager::addTaggedAgent(const Tag &tag, const AgentId aid)
    {
        auto it = mTagRegister.insert(std::make_pair(tag, std::set<AgentId>()));
        it.first->second.insert(aid);
    }

    void AgentManager::removeTaggedAgent(const Tag &tag, const AgentId aid)
    {
        auto it = mTagRegister.find(tag);

        if(mTagRegister.end() != it)
            it->second.erase(tag);
    }

    const std::set< AgentId > &AgentManager::agentTagged(Tag tag)
    {
        return mTagRegister.emplace(tag, std::set<AgentId>()).first->second;
    }

    Signal AgentManager::getSignal(AgentManager::PublicSignal signal) const
    {
#define STEEL_AGENTMANAGER_GETSIGNAL_CASE(NAME) case NAME:return SignalManager::instance().toSignal("Steel::AgentManager::"#NAME)

        switch(signal)
        {
                STEEL_AGENTMANAGER_GETSIGNAL_CASE(PublicSignal::agentCreated);
        }

#undef STEEL_AGENTMANAGER_GETSIGNAL_CASE
        return INVALID_SIGNAL;
    }


}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
