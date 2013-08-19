#ifndef STEEL_JSONUTILS_H
#define STEEL_JSONUTILS_H

#include <json/json.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <steeltypes.h>
#include <SignalManager.h>
#include <TagManager.h>

namespace Steel
{
    class JsonUtils
    {
        public:
            static void updateObjectWithOverrides(Json::Value const &src, Json::Value &dst);
            
            /// serialize the value into a Json::Value object.
            template<class T>
            static Json::Value toJson(T const &value)
            {
                return Json::Value(Ogre::StringConverter::toString(value).c_str());
            }
            
            /// serialize the value into a Json::Value object.
            static Json::Value toJson(std::string const &value)
            {
                return Json::Value(value.c_str());
            }
            
            /// serialize the value into a Json::Value object.
            template<class T>
            static Json::Value toJson(std::set<T> const &container)
            {
                Json::Value value;
                for(auto const &t:container)
                    value.append(Ogre::StringConverter::toString(t).c_str());
                return value;
            }
            
            /// serialize the value into a Json::Value object.
            template<class K, class V>
            static Json::Value toJson(std::map<K,V> const &container)
            {
                Json::Value value;
                for(auto const &item:container)
                    value[Ogre::StringConverter::toString(item.first).c_str()]=toJson(item.second);
                return value;
            }
            
            static bool asBool(Json::Value const &value, bool defaultValue);
            static bool asInt(Json::Value const &value, int defaultValue);
            static float asFloat(Json::Value const &value, float defaultValue);
            static unsigned long asUnsignedLong(Json::Value const &value, unsigned long defaultValue);
            
            
        private:
            /// Copies src keys to dst keys.
            static void updateObject(Json::Value const &src, Json::Value &dst, bool overrideDuplicates, bool warnOnDuplicates);
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
