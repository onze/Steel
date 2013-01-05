/*
 * Agent.cpp
 *
 *  Created on: 2011-06-15
 *      Author: onze
 */

#include <exception>

#include <json/json.h>

#include "Debug.h"
#include "Agent.h"
#include "_ModelManager.h"
#include "OgreModelManager.h"
#include "Level.h"
#include <tools/StringUtils.h>

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
//	Debug::log("Agent<")(mId)(">::fromJson():").endl()(value.toStyledString()).endl();
        int nModels=0;
        for (ModelType mt_it = (ModelType) ((int) MT_FIRST + 1); mt_it != MT_LAST; mt_it = (ModelType) ((int) mt_it + 1))
        {
            Json::Value modelTypeValue = value[modelTypesAsString[mt_it]];
            // possibly no model of this type
            if(modelTypeValue.isNull())
                continue;
            ModelId modelId = (ModelId) Ogre::StringConverter::parseUnsignedLong(modelTypeValue.asString());
            linkToModel(mt_it, modelId);
            ++nModels;
        }
        if(nModels==0)
        {
            Debug::error("Agent::fromJson(): agent ")(mId)("linked with 0 models ? json string:").endl()(value).endl();;
            return false;
        }
        return true;
    }

    bool Agent::linkToModel(ModelType modelType, ModelId modelId)
    {
//	Debug::log("Agent<")(mId)(">::linkToModel(ModelType ")(modelTypesAsString[modelType]);
//	Debug::log(", ModelId ")(modelId)(")").endl();
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

    void Agent::setSelected(bool selected)
    {
        OgreModel *om = ogreModel();
        if (om != NULL)
            om->setSelected(selected);
    }

    Json::Value Agent::toJson()
    {
//	Debug::log("Agent<")(mId)("> with ")(mModelIds.size())(" modelTypes:").endl();
        Json::Value root;
        // add the agent's model ids to its json representation
        for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
        {
            ModelType mt = (*it).first;
//		Debug::log("model type:")(modelTypesAsString[mt]).endl();
            ModelId mid = (*it).second;
            root[modelTypesAsString[mt]] = StringUtils::toJson(mid);
        }
        return root;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
