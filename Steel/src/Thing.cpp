/*
 * Thing.cpp
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#include <exception>

#include "Debug.h"
#include "Thing.h"
#include "_ModelManager.h"
#include "OgreModelManager.h"
#include "Level.h"

namespace Steel
{

ThingId Thing::sNextId = 1;

Thing::Thing(Level *level) :
	mId(Thing::getNextId()), mLevel(level)
{
	mModelIds = std::map<ModelType, ModelId>();
}

Thing::~Thing()
{
	for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
		mLevel->modelManager(it->first)->releaseModel(it->second);
}

Thing::Thing(const Thing &t) :
	mId(t.mId), mLevel(t.mLevel), mModelIds(t.mModelIds)
{
	for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
	{
		mLevel->modelManager(it->first)->at(it->second)->incRef();
	}
}

Thing &Thing::operator=(const Thing &t)
{
	mId = t.mId;
	mLevel = t.mLevel;
	mModelIds = t.mModelIds;
	for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
	{
		mLevel->modelManager(it->first)->at(it->second)->incRef();
	}
	return *this;
}

void Thing::addModel(ModelType modelType, ModelId modelId)
{
	Debug::log("Thing::addModel(ModelType ")(modelType)(", ModelId ")(modelId)(")").endl();
	mModelIds.insert(std::pair<ModelType, ModelId>(modelType, modelId));
	//cannot inref here if the model is newly created because its refcount is still 0 and then says it is free.
//	mLevel->modelManager(modelType)->at(modelId)->incRef();
}

Model *Thing::model(ModelType modelType)
{
	ModelId id=modelId(modelType);

	if (id == INVALID_ID)
		return NULL;

	switch (modelType)
	{
		case MT_OGRE:
			return mLevel->ogreModelMan()->at(id);
			break;
		default:
			Debug::error("in Thing::model(): unknown modelType: ")(modelType)(" for id: ")(id).endl();
			throw std::runtime_error("in Thing::model(): unknown modelType");
	}
}

ModelId Thing::modelId(ModelType modelType)
{
	std::map<ModelType, ModelId>::iterator it = mModelIds.find(modelType);
	return (it == mModelIds.end() ? INVALID_ID : it->second);
}

}
