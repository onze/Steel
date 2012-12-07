#include "tools/StringUtils.h"
#include <Rocket/Core/String.h>

namespace Steel
{
    std::vector<Ogre::String> StringUtils::_split(std::string src,std::string sep)
    {
        std::vector<Ogre::String> ret;
        std::size_t last=0,next=0;
        int i=0;
        while(1)
        {
            if(++i>10)
                break;
            next=src.find(sep,next);
            ret.push_back(Ogre::String(src.substr(last,next-last)));

            if(next==src.npos)
                break;
            last=next=next+sep.size();
        }
        return ret;
    }

    std::vector<Ogre::String> StringUtils::split(Ogre::String src,Ogre::String sep)
    {
        return StringUtils::_split(std::string(src.c_str()),std::string(sep.c_str()));
    }

    std::vector<Ogre::String> StringUtils::split(Rocket::Core::String src,Rocket::Core::String sep)
    {
        return StringUtils::_split(std::string(src.CString()),std::string(sep.CString()));
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
