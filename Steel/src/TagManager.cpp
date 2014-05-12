
#include "Debug.h"
#include "TagManager.h"
#include <SignalManager.h>

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

    std::vector<Tag> TagManager::tags() const
    {
        std::vector<Tag> _tags;
        _tags.reserve(mTagsMap.size());

        for(auto const & it : mTagsMap)
            _tags.push_back(it.second);

        return _tags;
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
            SignalManager::instance().emit(newTagCreatedSignal());
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

    Ogre::String const& TagManager::fromTag(const Steel::Tag tag) const
    {
        if(INVALID_TAG == tag)
        {
            static Ogre::String const invalid_tag = "<INVALID_TAG>";
            return invalid_tag;
        }

        auto it = mInverseTagsMap.find(tag);

        if(mInverseTagsMap.end() != it)
        {
            return it->second;
        }

        static Ogre::String const invalid_tag = "<UNKNOWN_TAG>";
        return invalid_tag;
    }
    
    std::list<Ogre::String> TagManager::fromTags(std::set< Steel::Tag > tags) const
    {
        std::list<Ogre::String> output;
        
        for(auto const & tag : tags)
            output.push_back(fromTag(tag));
        
        return output;
    }
    
    std::vector<Ogre::String> TagManager::fromTags(std::vector< Steel::Tag > tags) const
    {
        std::vector<Ogre::String> output;
        
        for(auto const & tag : tags)
            output.push_back(fromTag(tag));
        
        return output;
    }

    Signal TagManager::newTagCreatedSignal() const
    {
        static Signal newTagCreatedSignal = SignalManager::instance().toSignal("__TagManager::newTagCreatedSignal");
        return newTagCreatedSignal;
    }

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
