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
        // capacity is doubled each time, therefore it cannot start at 0.
        mModels = std::vector<M>(1);
        mModelsFreeList = std::list<ModelId>();
        mModelsFreeList.push_back(0L);
        mLevel = level;
        mLevel->registerManager(M::modelType(), this);
    }

    template<class M>
    _ModelManager<M>::~_ModelManager()
    {
        clear();
        mLevel = nullptr;
    }

    template<class M>
    M *_ModelManager<M>::at(ModelId id)
    {
        if(!isValid(id))
            return nullptr;

        return &(mModels[id]);
    }

    template<class M>
    void _ModelManager<M>::incRef(ModelId id)
    {
        if(id >= mModels.size())
        {
            Debug::error(logName() + "::incRef(): modelId ")(id)("\" does not exist.").endl();
            return;
        }

        mModels[id].incRef();
    }

    template<class M>
    void _ModelManager<M>::decRef(ModelId id)
    {
        if(id >= mModels.size())
        {
            Debug::error(logName() + "::decRef(): modelId ")(id)("\" does not exist.").endl();
            return;
        }

        if(!isValid(id))
            return;

        mModels[id].decRef();

        //TODO: use a heap (priority queue), with (mModelsFreeList.size()-id) as priority
        if(mModels[id].isFree())
            mModelsFreeList.push_front(id);
    }

    template<class M>
    ModelId _ModelManager<M>::allocateModel()
    {
        ModelId mid = INVALID_ID;
        return allocateModel(mid);
    }

    template<class M>
    ModelId _ModelManager<M>::allocateModel(ModelId &mid)
    {
        if(INVALID_ID == mid)
        {
            if(mModelsFreeList.size() > 0)
            {
                mid = mModelsFreeList.front();
                mModelsFreeList.pop_front();
            }
            else
            {
                mid = (ModelId) mModels.size();
                mModels.push_back(M());
            }
        }
        else if(isValid(mid))
        {
            if(isFree(mid))
            {
                // user asked for an already existing free slot: remove from free list
                auto it = std::find(mModelsFreeList.begin(), mModelsFreeList.end(), mid);

                if(mModelsFreeList.end() != it)
                    mModelsFreeList.erase(it);
            }
            else
            {
                mid = INVALID_ID;
            }
        }
        else
        {
            while(mModels.capacity() <= mid)
                mModels.reserve(mModels.capacity() * 2);

            for(ModelId i = mModels.size(); i < mid; ++i)
                mModelsFreeList.push_back(i);

            if(mModels.size() <= mid)
                mModels.resize(mid + 1, M());
        }

        return mid;
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
        for(Model & model : mModels)
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
        return isValid(id) && mModels[id].isFree();
    }

    template<class M>
    std::vector<ModelId> _ModelManager<M>::fromJson(Json::Value &models)
    {
        std::vector<ModelId> ids;

        for(Json::ValueIterator it = models.begin(); it != models.end(); ++it)
        {
            //TODO: implement id remapping, so that we stay in a low id range
            Json::Value value = *it;
            ModelId mid = INVALID_ID;
            mid = Ogre::StringConverter::parseUnsignedLong(it.memberName(), INVALID_ID);

            if(!this->fromSingleJson(value, mid))
                Debug::error(logName())("could not deserialize model ")(mid).endl();

            ids.push_back(mid);
        }

        return ids;
    }

    template<class M>
    bool _ModelManager<M>::fromSingleJson(Json::Value &model, ModelId &id)
    {
        id = allocateModel(id);

        if(!mModels[id].fromJson(model))
        {
            deallocateModel(id);
            id = INVALID_ID;
            return false;
        }

        return true;
    }

    template<class M>
    void _ModelManager<M>::toJson(Json::Value &root)
    {
        if(mModels.size())
        {
            for(ModelId id = firstId(); id < lastId(); ++id)
            {
                Model *m = (Model *) & (mModels[id]);

                if(m->isFree())
                    continue;

                // makes sure the serailization exist, even if empty, otherwise the child will be forgotten forever.
                Json::Value node(Json::objectValue);
                m->toJson(node);
                root[Ogre::StringConverter::toString(id)] = node;
            }
        }
    }

    /// Returns true if the linking was ok to the manager's pov.
    template<class M>
    bool _ModelManager<M>::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        // no problem with that
        return true;
    }
    
    /// Returns true if the linking was ok to the manager's pov.
    template<class M>
    void _ModelManager<M>::onAgentUnlinkedFromModel(Agent *agent, ModelId mid)
    {
        decRef(mid);
    }

    template<class M>
    std::set<Tag> _ModelManager<M>::modelTags(ModelId mid)
    {
        std::set<Tag> output;
        M *model = at(mid);

        if(nullptr == model)
            return output;

        return model->tags();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

