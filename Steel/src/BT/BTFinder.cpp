
#include <assert.h>
#include <iostream>
#include <json/value.h>

#include "BT/BTFinder.h"
#include <Debug.h>

namespace Steel
{

    BTFinder::BTFinder(const Steel::BTShapeToken& token) : BTNode(token)
    {
    }

    BTFinder::~BTFinder()
    {
    }

    bool BTFinder::parseNodeContent(Json::Value &root)
    {

        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
