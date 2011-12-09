/*
 * Manager.h
 *
 *  Created on: 2011-07-02
 *      Author: onze
 */

#ifndef MODELMANAGER_H_
#define MODELMANAGER_H_

#include "steeltypes.h"
#include "Model.h"

namespace Steel
{

class ModelManager
{
public:
	virtual Model *at(ModelId id)=0;
	virtual void releaseModel(ModelId modelId)=0;
	virtual void toJson(Json::Value &object)=0;
};

}
#endif
