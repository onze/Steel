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

    LocationModel::LocationModel(): Model(),
        mLocationModelMan(NULL), mSource(INVALID_ID), mDestination(INVALID_ID), mAttachedAgent(INVALID_ID),
        mPosition(Ogre::Vector3::ZERO), mPath("")
    {

    }

    LocationModel::LocationModel(const LocationModel& o): Model(o),
        mLocationModelMan(o.mLocationModelMan), mSource(o.mSource), mDestination(o.mDestination), mAttachedAgent(o.mAttachedAgent),
        mPosition(o.mPosition), mPath(o.mPath)
    {

    }

    LocationModel::~LocationModel()
    {

    }

    void LocationModel::cleanup()
    {
        if(hasSource())
            unsetSource();
        if(hasDestination())
            unsetDestination();
        
        mLocationModelMan = NULL;
        mSource = mDestination = INVALID_ID;
        mAttachedAgent = INVALID_ID;
        mPosition = Ogre::Vector3::ZERO;
        mPath = "";
        Model::cleanup();
    }


    LocationModel& LocationModel::operator=(const LocationModel& o)
    {
        Model::operator=(o);
        mLocationModelMan=o.mLocationModelMan;
        mSource=o.mSource;
        mDestination=o.mDestination;
        mAttachedAgent=o.mAttachedAgent;
        mPosition=o.mPosition;
        mPath=o.mPath;
        return *this;
    }

    bool LocationModel::operator==(const LocationModel& o) const
    {
        return Model::operator==(o) &&
               mLocationModelMan==o.mLocationModelMan &&
               mSource==o.mSource &&
               mDestination==o.mDestination &&
               mAttachedAgent==o.mAttachedAgent &&
               mPosition==o.mPosition &&
               mPath==o.mPath;
    }

    bool LocationModel::fromJson(const Json::Value& node)
    {
        throw std::runtime_error("LocationModel::fromJson should not be used, use LocationModel::fromJson");
    }

    bool LocationModel::init(LocationModelManager* const locationModelMan)
    {
        mLocationModelMan = locationModelMan;
        return NULL != mLocationModelMan;
    }

    bool LocationModel::fromJson(const Json::Value& node, LocationModelManager * const locationModelMan)
    {
        unsetSource();
        unsetDestination();

        if(!deserializeTags(node))
            return false;

        if(node.isMember(LocationModel::SOURCE_ATTRIBUTE))
            mSource=JsonUtils::asUnsignedLong(node[LocationModel::SOURCE_ATTRIBUTE],INVALID_ID);

        if(node.isMember(LocationModel::DESTINATION_ATTRIBUTE))
            mDestination=JsonUtils::asUnsignedLong(node[LocationModel::DESTINATION_ATTRIBUTE],INVALID_ID);

        if(node.isMember(LocationModel::ATTACHED_AGENT_ATTRIBUTE))
            mAttachedAgent=(AgentId)JsonUtils::asUnsignedLong(node[LocationModel::ATTACHED_AGENT_ATTRIBUTE],INVALID_ID);

        if(node.isMember(LocationModel::PATH_ATTRIBUTE))
            mPath=JsonUtils::asString(node[LocationModel::PATH_ATTRIBUTE],"");

        return init(locationModelMan);
    }

    void LocationModel::toJson(Json::Value &node)
    {
        if(INVALID_ID!=mSource)
            node[LocationModel::SOURCE_ATTRIBUTE] = JsonUtils::toJson(source());

        if(INVALID_ID!=mDestination)
            node[LocationModel::DESTINATION_ATTRIBUTE] = JsonUtils::toJson(destination());

        if(INVALID_ID!=mAttachedAgent)
            node[LocationModel::ATTACHED_AGENT_ATTRIBUTE] = JsonUtils::toJson(mAttachedAgent);

        if(hasPath())
            node[LocationModel::PATH_ATTRIBUTE] = JsonUtils::toJson(mPath);

        serializeTags(node);
    }

    bool LocationModel::setDestination(ModelId mid)
    {
        LocationModel *dst=mLocationModelMan->at(mid);
        if(NULL==dst)
        {
            mDestination = INVALID_ID;
            return false;
        }
        mDestination=mid;

        if(hasPath())
            if(!dst->setPath(mPath))
                return false;
        return true;
    }

    void LocationModel::unsetDestination()
    {
        mDestination=INVALID_ID;
    }

    bool LocationModel::setSource(ModelId mid)
    {
        LocationModel *src=mLocationModelMan->at(mid);
        if(NULL==src)
        {
            mSource = INVALID_ID;
            return false;
        }
        mSource=mid;

        if(hasPath() && !src->hasPath())
            if(!src->setPath(mPath))
                return false;
        return true;
    }

    void LocationModel::unsetSource()
    {
        mSource=INVALID_ID;
        unsetPath();
    }

    void LocationModel::attachAgent(AgentId aid)
    {
        mAttachedAgent=aid;
    }

    void LocationModel::setPosition(Ogre::Vector3 const &pos)
    {
        mPosition=pos;
    }

    Ogre::Vector3 LocationModel::position()
    {
        return mPosition;
//         Agent *agent=mAgentMan->getAgent(mAttachedAgent);
//         Ogre::Vector3 position=Ogre::Vector3::ZERO;
//         if(NULL==agent)
//             Debug::error("in LocationModel::position(): invalid attached agent ")(mAttachedAgent).endl();
//         else
//             position=agent->position();
//         return position;
    }

    bool LocationModel::setPath(Ogre::String const &name)
    {
        return _setPath(name, false);
    }

    void LocationModel::unsetPath()
    {
        _setPath("", true);
    }

    bool LocationModel::_setPath(Ogre::String const &name, bool force)
    {
        static const Ogre::String intro="in LocationModel::setPath(): ";
        
        std::stack<LocationModel *> fringe;
        fringe.push(this);
        
            
        if("" == name && !force)
        {
            Debug::error(intro)("path name cannot be the empty string").endl();
            return false;
        }
            
        while(fringe.size())
        {
            LocationModel *model=fringe.top();
            fringe.pop();

            if(NULL==model)
                continue;
                
            // already set: assuming next nodes also are
            if(name == model->mPath)
                continue;

            model->mPath=name;
            
            // propagate
            if(model->hasSource())
                fringe.push(mLocationModelMan->at(model->mSource));
            if(model->hasDestination())
                fringe.push(mLocationModelMan->at(model->mDestination));
        }
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
