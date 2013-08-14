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
#include <tools/JsonUtils.h>

namespace Steel
{

    AgentId Agent::sNextId = 0;

    Agent::Agent(Level *level)
        : mId(Agent::getNextId()), mLevel(level),mIsSelected(false)
    {
        mModelIds = std::map<ModelType, ModelId>();
    }

    Agent::~Agent()
    {
        for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
            mLevel->modelManager(it->first)->decRef(it->second);
        mModelIds.clear();
    }

    Agent::Agent(const Agent &o)
        : mId(o.mId), mLevel(o.mLevel), mModelIds(o.mModelIds),mIsSelected(o.mIsSelected)
    {
        for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
        {
            mLevel->modelManager(it->first)->at(it->second)->incRef();
        }
    }

    Agent &Agent::operator=(const Agent &o)
    {
        mId = o.mId;
        mLevel = o.mLevel;
        mModelIds = o.mModelIds;
        mIsSelected=o.mIsSelected;
        for (std::map<ModelType, ModelId>::iterator it = mModelIds.begin(); it != mModelIds.end(); ++it)
        {
            mLevel->modelManager(it->first)->at(it->second)->incRef();
        }
        return *this;
    }

    bool Agent::fromJson(Json::Value &value)
    {
//	Debug::log("Agent<")(mId)(">::fromJson():").endl()(value.toStyledString()).endl();
        int nModels = 0, nExpected = 0;
        for (ModelType mt_it = (ModelType) ((int) MT_FIRST + 1); mt_it != MT_LAST; mt_it = (ModelType) ((int) mt_it + 1))
        {
            Ogre::String mtName = modelTypesAsString[mt_it];
            Json::Value mTypeValue = value[mtName];
            // possibly no model of this type
            if (mTypeValue.isNull())
                continue;
            ++nExpected;
            ModelId modelId = (ModelId) Ogre::StringConverter::parseUnsignedLong(mTypeValue.asString());
            if (!linkToModel(mt_it, modelId))
            {
                Debug::error("Agent::fromJson(): agent ")(mId);
                Debug::error(" would not link with model<>")(mtName)("> ")(mId)(". Skipping.").endl();
                continue;
            }
            ++nModels;
        }
        if (nModels != nExpected)
        {
            Debug::warning("Agent::fromJson(): agent ")(mId)(" linked with (")(nModels)(" models, ");
            Debug::warning(nExpected)(" were expected. Json string:").endl()(value).endl();
        }
        return true;
    }

    bool Agent::linkToModel(ModelType mType, ModelId modelId)
    {
        Ogre::String intro = "Agent::linkToModel(type=" + modelTypesAsString[mType] + ", id="
                             + Ogre::StringConverter::toString(modelId) + "): ";
        if (!mLevel->modelManager(mType)->isValid(modelId))
        {
            Debug::error(intro)("model ")(modelId)(" is not valid. Aborting.").endl();
            return false;
        }

        //result.first==iterator placed at location, result.second==successful insertion flag
        auto result = mModelIds.insert(std::pair<ModelType, ModelId>(mType, modelId));
        if (!result.second)
        {
            Debug::error(intro)("Could not insert model (overwrites are not allowed). Aborting.").endl();
            return false;
        }

        mLevel->modelManager(mType)->incRef(modelId);
        if(!mLevel->modelManager(mType)->onAgentLinkedToModel(this, modelId))
        {
            unlinkFromModel(mType);
            Debug::error(intro)("Agent<->model linking aborted.").endl();
            return false;
        }
        return true;
    }

    void Agent::unlinkFromModel(ModelType mType)
    {
        auto it = mModelIds.find(mType);
        if (it != mModelIds.end())
        {
            switch (mType)
            {
                case MT_OGRE:
                    unlinkFromModel(MT_PHYSICS);
                    break;
                default:
                    break;
            }
            // general case
            mLevel->modelManager(mType)->decRef((*it).second);
            mModelIds.erase(it);
        }
    }

    Model *Agent::model(ModelType mType) const
    {
        ModelId id = modelId(mType);

        if (id == INVALID_ID)
            return NULL;

        return mLevel->modelManager(mType)->at(id);
    }

    ModelId Agent::modelId(ModelType mType) const
    {
        auto it = mModelIds.find(mType);
        return (it == mModelIds.end() ? INVALID_ID : it->second);
    }

    void Agent::setSelected(bool selected)
    {
        OgreModel *om = ogreModel();
        if (NULL!=om)
            om->setSelected(selected);

        PhysicsModel *pm = physicsModel();
        if(NULL!=pm)
            pm->setSelected(selected);

        mIsSelected=selected;
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
            root[modelTypesAsString[mt]] = JsonUtils::toJson(mid);
        }
        return root;
    }

    Ogre::Vector3 Agent::position() const
    {
        auto omodel = ogreModel();
        return NULL == omodel ? Ogre::Vector3::ZERO : omodel->position();
    }

    Ogre::Quaternion Agent::rotation() const
    {
        auto omodel = ogreModel();
        return NULL == omodel ? Ogre::Quaternion::ZERO : omodel->rotation();
    }

    Ogre::Vector3 Agent::scale() const
    {
        auto omodel = ogreModel();
        return NULL == omodel ? Ogre::Vector3::ZERO : omodel->scale();
    }

    void Agent::move(const Ogre::Vector3 &dpos)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->move(dpos);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->move(dpos);
    }

    void Agent::rotate(const Ogre::Vector3& rot)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->rotate(rot);
    }

    void Agent::rotate(const Ogre::Quaternion &q)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->rotate(q);
    }

    void Agent::rescale(const Ogre::Vector3 &sca)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->rescale(sca);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->rescale(sca);
    }

    void Agent::setPosition(const Ogre::Vector3 &pos)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->setPosition(pos);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->setPosition(pos);
    }

    void Agent::setRotation(const Ogre::Quaternion &rot)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->setRotation(rot);
    }

    void Agent::setScale(const Ogre::Vector3 &sca)
    {
        auto omodel = ogreModel();
        if (NULL != omodel)
            omodel->setScale(sca);
        auto pmodel = physicsModel();
        if (NULL != pmodel)
            pmodel->setScale(sca);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
