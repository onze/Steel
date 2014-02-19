
#include "tools/JsonUtils.h"
#include "tools/StringUtils.h"
#include "Debug.h"
#include <TagManager.h>

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
        bool returned = defaultValue;

        if(value.isString())
            returned = Ogre::StringConverter::parseBool(value.asString(), defaultValue);

        if(value.isBool())
            returned = value.asBool();

        return returned;
    }

    int JsonUtils::asInt(const Json::Value &value, int defaultValue)
    {
        int returned = defaultValue;

        if(value.isString())
            returned = Ogre::StringConverter::parseInt(value.asString(), defaultValue);

        if(value.isIntegral())
            returned = value.asInt();

        return returned;
    }

    float JsonUtils::asFloat(const Json::Value &value, float defaultValue)
    {
        float returned = defaultValue;

        if(value.isString())
            returned =  Ogre::StringConverter::parseReal(value.asString(), defaultValue);

        if(value.isNumeric())
            returned = value.asFloat();

        return returned;
    }

    unsigned long JsonUtils::asUnsignedLong(const Json::Value &value, long unsigned int defaultValue)
    {
        long unsigned int returned = defaultValue;

        if(value.isString())
            returned = Ogre::StringConverter::parseUnsignedLong(value.asString(), defaultValue);

        if(value.isIntegral())
            returned = value.asUInt64();

        return returned;
    }

    Ogre::String JsonUtils::asString(Json::Value const &value, const Ogre::String &defaultValue)
    {
        Ogre::String returned = defaultValue;

        if(value.isString())
            returned = value.asString();

        return returned;
    }

    Ogre::Vector3 JsonUtils::asVector3(Json::Value const &value, const Ogre::Vector3 &defaultValue)
    {
        if(value.isString())
        {
            auto const &s = value.asString();

            if(Ogre::StringConverter::isNumber(s))
            {
                float f = Ogre::StringConverter::parseReal(s);
                return Ogre::Vector3(f, f, f);
            }
            else
            {
                return Ogre::StringConverter::parseVector3(s);
            }
        }
        else if(value.isNumeric())
        {
            float f = value.asFloat();
            return Ogre::Vector3(f, f, f);
        }

        return defaultValue;
    }

    Ogre::Quaternion JsonUtils::asQuaternion(Json::Value const &value, const Ogre::Quaternion &defaultValue)
    {
        Ogre::Quaternion returned = defaultValue;

        if(value.isString())
            returned = Ogre::StringConverter::parseQuaternion(value.asString(), defaultValue);

        return returned;
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
