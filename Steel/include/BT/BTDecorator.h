/*
 * BTDecorator.h
 *
 *  Created on: 2011-07-21
 *      Author: onze
 */

#ifndef BTDECORATOR_H_
#define BTDECORATOR_H_

#include "BT/BTNode.h"

namespace Steel
{

class BTDecorator: public BTNode
{
public:
	BTDecorator(BTNode *parent);
	virtual ~BTDecorator();
	///decorated to throw an exception if there's more than one single child attached.
	virtual void attachChild(BTNode *child);
	virtual BTState run();
};

}

#endif /* BTDECORATOR_H_ */
