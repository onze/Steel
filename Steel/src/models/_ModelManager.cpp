/*
 * Manager.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "models/_ModelManager.h"
#include "Debug.h"
#include "Level.h"
#include "models/Agent.h"
#include "models/Model.h"
#include <tools/JsonUtils.h>

namespace Steel
{

    template<class M>
    _ModelManager<M>::_ModelManager(Level *level)
        : ModelManager(),
        mLevel(level), 
        mModels(1), // capacity is doubled each time, therefore it cannot start at 0.
        mModelsFreeList({0L}) // first model is free
    {
        mLevel->registerManager(ManagedModel::staticModelType(), this);
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
        {
            mModels[id].cleanup();
            mModels[id].Model::cleanup();
            mModelsFreeList.push_front(id);
        }
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
                mModels.reserve(mModels.size() * 2);
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
    std::vector<ModelId> _ModelManager<M>::fromJson(Json::Value const &nodes)
    {
        std::vector<ModelId> ids;

        for(Json::ValueIterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            Json::Value node = *it;
            ModelId mid = INVALID_ID;
            mid = Ogre::StringConverter::parseUnsignedLong(it.memberName(), INVALID_ID);

            if(!this->fromSingleJson(node, mid))
                Debug::error(logName())("could not deserialize model ")(mid).endl();

            ids.push_back(mid);
        }

        return ids;
    }

    template<class M>
    bool _ModelManager<M>::fromSingleJson(Json::Value const &node, ModelId &mid, bool updateModel /*= false*/)
    {
        ModelId id = mid;
        id = allocateModel(id);

        // id points to an already existing model
        if(INVALID_ID == id && !updateModel)
            return false;


        if(!deserializeToModel(node, id))
        {
            deallocateModel(id);
            mid = INVALID_ID;
            return false;
        }

        mid = id;
        return true;
    }

    template<class M>
    bool _ModelManager<M>::deserializeToModel(Json::Value const &node, ModelId &mid)
    {
        return mModels[mid].fromJson(node);
    }

    template<class M>
    void _ModelManager<M>::toJson(Json::Value &nodes, std::list<ModelId> const &modelIds)
    {
        if(mModels.size())
        {
            for(ModelId const & id : modelIds)
            {
                ManagedModel *managedModel = at(id);

                if(nullptr == managedModel || managedModel->isFree())
                    continue;

                Json::Value node(Json::objectValue);
                managedModel->toJson(node);
                nodes[Ogre::StringConverter::toString(id)] = node;
            }
        }
    }

    template<class M>
    bool _ModelManager<M>::toSingleJson(ModelId mid, Json::Value &value)
    {
        ManagedModel *managedModel = at(mid);

        if(nullptr != managedModel)
        {
            managedModel->toJson(value);
            return true;
        }

        return false;
    }

// not used so far
//     template<class M>
//     void _ModelManager<M>::swapModels(ModelId midLeft, ModelId midRight)
//     {
//         bool isValidLeft = isValid(midLeft);
//         bool isValidRight = isValid(midRight);
// 
//         if(isValidLeft || isValidRight)
//             std::swap(mModels[midLeft], mModels[midRight]);
//     }

    /// Returns true if the linking was successful from the manager's pov.
    template<class M>
    bool _ModelManager<M>::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        return true; // no problem with that
    }

    /// Returns true if the linking was successful from the manager's pov.
    template<class M>
    void _ModelManager<M>::onAgentUnlinkedFromModel(Agent *agent, ModelId mid)
    {
        if(isValid(mid) && !isFree(mid))
            decRef(mid);
    }

    template<class M>
    std::set<Tag> _ModelManager<M>::modelTags(ModelId mid)
    {
        std::set<Tag> output;
        ManagedModel *managedModel = at(mid);

        if(nullptr == managedModel)
            return output;

        return managedModel->tags();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

