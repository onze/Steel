#ifndef STEEL_BTBUILDER_H_
#define STEEL_BTBUILDER_H_

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

#endif /* STEEL_BTBUILDER_H_ */
