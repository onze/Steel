#ifndef STEEL_TAGMANAGER_H
#define STEEL_TAGMANAGER_H

#include <map>
#include <set>

#include <OgreString.h>

#include "steeltypes.h"

namespace Steel
{
    class TagManager
    {
        public:
            inline static TagManager *instance()
            {
                if(NULL==TagManager::sInstance)
                    TagManager::sInstance=new TagManager();
                return TagManager::sInstance;
            }

            TagManager();
            virtual ~TagManager();

            Signal toTag(const Ogre::String& tag);
#ifdef DEBUG
            Ogre::String fromTag(const Tag tag);
#endif

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
