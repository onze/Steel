/*
 * Agent.cpp
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#include <exception>

#include <json/json.h>

#include "models/Agent.h"
#include "Debug.h"
#include "models/_ModelManager.h"
#include "models/OgreModelManager.h"
#include "Level.h"
#include "tools/JsonUtils.h"
#include "TagManager.h"
#include "models/LocationModelManager.h"
#include "models/PhysicsModel.h"
#include "models/BTModel.h"
#include "models/BTModelManager.h"
#include "models/AgentManager.h"
#include "SignalManager.h"

namespace Steel
{
    const char *Agent::NAME_ATTRIBUTE = "name";
    const char *Agent::TAGS_ATTRIBUTE = "tags";
    const char *Agent::ID_ATTRIBUTE = "aid";
    const char *Agent::BEHAVIORS_STACK_ATTRIBUTE = "behaviorsStack";

    Agent::PropertyTags Agent::sPropertyTags;

    Agent::Agent(AgentId id, Steel::Level *level): mId(id), mLevel(level),
        mModelIds(), mIsSelected(false), mTags(),
        mBehaviorsStack()
    {
    }

    Agent::~Agent()
    {
        cleanup();
    }

    void Agent::cleanup()
    {
        while(mModelIds.size())
            unlinkFromModel(mModelIds.begin()->first);

        mBehaviorsStack.clear();

        while(mTags.size())
            untag(mTags.begin()->first);

        mLevel = nullptr;
        mId = INVALID_ID;
    }

    Agent::Agent(const Agent &o)
        : mId(o.mId), mLevel(o.mLevel), mModelIds(o.mModelIds), mIsSelected(o.mIsSelected), mTags(o.mTags),
          mBehaviorsStack(o.mBehaviorsStack)
    {
    }

    Agent &Agent::operator=(const Agent &o)
    {
        if(this == &o)
            return *this;

        bool wasInUse = INVALID_ID != mId;

        mId = o.mId;
        bool isInUse = INVALID_ID != mId;
        mLevel = o.mLevel;

        if(wasInUse)
        {
            for(std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
                unlinkFromModel(it->first);
        }
        else
        {
            assert(mModelIds.size() == 0);
        }

        mModelIds = o.mModelIds;

        if(isInUse)
        {
            for(std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
                linkToModel(it->first, it->second);
        }

        mTags = o.mTags;

        if(mIsSelected != o.mIsSelected)
        {
            setSelected(mIsSelected);
        }

        mBehaviorsStack = o.mBehaviorsStack;

        return *this;
    }

    void Agent::staticInit()
    {
        sPropertyTags.persistent = TagManager::instance().toTag("__Agent::PropertyTag.persistent");
    }

    bool Agent::fromJson(Json::Value &value)
    {
        // name
        if(value.isMember(Agent::NAME_ATTRIBUTE))
            setName(JsonUtils::asString(value[Agent::NAME_ATTRIBUTE]));

        // models
        int nModels = 0, nExpected = 0;

        for(ModelType mt_it = (ModelType)((int) ModelType::FIRST + 1); mt_it != ModelType::LAST; mt_it = (ModelType)((int) mt_it + 1))
        {
            Ogre::String mtName = toString(mt_it);
            Json::Value mTypeValue = value[mtName];

            // possibly no model of this type"__Agent::PropertyTag.persistent"
            if(mTypeValue.isNull())
                continue;

            ++nExpected;
            ModelId modelId = (ModelId) Ogre::StringConverter::parseUnsignedLong(mTypeValue.asString());

            if(!linkToModel(mt_it, modelId))
            {
                Debug::error(STEEL_METH_INTRO, "agent: ", mId, " would not link with model<", mtName, "> ", mId, ". Skipping.").endl();
                continue;
            }

            ++nModels;
        }

        if(nModels != nExpected)
            Debug::warning(STEEL_METH_INTRO, "agent: ", mId, " is linked with ", nModels, " models, but ", nExpected, " were expected. Json string:").endl()(value).endl();

        // behaviors
        mBehaviorsStack = JsonUtils::asModelIdList(value[Agent::BEHAVIORS_STACK_ATTRIBUTE], std::list<ModelId>(), INVALID_ID);

        // tags
        mTags.clear();

        for(auto const & _tag : JsonUtils::asTagsSet(value[Agent::TAGS_ATTRIBUTE]))
            tag(_tag);

        return true;
    }

    void Agent::tag(Tag tag)
    {
        if(INVALID_TAG == tag)
            return;

        std::map<Tag, unsigned>::iterator it = mTags.find(tag);

        if(mTags.end() == it)
        {
            mTags.insert(std::pair<Tag, unsigned>(tag, 1));
            mLevel->agentMan()->addTaggedAgent(tag, mId);
        }
        else
            it->second++;
    }

    void Agent::tag(std::set<Tag> _tags)
    {
        for(auto const & _tag : _tags)
            tag(_tag);
    }

    void Agent::untag(Tag tag)
    {
        std::map<Tag, unsigned>::iterator it = mTags.find(tag);

        if(mTags.end() == it)
            return;

        it->second--;

        if(0 == it->second)
        {
            mTags.erase(it);
            mLevel->agentMan()->removeTaggedAgent(tag, mId);
        }
    }

    void Agent::untag(std::set<Tag> _tags)
    {
        for(auto const & _tag : _tags)
            untag(_tag);
    }

    std::set<Tag> Agent::tags() const
    {
        std::set<Tag> _tags;

        for(auto const & it : mTags)
            _tags.insert(it.first);

        return _tags;
    }

    bool Agent::isTagged(Tag tag)
    {
        return mTags.end() != mTags.find(tag);
    }

    bool Agent::linkToModel(ModelType mType, ModelId modelId)
    {
        Ogre::String intro = "Agent::linkToModel(type=" + toString(mType) + ", id="
                             + Ogre::StringConverter::toString(modelId) + "): ";
        ModelManager *mm = mLevel->modelManager(mType);

        if(nullptr == mm)
        {
            Debug::error(intro)("no suitable manager found. Aborting.").endl();
            return false;
        }

        if(!mm->isValid(modelId))
        {
            Debug::error(intro)("model is not valid. Aborting.").endl();
            return false;
        }

        //result.first==iterator placed at location, result.second==successful insertion flag
        auto result = mModelIds.insert(std::pair<ModelType, ModelId>(mType, modelId));

        if(!result.second)
        {
            Debug::error(intro)("Could not insert model (overwrites are not allowed). Aborting.").endl();
            return false;
        }

        mm->incRef(modelId);

        if(!mm->onAgentLinkedToModel(this, modelId))
        {
            unlinkFromModel(mType);
            Debug::error(intro)("Agent<->model linking aborted.").endl();
            return false;
        }

        tag(mm->modelTags(modelId));

        return true;
    }

    void Agent::unlinkFromModel(ModelType mType)
    {
        auto it = mModelIds.find(mType);

        if(it == mModelIds.end())
            return;

        ModelId mid = it->second;
        // dependencies first
        ModelManager *mm = mLevel->modelManager(mType);

        // pre unlinking
        switch(mType)
        {
            case ModelType::OGRE:
                while(INVALID_ID != btModelId())
                    popBT();

                unlinkFromModel(ModelType::BT);
                unlinkFromModel(ModelType::LOCATION);
                unlinkFromModel(ModelType::PHYSICS);
                break;

            case ModelType::LOCATION:
                if(mm->at(mid)->refCount() == 1)
                    ((LocationModelManager *)mm)->unlinkLocation(mid);

            case ModelType::BT:
            case ModelType::PHYSICS:
            case ModelType::BLACKBOARD:
            case ModelType::FIRST:
            case ModelType::LAST:
                break;
        }

        untag(mm->modelTags(mid));
        mm->onAgentUnlinkedFromModel(this, mid);
        mModelIds.erase(it);
    }

    void Agent::popBT()
    {
        if(INVALID_ID != btModelId())
            unlinkFromModel(ModelType::BT);

        if(0 == mBehaviorsStack.size())
            return;

        auto mid = mBehaviorsStack.back();
        mBehaviorsStack.pop_back();

        // stack had added an extra ref to keep it alive
        if(linkToModel(ModelType::BT, mid))
        {
            btModel()->decRef();
            btModel()->unpause();
        }
    }

    bool Agent::pushBT(ModelId btid)
    {
        static const Ogre::String intro = "Agent::pushBT(): ";

        // validate parameter
        if(!mLevel->BTModelMan()->isValid(btid))
        {
            Debug::log(intro)("won't push invalid id ").quotes(btid)(". Aborting.").endl();
            return false;
        }

        // save current
        BTModel *model = btModel();

        if(nullptr != model)
        {
            model->pause();
            // keep it alive after it is unlinked
            model->incRef();
            mBehaviorsStack.push_back(btModelId());
            unlinkFromModel(ModelType::BT);
        }

        // set given on as current
        return linkToModel(ModelType::BT, btid);
    }

    Model *Agent::model(ModelType mType) const
    {
        ModelId id = modelId(mType);

        if(id == INVALID_ID)
            return nullptr;

        return mLevel->modelManager(mType)->at(id);
    }

    bool Agent::hasModel(ModelType modelType) const
    {
        return INVALID_ID != modelId(modelType);
    }

    ModelId Agent::modelId(ModelType mType) const
    {
        auto it = mModelIds.find(mType);
        return it == mModelIds.end() ? INVALID_ID : it->second;
    }

    void Agent::setSelected(bool selected)
    {
        OgreModel *om = ogreModel();

        if(nullptr != om)
            om->setSelected(selected);

        PhysicsModel *pm = physicsModel();

        if(nullptr != pm)
            pm->setSelected(selected);

        BTModel *btm = btModel();

        if(nullptr != btm)
            btm->setSelected(selected);

        mIsSelected = selected;
        emit(mIsSelected ? EventType::SELECTED : EventType::UNSELECTED);
    }

    void Agent::emit(Agent::EventType e)
    {
        auto const it = mSignalMap.find(e);

        if(mSignalMap.end() != it)
            this->SignalEmitter::emit(it->second);
    }

    Json::Value Agent::toJson()
    {
        Json::Value root;
        
        // name
        if(hasName())
            root[Agent::NAME_ATTRIBUTE] = name();

        // model ids
        for(std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
        {
            ModelType mt = (*it).first;
            ModelId mid = (*it).second;
            root[toString(mt)] = JsonUtils::toJson(mid);
        }

        // tags
        if(mTags.size())
        {
            auto _tags = tags();
            root[Agent::TAGS_ATTRIBUTE] = JsonUtils::toJson(TagManager::instance().fromTags(_tags));
        }

        if(mBehaviorsStack.size())
        {
            root[Agent::BEHAVIORS_STACK_ATTRIBUTE] = JsonUtils::toJson(mBehaviorsStack);
        }

        return root;
    }

    Ogre::Vector3 Agent::position() const
    {
        auto model = ogreModel();
        return nullptr == model ? Ogre::Vector3::ZERO : model->position();
    }

    void Agent::move(const Ogre::Vector3 &dpos)
    {
        auto pmodel = physicsModel();

        if(nullptr != pmodel)
        {
            pmodel->move(dpos);
        }
        else
        {
            auto omodel = ogreModel();

            if(nullptr != omodel)
                omodel->move(dpos);
        }

        auto lmodel = locationModel();

        if(nullptr != lmodel)
            mLevel->locationModelMan()->moveLocation(locationModelId(), position());
    }

    void Agent::setPosition(const Ogre::Vector3 &pos)
    {
        auto pmodel = physicsModel();

        if(nullptr != pmodel)
        {
            pmodel->setPosition(pos);
        }
        else
        {
            auto omodel = ogreModel();

            if(nullptr != omodel)
                omodel->setPosition(pos);
        }

        auto lmodel = locationModel();

        if(nullptr != lmodel)
            mLevel->locationModelMan()->moveLocation(locationModelId(), position());
    }

    Ogre::Quaternion Agent::rotation() const
    {
        auto model = ogreModel();
        return nullptr == model ? Ogre::Quaternion::ZERO : model->rotation();
    }

    void Agent::rotate(const Ogre::Vector3 &rot)
    {
        auto pmodel = physicsModel();

        if(nullptr != pmodel)
        {
            pmodel->rotate(Ogre::Quaternion(&rot));
        }
        else
        {
            auto model = ogreModel();

            if(nullptr != model)
                model->rotate(rot);
        }
    }

    void Agent::rotate(const Ogre::Quaternion &q)
    {
        auto model = physicsModel();

        if(nullptr != model)
        {
            model->rotate(q);
        }
        else
        {
            auto model = ogreModel();

            if(nullptr != model)
                model->rotate(q);
        }
    }

    void Agent::setRotation(const Ogre::Quaternion &q)
    {
        auto model = physicsModel();

        if(nullptr != model)
        {
            model->setRotation(q);
        }
        else
        {
            auto omodel = ogreModel();

            if(nullptr != omodel)
                omodel->setRotation(q);
        }
    }

    Ogre::Vector3 Agent::scale() const
    {
        auto model = ogreModel();
        return nullptr == model ? Ogre::Vector3::ZERO : model->scale();
    }

    void Agent::rescale(const Ogre::Vector3 &sca)
    {
        auto pmodel = physicsModel();

        if(nullptr != pmodel)
            pmodel->rescale(sca);

        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->rescale(sca);
    }

    void Agent::setScale(const Ogre::Vector3 &sca)
    {
        auto pmodel = physicsModel();

        if(nullptr != pmodel)
            pmodel->setScale(sca);

        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->setScale(sca);
    }

    Ogre::Vector3 Agent::angularVelocity() const
    {
        auto model = physicsModel();
        return nullptr == model ? Ogre::Vector3::ZERO : model->angularVelocity();
    }

    Ogre::Vector3 Agent::velocity() const
    {
        auto model = physicsModel();
        return nullptr == model ? Ogre::Vector3::ZERO : model->velocity();
    }

    float Agent::mass() const
    {
        auto model = physicsModel();
        return nullptr == model?.0f : model->mass();
    }

    void Agent::applyCentralImpulse(Ogre::Vector3 const &f)
    {
        auto model = physicsModel();

        if(nullptr != model)
            model->applyCentralImpulse(f);
    }

    void Agent::applyCentralForce(Ogre::Vector3 const &f)
    {
        auto model = physicsModel();

        if(nullptr != model)
            model->applyCentralForce(f);
    }

    void Agent::applyTorque(Ogre::Vector3 const &tq)
    {
        auto model = physicsModel();

        if(nullptr != model)
            model->applyTorque(tq);
    }

    void Agent::applyTorqueImpulse(Ogre::Vector3 const &tq)
    {
        auto model = physicsModel();

        if(nullptr != model)
            model->applyTorqueImpulse(tq);
    }

    bool Agent::setLocationPath(LocationPathName const &name)
    {
        auto model = locationModel();

        if(nullptr == model)
        {
            auto locationMan = mLevel->locationModelMan();

            if(nullptr == locationMan)
                return false;

            ModelId mid = locationMan->newModel();
            linkToModel(ModelType::LOCATION, mid);
            model = locationModel();
        }

        mLevel->locationModelMan()->setModelPath(locationModelId(), name);
        return true;
    }

    void Agent::unsetLocationPath()
    {
        mLevel->locationModelMan()->unsetModelPath(locationModelId());
    }

    Ogre::String Agent::locationPath()
    {
        return hasLocationPath() ? locationModel()->path() : LocationModel::EMPTY_PATH;
    }

    bool Agent::hasLocationPath()
    {
        auto model = locationModel();

        if(nullptr == model)
            return false;

        return model->hasPath();
    }

    bool Agent::setBTPath(Ogre::String const &name)
    {
        Debug::warning("Agent::setBTPath(): untested code.");

        if(nullptr != btModel())
            btModel()->setPath(name);

        return true;
    }

    void Agent::unsetBTPath()
    {
        Debug::warning("Agent::unsetBTPath(): untested code.");

        if(nullptr != btModel())
            btModel()->unsetPath();
    }

    bool Agent::hasBTPath()
    {
        return INVALID_ID != btModelId() && btModel()->hasPath();
    }

    bool Agent::followNewPath(AgentId aid)
    {
        static const Ogre::String intro = "Agent::followNewPath(): ";

        Agent *target;

        if(nullptr == (target = mLevel->agentMan()->getAgent(aid)))
        {
            Debug::error(intro)("cannot follow invalid agent ").quotes(aid)(". Aborting.").endl();
            return false;
        }

        Ogre::String targetPath = target->locationPath();

        if(LocationModel::EMPTY_PATH == targetPath)
        {
            Debug::error(intro)("target with id ").quotes(aid)(" has no path set. Aborting.").endl();
            return false;
        }

        ModelId newBTModelId = INVALID_ID;
        Ogre::String path = mLevel->BTModelMan()->genericFollowPathModelPath();

        if(!mLevel->BTModelMan()->buildFromFile(path, newBTModelId, false))
        {
            Debug::error(intro)("could not retrieve a BTModel for bt file ").quotes(path)(". Aborting.").endl();
            return false;
        }

        BTModel *newBTModel = mLevel->BTModelMan()->at(newBTModelId);

        if(INVALID_ID == newBTModelId || nullptr == newBTModel)
        {
            Debug::error(intro)("caanot get pointer to model ").quotes(newBTModelId)(". Aborting.").endl();
            return false;
        }

        // model must be linked to agent before it is assigned a path,
        // so that a blackboard is available from the start
        pushBT(newBTModelId);
        newBTModel->setPath(targetPath);
        return true;
    }

    bool Agent::stopFollowingPath(AgentId aid)
    {
        Agent *pathAgent;

        if(nullptr == (pathAgent = mLevel->agentMan()->getAgent(aid)))
        {
            Debug::error(STEEL_METH_INTRO, "was asked to stop following an invalid AgentId ", aid, ". Aborting").endl();
            return false;
        }

        if(!pathAgent->hasLocationPath())
        {
            Debug::error(STEEL_METH_INTRO, "target agent ", aid, " has no location path to unfollow. Aborting").endl();
            return false;
        }

        if(pathAgent->locationPath() != BTPath())
        {
            Debug::error(STEEL_METH_INTRO, "target agent ", aid, "'s location path ").quotes(pathAgent->locationPath())(" is different from selected agents (").quotes(BTPath())("). Aborting").endl();
            return false;
        }

        popBT();
        return true;
    }

    Ogre::String Agent::BTPath()
    {
        return nullptr == btModel() ? LocationModel::EMPTY_PATH : btModel()->path();
    }

    Signal Agent::signal(EventType e)
    {
        auto it = mSignalMap.find(e);
        return mSignalMap.end() == it ? mSignalMap.emplace(e, SignalManager::instance().anonymousSignal()).first->second : it->second;

    }

    bool Agent::isPersistent()
    {
        return isTagged(sPropertyTags.persistent);
    }

    void Agent::setPersistent(bool flag)
    {
        flag ? tag(sPropertyTags.persistent) : untag(sPropertyTags.persistent);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

