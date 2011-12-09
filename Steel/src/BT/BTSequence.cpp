/*
 * BTSequence.cpp
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */
#include <iostream>
#include "BT/BTSequence.h"
#include <assert.h>

namespace Steel
{

BTSequence::BTSequence(BTNode *parent) :
	BTNode(parent)
{

}

BTSequence::~BTSequence()
{
}

void BTSequence::onStartRunning()
{
	std::cout << "BTSequence::onStartRunning()" << std::endl;
	it = mChildren.begin();
	mState = RUNNING;
}

void BTSequence::onStopRunning()
{
	std::cout << "BTSequence::onStopRunning()" << std::endl;
	mState = READY;
}

BTNode::BTState BTSequence::run()
{
	if (mState == READY)
		onStartRunning();
	std::cout << "BTSequence::run()" << std::endl;

	BTState state;
	while (it != mChildren.end())
	{
		state = (*it)->run();
		switch (state)
		{
			case RUNNING:
				return RUNNING;
			case READY:
			case SUCCESS:
				++it;
				continue;
			case FAILURE:
				mState = READY;
				return FAILURE;
			case ERROR:
				std::cout << "ERROR !" << std::endl;
				break;
		}
	}
	onStopRunning();
	if (it == mChildren.end())
		return SUCCESS;
	std::cout << "in BTSequence::run(): bad code path: not all children have been consumed, "
			<< "yet we're not looping on them" << std::endl;
	assert(false);
	return FAILURE;
}

}
