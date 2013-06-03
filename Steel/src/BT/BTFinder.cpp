
#include <assert.h>
#include <iostream>
#include <json/value.h>

#include "BT/BTFinder.h"
#include <Debug.h>

namespace Steel
{
    const Ogre::String BTFinder::LOCALIZATION_MODE="lmode";
    const Ogre::String BTFinder::LOCALIZATION_MODE_STATIC="static";
    const Ogre::String BTFinder::LOCALIZATION_MODE_DYNAMIC="dynamic";

    BTFinder::BTFinder(const Steel::BTShapeToken& token) : BTNode(token),
        mLocalizationMode(LM_STATIC)
    {
    }

    BTFinder::~BTFinder()
    {
    }

    bool BTFinder::parseNodeContent(Json::Value &root)
    {
        Ogre::String intro="in BTFinder::parseNodeContent(): ";
        if(root.isMember(BTNode::AGENT_SPEC))
        {
            if(!mAgentSpec.parseJson(root[BTNode::AGENT_SPEC]))
            {
                Debug::warning(intro)("Could not parse \"")(BTNode::AGENT_SPEC)("\" attribute.").endl();
                return false;
            }
        }
        else
        {
            Debug::warning(intro)("missing \"")(BTNode::AGENT_SPEC)("\" attribute.").endl();
            return false;
        }

        // parse
        bool useDefault=true;
        if(root.isMember(BTFinder::LOCALIZATION_MODE))
        {
            Ogre::String lmode=root[BTFinder::LOCALIZATION_MODE].asCString();
            if(lmode==BTFinder::LOCALIZATION_MODE_STATIC)
            {
                mLocalizationMode=LM_STATIC;
                useDefault=false;
            }
            else if(lmode==BTFinder::LOCALIZATION_MODE_DYNAMIC)
            {
                mLocalizationMode=LM_DYNAMIC;
                useDefault=false;
            }
            else
            {
                Debug::error(intro)("invalid value \"")(lmode)("\" for key \"")(BTFinder::LOCALIZATION_MODE)("\". ");
            }
        }
        else
        {
            Debug::error(intro)("key \"")(BTFinder::LOCALIZATION_MODE)("\" not found. ");
        }
        if(useDefault)
        {
            Debug::error("Using default value \"")(BTFinder::LOCALIZATION_MODE_STATIC)("\"").endl();
            mLocalizationMode=LM_STATIC;
        }

        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
