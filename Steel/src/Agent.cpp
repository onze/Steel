/*
 * Agent.cpp
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#include <exception>

#include <json/json.h>

#include "Debug.h"
#include "Agent.h"
#include "_ModelManager.h"
#include "OgreModelManager.h"
#include "Level.h"
#include "tools/JsonUtils.h"
#include "TagManager.h"
#include "LocationModelManager.h"
#include "PhysicsModel.h"
#include <BTModel.h>
#include <BTModelManager.h>
#include <AgentManager.h>

namespace Steel
{
    const char *Agent::TAGS_ATTRIBUTE = "tags";
    const char *Agent::ID_ATTRIBUTE = "aid";
    const char *Agent::BEHAVIORS_STACK_ATTRIBUTE = "behaviorsStack";

    Agent::Agent(AgentId id, Steel::Level *level): mId(id), mLevel(level),
        mModelIds(std::map<ModelType, ModelId>()), mIsSelected(false), mTags(std::map<Tag, unsigned>()),
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

        mTags.clear();
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

    bool Agent::fromJson(Json::Value &value)
    {
//  Debug::log("Agent<")(mId)(">::fromJson():").endl()(value.toStyledString()).endl();
        int nModels = 0, nExpected = 0;

        for(ModelType mt_it = (ModelType)((int) MT_FIRST + 1); mt_it != MT_LAST; mt_it = (ModelType)((int) mt_it + 1))
        {
            Ogre::String mtName = modelTypesAsString[mt_it];
            Json::Value mTypeValue = value[mtName];

            // possibly no model of this type
            if(mTypeValue.isNull())
                continue;

            ++nExpected;
            ModelId modelId = (ModelId) Ogre::StringConverter::parseUnsignedLong(mTypeValue.asString());

            if(!linkToModel(mt_it, modelId))
            {
                Debug::error("Agent::fromJson(): agent ")(mId);
                Debug::error(" would not link with model<")(mtName)("> ")(mId)(". Skipping.").endl();
                continue;
            }

            ++nModels;
        }

        if(nModels != nExpected)
        {
            Debug::warning("Agent::fromJson(): agent ")(mId)(" linked with (")(nModels)(" models, ");
            Debug::warning(nExpected)(" were expected. Json string:").endl()(value).endl();
        }

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
        std::map<Tag, unsigned>::iterator it = mTags.find(tag);

        if(mTags.end() == it)
            mTags.insert(std::pair<Tag, unsigned>(tag, 1));
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
            mTags.erase(it);
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

    bool Agent::linkToModel(ModelType mType, ModelId modelId)
    {
        Ogre::String intro = "Agent::linkToModel(type=" + modelTypesAsString[mType] + ", id="
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
            case MT_OGRE:
                while(INVALID_ID != btModelId())
                    popBT();

                unlinkFromModel(MT_BT);
                unlinkFromModel(MT_LOCATION);
                unlinkFromModel(MT_PHYSICS);
                break;

            case MT_LOCATION:
                if(mm->at(mid)->refCount() == 1)
                    ((LocationModelManager *)mm)->unlinkLocation(mid);

            case MT_BT:
            case MT_PHYSICS:
            case MT_BLACKBOARD:
            case MT_FIRST:
            case MT_LAST:
                break;
        }

        untag(mm->modelTags(mid));
        mm->decRef(mid);
        mModelIds.erase(it);
    }

    bool Agent::popBT()
    {
        if(0 == mBehaviorsStack.size())
            return false;

        unlinkFromModel(MT_BT);
        auto mid = mBehaviorsStack.back();
        mBehaviorsStack.pop_back();
        bool ok = linkToModel(MT_BT, mid);
        ok &= nullptr != btModel();

        // stack had added an extra ref to keep it alive
        if(ok)
            btModel()->decRef();

        return ok;
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
            // keep alive
            model->incRef();
            mBehaviorsStack.push_back(btModelId());
            unlinkFromModel(MT_BT);
        }

        // set given on as current
        return linkToModel(MT_BT, btid);
    }

    Model *Agent::model(ModelType mType) const
    {
        ModelId id = modelId(mType);

        if(id == INVALID_ID)
            return nullptr;

        return mLevel->modelManager(mType)->at(id);
    }

    ModelId Agent::modelId(ModelType mType) const
    {
        auto it = mModelIds.find(mType);
        return (it == mModelIds.end() ? INVALID_ID : it->second);
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
    }

    Json::Value Agent::toJson()
    {
        Json::Value root;

        // model ids
        for(std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
        {
            ModelType mt = (*it).first;
            ModelId mid = (*it).second;
            root[modelTypesAsString[mt]] = JsonUtils::toJson(mid);
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
        auto omodel = ogreModel();
        return nullptr == omodel ? Ogre::Vector3::ZERO : omodel->position();
    }

    Ogre::Quaternion Agent::rotation() const
    {
        auto omodel = ogreModel();
        return nullptr == omodel ? Ogre::Quaternion::ZERO : omodel->rotation();
    }

    Ogre::Vector3 Agent::scale() const
    {
        auto omodel = ogreModel();
        return nullptr == omodel ? Ogre::Vector3::ZERO : omodel->scale();
    }

    void Agent::move(const Ogre::Vector3 &dpos)
    {
        //Debug::log("agent ")(id())(" moves ogreModel ")(ogreModelId())(" and physicsModel ")(physicsModelId()).endl();
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->move(dpos);

        auto pmodel = physicsModel();

        if(nullptr != pmodel)
            pmodel->move(dpos);

        auto lmodel = locationModel();

        if(nullptr != lmodel)
            mLevel->locationModelMan()->moveLocation(locationModelId(), position());
    }

    void Agent::rotate(const Ogre::Vector3 &rot)
    {
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->rotate(rot);
    }

    void Agent::rotate(const Ogre::Quaternion &q)
    {
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->rotate(q);
    }

    void Agent::rescale(const Ogre::Vector3 &sca)
    {
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->rescale(sca);

        auto pmodel = physicsModel();

        if(nullptr != pmodel)
            pmodel->rescale(sca);
    }

    void Agent::setPosition(const Ogre::Vector3 &pos)
    {
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->setPosition(pos);

        auto pmodel = physicsModel();

        if(nullptr != pmodel)
            pmodel->setPosition(pos);

        auto lmodel = locationModel();

        if(nullptr != lmodel)
            mLevel->locationModelMan()->moveLocation(locationModelId(), position());
    }

    void Agent::setRotation(const Ogre::Quaternion &rot)
    {
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->setRotation(rot);
    }

    void Agent::setScale(const Ogre::Vector3 &sca)
    {
        auto omodel = ogreModel();

        if(nullptr != omodel)
            omodel->setScale(sca);

        auto pmodel = physicsModel();

        if(nullptr != pmodel)
            pmodel->setScale(sca);
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
            linkToModel(MT_LOCATION, mid);
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
        return true;
    }

    void Agent::unsetBTPath()
    {

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

        if(!mLevel->BTModelMan()->buildFromFile(path, newBTModelId))
        {
            Debug::error(intro)("could not retrieve a BTModel for bt file ")
            .quotes(path)(". Aborting.").endl();
            return false;
        }

        BTModel *newBTModel = mLevel->BTModelMan()->at(newBTModelId);

        if(INVALID_ID == newBTModelId || nullptr == newBTModel)
        {
            Debug::error(intro)("caanot get pointer to model ").quotes(newBTModelId)(". Aborting.").endl();
            return false;
        }

        newBTModel->setPath(targetPath);

        pushBT(newBTModelId);

//         mBehaviorsStack

//         TODO: implement me ! and other method up there too:
//          move BTPath stuff to the agent. The path following bt should be shared among agents, and owned (via an extra refcount) by the locationModelManager.

        return true;
    }

    bool Agent::stopFollowingPath(AgentId aid)
    {
//         Agent *pathAgent;
//         if(nullptr == (pathAgent = mLevel->agentMan()->getAgent(aid)))
        return true;

//         Ogre::String path = pathAgent->locationPath();

//         return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

