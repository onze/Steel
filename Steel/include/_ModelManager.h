/*
 * Manager.h
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#ifndef _MODELMANAGER_H_
#define _MODELMANAGER_H_

#include <vector>
#include <list>

#include "steeltypes.h"
#include "ModelManager.h"

namespace Steel
{

//M is the managed model type
template<class M>
class _ModelManager : public ModelManager
{
public:
	_ModelManager();
	virtual ~_ModelManager();
	/**
	 * Returns a pointer to the model referenced by the given id, if the model is in use. Return NULL otherwise.
	 * Since model addresses can change without notice, models pointers are not supposed to be stored.
	 * ModelIds are meant for this, as seen in Thing.h/cpp.
	 */
	virtual M *at(ModelId id);
	/**
	 * clears every models from its memory.
	 */
	void clear();
	/**
	 * returns true if the given model is in use.
	 */
	bool isValid(ModelId id);
	/**
	 * decrements the given model, and deletes it if nobody uses it.
	 */
	void releaseModel(ModelId modelId);
protected:
	/**
	 * finds a free id to place the given model at. returns this ModelId.
	 */
	ModelId insertModel(M model);
	/**
	 * contains models.
	 */
	std::vector<M> mModels;
	typedef typename std::vector<M>::iterator ModelIterator;
	std::list<ModelId> mModelsFreeList;

};
}

#endif /* _MODELMANAGER_H_ */
