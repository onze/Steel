#ifndef STEEL_JSONUTILS_H
#define STEEL_JSONUTILS_H

#include <json/json.h>
#include <OgreString.h>

namespace Steel
{
    class JsonUtils
    {
        public:
            static void updateObjectWithOverrides(Json::Value const &src, Json::Value &dst);
        private:
            /// Copies src keys to dst keys.
            static void updateObject(Json::Value const &src, Json::Value &dst, bool overrideDuplicates, bool warnOnDuplicates);
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
