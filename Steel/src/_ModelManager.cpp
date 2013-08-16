/*
 * Manager.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "_ModelManager.h"
#include "Debug.h"
#include "Model.h"
#include <Agent.h>
#include <Level.h>

namespace Steel
{

    template<class M>
    _ModelManager<M>::_ModelManager(Level *level)
        : ModelManager()
    {
        mModels = std::vector<M>();
        mModelsFreeList = std::list<ModelId>();
        mLevel = level;
    }

    template<class M>
    _ModelManager<M>::~_ModelManager()
    {
        clear();
        mLevel = NULL;
    }

    template<class M>
    M *_ModelManager<M>::at(ModelId id)
    {
        if (!isValid(id))
            return NULL;

        return &(mModels[id]);
    }

    template<class M>
    void _ModelManager<M>::incRef(ModelId id)
    {
        if (id >= mModels.size())
        {
            Debug::error(logName() + "::incRef(): modelId ")(id)("\" does not exist.").endl();
            return;
        }
        mModels[id].incRef();
    }

    template<class M>
    void _ModelManager<M>::decRef(ModelId id)
    {
        if (id >= mModels.size())
        {
            Debug::error(logName() + "::decRef(): modelId ")(id)("\" does not exist.").endl();
            return;
        }
        if (!isValid(id))
            return;

        mModels[id].decRef();

        //TODO: use a heap (priority queue), with (mModelsFreeList.size()-id) as priority
        if (mModels[id].isFree())
            mModelsFreeList.push_front(id);
    }

    template<class M>
    ModelId _ModelManager<M>::allocateModel()
    {
        ModelId id = INVALID_ID;
        if (mModelsFreeList.size() > 0)
        {
            id = mModelsFreeList.front();
            mModelsFreeList.pop_front();
        }
        else
        {
            id = (ModelId) mModels.size();
            mModels.push_back(M());
        }
        return id;
    }

    template<class M>
    void _ModelManager<M>::deallocateModel(ModelId id)
    {
        if(!mModels[id].isFree())
        {
            mModels[id].cleanup();
            mModelsFreeList.push_back(id);
        }
#ifdef DEBUG
        assert(mModels[id].isFree());
#endif
    }

    template<class M>
    void _ModelManager<M>::clear()
    {
        // deallocate all models
        for(Model &model:mModels)
            if(!model.isFree())
                model.cleanup();
        // remove space
        mModels.clear();
        mModelsFreeList.clear();
    }

    template<class M>
    bool _ModelManager<M>::isValid(ModelId id)
    {
        bool ret = true;
        ret &= id != INVALID_ID;
        ret &= id < mModels.size();
        return ret;
    }

    template<class M>
    bool _ModelManager<M>::isFree(ModelId id)
    {
        return isValid(id) && !mModels[id].isFree();
    }

    template<class M>
    std::vector<ModelId> _ModelManager<M>::fromJson(Json::Value &models)
    {
        Debug::log(logName() + "::fromJson()")(models).endl();
        std::vector<ModelId> ids;
        for (Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            //TODO: implement id remapping, so that we stay in a low id range
            Json::Value value = *it;
            ModelId mid=INVALID_ID;
            if(!fromSingleJson(value,mid))
                Debug::error("could not deserialize model.").endl();
            ids.push_back(mid);
        }
        return ids;
    }

    template<class M>
    bool _ModelManager<M>::fromSingleJson(Json::Value &value, ModelId &id)
    {
        id = allocateModel();
        //get values for load
        //incRef(id);
        int loadingOk = mModels[id].fromJson(value);
        //TODO discard, quarantine, repair ?
        if (!loadingOk)
        {
            decRef(id);
            id = INVALID_ID;
            return false;
        }
        return true;
    }

    template<class M>
    void _ModelManager<M>::toJson(Json::Value &object)
    {
//         Debug::log("_ModelManager(): ")(mModels.size())(" models in stock").endl();
        for (ModelId id = firstId(); id < lastId(); ++id)
        {
            Model *m = (Model *) &(mModels[id]);
            if (m->isFree())
                continue;
            m->toJson(object[Ogre::StringConverter::toString(id)]);
        }
    }

//     template<class M>
//     bool _ModelManager<M>::linkAgentToModel(AgentId aid, ModelId mid)
//     {
//         Agent *agent = mLevel->getAgent(aid);
//         if(!agent->linkToModel(modelType(),mid))
//         {
//             Debug::error("_ModelManager::linkAgentToModel(")(aid)(" to ")(mid);
//             Debug::error("): agent could not link to model.Aborted.").endl();
//             return false;
//         }
//
//         if(!onAgentLinkedToModel(aid,mid))
//         {
//             agent->unlinkFromModel(modelType());
//             Debug::error("_ModelManager::linkAgentToModel(")(aid)(" to ")(mid);
//             Debug::error("): specialized linking errored. Agent unlinked. Aborted.").endl();
//             return false;
//         }
//
//         return incRef(mid);
//     }

    template<class M>
    bool _ModelManager<M>::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        // no problem with that
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

