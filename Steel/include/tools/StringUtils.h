#ifndef STRINGUTILS_H
#define STRINGUTILS_H
#include <vector>
#include <string>
#include <OgreString.h>
#include <Rocket/Core/String.h>

namespace Steel
{
    class StringUtils
    {
        public:
            /// splits a string into parts, delimited by a given separator, not included in result.
            static std::vector<Ogre::String> _split(std::string src,std::string sep);
            static std::vector<Ogre::String> split(Ogre::String src,Ogre::String sep);
            static std::vector<Ogre::String> split(Rocket::Core::String src,Rocket::Core::String sep);
    };
}
#endif // STRINGUTILS_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
