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

namespace Steel
{
    const char *Agent::TAGS_ATTRIBUTE = "tags";
    const char *Agent::ID_ATTRIBUTE = "aid";

    Agent::Agent(AgentId id, Steel::Level *level): mId(id), mLevel(level),
        mModelIds(std::map<ModelType, ModelId>()), mIsSelected(false), mTags(std::map<Tag, unsigned>())
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

        mTags.clear();
        mLevel = nullptr;
        mId = INVALID_ID;
    }

    Agent::Agent(const Agent &o)
        : mId(o.mId), mLevel(o.mLevel), mModelIds(o.mModelIds), mIsSelected(o.mIsSelected), mTags(o.mTags)
    {
        if(this == &o)
            return;

        this->operator=(o);
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

        switch(mType)
        {
            case MT_OGRE:
                unlinkFromModel(MT_PHYSICS);
                unlinkFromModel(MT_LOCATION);
                break;

            case MT_LOCATION:
                if(mm->at(mid)->refCount() == 1)
                    ((LocationModelManager *)mm)->unlinkLocation(mid);

            case MT_BT:
            case MT_PHYSICS:
            case MT_FIRST:
            case MT_LAST:
                break;
        }

        untag(mm->modelTags(mid));
        mm->decRef(mid);
        mModelIds.erase(it);
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
        auto _tags = tags();
        root[Agent::TAGS_ATTRIBUTE] = JsonUtils::toJson(TagManager::instance().fromTags(_tags));
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
            mLevel->locationMan()->moveLocation(locationModelId(), position());
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
            mLevel->locationMan()->moveLocation(locationModelId(), position());
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

    bool Agent::setPath(Ogre::String const &name)
    {
        auto model = locationModel();

        if(nullptr == model)
        {
            auto locationMan = mLevel->locationMan();

            if(nullptr == locationMan)
                return false;

            ModelId mid = locationMan->newModel();
            linkToModel(MT_LOCATION, mid);
            model = locationModel();
        }

        return model->setPath(name);
    }

    void Agent::unsetPath()
    {
        auto model = locationModel();

        if(nullptr == model)
            return;

        model->unsetPath();
    }

    bool Agent::hasPath()
    {
        auto model = locationModel();

        if(nullptr == model)
            return false;

        return model->hasPath();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

