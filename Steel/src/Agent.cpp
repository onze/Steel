/*
 * Agent.cpp
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#include <exception>

#include "Debug.h"
#include "Agent.h"
#include "_ModelManager.h"
#include "OgreModelManager.h"
#include "Level.h"

namespace Steel
{

AgentId Agent::sNextId = 0;

Agent::Agent(Level *level) :
		mId(Agent::getNextId()), mLevel(level)
{
	mModelIds = std::map<ModelType, ModelId>();
}

Agent::~Agent()
{
	for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
		mLevel->modelManager(it->first)->releaseModel(it->second);
}

Agent::Agent(const Agent &t) :
		mId(t.mId), mLevel(t.mLevel), mModelIds(t.mModelIds)
{
	for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
	{
		mLevel->modelManager(it->first)->at(it->second)->incRef();
	}
}

Agent &Agent::operator=(const Agent &t)
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

bool Agent::fromJson(Json::Value &value)
{
	Debug::log("Agent<")(mId)(">::fromJson():").endl()(value.toStyledString()).endl();
	for (ModelType mt_it = (ModelType) ((int) MT_FIRST + 1); mt_it != MT_LAST; mt_it = (ModelType) ((int) mt_it + 1))
	{
		Json::Value modelTypeValue = value[modelTypesAsString[mt_it]];
		assert(!modelTypeValue.isNull());
		ModelId modelId = (ModelId) Ogre::StringConverter::parseLong(modelTypeValue.asString());
		linkToModel(mt_it, modelId);
	}
	return true;
}

bool Agent::linkToModel(ModelType modelType, ModelId modelId)
{
	Debug::log("Agent<")(mId)(">::linkToModel(ModelType ")(modelTypesAsString[modelType]);
	Debug::log(", ModelId ")(modelId)(")").endl();
	mModelIds.insert(std::pair<ModelType, ModelId>(modelType, modelId));
	return mLevel->modelManager(modelType)->incRef(modelId);
}

Model *Agent::model(ModelType modelType)
{
	ModelId id = modelId(modelType);

	if (id == INVALID_ID)
		return NULL;

	switch (modelType)
	{
		case MT_OGRE:
			return mLevel->ogreModelMan()->at(id);
			break;
		default:
			Debug::error("in Agent::model(): unknown modelType: ")(modelType)(" for id: ")(id).endl();
			throw std::runtime_error("in Agent::model(): unknown modelType");
	}
}

ModelId Agent::modelId(ModelType modelType)
{
	std::map<ModelType, ModelId>::iterator it = mModelIds.find(modelType);
	return (it == mModelIds.end() ? INVALID_ID : it->second);
}

}
