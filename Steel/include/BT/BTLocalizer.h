#ifndef STEEL_BTLOCALIZER_H_
#define STEEL_BTLOCALIZER_H_

//#include "BT/BTNode.h"

#include "BT/btnodetypes.h"

namespace Steel
{

    class BTLocalizer//: public BTNode
    {
        public:
            BTLocalizer();
            virtual ~BTLocalizer();
            BTState mState;
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
