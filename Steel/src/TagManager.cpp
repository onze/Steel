
#include "Debug.h"
#include "TagManager.h"

namespace Steel
{
    TagManager* TagManager::sInstance = NULL;

    TagManager::TagManager(): mNextTag(0L), mTagsMap(std::map<Ogre::String, Tag>())
#ifdef DEBUG
        , mInverseTagsMap(std::map<Tag, Ogre::String>())
#endif
    {
    }

    TagManager::~TagManager()
    {

    }

    Tag TagManager::toTag(const Ogre::String& tag)
    {
        if(0==tag.size())
            return INVALID_TAG;
        
        auto it=mTagsMap.find(tag);
        Tag returnedValue=INVALID_TAG;
        if(mTagsMap.end()==it)
        {
            mTagsMap[tag]=returnedValue=mNextTag;
#ifdef DEBUG
            mInverseTagsMap[mNextTag]=tag;
#endif
            ++mNextTag;
        }
        else
        {
            returnedValue=it->second;
        }
        return returnedValue;
    }

#ifdef DEBUG
    Ogre::String TagManager::fromTag(const Tag tag)
    {
        if(INVALID_TAG == tag)
        {
            return "<INVALID_TAG>";
        }

        auto it=mInverseTagsMap.find(tag);
        if(mInverseTagsMap.end()!=it)
        {
            return it->second;
        }

        return "unknown tag";
    }
#endif

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
