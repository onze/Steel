#include "models/LocationModelManager.h"
#include <tools/DynamicLines.h>
#include <tools/JsonUtils.h>
#include <models/Agent.h>
#include <Level.h>
#include <models/AgentManager.h>
#include <Debug.h>

namespace Steel
{
    const char *LocationModelManager::PATH_ROOTS_ATTRIBUTE = "pathRoots";

    LocationModelManager::LocationModelManager(Level *level):
        _ModelManager<LocationModel>(level)
    {
        mDebugLines.clear();
    }

    LocationModelManager::~LocationModelManager()
    {
        for(auto it : mDebugLines)
        {
            mLevel->levelRoot()->detachObject(it.second);
            delete it.second;
        }

        mDebugLines.clear();
    }

    std::vector<ModelId> LocationModelManager::fromJson(Json::Value &root)
    {
        mPathsRoots = JsonUtils::asStringUnsignedLongMap(root[LocationModelManager::PATH_ROOTS_ATTRIBUTE]);
        return _ModelManager<LocationModel>::fromJson(root[ModelManager::MODELS_ATTRIBUTES]);
    }

    ModelId LocationModelManager::newModel()
    {
        ModelId mid = INVALID_ID;
        return newModel(mid);
    }

    ModelId LocationModelManager::newModel(ModelId &mid)
    {
        allocateModel(mid);
        mModels[mid].init(this);
        return mid;
    }

    bool LocationModelManager::fromSingleJson(Json::Value &model, ModelId &id)
    {
        id = newModel(id);

        if(INVALID_ID == id)
            return false;

        if(!mModels[id].fromJson(model, this))
        {
            deallocateModel(id);
            id = INVALID_ID;
            return false;
        }

        return true;
    }

    void LocationModelManager::toJson(Json::Value &root, std::list<ModelId> const &modelIds)
    {
        if(mPathsRoots.size())
            root[LocationModelManager::PATH_ROOTS_ATTRIBUTE] = JsonUtils::toJson(mPathsRoots);

        Json::Value models;
        _ModelManager<LocationModel>::toJson(models, modelIds);

        if(!models.isNull())
            root[ModelManager::MODELS_ATTRIBUTES] = models;
    }

    bool LocationModelManager::linkAgents(AgentId srcAgentId, AgentId dstAgentId)
    {
        Agent *src = mLevel->agentMan()->getAgent(srcAgentId);
        Agent *dst = mLevel->agentMan()->getAgent(dstAgentId);

        if(nullptr == src || nullptr == dst)
        {
            Debug::error("LocationModelManager::linkAgents(): an agent id is invalid: src=")
            (srcAgentId)(", dst:")(dstAgentId)(". Aborting.").endl();
            return false;
        }

        // allocate new models for agents if necessary
        if(INVALID_ID == src->locationModelId())
            src->linkToModel(ModelType::LOCATION, newModel());

        if(INVALID_ID == dst->locationModelId())
            dst->linkToModel(ModelType::LOCATION, newModel());

        return linkLocations(src->locationModelId(), dst->locationModelId());
    }

    bool LocationModelManager::linkLocations(ModelId srcId, ModelId dstId)
    {
        LocationModel *src = at(srcId), *dst = at(dstId);

        if(nullptr == src)
        {
            Debug::error(STEEL_METH_INTRO, "source model ")(srcId)(" is not valid.").endl();
            return false;
        }

        if(nullptr == dst)
        {
            Debug::error(STEEL_METH_INTRO, "destination model ")(dstId)(" is not valid.").endl();
            return false;
        }

        if(srcId == dstId)
        {
            Debug::error("LocationModelManager::linkLocations(): src is dst. Aborting.").endl();
            return false;
        }

        if(!src->addDestination(dst->attachedAgent()))
        {
            Debug::error(STEEL_METH_INTRO, "source model ")(srcId)(" cannot take model ")
            (dstId)("'s agent ")(dst->attachedAgent())(") as a new destination. Aborting").endl();
            return false;
        }

        if(!dst->addSource(src->attachedAgent()))
        {
            Debug::error(STEEL_METH_INTRO, "destination model ")(dstId)(" cannot take model ")
            (srcId)("'s agent ")(src->attachedAgent())(") as a new source. Aborting").endl();
            src->removeDestination(dst->attachedAgent());
            return false;
        }

        updateDebugLine(makeKey(srcId, dstId));
        updateDebugLine(makeKey(dstId, srcId));
        return true;
    }

