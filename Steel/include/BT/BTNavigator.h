#ifndef STEEL_BTNAVIGATOR_H_
#define STEEL_BTNAVIGATOR_H_

#include "BT/btnodetypes.h"

namespace Steel
{

    class BTNavigator//:public BTNode
    {
        public:
            BTNavigator();
            virtual ~BTNavigator();
            BTState mState;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
