#include "tools/JsonUtils.h"
#include "Debug.h"

namespace Steel
{

    void JsonUtils::updateObjectWithOverrides(Json::Value const &src, Json::Value &dst)
    {
        JsonUtils::updateObject(src, dst, false, true);
    }

    void JsonUtils::updateObject(Json::Value const &src, Json::Value &dst, bool const overrideDuplicates,
                                 bool const warnOnDuplicates)
    {
        Ogre::String intro = "JsonUtils::updateObject";
        auto names = src.getMemberNames();
        bool detectDuplicates = overrideDuplicates || warnOnDuplicates;
        for (auto it = names.begin(); it != names.end(); ++it)
        {
            if (detectDuplicates && dst.isMember(*it))
            {
                if (warnOnDuplicates)
                    Debug::warning(intro)("duplicate key \"")(*it)("\"").endl();
                if (!overrideDuplicates)
                    continue;
            }
            dst[*it] = src[*it];
        }
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
