#ifndef STEEL_JSONUTILS_H
#define STEEL_JSONUTILS_H

#include <map>
#include <list>
#include <set>

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

        static Json::Value toJson(std::list<Ogre::String> const &container)
        {
            Json::Value value;

            if(container.size())
                for(auto const & t : container)
                    value.append(t.c_str());

            return value;
        }

        /// serialize the value into a Json::Value object.
        template<class T>
        static Json::Value toJson(std::list<T> const &container)
        {
            Json::Value value;

            if(container.size())
                for(auto const & t : container)
                    value.append(Ogre::StringConverter::toString(t).c_str());

            return value;
        }

        /// serialize the value into a Json::Value object.
        static Json::Value toJson(std::set<Ogre::String> const &container)
        {
            Json::Value value;

            if(container.size())
                for(auto const & t : container)
                    value.append(t.c_str());

            return value;
        }

        /// serialize the value into a Json::Value object.
        template<class T>
        static Json::Value toJson(std::set<T> const &container)
        {
            Json::Value value;

            if(container.size())
                for(auto const & t : container)
                    value.append(Ogre::StringConverter::toString(t).c_str());

            return value;
        }

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
        
        template<class V>
        static Json::Value toJson(std::map<Ogre::String, V> const &container)
        {
            Json::Value value;
            
            for(auto const & item: container)
                value[item.first.c_str()] = toJson(item.second);
                
            return value;
        }

        /// serialize the value into a Json::Value object.
        template<class K, class V>
        static Json::Value toJson(std::map<K, V> const &container)
        {
            Json::Value value;

            for(auto const & item : container)
                value[Ogre::StringConverter::toString(item.first).c_str()] = toJson(item.second);

            return value;
        }

        static bool asBool(Json::Value const &value, bool defaultValue);
        static bool asInt(Json::Value const &value, int defaultValue);
        static float asFloat(Json::Value const &value, float defaultValue);
        static unsigned long asUnsignedLong(Json::Value const &value, unsigned long defaultValue);
        static inline ModelId asModelId(Json::Value const &value, unsigned long defaultValue) {return asUnsignedLong(value, defaultValue);};
        static Ogre::String asString(Json::Value const &value, const Ogre::String &defaultValue);

        static std::list<Ogre::String> asStringsList(const Json::Value &value,
                std::list< Ogre::String > defaultValue = std::list< Ogre::String >(),
                                                     Ogre::String defaultItemValue = Ogre::StringUtil::BLANK);
        
        static std::set<Tag> asTagsSet(const Json::Value &value,
                                       const std::set<Tag> defaultValue = std::set<Tag>());
        
        static std::set<unsigned long> asUnsignedLongSet(const Json::Value &value,
                                               const std::set<unsigned long> defaultValue = std::set<unsigned long>(),
                                               unsigned long defaultItemValue = 0
                                              );

        static std::list<ModelId> asModelIdList(const Json::Value &value,
                                                std::list<ModelId> defaultValue = std::list<ModelId>(),
                                                ModelId defaultItemValue = INVALID_ID);

        static std::map<Ogre::String, Ogre::String> asStringStringMap(const Json::Value &value);


    private:
        /// Copies src keys to dst keys.
        static void updateObject(Json::Value const &src, Json::Value &dst, bool overrideDuplicates, bool warnOnDuplicates);
    };
}


#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
