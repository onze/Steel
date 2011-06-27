/*
 * Thing.cpp
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#include <exception>

#include "Thing.h"
#include "ModelManager.h"
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
}

Thing::Thing(const Thing &t):mId(t.mId),mLevel(t.mLevel),mModelIds(t.mModelIds)
{

}

Thing &Thing::operator=(const Thing &t)
{
	mId=t.mId;
	mLevel=t.mLevel;
	mModelIds=t.mModelIds;
	return *this;
}

void Thing::addModel(ModelType modelType, ModelId modelId)
{
	mModelIds.insert(std::pair<ModelType, ModelId>(modelType, modelId));
}

OgreModel *Thing::ogreModel()
{
	return (OgreModel *) model(MT_OGRE);
}

Model *Thing::model(ModelType modelType)
{
	std::map<ModelType, ModelId>::iterator it = mModelIds.find(modelType);

	if (it == mModelIds.end() || it->second == 0L)
		return NULL;

	switch (modelType)
	{
		case MT_OGRE:
			return mLevel->ogreModelMan()->at(it->second);
			break;
		default:
			throw std::runtime_error("unknown modelType");
	}
}

}
