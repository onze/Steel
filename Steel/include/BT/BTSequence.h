#ifndef STEEL_BTSEQUENCE_H
#define STEEL_BTSEQUENCE_H

#include <BT/BTNode.h>

namespace Steel
{
    /**
     * A BTSequence runs its children one after the other until one of them returns FAILURE.
     * The sequence returns the same state as its last running child.
     * Its descriptor can contain parameters, see the following keys' docstrings for details:
     * - MAX_LOOPS_ATTRIBUTE
     */
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
            return BTShapeTokenType::BTSequenceToken;
        }

        BTSequence(BTShapeToken const &token);
        virtual ~BTSequence();

        BTStateIndex nodeSkippedTo();
        void childReturned(BTNode const *const node, BTNodeState state);
        void onParentNotified();

        inline BTStateIndex currentChildNodeIndex()
        {
            return mCurrentChildNodeIndex;
        }

        bool parseNodeContent(Json::Value &root);

    private:
        /// Loops currentChildNodeIndex() through children indices
        BTStateIndex switchToNextChild(const BTNode *const child);
        BTStateIndex mCurrentChildNodeIndex;
        unsigned mNLoops;
        unsigned mMaxLoops;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
