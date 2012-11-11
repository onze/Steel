/*
 * BTBuilder.h
 *
 *  Created on: 2011-07-18
 *      Author: onze
 */

#ifndef BTBUILDER_H_
#define BTBUILDER_H_

#include <string>
#include "BT/BTNode.h"

namespace Steel
{

class BTBuilder
{
public:
	/**
	 * This class takes the path to an xml file describing a behavioral tree, or the file content as a string,
	 * builds and returns the corresponding tree.
	 */
	BTBuilder();
	virtual ~BTBuilder();
protected:
};

}

#endif /* BTBUILDER_H_ */
