/*
 * Model.cpp
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */
#include <list>

#include <OgreString.h>

#include "Agent.h"
#include "Model.h"
#include "tools/JsonUtils.h"

namespace Steel
{
    const char *Model::AGENT_TAGS_ATTRIBUTES = "agentTags";

    Model::Model() :
        mRefCount(0), mTags(std::set<Tag>())
    {

    }

    Model::Model(const Model &o): mRefCount(o.mRefCount), mTags(o.mTags)
    {
    }

    Model::~Model()
    {
    }

    Model &Model::operator=(const Model &o)
    {
        mRefCount = o.mRefCount;
        mTags = o.mTags;
        return *this;
    }

    bool Model::operator==(const Model &o) const
    {
        return mTags == o.mTags;
    }

    void Model::cleanup()
    {
        mTags.clear();
    }

    void Model::serializeTags(Json::Value &value)
    {
        if(!mTags.size())
            return;

        std::list<Ogre::String> tags = TagManager::instance().fromTags(mTags);
        value[Model::AGENT_TAGS_ATTRIBUTES] = JsonUtils::toJson(tags);
    }

    bool Model::deserializeTags(const Json::Value &value)
    {
        if(!value.isMember(Model::AGENT_TAGS_ATTRIBUTES))
            return true;

        mTags.clear();
        
        Json::Value member(value[Model::AGENT_TAGS_ATTRIBUTES]);
        if(member.isArray())
        {
            auto _tags = JsonUtils::asTagsSet(value[Model::AGENT_TAGS_ATTRIBUTES]);
            mTags.insert(_tags.begin(), _tags.end());
        }
        else if(member.isString())
        {
            mTags.insert(TagManager::instance().toTag(member.asString()));
        }
        else
        {
            Debug::error("Model::deserializeTags(): can get tags out of member ")
            .quotes(Model::AGENT_TAGS_ATTRIBUTES)(" while deserializing ")(value)("). Skipping.").endl();
        }
        return true;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
