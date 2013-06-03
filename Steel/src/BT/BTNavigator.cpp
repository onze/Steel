#include <assert.h>
#include <iostream>

#include "BT/BTNavigator.h"

namespace Steel
{


    BTNavigator::BTNavigator(const Steel::BTShapeToken& token) :BTNode(token)
    {
    }

    BTNavigator::~BTNavigator()
    {
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
