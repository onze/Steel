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
            Json::Value mTypeValue = value[modelTypesAsString[mt_it]];
            // possibly no model of this type
            if(mTypeValue.isNull())
                continue;
            ModelId modelId = (ModelId) Ogre::StringConverter::parseUnsignedLong(mTypeValue.asString());
            linkToModel(mt_it, modelId);
            ++nModels;
        }
        if(nModels==0)
        {
            Debug::error("Agent::fromJson(): agent ")(mId);
            Debug::error("linked with 0 models ? json string:").endl()(value).endl();
            return false;
        }
        return true;
    }

    bool Agent::linkToModel(ModelType mType, ModelId modelId)
    {
        Ogre::String intro="Agent::linkToModel(type="+modelTypesAsString[mType]+", id="+Ogre::StringConverter::toString(modelId)+"): ";
        if (modelId == INVALID_ID)
        {
            Debug::error(intro)("Agent ")(mId)("'s model id is invalid. Aborting.").endl();
            return false;
        }

        //result.first==iterator placed at location, result.second==successful insertion flag
        auto result=mModelIds.insert(std::pair<ModelType, ModelId>(mType, modelId));
        if(!result.second)
        {
            Debug::error(intro)("Could not insert model (overwrites are not allowed). Aborting.").endl();
            return false;
        }

        mLevel->modelManager(mType)->incRef(modelId);
        return true;
    }

    void Agent::unlinkFromModel(ModelType mType)
    {
        auto it=mModelIds.find(mType);
        if(it!=mModelIds.end())
        {
            mLevel->modelManager(mType)->decRef((*it).second);
            mModelIds.erase(it);
        }
    }

    Model *Agent::model(ModelType mType)
    {
        ModelId id = modelId(mType);

        if (id == INVALID_ID)
            return NULL;

        return mLevel->modelManager(mType)->at(id);
    }

    ModelId Agent::modelId(ModelType mType)
    {
        std::map<ModelType, ModelId>::iterator it = mModelIds.find(mType);
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
//	Debug::log("Agent<")(mId)("> with ")(mModelIds.size())(" mTypes:").endl();
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
