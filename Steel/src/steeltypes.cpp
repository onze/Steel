
#include "steeltypes.h"

namespace Steel
{
    Duration DURATION_MAX = LONG_MAX;
    Duration DURATION_MIN = LONG_MIN;

    Ogre::String toString(ModelType e)
    {
        STEEL_ENUM_TO_STRING_START(ModelType)
        {
            STEEL_ENUM_TO_STRING_CASE(ModelType::FIRST);
            STEEL_ENUM_TO_STRING_CASE(ModelType::OGRE);
            STEEL_ENUM_TO_STRING_CASE(ModelType::PHYSICS);
            STEEL_ENUM_TO_STRING_CASE(ModelType::LOCATION);
            STEEL_ENUM_TO_STRING_CASE(ModelType::BLACKBOARD);
            STEEL_ENUM_TO_STRING_CASE(ModelType::BT);
            STEEL_ENUM_TO_STRING_CASE(ModelType::LAST);

        default: break;
        }
        return Ogre::StringUtil::BLANK;
    }

    ModelType toModelType(Ogre::String s)
    {
        STEEL_STRING_TO_ENUM_CASE(ModelType::FIRST);
        STEEL_STRING_TO_ENUM_CASE(ModelType::OGRE);
        STEEL_STRING_TO_ENUM_CASE(ModelType::PHYSICS);
        STEEL_STRING_TO_ENUM_CASE(ModelType::LOCATION);
        STEEL_STRING_TO_ENUM_CASE(ModelType::BLACKBOARD);
        STEEL_STRING_TO_ENUM_CASE(ModelType::BT);
        STEEL_STRING_TO_ENUM_CASE(ModelType::LAST);
        return ModelType::LAST;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 



