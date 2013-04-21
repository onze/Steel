#include <Rocket/Core/String.h>

#include "tools/StringUtils.h"

namespace Steel
{
    template<class T>
    std::vector<T> StringUtils::split(T const &src,T const &sep)
    {
        std::vector<T> ret;
        std::size_t last=0,next=0;
        int i=0;
        while(1)
        {
            if(++i>10)
                break;
            next=src.find(sep,next);
            ret.push_back(T(src.substr(last,next-last)));

            if(next==src.npos)
                break;
            last=next=next+sep.size();
        }
        return ret;
    }

    std::vector<Ogre::String> StringUtils::split(Ogre::String src,Rocket::Core::String sep)
    {
        return StringUtils::split(src,Ogre::String(sep.CString()));
    }

    std::vector<Ogre::String> StringUtils::split(Rocket::Core::String src,Ogre::String sep)
    {
        return StringUtils::split(Ogre::String(src.CString()),sep);
    }

    std::vector<Ogre::String> StringUtils::split(const char src[],const char sep[])
    {
        return StringUtils::split(Ogre::String(src),Ogre::String(sep));
    }

    Ogre::String StringUtils::join(std::vector<Ogre::String> const &vec,Ogre::String const &joiner,int start,int end)
    {
        // force use of the templated version to avoid infinite loop
        // this overload is still usefull though, because many types convert to Ogre::String (especially cont char *)
        return StringUtils::join<Ogre::String>(vec,joiner,start,end);
    }

    Rocket::Core::String StringUtils::join(Rocket::Core::StringList const &vec,Rocket::Core::String const &joiner,int start,int end)
    {
        return StringUtils::join(std::vector<Rocket::Core::String>(vec),joiner,start,end);
    }

    Ogre::String StringUtils::BTShapeTokenTypeToString(BTShapeTokenType type)
    {
        if(type<_BTFirst) return "<_BTFirst";
        if(type==_BTFirst)return "_BTFirst";
        if(type==_BTLast) return "_BTLast";
        if(type>_BTLast)  return ">_BTLast";
        return BTShapeTokenTypeAsString[type];
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


