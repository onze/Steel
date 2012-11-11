/*
 * BTAction.h
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#ifndef BTACTION_H_
#define BTACTION_H_

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

#endif /* BTACTION_H_ */
