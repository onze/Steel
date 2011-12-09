/*
 * BTCounter.h
 *
 *  Created on: 2011-07-21
 *      Author: onze
 */

#ifndef BTCOUNTER_H_
#define BTCOUNTER_H_

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

#endif /* BTCOUNTER_H_ */
