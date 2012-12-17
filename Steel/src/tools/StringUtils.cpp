#include "tools/StringUtils.h"
#include <Debug.h>
#include <Rocket/Core/String.h>

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

    template<class T>
    T StringUtils::join(std::vector<T> const &vec,T const &joiner,int start,int end)
    {
        T res;
        if(vec.size()==0)
            return res;

        while(start<0)start+=vec.size();
        if(end==INT_MIN)
            end=vec.size();
        while(end<0)end+=vec.size();

        if(start>=end)
            return res;

//         Debug::log("range ")(start)(" ")(end).endl();

        for(auto i=start; i<end; ++i)
        {
            if(res.length()>0)
                res.append(joiner);
            res.append(vec.at(i));
//             Debug::log("i: ")(i)(" res:")(res).endl();
        }
        return res;
    }

    Ogre::String StringUtils::join(std::vector<Ogre::String> const &vec,Ogre::String const &joiner,int start,int end)
    {
        return StringUtils::join(vec,joiner,start,end);
    }

    Rocket::Core::String StringUtils::join(Rocket::Core::StringList const &vec,Rocket::Core::String const &joiner,int start,int end)
    {
        return StringUtils::join(std::vector<Rocket::Core::String>(vec),joiner,start,end);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


