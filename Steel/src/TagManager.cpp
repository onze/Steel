
#include "Debug.h"
#include "TagManager.h"

namespace Steel
{
    TagManager *TagManager::sInstance = nullptr;

    TagManager::TagManager(): mNextTag(0L), mTagsMap(std::map<Ogre::String, Tag>())
#ifdef DEBUG
        , mInverseTagsMap(std::map<Tag, Ogre::String>())
#endif
    {
    }

    TagManager::~TagManager()
    {
    }

    Tag TagManager::toTag(const Ogre::String &tag)
    {
        if(0 == tag.size())
            return INVALID_TAG;

        auto it = mTagsMap.find(tag);
        Tag returnedValue = INVALID_TAG;

        if(mTagsMap.end() == it)
        {
            mTagsMap[tag] = returnedValue = mNextTag;
            mInverseTagsMap[mNextTag] = tag;
            ++mNextTag;
        }
        else
        {
            returnedValue = it->second;
        }

        return returnedValue;
    }

    std::list<Tag> TagManager::toTags(std::list< Ogre::String > tags)
    {
        std::list<Tag> output;

        for(auto const & tag : tags)
            output.push_back(toTag(tag));

        return output;
    }

    Ogre::String TagManager::fromTag(const Steel::Tag tag) const
    {
        if(INVALID_TAG == tag)
        {
            return "<INVALID_TAG>";
        }

        auto it = mInverseTagsMap.find(tag);

        if(mInverseTagsMap.end() != it)
        {
            return it->second;
        }

        return "unknown tag";
    }

    std::list<Ogre::String> TagManager::fromTags(std::set< Steel::Tag > tags) const
    {
        std::list<Ogre::String> output;

        for(auto const & tag : tags)
            output.push_back(fromTag(tag));

        return output;
    }

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
