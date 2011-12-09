/*
 * PrioritySelector.cpp
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#include <iostream>

#include "BT/BTSelector.h"

namespace Steel
{

BTSelector::BTSelector(BTNode *parent):BTNode(parent)
{
}

BTSelector::~BTSelector()
{
}


void BTSelector::onStartRunning()
{
	std::cout<<"BTSelector::onStartRunning()"<<std::endl;
	it=mChildren.begin();
	mState=RUNNING;
}

void BTSelector::onStopRunning()
{
	std::cout<<"BTSelector::onStopRunning()"<<std::endl;
	mState=READY;
}

BTNode::BTState BTSelector::run()
{
	if(mState==READY)
		onStartRunning();
	std::cout<<"BTSelector::run()"<<std::endl;

	BTState state;
	while(it!=mChildren.end())
	{
		state=(*it)->run();
		switch(state)
		{
			case RUNNING:
				return RUNNING;
			case READY:
			case SUCCESS:
				mState=READY;
				return SUCCESS;
			case FAILURE:
				++it;
				break;
			case ERROR:
				std::cout << "in BTSelector::run() child returned ERROR !" << std::endl;
				++it;
				break;
		}
	}
	if(it==mChildren.end())
		mState=FAILURE;
	onStopRunning();
	return mState;
}

}
