/*
 * BTAction.cpp
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#include <iostream>
#include "BT/BTAction.h"

namespace Steel
{
BTAction::BTAction(BTNode *parent, std::string action) :
	BTNode(parent), mAction(action)
{

}

BTAction::~BTAction()
{
}

BTNode::BTState BTAction::run()
{
	std::cout << mAction << std::endl;
	return BTNode::SUCCESS;
}
}
