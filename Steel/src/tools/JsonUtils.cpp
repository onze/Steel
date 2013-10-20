
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

        auto names = src.getMemberNames();
        bool detectDuplicates = overrideDuplicates || warnOnDuplicates;

        for(auto it = names.begin(); it != names.end(); ++it)
        {
            if(detectDuplicates && dst.isMember(*it))
            {
                if(warnOnDuplicates)
                    Debug::warning(intro)("duplicate key \"")(*it)("\"").endl();

                if(!overrideDuplicates)
                    continue;
            }

            dst[*it] = src[*it];
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

    std::list<Ogre::String> JsonUtils::asStringsList(Json::Value const &value, std::list<Ogre::String> defaultValue, Ogre::String defaultItemValue)
    {
        std::list<Ogre::String> output;

        if(value.isArray())
            for(Json::ValueIterator it = value.begin(); it != value.end(); ++it)
                output.push_back(asString(*it, defaultItemValue));
        else
            output.insert(output.end(), defaultValue.begin(), defaultValue.end());

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
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
