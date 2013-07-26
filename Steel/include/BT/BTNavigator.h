#ifndef STEEL_BTNAVIGATOR_H_
#define STEEL_BTNAVIGATOR_H_

#include <BT/btnodetypes.h>
#include <BT/BTNode.h>

namespace Steel
{

    class BTNavigator: public BTNode
    {
        public:
            inline static BTShapeTokenType tokenType()
            {
                return BTNavigatorToken;
            }

            BTNavigator(BTShapeToken const &token);
            virtual ~BTNavigator();
        protected:
            BTState mState;
            BTShapeToken mToken;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
