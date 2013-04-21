#ifndef STEEL_BTSEQUENCE_H_
#define STEEL_BTSEQUENCE_H_

// #include "BT/BTNode.h"
#include "BT/btnodetypes.h"

namespace Steel
{

    class BTSequence
    {
        public:
            BTSequence();
            virtual ~BTSequence();
            BTState mState;
// 	virtual void onStartRunning();
// 	virtual void onStopRunning();
        protected:
//             std::list<BTNode *>::iterator it;
    };

}
#endif /* STEEL_BTSEQUENCE_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
