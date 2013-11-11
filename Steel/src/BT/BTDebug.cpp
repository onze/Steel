/*
 *
 */

#include "BT/BTDebug.h"
#include <Debug.h>

namespace Steel
{
    const char *BTDebug::TEXT_ATTRIBUTE="text";

    BTDebug::BTDebug(BTShapeToken const &token):BTNode(token),
        mDebugText("no debug text set !")
    {

    }

    BTDebug::~BTDebug()
    {

    }

    void BTDebug::run(float timestep)
    {
        Debug::log("BTDebug<")(reinterpret_cast<unsigned long>(this))("> >> \"")(mDebugText)("\"").endl();
        mState=BTNodeState::SUCCESS;
    }

    bool BTDebug::parseNodeContent(Json::Value &root)
    {
        if(root.isMember(TEXT_ATTRIBUTE))
        {
            Json::Value value=root[TEXT_ATTRIBUTE];
            if(value.isString())
            {
                mDebugText=value.asString();
            }
        }
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
