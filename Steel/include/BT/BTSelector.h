#ifndef STEEL_BTSELECTOR_H_
#define STEEL_BTSELECTOR_H_

#include "BT/btnodetypes.h"
#include "BTNode.h"

namespace Steel
{

    class BTSelector: public BTNode
    {
        public:
            BTSelector(BTShapeToken const &token);
            virtual ~BTSelector();

            // getters
            inline unsigned begin()
            {
                return mToken.begin;
            }

            inline unsigned end()
            {
                return mToken.end;
            }

            inline unsigned currentChildNodeIndex()
            {
                return mCurrentChildNodeIndex;
            }
        protected:
            BTState mState;
            BTShapeToken mToken;
            unsigned mCurrentChildNodeIndex;
    };

}

#endif /* STEEL_BTPRIORITYSELECTOR_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
