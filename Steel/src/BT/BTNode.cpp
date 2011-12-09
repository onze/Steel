/*
 * Node.cpp
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */
#include <iostream>

#include "BT/BTNode.h"

namespace Steel
{

BTNode::BTNode(BTNode *parent) :
	mState(BTNode::READY), mParent(parent), mChildren(std::list<BTNode *>()), mDbgName("unnamedNode")
{
	if (mParent)
		mParent->attachChild(this);
}

BTNode::~BTNode()
{
	while (mChildren.size())
	{
		delete mChildren.front();
		mChildren.pop_front();
	}
}

void BTNode::attachChild(BTNode *child)
{
	//	std::cout<<mDbgName<<".attachChild("<<child->mDbgName<<")"<<std::endl;
	mChildren.push_back(child);
}

}
