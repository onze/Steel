/*
 * Manager.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "_ModelManager.h"
#include "Debug.h"

namespace Steel
{

template<class M>
_ModelManager<M>::_ModelManager() :
	ModelManager()
{
	mModels = std::vector<M>();
	mModelsFreeList = std::list<ModelId>();
}

template<class M>
_ModelManager<M>::~_ModelManager()
{
	clear();
}

template<class M>
M *_ModelManager<M>::at(ModelId id)
{
	if (!isValid(id))
		return NULL;

	return &(mModels[id]);
}

template<class M>
void _ModelManager<M>::clear()
{
	mModels.clear();
	mModelsFreeList.clear();
}

template<class M>
ModelId _ModelManager<M>::insertModel(M model)
{
	ModelId id= 0L;
	if(mModelsFreeList.size()>0)
	{
		id=mModelsFreeList.front();
		mModelsFreeList.pop_front();
		mModels[id]=model;
	}
	else
	{
		id=(ModelId) mModels.size();
		mModels.push_back(model);
	}
	mModels[id].incRef();
	return id;
}

template<class M>
bool _ModelManager<M>::isValid(ModelId id)
{
	return id >= 0 && id < mModels.size() && !mModels[id].isFree();
}

template<class M>
void _ModelManager<M>::releaseModel(ModelId id)
{
	Debug::log("_ModelManager<M>::releaseModel(")(id)(")");
	if (!isValid(id))
		return;
	M *m = at(id);
	Debug::log(" refCount going from ")(m->refCount());
	m->decRef();
	Debug::log(" to ")(m->refCount()).endl();
	//TODO: use a heap (priority queue)
	if (m->isFree())
		mModelsFreeList.push_front(id);
}

}
