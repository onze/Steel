/*
 * BTDecorator.cpp
 *
 *  Created on: 2011-07-21
 *      Author: onze
 */

#include <stdexcept>

#include "BT/BTDecorator.h"

namespace Steel
{

BTDecorator::BTDecorator(BTNode *parent) :
	BTNode(parent)
{

}

BTDecorator::~BTDecorator()
{
}

void BTDecorator::attachChild(BTNode *child)
{
	BTNode::attachChild(child);
	if (mChildren.size() > 1)
		throw std::runtime_error("ERROR in BTDecorator::attachChild(): can\'t accept more than one child.");
}

BTNode::BTState BTDecorator::run()
{
	if (mChildren.size())
		return mChildren.front()->run();
	else
		return FAILURE;

}

}
