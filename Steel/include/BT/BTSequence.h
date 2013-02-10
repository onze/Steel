#ifndef STEEL_BTSEQUENCE_H_
#define STEEL_BTSEQUENCE_H_

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
#endif /* STEEL_BTSEQUENCE_H_ */
