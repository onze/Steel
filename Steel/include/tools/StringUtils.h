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
            /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Specialized for mixed types.
            static std::vector<Ogre::String> split(Ogre::String src,Rocket::Core::String sep);

            /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Specialized for mixed types.
            static std::vector<Ogre::String> split(Rocket::Core::String src,Ogre::String sep);

            /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Specialized for old types.
            static std::vector<Ogre::String> split(const char src[],const char sep[]);

            /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Generic version.
            template<class T>
            static std::vector<T> split(T const &src,T const &sep);

            /// join a vector of strings with the given joiner string. See generic header for details. Specialized for mixed types.
            static Ogre::String join(std::vector<Ogre::String> const &vec,Ogre::String const &joiner="",int start=0,int end=INT_MIN);
            
            /// join a Rocket::Core::StringList with the given joiner string. See generic header for details. Specialized for mixed types.
            static Rocket::Core::String join(Rocket::Core::StringList const &vec,Rocket::Core::String const &joiner="",int start=0,int end=INT_MIN);

            /**
             * join a vector of strings with the given joiner string, from start to end indices in the given vector (uses the whole vector by default). 
             * Negative indices start from vec.size() (start=0, end=-1 will join the whole vector except for the last item). 
             * Generic version.
             */
            template<class T>
            static T join(std::vector<T> const &vec,T const &joiner,int start,int end);
    };
}
#endif // STRINGUTILS_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
