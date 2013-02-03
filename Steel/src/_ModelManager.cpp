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
    _ModelManager<M>::_ModelManager(Level *level) :
        ModelManager()
    {
        mModels = std::vector<M>();
        mModelsFreeList = std::list<ModelId>();
        mLevel=level;
    }

    template<class M>
    _ModelManager<M>::~_ModelManager()
    {
        clear();
        mLevel=NULL;
    }

    template<class M>
    M *_ModelManager<M>::at(ModelId id)
    {
        if (!isValid(id))
            return NULL;

        return &(mModels[id]);
    }

    template<class M>
    bool _ModelManager<M>::incRef(ModelId id)
    {
        if (id >= mModels.size())
        {
            Debug::error(logName()+"::incRef(): modelId ")(id)("\" does not exist.").endl();
            return false;
        }
        mModels[id].incRef();
        return true;
    }
    
    template<class M>
    bool _ModelManager<M>::decRef(ModelId id)
    {
        if (id >= mModels.size())
        {
            Debug::error(logName()+"::decRef(): modelId ")(id)("\" does not exist.").endl();
            return false;
        }
        mModels[id].decRef();
        return true;
    }

    template<class M>
    void _ModelManager<M>::clear()
    {
        mModels.clear();
        mModelsFreeList.clear();
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
// 	mModels[id].incRef();
        return id;
    }
    
    template<class M>
    void _ModelManager<M>::releaseModel(ModelId id)
    {
        if (!isValid(id))
            return;
        M *m = at(id);
        m->decRef();
        //TODO: use a heap (priority queue), with (mModelsFreeList.size()-id) as priority
        if (m->isFree())
            mModelsFreeList.push_front(id);
    }

    template<class M>
    bool _ModelManager<M>::isValid(ModelId id)
    {
        bool ret=true;
        ret&=id >= 0;
        ret&=id < mModels.size();
        ret&=!mModels[id].isFree();
        return ret;
    }

    template<class M>
    void _ModelManager<M>::toJson(Json::Value &object)
    {
//         Debug::log("_ModelManager(): ")(mModels.size())(" models in stock").endl();
        for (ModelId id = 0; id < mModels.size(); ++id)
        {
            Model *m = (Model *) &(mModels[id]);
            if (m->isFree())
                continue;
            m->toJson(object[Ogre::StringConverter::toString(id)]);
        }
    }
    
        template<class M>
        bool _ModelManager<M>::onAgentLinkedToModel(AgentId aid, ModelId mid)
        {
            // no problem with that
            return true;
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

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
