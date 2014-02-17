#include <Rocket/Core/String.h>

#include "tools/StringUtils.h"
#include <Debug.h>

namespace Steel
{

#ifdef __unix
    const Ogre::String StringUtils::LINE_SEP = "\n";
#else //if defined(_WIN32)
    const Ogre::String StringUtils::LINE_SEP = "\r\n";
#endif

    template<class T>
    std::vector<T> StringUtils::split(T const &src, T const &sep)
    {
        std::vector<T> ret;
        std::size_t last = 0, next = 0;
        int i = 0;

        while(1)
        {
            if(++i > 10)
                break;

            next = src.find(sep, next);
            ret.push_back(T(src.substr(last, next - last)));

            if(next == src.npos)
                break;

            last = next = next + sep.size();
        }

        return ret;
    }

    std::vector<Ogre::String> StringUtils::split(Ogre::String src, Rocket::Core::String sep)
    {
        return StringUtils::split(src, Ogre::String(sep.CString()));
    }

    std::vector<Ogre::String> StringUtils::split(Rocket::Core::String src, Ogre::String sep)
    {
        return StringUtils::split(Ogre::String(src.CString()), sep);
    }

    std::vector<Ogre::String> StringUtils::split(const char src[], const char sep[])
    {
        return StringUtils::split(Ogre::String(src), Ogre::String(sep));
    }

    Ogre::String StringUtils::join(std::vector<Ogre::String> const &vec, Ogre::String const &joiner, int start, int end)
    {
        // force use of the templated version to avoid infinite loop
        // this overload is still usefull though, because many types convert to Ogre::String (especially cont char *)
        return StringUtils::join<Ogre::String>(vec, joiner, start, end);
    }

    Ogre::String StringUtils::join(std::list<Ogre::String> const &list, Ogre::String const &joiner, int start, int end)
    {
        std::vector<Ogre::String> vec(list.begin(), list.end());
        return StringUtils::join<Ogre::String>(vec, joiner, start, end);
    }

    Rocket::Core::String StringUtils::join(Rocket::Core::StringList const &vec, Rocket::Core::String const &joiner, int start, int end)
    {
        return StringUtils::join(std::vector<Rocket::Core::String>(vec), joiner, start, end);
    }

    Ogre::String StringUtils::BTShapeTokenTypeToString(BTShapeTokenType type)
    {
        if(type < _BTFirst) return "<_BTFirst";

        if(type == _BTFirst)return "_BTFirst";

        if(type == _BTLast) return "_BTLast";

        if(type > _BTLast)  return ">_BTLast";

        return BTShapeTokenTypeAsString[type];
    }

    ////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS

    bool utest_StringUtils(UnitTestExecutionContext const* context)
    {
        Ogre::String src = "a.b,c.d", sep0 = ",", sep1 = ".", sep2 = "!";
        std::vector<Ogre::String> dst;

        dst = StringUtils::split(src, sep2);
        assert(dst[0] == src);
        assert(dst.size() == 1);

        dst = StringUtils::split(Ogre::StringUtil::BLANK, sep2);
        assert(dst[0] == Ogre::StringUtil::BLANK);
        assert(dst.size() == 1);

        dst = StringUtils::split(src, sep0);
        assert(dst[0] == "a.b");
        assert(dst[1] == "c.d");
        assert(dst.size() == 2);

        dst = StringUtils::split(src, sep1);
        assert(dst[0] == "a");
        assert(dst[1] == "b,c");
        assert(dst[2] == "d");
        assert(dst.size() == 3);

        assert(src == StringUtils::join(StringUtils::split(src, sep0), sep0));
        assert("a.b" == StringUtils::join(StringUtils::split("a.b;c", ";"), ".", 0, -1));
        assert(Ogre::StringUtil::BLANK == StringUtils::join(StringUtils::split("a;b;c", "."), ".", 0, -1));
        assert("a;b;c" == StringUtils::join(StringUtils::split("a;b;c", ";"), ";"));

        Debug::log("tests_StringUtils(): passed").endl();
        return true;
    }
}

