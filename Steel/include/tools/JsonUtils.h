#ifndef STEEL_JSONUTILS_H
#define STEEL_JSONUTILS_H

#include <json/json.h>
#include <OgreString.h>
#include <OgreStringConverter.h>

namespace Steel
{
    class JsonUtils
    {
        public:
            static void updateObjectWithOverrides(Json::Value const &src, Json::Value &dst);
            
            /// serialize the value into a Json::Value object.
            template<class T>
            inline static Json::Value toJson(T const &value)
            {
                return Json::Value(Ogre::StringConverter::toString(value).c_str());
            }
            
            /// serialize the value into a Json::Value object.
            template<class T>
            inline static Json::Value toJson(std::set<T> const &container)
            {
                Json::Value value;
                for(auto t:container)
                    value.append(Ogre::StringConverter::toString(t).c_str());
                return value;
            }
            
            /// serialize the value into a Json::Value object.
            inline static Json::Value toJson(std::string const &value)
            {
                return Json::Value(value.c_str());
            }
            
        private:
            /// Copies src keys to dst keys.
            static void updateObject(Json::Value const &src, Json::Value &dst, bool overrideDuplicates, bool warnOnDuplicates);
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
