#ifndef STEEL_TAGMANAGER_H
#define STEEL_TAGMANAGER_H

#include "steeltypes.h"

namespace Steel
{
    class TagManager
    {
    public:
        inline static TagManager &instance()
        {
            if(nullptr == TagManager::sInstance)
                TagManager::sInstance = new TagManager();

            return *TagManager::sInstance;
        }

        TagManager();
        virtual ~TagManager();

        std::vector<Tag> tags() const;

        Tag toTag(const Ogre::String &tag);

        std::list<Tag> toTags(std::list<Ogre::String> tags);

        Ogre::String const &fromTag(const Tag tag) const;

        std::list<Ogre::String> fromTags(std::set<Tag> tags) const;
        std::vector<Ogre::String> fromTags(std::vector<Tag> tags) const;

        /// Emitted each time a new tag is created
        Signal newTagCreatedSignal() const;

    private:
        static TagManager *sInstance;

        /// Value of the next created signal.
        Tag mNextTag;

        /// Maps string tags to long values, used internally.
        std::map<Ogre::String, Tag> mTagsMap;
#ifdef DEBUG
        /// mTagsMap's reverse mapping, for debug purposes.
        std::map<Tag, Ogre::String> mInverseTagsMap;
#endif
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
