/*
 * Manager.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef MODELMANAGER_H_
#define MODELMANAGER_H_

#include <map>

#include "steeltypes.h"

namespace Steel
{

//M is the managed model type
template<class M>
class ModelManager
{
public:
	ModelManager();
	virtual ~ModelManager();
	/**
	 * Returns a pointer to the model referenced by the given id, if the model is in use. Return NULL otherwise.
	 * Since model addresses can change without notice, models pointers are not supposed to be stored.
	 * ModelIds are though.
	 */
	M *at(ModelId id);
	/**
	 * clears every models from its memory.
	 */
	void clear();
	/**
	 * returns true if the given model is in use.
	 */
	bool isValid(ModelId id);
protected:
	std::map<ModelId, M> mModels;
	typedef typename std::map<ModelId, M>::iterator ModelMapIterator;
};
}

#endif /* MODELMANAGER_H_ */
