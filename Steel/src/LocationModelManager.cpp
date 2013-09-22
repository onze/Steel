#include "LocationModelManager.h"
#include <tools/DynamicLines.h>
#include <Agent.h>
#include <Level.h>

namespace Steel
{
    LocationModelManager::LocationModelManager(Level *level):
        _ModelManager<LocationModel>(level)
    {
        mDebugLines.clear();
    }

    LocationModelManager::~LocationModelManager()
    {
        for(auto it:mDebugLines)
        {
            mLevel->levelRoot()->detachObject(it.second);
            delete it.second;
        }
        mDebugLines.clear();
    }

    ModelId LocationModelManager::newModel()
    {
        ModelId id = allocateModel();
        if(!mModels[id].init(this))
        {
            deallocateModel(id);
            id=INVALID_ID;
        }
        return id;
    }

    std::vector<ModelId> LocationModelManager::fromJson(Json::Value &models)
    {
        auto ret=_ModelManager<LocationModel>::fromJson(models);
        static const Ogre::String intro="in LocationModelManager::fromJson(): ";
        // post processing:
        // - check all connections and delete/warn about invalid ones
        // - build debug lines from connected models
        LocationModel *current, *dst;
        ModelId dstId;
        for(ModelId id=firstId(); id<lastId(); ++id)
        {
            current=at(id);
            if(NULL==current)
                continue;
            if(!current->hasDestination())
                continue;
            dstId = current->destination();
            dst=at(dstId);
            if(NULL==dst)
            {
                Debug::error(intro)("source model ")(id)(" has invalid destination ")(dstId)
                (". Unlinking.").endl();
                current->unsetDestination();
                continue;
            }
            updateDebugLine(id, dstId);
        }
        return ret;
    }

    bool LocationModelManager::fromSingleJson(Json::Value &model, ModelId &id)
    {
        id = allocateModel(id);
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

    bool LocationModelManager::linkLocations(ModelId srcId, ModelId dstId)
    {
        static const Ogre::String intro="in LocationModelManager::linkLocations(): ";
        LocationModel *src=at(srcId), *dst=at(dstId);
        if(NULL==src)
        {
            Debug::error(intro)("source model ")(srcId)(" is not valid.").endl();
            return false;
        }
        if(NULL==dst)
        {
            Debug::error(intro)("destination model ")(dstId)(" is not valid.").endl();
            return false;
        }
        
        if(src->hasDestination())
        {
            Debug::error(intro)("source model ")(srcId)(" already have a destination: ")(dstId).endl();
            return false;
        }
        if(dst->hasSource())
        {
            Debug::error(intro)("destination model ")(dstId)(" already have a source: ")(srcId).endl();
            return false;
        }

        if(!src->setDestination(dstId))
        {
            Debug::error(intro)("source model ")(srcId)(" cannot use a new destination.").endl();
            return false;
        }

        if(!dst->setSource(srcId))
        {
            Debug::error(intro)("destination model ")(dstId)(" cannot use a new source.").endl();
            src->unsetDestination();
            return false;
        }
        updateDebugLine(srcId, dstId);
        return true;
    }
    
    void LocationModelManager::unlinkLocation(ModelId mid)
    {
        static const Ogre::String intro="in LocationModelManager::linkLocation(): ";
        
        LocationModel *model=at(mid);
        if(NULL==model)
        {
            Debug::error(intro)("model ")(mid)(" is not valid.").endl();
            return;
        }
        if(model->hasSource())
            unlinkLocations(mid, model->source());
        if(model->hasDestination())
            unlinkLocations(mid, model->destination());
    }

    bool LocationModelManager::unlinkLocations(ModelId mid0, ModelId mid1)
    {
        static const Ogre::String intro="in LocationModelManager::linkLocations(): ";

        LocationModel *m0=at(mid0), *m1=at(mid1);
        if(NULL==m0)
        {
            Debug::error(intro)("model ")(mid0)(" is not valid.").endl();
            return false;
        }
        if(NULL==m1)
        {
            Debug::error(intro)("model ")(mid1)(" is not valid.").endl();
            return false;
        }

        if(m0->destination()==mid1 && m1->source()==mid0)
        {
            m0->unsetDestination();
            m1->unsetSource();
            if(!(m1->destination()==mid0 && m0->source()==mid1))
                removeDebugLine(mid0, mid1);
            return true;
        }
        if(m1->destination()==mid0 && m0->source()==mid1)
        {
            m1->unsetDestination();
            m0->unsetSource();
            if(!(m0->destination()==mid1 && m1->source()==mid0))
                removeDebugLine(mid0, mid1);
            return true;
        }
        return false;
    }

    ModelPair LocationModelManager::makeKey(ModelId mid0, ModelId mid1)
    {
        // have mid0<mid1, so that one pair has one single possible key in mDebugLines
        return mid0>mid1?ModelPair(mid1,mid0):ModelPair(mid0,mid1);
    }

    void LocationModelManager::removeDebugLine(ModelId mid0, ModelId mid1)
    {
        if(INVALID_ID==mid0 || INVALID_ID==mid1)
            return;
        
        auto key=makeKey(mid0, mid1);
        DynamicLines *line;
        getDebugLine(mid0, mid1, line);
        if(NULL!=line)
        {
            line->clear();
            line->update();
            mLevel->levelRoot()->detachObject(line);
            delete line;
        }
        mDebugLines.erase(key);
    }

    bool LocationModelManager::getDebugLine(ModelId mid0, ModelId mid1, DynamicLines *&line)
    {
        ModelPair key=makeKey(mid0, mid1);
        auto it=mDebugLines.find(key);

        if(mDebugLines.end()==it)
        {
            line=new DynamicLines();
            auto ret=mDebugLines.insert(std::pair<ModelPair, DynamicLines *>(key,line));
            if(!ret.second)
            {
                delete line;
                Debug::error("LocationModelManager::getDebugLine(): can't insert ").endl();
                line=NULL;
                return false;
            }
            mLevel->levelRoot()->attachObject(line);
        }
        else
        {
            line=it->second;
        }
        return true;
    }

    bool LocationModelManager::onAgentLinkedToModel(Agent *agent, ModelId mid)
    {
        LocationModel *model=at(mid);
        if(NULL==model)
            return false;
        model->attachAgent(agent->id());
        moveLocation(mid, agent->position());
        return true;
    }

    void LocationModelManager::moveLocation(ModelId mid, Ogre::Vector3 const &pos)
    {
        LocationModel *model=at(mid);
        if(NULL==model)
            return;
        model->setPosition(pos);
        updateDebugLine(mid);
    }

    void LocationModelManager::updateDebugLine(ModelId mid)
    {
        LocationModel *model=at(mid);
        if(NULL==model)
            return;
        if(model->hasSource())
            updateDebugLine(model->source(), mid);
        if(model->hasDestination())
            updateDebugLine(mid, model->destination());
    }

    void LocationModelManager::updateDebugLine(ModelId srcId, ModelId dstId)
    {
        if(!(isValid(srcId) && isValid(dstId)))
            return;

        DynamicLines *line=NULL;
        if(getDebugLine(srcId, dstId, line))
        {
            line->clear();
            line->addPoint(at(srcId)->position());
            line->addPoint(at(dstId)->position());
            line->update();
        }
    }
}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