    bool LocationModelManager::unlinkAgents(AgentId srcAgentId, AgentId dstAgentId)
    {
        Agent *src = mLevel->agentMan()->getAgent(srcAgentId);
        Agent *dst = mLevel->agentMan()->getAgent(dstAgentId);

        if(nullptr == src || nullptr == dst)
        {
            Debug::error("LocationModelManager::unlinkAgents(): an agent id is invalid: src=")
            (srcAgentId)(", dst:")(dstAgentId)(". Aborting.").endl();
            return false;
        }
        else if(srcAgentId == dstAgentId)
        {
            Debug::error("LocationModelManager::unlinkAgents(): src is dst. Aborting.").endl();
            return false;
        }

        return unlinkLocations(src->locationModelId(), dst->locationModelId());
    }

    bool LocationModelManager::unlinkLocations(ModelId mid0, ModelId mid1)
    {
        LocationModel *m0 = at(mid0), *m1 = at(mid1);

        if(nullptr == m0)
        {
            Debug::error(STEEL_METH_INTRO, "model ", mid0, " is not valid.").endl();
            return false;
        }

        if(nullptr == m1)
        {
            Debug::error(STEEL_METH_INTRO, "model ", mid1, " is not valid.").endl();
            return false;
        }

        AgentId aid0 = m0->attachedAgent();
        AgentId aid1 = m1->attachedAgent();

        if(m0->hasSource(aid1))
            m0->removeSource(aid1);

        if(m1->hasSource(aid0))
            m1->removeSource(aid0);

        if(m0->hasDestination(aid1))
            m0->removeDestination(aid1);

        if(m1->hasSource(aid0))
            m1->removeSource(aid0);

        removeDebugLine(makeKey(mid0, mid1));
        removeDebugLine(makeKey(mid1, mid0));

        return false;
    }

    void LocationModelManager::unlinkLocation(ModelId mid)
    {
        LocationModel *model = at(mid);

        if(nullptr == model)
        {
            Debug::error(STEEL_METH_INTRO, "model ", mid, " is not valid.").endl();
            return;
        }

        for(auto const & aid : model->sources())
        {
            Agent *agent = mLevel->agentMan()->getAgent(aid);

            if(nullptr == agent)
                continue;

            unlinkLocations(agent->locationModelId(), mid);
        }

        for(auto const & aid : model->destinations())
        {
            Agent *agent = mLevel->agentMan()->getAgent(aid);

            if(nullptr == agent)
                continue;

            unlinkLocations(mid, agent->locationModelId());
        }
    }

    ModelPair LocationModelManager::makeKey(ModelId mid0, ModelId mid1)
    {
//         return mid0 <= mid1 ? ModelPair(mid0, mid1) : ModelPair(mid1, mid0);
        return ModelPair(mid0, mid1);
    }

    void LocationModelManager::removeDebugLines(ModelId mid)
    {
        std::list<ModelPair> keys = collectModelPairs(mid);
        std::for_each(keys.begin(), keys.end(), std::bind(&LocationModelManager::removeDebugLine, this, std::placeholders::_1));
    }

    void LocationModelManager::removeDebugLine(ModelPair const &key)
    {
        if(INVALID_ID == key.first || INVALID_ID == key.second)
            return;

        DynamicLines *line;

        if(getDebugLine(key, line))
        {
            line->clear();
            line->update();
            mLevel->levelRoot()->detachObject(line);
            delete line;
        }

        mDebugLines.erase(key);
    }

