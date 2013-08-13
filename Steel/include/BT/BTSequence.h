#ifndef STEEL_BTSEQUENCE_H
#define STEEL_BTSEQUENCE_H

#include <BT/BTNode.h>

namespace Steel
{

    class BTSequence: public BTNode
    {
        private:
            /**
             * Max number of times a loop through children is initiated.
             * Defaults to 0, meaning an undefinitely.
             * If set, after the limit is reached, freezes its state to its last value.
             */
            static const char *MAX_LOOPS_ATTRIBUTE;
            
        public:
            inline static BTShapeTokenType tokenType()
            {
                return BTSequenceToken;
            }

            BTSequence(BTShapeToken const &token);
            virtual ~BTSequence();

            BTStateIndex nodeSkippedTo();
            void childReturned(BTState state);
            void onParentNotified();

            inline BTStateIndex currentChildNodeIndex()
            {
                return mCurrentChildNodeIndex;
            }

            bool parseNodeContent(Json::Value &root);

        private:
            BTStateIndex switchToNextChild();
            BTStateIndex mCurrentChildNodeIndex;
            unsigned mNLoops;
            unsigned mMaxLoops;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
