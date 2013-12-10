#include "LocationModel.h"
#include "tools/JsonUtils.h"
#include "Debug.h"
#include "AgentManager.h"
#include <Agent.h>
#include <LocationModelManager.h>

namespace Steel
{
    const char *LocationModel::SOURCE_ATTRIBUTE = "source";
    const char *LocationModel::DESTINATION_ATTRIBUTE = "destination";
    const char *LocationModel::ATTACHED_AGENT_ATTRIBUTE = "attachedAgent";
    const char *LocationModel::PATH_ATTRIBUTE = "path";

    const LocationPathName LocationModel::EMPTY_PATH = Ogre::StringUtil::BLANK;

    LocationModel::LocationModel(): Model(),
        mLocationModelMan(nullptr), mSources(), mDestinations(), mAttachedAgent(INVALID_ID),
        mPosition(Ogre::Vector3::ZERO), mPath(LocationModel::EMPTY_PATH)
    {

    }

    LocationModel::LocationModel(const LocationModel &o): Model(o),
        mLocationModelMan(o.mLocationModelMan), mSources(o.mSources), mDestinations(o.mDestinations), mAttachedAgent(o.mAttachedAgent),
        mPosition(o.mPosition), mPath(o.mPath)
    {

    }

    LocationModel::~LocationModel()
    {

    }

    void LocationModel::cleanup()
    {
        removeAllSources();
        removeAllDestinations();

        mLocationModelMan = nullptr;
        mAttachedAgent = INVALID_ID;
        mPosition = Ogre::Vector3::ZERO;
        mPath = LocationModel::EMPTY_PATH;
        Model::cleanup();
    }


    LocationModel &LocationModel::operator=(const LocationModel &o)
    {
        Model::operator=(o);
        mLocationModelMan = o.mLocationModelMan;
        mSources = o.mSources;
        mDestinations = o.mDestinations;
        mAttachedAgent = o.mAttachedAgent;
        mPosition = o.mPosition;
        mPath = o.mPath;
        return *this;
    }

    bool LocationModel::operator==(const LocationModel &o) const
    {
        return Model::operator==(o) &&
               mLocationModelMan == o.mLocationModelMan &&
               mSources == o.mSources &&
               mDestinations == o.mDestinations &&
               mAttachedAgent == o.mAttachedAgent &&
               mPosition == o.mPosition &&
               mPath == o.mPath;
    }

    bool LocationModel::fromJson(const Json::Value &node)
    {
        throw std::runtime_error("LocationModel::fromJson should not be used, use LocationModel::fromJson");
    }

    bool LocationModel::init(LocationModelManager *const locationModelMan)
    {
        mLocationModelMan = locationModelMan;
        return true;
    }

    bool LocationModel::fromJson(const Json::Value &node, LocationModelManager *const locationModelMan)
    {
        mLocationModelMan = locationModelMan;
        
        mSources.clear();
        mDestinations.clear();

        if(!deserializeTags(node))
            return false;

        if(node.isMember(LocationModel::SOURCE_ATTRIBUTE))
            mSources = JsonUtils::asUnsignedLongSet(node[LocationModel::SOURCE_ATTRIBUTE], std::set<AgentId>(), INVALID_ID);

        if(node.isMember(LocationModel::DESTINATION_ATTRIBUTE))
            mDestinations = JsonUtils::asUnsignedLongSet(node[LocationModel::DESTINATION_ATTRIBUTE], std::set<AgentId>(), INVALID_ID);

        if(node.isMember(LocationModel::ATTACHED_AGENT_ATTRIBUTE))
            mAttachedAgent = (AgentId)JsonUtils::asUnsignedLong(node[LocationModel::ATTACHED_AGENT_ATTRIBUTE], INVALID_ID);

        if(node.isMember(LocationModel::PATH_ATTRIBUTE))
            mPath = JsonUtils::asString(node[LocationModel::PATH_ATTRIBUTE], LocationModel::EMPTY_PATH);

        return init(locationModelMan);
    }

    void LocationModel::toJson(Json::Value &node)
    {
        if(mSources.size() > 0)
            node[LocationModel::SOURCE_ATTRIBUTE] = JsonUtils::toJson(mSources);

        if(mDestinations.size() > 0)
            node[LocationModel::DESTINATION_ATTRIBUTE] = JsonUtils::toJson(mDestinations);

        if(INVALID_ID != mAttachedAgent)
            node[LocationModel::ATTACHED_AGENT_ATTRIBUTE] = JsonUtils::toJson(mAttachedAgent);

        if(hasPath())
            node[LocationModel::PATH_ATTRIBUTE] = JsonUtils::toJson(mPath);

        serializeTags(node);
    }

