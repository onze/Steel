#include "tests/utests.h"

#include "_ModelManager.h"
#include "tools/StringUtils.h"
#include "tools/File.h"
#include "tools/ConfigFile.h"
#include "Debug.h"
#include "tests/utests_BTrees.h"
#include "tests/utests_BTShapeStream.h"
#include "tests/utests_BTStateStream.h"
#include <Engine.h>

namespace Steel
{

    bool test_File()
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

        return true;
    }

    bool test_StringUtils()
    {
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
        return true;
    }

    bool test_ConfigFile()
    {
        Debug::log("tests_ConfigFile(): passed").endl();

        Ogre::String path="/tmp/test.cfg";
        Ogre::String path2="/tmp/test.cfg2";
        File file(path);
        if(file.exists())
            file.rm();

        ConfigFile cf(path);
        assert(cf.getSetting("iop")=="");
        Ogre::String ret,value="haha";
        cf.setSetting("iop",value);
        ret=cf.getSetting("iop");
        assert(ret==value);
        cf.save();

        auto cf2=ConfigFile(path2);
        cf=cf2;
        ret=cf.getSetting("iop");
        assert(ret=="");
        cf=ConfigFile(path);
        ret=cf.getSetting("iop");
        assert(ret==value);

        Debug::log("tests_ConfigFile(): passed").endl();
        return true;
    }

    bool startTests(Engine *engine,bool abortOnFail)
    {
        bool passed=true;
        if(!(passed&=test_StringUtils()) && abortOnFail)return false;
        if(!(passed&=test_File()) && abortOnFail)return false;
        if(!(passed&=test_ConfigFile()) && abortOnFail)return false;
        if(!(passed&=test_BTShapeStream()) && abortOnFail)return false;
        if(!(passed&=test_BTStateStream()) && abortOnFail)return false;
        if(!(passed&=test_BTrees(engine)) && abortOnFail)return false;

        if(passed)
            Debug::log("start_tests(): all direct unit tests passed. Registering gameplay tests...").endl();
        else
            Debug::error("Some unit tests failed.");

        registerGameplayUTests(engine);
        return passed;
    }

    void registerGameplayUTests(Engine *e)
    {
        // should those be grabbed from somewhere else ?
        Ogre::String res="resources";
        Ogre::String btrees="BTree models";
        Ogre::String composites="Model Composites";
        auto ddir=e->dataDir();
        e->registerCommand("editor.instanciate."+ddir.subfile(res).subfile(composites).subfile("patrol.models").fullPath());
        //e->registerCommand("editor.instanciate."+ddir.subfile(res).subfile(btrees).subfile("patrol.model").fullPath());
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
