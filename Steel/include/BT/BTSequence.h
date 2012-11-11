/*
 * BTSequence.h
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#ifndef BTSEQUENCE_H_
#define BTSEQUENCE_H_

#include "BT/BTNode.h"

namespace Steel
{

class BTSequence:public BTNode
{
public:
	BTSequence(BTNode *parent = NULL);
	virtual ~BTSequence();
	virtual BTState run();
	virtual void onStartRunning();
	virtual void onStopRunning();
protected:
	std::list<BTNode *>::iterator it;
};

}
#endif /* BTSEQUENCE_H_ */
