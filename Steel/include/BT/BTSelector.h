#ifndef STEEL_BTSELECTOR_H_
#define STEEL_BTSELECTOR_H_

#include "BT/btnodetypes.h"

namespace Steel
{

    class BTSelector
    {
        public:
            BTSelector();
            virtual ~BTSelector();
            BTState mState;
    };

}

#endif /* STEEL_BTPRIORITYSELECTOR_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
