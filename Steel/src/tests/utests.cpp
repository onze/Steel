#include <tools/StringUtils.h>
#include <tools/File.h>
#include <Debug.h>

namespace Steel
{

    void test_File()
    {
        /*
        Debug::log("test_File(): start").endl();
        //dirs
        File f("/a/b/c/"),g("/a/d/e/"),h("e/"),i("f/g");
        Debug::log("f:")(f)(" g:")(g)(" h:")(h)(" i:")(i).endl();
        assert(f.relpath(g)=="../../d/e/");
        assert(g.relpath(f)=="../");
        assert(h.relpath(i)=="");
        assert(i.relpath(h)=="");
        assert(false);
        Debug::log("test_File(): passed").endl();
        */
    }

    void tests_StringUtils()
    {
        Debug::log("tests_StringUtils(): start").endl();
        Ogre::String src="a.b,c.d",sep0=",",sep1=".",sep2="!";
        std::vector<Ogre::String> dst;

        dst=StringUtils::split(src,sep2);
        assert(dst[0]==src);
        assert(dst.size()==1);

        dst=StringUtils::split("",sep2);
        assert(dst[0]=="");
        assert(dst.size()==1);

        dst=StringUtils::split(src,sep0);
        assert(dst[0]=="a.b");
        assert(dst[1]=="c.d");
        assert(dst.size()==2);

        dst=StringUtils::split(src,sep1);
        assert(dst[0]=="a");
        assert(dst[1]=="b,c");
        assert(dst[2]=="d");
        assert(dst.size()==3);
        
        assert(src==StringUtils::join(StringUtils::split(src,sep0),sep0));
        assert("a.b"==StringUtils::join(StringUtils::split("a.b;c",";"),".",0,-1));
        assert(""==StringUtils::join(StringUtils::split("a;b;c","."),".",0,-1));
        assert("a;b;c"==StringUtils::join(StringUtils::split("a;b;c",";"),";"));

        Debug::log("tests_StringUtils(): passed").endl();
    }

    void start_tests()
    {
        tests_StringUtils();
//         test_File();
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
