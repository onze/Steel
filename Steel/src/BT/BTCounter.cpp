/*
 * BTCounter.cpp
 *
 *  Created on: 2011-07-21
 *      Author: onze
 */

#include <assert.h>
#include <iostream>

#include "BT/BTCounter.h"

namespace Steel
{


BTCounter::BTCounter(BTNode *parent, int maxCount) :
	BTDecorator(parent), mMaxCount(maxCount)
{
	mCount = 0;
	assert(maxCount>=0);
}

BTCounter::~BTCounter()
{
}

void BTCounter::onStartRunning()
{
}

void BTCounter::onStopRunning()
{

}

BTNode::BTState BTCounter::run()
{
	std::cout << "<BTCounter " << mDbgName << ">::run() mCount:" << mCount << std::endl;
	if (mCount == mMaxCount)
	{
		mCount = 0;
		return FAILURE;
	}
	else
	{
		++mCount;
		return mChildren.front()->run();
	}

}

}
