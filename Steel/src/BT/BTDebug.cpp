/*
 *
 */

#include "BT/BTDebugPrinter.h"
#include <Debug.h>

namespace Steel
{
    const char *BTDebugPrinter::TEXT_ATTRIBUTE="text";

    BTDebugPrinter::BTDebugPrinter(BTShapeToken const &token):BTNode(token),
        mDebugText("no debug text set !")
    {

    }

    BTDebugPrinter::~BTDebugPrinter()
    {

    }

    void BTDebugPrinter::run(float timestep)
    {
        Debug::log("BTDebugPrinter<")(reinterpret_cast<unsigned long>(this))("> >> \"")(mDebugText)("\"").endl();
        mState=SUCCESS;
    }

    bool BTDebugPrinter::parseNodeContent(Json::Value &root)
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
