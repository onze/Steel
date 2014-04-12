#include <Rocket/Core/String.h>
#include <OgreString.h>

#include "tools/StringUtils.h"
#include "Debug.h"
#include "tests/UnitTestManager.h"

namespace Steel
{
    Ogre::String const StringUtils::BLANK = Ogre::StringUtil::BLANK;
    Ogre::String const StringUtils::blank() {
        static const Ogre::String BLANK = Ogre::StringUtil::BLANK;
        return BLANK;
    }

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

    ////////////////////////////////////////////////////////////////////////////
    // UNIT TESTS

    bool utest_StringUtils(UnitTestExecutionContext const *context)
    {
        Ogre::String src = "a.b,c.d", sep0 = ",", sep1 = ".", sep2 = "!";
        std::vector<Ogre::String> dst;

        dst = StringUtils::split(src, sep2);

        STEEL_UT_ASSERT(dst[0] == src, "[UT001] StringUtils::split failed");
        STEEL_UT_ASSERT(dst.size() == 1, "[UT002] StringUtils::split failed");

        dst = StringUtils::split(StringUtils::BLANK, sep2);
        STEEL_UT_ASSERT(dst[0] == StringUtils::BLANK, "[UT] StringUtils::split failed");
        STEEL_UT_ASSERT(dst.size() == 1, "[UT003] StringUtils::split failed");

        dst = StringUtils::split(src, sep0);
        STEEL_UT_ASSERT(dst[0] == "a.b", "[UT004] StringUtils::split failed");
        STEEL_UT_ASSERT(dst[1] == "c.d", "[UT005] StringUtils::split failed");
        STEEL_UT_ASSERT(dst.size() == 2, "[UT006] StringUtils::split failed");

        dst = StringUtils::split(src, sep1);
        STEEL_UT_ASSERT(dst[0] == "a", "[UT007] StringUtils::split failed");
        STEEL_UT_ASSERT(dst[1] == "b,c", "[UT008] StringUtils::split failed");
        STEEL_UT_ASSERT(dst[2] == "d", "[UT009] StringUtils::split failed");
        STEEL_UT_ASSERT(dst.size() == 3, "[UT010] StringUtils::split failed");

        STEEL_UT_ASSERT(src == StringUtils::join(StringUtils::split(src, sep0), sep0), "[UT011] StringUtils::join failed");
        STEEL_UT_ASSERT("a.b" == StringUtils::join(StringUtils::split("a.b;c", ";"), ".", 0, -1), "[UT012] StringUtils::join failed");
        STEEL_UT_ASSERT(StringUtils::BLANK == StringUtils::join(StringUtils::split("a;b;c", "."), ".", 0, -1), "[UT013] StringUtils::join failed");
        STEEL_UT_ASSERT("a;b;c" == StringUtils::join(StringUtils::split("a;b;c", ";"), ";"), "[UT014] StringUtils::join failed");

        return true;
    }
}

