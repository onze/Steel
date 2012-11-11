/*
 * PrioritySelector.h
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#ifndef BTSELECTOR_H_
#define BTSELECTOR_H_

#include "BT/BTNode.h"

namespace Steel
{

class BTSelector: public BTNode
{
public:
	BTSelector(BTNode *parent = NULL);
	virtual ~BTSelector();
	virtual void onStartRunning();
	virtual void onStopRunning();
	virtual BTState run();
protected:
	std::list<BTNode *>::iterator it;
};

}

#endif /* BTPRIORITYSELECTOR_H_ */
