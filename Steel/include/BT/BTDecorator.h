#ifndef STEEL_BTDECORATOR_H_
#define STEEL_BTDECORATOR_H_

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

#endif /* STEEL_BTDECORATOR_H_ */