    bool LocationModel::addDestination(AgentId aid)
    {
        Agent *agent = mLocationModelMan->level()->agentMan()->getAgent(aid);
        
        if(nullptr == agent)
            return false;
        
        ModelId mid = agent->locationModelId();
        LocationModel *dst = mLocationModelMan->at(mid);

        if(nullptr == dst)
            return false;

        // propagate path if either has one and not the other
        if(!propagatePath(this, dst))
        {
            Debug::warning("in LocationModel::addDestination(): cannot propagate paths. Aborting.").endl();
            return false;
        }

        mDestinations.insert(aid);
        return true;
    }

    void LocationModel::removeDestination(AgentId aid)
    {
        if(mDestinations.erase(aid) > 0)
        {
            Agent *agent = mLocationModelMan->level()->agentMan()->getAgent(aid);

            if(nullptr == agent)
                return;

            LocationModel *model = agent->locationModel();

            if(nullptr == model)
                return;

            model->removeSource(attachedAgent());
        }
    }

    void LocationModel::removeAllDestinations()
    {
        while(hasAnyDestination())
            removeDestination(*mDestinations.begin());
    }

    bool LocationModel::propagatePath(LocationModel *m0, LocationModel *m1)
    {
        if(m0->hasPath())
        {
            if(m1->hasPath())
            {
                if(m1->path() != m0->path())
                {
                    return false;
                }
            }
            else
                m1->_setPath(m0->path());
        }
        else if(m1->hasPath())
            m0->_setPath(m1->path());

        return true;
    }

    bool LocationModel::addSource(AgentId aid)
    {
        Agent *agent = mLocationModelMan->level()->agentMan()->getAgent(aid);

        if(nullptr == agent)
            return false;

        ModelId mid = agent->locationModelId();
        LocationModel *src = mLocationModelMan->at(mid);

        if(nullptr == src)
            return false;


        if(!propagatePath(this, src))
        {
            Debug::warning("in LocationModel::addSource(): cannot propagate paths. Aborting.").endl();
            return false;
        }

        mSources.insert(aid);

        return true;
    }

    void LocationModel::removeSource(AgentId aid)
    {
        if(mSources.erase(aid) > 0)
        {
            Agent *agent = mLocationModelMan->level()->agentMan()->getAgent(aid);

            if(nullptr == agent)
                return;

            LocationModel *model = agent->locationModel();

            if(nullptr == model)
                return;

            model->removeDestination(attachedAgent());
        }
    }

    void LocationModel::removeAllSources()
    {
        while(hasAnySource())
            removeSource(*mSources.begin());
    }

    void LocationModel::attachAgent(AgentId aid)
    {
        mAttachedAgent = aid;
    }

    void LocationModel::setPosition(Ogre::Vector3 const &pos)
    {
        mPosition = pos;
    }

    Ogre::Vector3 LocationModel::position()
    {
        return mPosition;
//         Agent *agent=mAgentMan->getAgent(mAttachedAgent);
//         Ogre::Vector3 position=Ogre::Vector3::ZERO;
//         if(nullptr==agent)
//             Debug::error("in LocationModel::position(): invalid attached agent ")(mAttachedAgent).endl();
//         else
//             position=agent->position();
//         return position;
    }

    void LocationModel::applyToNetWork(std::function<void(LocationModel *)> f)
    {
        std::set<LocationModel *> network;
        insertNetworkModels(network);
        std::for_each(network.begin(), network.end(), f);
    }

    void LocationModel::insertNetworkModels(std::set<LocationModel *> &network)
    {
        std::set<LocationModel *> fringe;
        fringe.insert(this);

        auto agentMan = mLocationModelMan->level()->agentMan();
        auto insertLocationModelsInFringeIfNotInNetwork = [&network, &fringe, agentMan](std::set<AgentId> const & nodes)->void
        {
            for(auto const & aid : nodes)
            {
                Agent *agent = agentMan->getAgent(aid);

                if(nullptr == agent || INVALID_ID == agent->locationModelId() || network.end() != network.find(agent->locationModel()))
                    continue;

                fringe.insert(agent->locationModel());
            }
        };

        while(fringe.size())
        {
            LocationModel *model = *fringe.begin();
            fringe.erase(fringe.begin());

            if(nullptr == model)
                continue;

            network.insert(model);
            insertLocationModelsInFringeIfNotInNetwork(model->sources());
            insertLocationModelsInFringeIfNotInNetwork(model->destinations());
        }
    }

    void LocationModel::_setPath(const Steel::LocationPathName &name)
    {
        applyToNetWork(std::bind([&name](LocationModel * model)->void {model->mPath = name;}, std::placeholders::_1));
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
