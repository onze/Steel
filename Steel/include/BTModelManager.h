/*
 * BTManager.h
 *
 *  Created on: Jan 18, 2012
 *      Author: onze
 */

#ifndef BTMODELMANAGER_H_
#define BTMODELMANAGER_H_

#include "steeltypes.h"
#include "_ModelManager.h"
#include "BTModel.h"

namespace Steel
{

class BTModelManager: public _ModelManager<BTModel>
{
private:
	BTModelManager();
public:
	BTModelManager(Ogre::String mPath);
	virtual ~BTModelManager();

protected:
	///base path to BT files
	Ogre::String mPath;
};

} /* namespace Steel */
#endif /* BTMODELMANAGER_H_ */
