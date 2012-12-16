#ifndef STRINGUTILS_H
#define STRINGUTILS_H
#include <limits.h>
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
            static std::vector<Ogre::String> split(Ogre::String src,Rocket::Core::String sep);
            static std::vector<Ogre::String> split(Rocket::Core::String src,Ogre::String sep);
            static std::vector<Ogre::String> split(const char src[],const char sep[]);
            template<class T>
            static std::vector<T> split(T const &src,T const &sep);
            
            static Ogre::String join(Ogre::String const &joiner,std::vector<Ogre::String> const &vec,int start=0,int end=INT_MIN);
            template<class T>
            static T _join(T const &joiner,std::vector<T> const &vec,int start,int end);
    };
}
#endif // STRINGUTILS_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
