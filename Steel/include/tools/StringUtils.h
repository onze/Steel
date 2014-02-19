#ifndef STEEL_STRINGUTILS_H
#define STEEL_STRINGUTILS_H

#include <OgreStringConverter.h>
#include <Rocket/Core/String.h>

#include <steeltypes.h>

namespace Steel
{
    class UnitTestExecutionContext;

    class StringUtils
    {
    public:
        /// Line separator.
        static const Ogre::String LINE_SEP;

        /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Specialized for mixed types.
        static std::vector<Ogre::String> split(Ogre::String src, Rocket::Core::String sep);

        /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Specialized for mixed types.
        static std::vector<Ogre::String> split(Rocket::Core::String src, Ogre::String sep);

        /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Specialized for old types.
        static std::vector<Ogre::String> split(const char src[], const char sep[]);

        /// split a string into parts, delimited by a given separator, not included in the returned resulting vector. Generic version.
        template<class T>
        static std::vector<T> split(T const &src, T const &sep);

        /// join a vector of strings with the given joiner string. See generic header for details. Specialized for mixed&old types.
        static Ogre::String join(std::vector<Ogre::String> const &vec,
                                 Ogre::String const &joiner = Ogre::StringUtil::BLANK,
                                 int start = 0, int end = INT_MIN);

        /// join a list of strings with the given joiner string. See generic header for details. Specialized for mixed&old types.
        static Ogre::String join(std::list<Ogre::String> const &vec,
                                 Ogre::String const &joiner = Ogre::StringUtil::BLANK,
                                 int start = 0, int end = INT_MIN);

        /// join a Rocket::Core::StringList with the given joiner string. See generic header for details. Specialized for mixed types.
        static Rocket::Core::String join(Rocket::Core::StringList const &vec,
                                         Rocket::Core::String const &joiner = "",
                                         int start = 0, int end = INT_MIN);

        /**
         * join a vector of strings with the given joiner string, from start to end indices in the given vector (uses the whole vector by default).
         * Negative indices start from vec.size() (start=0, end=-1 will join the whole vector except for the last item).
         * Generic version.
         */
        template<class T>
        static T join(std::vector<T> const &vec, T const &joiner = Ogre::StringUtil::BLANK, int start = 0, int end = INT_MIN)
        {
            T res;

            if(vec.size() == 0)
                return res;

            while(start < 0)
                start += vec.size();

            if(end == INT_MIN)
                end = vec.size();

            while(end < 0)
                end += vec.size();

            if(start >= end)
                return res;

            for(auto i = start; i < end; ++i)
            {
                if(res.length() > 0)
                    res.append(joiner);

                res.append(vec.at(i));
            }

            return res;
        }
    };

    bool utest_StringUtils(UnitTestExecutionContext const *context);
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
