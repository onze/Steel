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
#include <tools/JsonUtils.h>
#include <TagManager.h>

namespace Steel
{
    const char *Agent::TAGS_ATTRIBUTES="tags";

    Agent::Agent(AgentId id, Steel::Level* level): mId(id), mLevel(level),
        mModelIds(std::map<ModelType, ModelId>()),mIsSelected(false),mTags(std::set<Tag>())
    {
        mTags.insert(TagManager::instance().toTag("test"));
    }

    Agent::~Agent()
    {
        cleanup();
    }

    void Agent::cleanup()
    {
        for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
            mLevel->modelManager(it->first)->decRef(it->second);
        mLevel=NULL;
        mModelIds.clear();
        mTags.clear();
        mId=INVALID_ID;
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
        if(this==&o)
            return *this;
        bool wasInUse=INVALID_ID!=mId;

        mId = o.mId;
        bool isInUse=INVALID_ID!=mId;
        mLevel = o.mLevel;

        if(wasInUse)
        {
            for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
                unlinkFromModel(it->first);
        }
        else
        {
            assert(mModelIds.size()==0);
        }

        mModelIds = o.mModelIds;
        if(isInUse)
        {
            for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
                linkToModel(it->first,it->second);
        }
        
        mTags=o.mTags;
        if(mIsSelected!=o.mIsSelected)
        {
            setSelected(mIsSelected);
        }
        return *this;
    }

    bool Agent::fromJson(Json::Value &value)
    {
//	Debug::log("Agent<")(mId)(">::fromJson():").endl()(value.toStyledString()).endl();
        int nModels = 0, nExpected = 0;
        for (ModelType mt_it = (ModelType) ((int) MT_FIRST + 1); mt_it != MT_LAST; mt_it = (ModelType) ((int) mt_it + 1))
        {
            Ogre::String mtName = modelTypesAsString[mt_it];
            Json::Value mTypeValue = value[mtName];
            // possibly no model of this type
            if (mTypeValue.isNull())
                continue;
            ++nExpected;
            ModelId modelId = (ModelId) Ogre::StringConverter::parseUnsignedLong(mTypeValue.asString());
            if (!linkToModel(mt_it, modelId))
            {
                Debug::error("Agent::fromJson(): agent ")(mId);
                Debug::error(" would not link with model<")(mtName)("> ")(mId)(". Skipping.").endl();
                continue;
            }
            ++nModels;
        }
        if (nModels != nExpected)
        {
            Debug::warning("Agent::fromJson(): agent ")(mId)(" linked with (")(nModels)(" models, ");
            Debug::warning(nExpected)(" were expected. Json string:").endl()(value).endl();
        }
        
        // tags
        std::list<Ogre::String> stringTags=JsonUtils::asStringsList(value[Agent::TAGS_ATTRIBUTES]);
        std::list<Tag> tags=TagManager::instance().toTags(stringTags);
        mTags.clear();
        mTags.insert(tags.begin(),tags.end());
        return true;
    }

    bool Agent::linkToModel(ModelType mType, ModelId modelId)
    {
        Ogre::String intro = "Agent::linkToModel(type=" + modelTypesAsString[mType] + ", id="
                             + Ogre::StringConverter::toString(modelId) + "): ";
        if (!mLevel->modelManager(mType)->isValid(modelId))
        {
            Debug::error(intro)("model ")(modelId)(" is not valid. Aborting.").endl();
            return false;
        }

        //result.first==iterator placed at location, result.second==successful insertion flag
        auto result = mModelIds.insert(std::pair<ModelType, ModelId>(mType, modelId));
        if (!result.second)
        {
            Debug::error(intro)("Could not insert model (overwrites are not allowed). Aborting.").endl();
            return false;
        }

        mLevel->modelManager(mType)->incRef(modelId);
        if(!mLevel->modelManager(mType)->onAgentLinkedToModel(this, modelId))
        {
            unlinkFromModel(mType);
            Debug::error(intro)("Agent<->model linking aborted.").endl();
            return false;
        }
        return true;
    }

    void Agent::unlinkFromModel(ModelType mType)
    {
        auto it = mModelIds.find(mType);
        if (it != mModelIds.end())
        {
            switch (mType)
            {
                case MT_OGRE:
                    unlinkFromModel(MT_PHYSICS);
                    break;
                default:
                    break;
            }
            // general case
            mLevel->modelManager(mType)->decRef((*it).second);
            mModelIds.erase(it);
        }
    }

    Model *Agent::model(ModelType mType) const
    {
        ModelId id = modelId(mType);

        if (id == INVALID_ID)
            return NULL;

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
        if (NULL!=om)
            om->setSelected(selected);

        PhysicsModel *pm = physicsModel();
        if(NULL!=pm)
            pm->setSelected(selected);

        mIsSelected=selected;
    }

    Json::Value Agent::toJson()
    {
        Json::Value root;
        
        // model ids
        for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
        {
            ModelType mt = (*it).first;
            ModelId mid = (*it).second;
            root[modelTypesAsString[mt]] = JsonUtils::toJson(mid);
        }
        
        // tags
        root[Agent::TAGS_ATTRIBUTES]=JsonUtils::toJson(TagManager::instance().fromTags(mTags));
        return root;
    }

    Ogre::Vector3 Agent::position() const
    {
        auto omodel = ogreModel();
        return NULL == omodel ? Ogre::Vector3::ZERO : omodel->position();
    }

    Ogre::Quaternion Agent::rotation() const
    {
        auto omodel = ogreModel();
        return NULL == omodel ? Ogre::Quaternion::ZERO : omodel->rotation();
    }

    Ogre::Vector3 Agent::scale() const
    {
        auto omodel = ogreModel();
        return NULL == omodel ? Ogre::Vector3::ZERO : omodel->scale();
    }

    void Agent::move(const Ogre::Vector3 &dpos)
    {
        //Debug::log("agent ")(id())(" moves ogreModel ")(ogreModelId())(" and physicsModel ")(physicsModelId()).endl();
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->move(dpos);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->move(dpos);
    }

    void Agent::rotate(const Ogre::Vector3& rot)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->rotate(rot);
    }

    void Agent::rotate(const Ogre::Quaternion &q)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->rotate(q);
    }

    void Agent::rescale(const Ogre::Vector3 &sca)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->rescale(sca);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->rescale(sca);
    }

    void Agent::setPosition(const Ogre::Vector3 &pos)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->setPosition(pos);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->setPosition(pos);
    }

    void Agent::setRotation(const Ogre::Quaternion &rot)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->setRotation(rot);
    }

    void Agent::setScale(const Ogre::Vector3 &sca)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->setScale(sca);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->setScale(sca);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
