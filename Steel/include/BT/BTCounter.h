#ifndef STEEL_BTCOUNTER_H_
#define STEEL_BTCOUNTER_H_

#include "BT/BTNode.h"
#include "BT/BTDecorator.h"

namespace Steel
{

class BTCounter: public BTDecorator
{
public:
	BTCounter(BTNode *parent, int maxCount = 0);
	virtual ~BTCounter();
	virtual void onStartRunning();
	virtual void onStopRunning();
	virtual BTState run();
protected:
	int mCount, mMaxCount;
};

}

#endif /* STEEL_BTCOUNTER_H_ */
