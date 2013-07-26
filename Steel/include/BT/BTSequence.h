#ifndef STEEL_BTSEQUENCE_H
#define STEEL_BTSEQUENCE_H

#include <BT/BTNode.h>

namespace Steel
{

class BTSequence: public BTNode
{
public:
    inline static BTShapeTokenType tokenType()
    {
        return BTSequenceToken;
    }

    BTSequence(BTShapeToken const &token);
    virtual ~BTSequence();

    unsigned switchToNextChild();

    inline unsigned currentChildNodeIndex()
    {
        return mCurrentChildNodeIndex;
    }

// 	virtual void onStartRunning();
// 	virtual void onStopRunning();
protected:
    unsigned mCurrentChildNodeIndex;
};
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
