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
/**
 * Abstract class used as a common interface for the templated/specialized versions of modelManagers.
 * The way modelManager design is laid out is thusly:
 * - ModelManager "implements" the common interface, so that any model manager can be pointed at with the same pointer.
 * - _ModelManager<M> is a templated subclass of ModelManager, that implements common behavior.
 * - ModelManager<BlahModel> are subclasses that specialize in one single type of model, and implements details specifics to
 * the respective model they take care of.
 */
class ModelManager
{
public:
	virtual Model *at(ModelId id)=0;
	virtual bool incRef(ModelId id)=0;
	virtual void releaseModel(ModelId modelId)=0;
	virtual bool fromJson(Json::Value &object)=0;
	virtual void toJson(Json::Value &object)=0;
};

}
#endif
