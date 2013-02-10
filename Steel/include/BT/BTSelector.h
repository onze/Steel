#ifndef STEEL_BTSELECTOR_H_
#define STEEL_BTSELECTOR_H_

#include "BT/BTNode.h"

namespace Steel
{

    class BTSelector: public BTNode
    {
        public:
            BTSelector(BTNode *parent = NULL);
            virtual ~BTSelector();
            virtual void onStartRunning();
            virtual void onStopRunning();
            virtual BTState run();
        protected:
            std::list<BTNode *>::iterator it;
    };

}

#endif /* STEEL_BTPRIORITYSELECTOR_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