    bool LocationModelManager::getDebugLine(ModelPair const &key, DynamicLines *&line)
    {
        auto it = mDebugLines.find(key);

        if(mDebugLines.end() == it)
        {
            line = new DynamicLines();
            line->init(mLevel->name(), Ogre::RenderOperation::OT_LINE_LIST, false);
            auto ret = mDebugLines.emplace(key, line);

            if(!ret.second)
            {
                delete line;
                Debug::error("LocationModelManager::getDebugLine(): can't insert ").endl();
                line = nullptr;
                return false;
            }

            mLevel->levelRoot()->attachObject(line);
        }
        else
        {
            line = it->second;
        }

        return true;
    }

    bool LocationModelManager::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        LocationModel *model = at(mid);

        if(nullptr == model)
            return false;

        model->attachAgent(agent->id());
        moveLocation(mid, agent->position());
        return true;
    }

    void LocationModelManager::onAgentUnlinkedFromModel(Agent *agent, ModelId mid)
    {
        removeDebugLines(mid);
        super::onAgentUnlinkedFromModel(agent, mid);
    }

    void LocationModelManager::moveLocation(ModelId mid, Ogre::Vector3 const &pos)
    {
        LocationModel *model = at(mid);

        if(nullptr == model)
            return;

        model->setPosition(pos);
        updateDebugLines(mid);
    }

    std::list<ModelPair> LocationModelManager::collectModelPairs(ModelId mid)
    {
        std::list<ModelPair> keys;
        LocationModel *model = at(mid);

        if(nullptr != model)
        {
            for(AgentId const aid : model->sources())
            {
                Agent *agent = mLevel->agentMan()->getAgent(aid);

                if(nullptr == agent)
                    continue;

                keys.push_back(makeKey(agent->locationModelId(), mid));
                keys.push_back(makeKey(mid, agent->locationModelId()));
            }

            for(AgentId const aid : model->destinations())
            {
                Agent *agent = mLevel->agentMan()->getAgent(aid);

                if(nullptr == agent)
                    continue;

                keys.push_back(makeKey(agent->locationModelId(), mid));
                keys.push_back(makeKey(mid, agent->locationModelId()));
            }
        }

        return keys;
    }

    void LocationModelManager::updateDebugLines(ModelId mid)
    {
        std::list<ModelPair> keys = collectModelPairs(mid);
        std::for_each(keys.begin(), keys.end(), std::bind(&LocationModelManager::updateDebugLine, this, std::placeholders::_1));
    }

    void LocationModelManager::updateDebugLine(ModelPair const &key)
    {
        if(!(isValid(key.first) && isValid(key.second)))
            return;

        DynamicLines *line = nullptr;

        if(getDebugLine(key, line))
        {
            line->clear();
            line->addPoint(at(key.first)->position());
            line->addPoint(at(key.second)->position());
            line->update();
        }
    }

    void LocationModelManager::setModelPath(ModelId mid, LocationPathName const &name)
    {
        if(LocationModel::EMPTY_PATH == name)
        {
            Debug::error(STEEL_METH_INTRO, "path name cannot be the empty string").endl();
            return;
        }

        LocationModel *model = at(mid);

        if(nullptr == model)
            return;

        model->_setPath(name);

        // model is the default root of its path
        if(INVALID_ID == pathRoot(name))
        {
            setPathRoot(model->attachedAgent());
        }
    }

    void LocationModelManager::unsetModelPath(ModelId mid)
    {
        LocationModel *model = at(mid);

        if(nullptr == model)
            return;

        auto name = model->path();

        if(LocationModel::EMPTY_PATH != name)
        {
            model->_setPath(LocationModel::EMPTY_PATH);
            mPathsRoots.erase(name);
        }
    }

    bool LocationModelManager::hasModelPath(ModelId mid)
    {
        return isValid(mid) && at(mid)->hasPath();
    }

    void LocationModelManager::setPathRoot(AgentId aid, bool force/* = false*/)
    {
        Agent *agent = mLevel->agentMan()->getAgent(aid);

        if(nullptr == agent)
            return;

        auto name = agent->locationPath();

        if(force)
            mPathsRoots.erase(name);

        mPathsRoots.emplace(name, aid);
    }

    AgentId LocationModelManager::pathRoot(LocationPathName const &name)
    {
        auto it = mPathsRoots.find(name);
        return mPathsRoots.end() == it ? INVALID_ID : it->second;
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
