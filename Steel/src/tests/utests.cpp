#include <tools/StringUtils.h>
#include <Debug.h>
 
namespace Steel
{
    

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
    
    Debug::log("tests_StringUtils(): passed").endl();
}

void start_tests()
{
    tests_StringUtils();
}
    
}