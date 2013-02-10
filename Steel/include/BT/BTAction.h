#ifndef STEEL_BTACTION_H_
#define STEEL_BTACTION_H_

#include <string>
#include "BTNode.h"

namespace Steel
{

class BTAction: public BTNode
{
public:
	BTAction(BTNode *parent, std::string action);
	virtual ~BTAction();
	virtual BTState run();
protected:
	std::string mAction;
};

}

#endif /* STEEL_BTACTION_H_ */
