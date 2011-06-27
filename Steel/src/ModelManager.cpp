/*
 * Manager.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "ModelManager.h"

namespace Steel
{

template<class M>
ModelManager<M>::ModelManager()
{
	mModels = std::map<ModelId, M>();

}

template<class M>
ModelManager<M>::~ModelManager()
{
	// TODO Auto-generated destructor stub
}

template<class M>
M *ModelManager<M>::at(ModelId id)
{
	if (!isValid(id))
		return NULL;

	return &(mModels.find(id)->second);
}

template<class M>
void ModelManager<M>::clear()
{
	mModels.clear();
}

template<class M>
bool ModelManager<M>::isValid(ModelId id)
{

	ModelMapIterator it = mModels.find(id);
	return (mModels.find(id) != mModels.end() && !mModels.find(id)->second.isFree()) ? true : false;
}

}
