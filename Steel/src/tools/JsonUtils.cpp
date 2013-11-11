
#include "tools/JsonUtils.h"
#include "tools/StringUtils.h"
#include "Debug.h"

namespace Steel
{

    void JsonUtils::updateObjectWithOverrides(Json::Value const &src, Json::Value &dst)
    {
        JsonUtils::updateObject(src, dst, false, true);
    }

    void JsonUtils::updateObject(Json::Value const &src, Json::Value &dst,
                                 bool const overrideDuplicates,
                                 bool const warnOnDuplicates)
    {
        Ogre::String intro = "JsonUtils::updateObject";

        if(src.isNull() || dst.isNull() || !src.isObject() || !dst.isObject())
            return;

        bool detectDuplicates = overrideDuplicates || warnOnDuplicates;

        for(auto const & name : src.getMemberNames())
        {
            if(detectDuplicates && dst.isMember(name))
            {
                if(warnOnDuplicates)
                    Debug::warning(intro)("duplicate key \"")(name)("\"").endl();

                if(!overrideDuplicates)
                    continue;
            }

            dst[name] = src[name];
        }
    }

    bool JsonUtils::asBool(const Json::Value &value, bool defaultValue)
    {
        if(value.isString())
            return Ogre::StringConverter::parseBool(value.asString(), defaultValue);

        if(value.isBool())
            return value.asBool();

        return defaultValue;
    }

    bool JsonUtils::asInt(const Json::Value &value, int defaultValue)
    {
        if(value.isString())
            return Ogre::StringConverter::parseInt(value.asString(), defaultValue);

        if(value.isIntegral())
            return value.asInt();

        return defaultValue;
    }

    float JsonUtils::asFloat(const Json::Value &value, float defaultValue)
    {
        if(value.isString())
            return Ogre::StringConverter::parseReal(value.asString(), defaultValue);

        if(value.isNumeric())
            return value.asFloat();

        return defaultValue;
    }

    unsigned long JsonUtils::asUnsignedLong(const Json::Value &value, long unsigned int defaultValue)
    {
        if(value.isString())
            return Ogre::StringConverter::parseUnsignedLong(value.asString(), defaultValue);

        if(value.isIntegral())
            return value.asUInt64();

        return defaultValue;
    }

    Ogre::String JsonUtils::asString(Json::Value const &value, const Ogre::String &defaultValue)
    {
        if(value.isString())
            return value.asString();

        return defaultValue;
    }
    
    Ogre::Vector3 JsonUtils::asVector3(Json::Value const &value, const Ogre::Vector3 &defaultValue)
    {
        if(value.isString())
            return Ogre::StringConverter::parseVector3(value.asString());
        
        return defaultValue;
    }

    std::list<Ogre::String> JsonUtils::asStringsList(Json::Value const &value, std::list<Ogre::String> defaultValue, Ogre::String defaultItemValue)
    {
        if(value.isNull() || !value.isArray())
            return defaultValue;

        std::list<Ogre::String> output;

        for(Json::ValueIterator it = value.begin(); it != value.end(); ++it)
            output.push_back(asString(*it, defaultItemValue));

        return output;
    }

    std::list<ModelId> JsonUtils::asModelIdList(const Json::Value &value,
            std::list<ModelId> defaultValue,
            ModelId defaultItemValue)
    {
        if(value.isNull() || !value.isArray())
            return defaultValue;

        std::list<ModelId> output;

        for(Json::ValueIterator it = value.begin(); it != value.end(); ++it)
            output.push_back(asModelId(*it, defaultItemValue));

        return output;
    }

    std::set<Tag> JsonUtils::asTagsSet(const Json::Value &value, const std::set<Tag> defaultValue)
    {
        if(value.isNull() || !value.isArray())
            return defaultValue;

        std::list<Ogre::String> stringTags = JsonUtils::asStringsList(value);
        auto tagsList = TagManager::instance().toTags(stringTags);
        std::set<Tag> tags(tagsList.begin(), tagsList.end());
        return tags;
    }

    std::set<unsigned long> JsonUtils::asUnsignedLongSet(const Json::Value &value, const std::set<unsigned long> defaultValue, unsigned long defaultItemValue)
    {
        if(value.isNull() || !value.isArray())
            return defaultValue;

        std::set<unsigned long> output;

        for(Json::ValueIterator it = value.begin(); it != value.end(); ++it)
            output.insert(asUnsignedLong(*it, defaultItemValue));

        return output;
    }
    
    std::list<unsigned long> JsonUtils::asUnsignedLongList(const Json::Value &value, const std::list<unsigned long> defaultValue, unsigned long defaultItemValue)
    {
        if(value.isNull() || !value.isArray())
            return defaultValue;
        
        std::list<unsigned long> output;
        
        for(Json::ValueIterator it = value.begin(); it != value.end(); ++it)
            output.push_back(asUnsignedLong(*it, defaultItemValue));
        
        return output;
    }

    std::map<Ogre::String, Ogre::String> JsonUtils::asStringStringMap(const Json::Value &value)
    {
        std::map<Ogre::String, Ogre::String> map;

        if(value.isNull() || !value.isObject())
            return map;

        for(auto const & name : value.getMemberNames())
            map.emplace(Ogre::String(name), asString(value[name], Ogre::StringUtil::BLANK));

        return map;
    }
    
    std::map<Ogre::String, unsigned long> JsonUtils::asStringUnsignedLongMap(const Json::Value &value)
    {
        std::map<Ogre::String, unsigned long> map;
        
        if(value.isNull() || !value.isObject())
            return map;
        
        for(auto const & name : value.getMemberNames())
            map.emplace(Ogre::String(name), asUnsignedLong(value[name], 0));
        
        return map;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
