#ifndef STEEL__MODELMANAGER_H_
#define STEEL__MODELMANAGER_H_

#include <json/json.h>

#include <vector>
#include <list>

#include "Debug.h"
#include "steeltypes.h"
#include "ModelManager.h"

namespace Steel
{
    class Level;
    //M is the managed model class
    template<class M>
    class _ModelManager: public ModelManager
    {
    private:
        _ModelManager() {}

    public:
        _ModelManager(Level *level);
        virtual ~_ModelManager();
        /**
         * Returns a pointer to the model referenced by the given id, if the model is in use. Return nullptr
         * otherwise. Since model addresses can change without notice, models pointers lifetime is not
         * supposed to be longer than the function they're created in (and watchout when multithreading).
         * ModelIds are meant for this, as seen in Agent.h/cpp.
         */
        virtual M *at(ModelId id);

        virtual ModelId newModel() {return allocateModel();};

        /// Increments the ref count of the given model.
        virtual void incRef(ModelId id);

        /// Decrements the ref count of the given model.
        virtual void decRef(ModelId id);

        /// Clears every models from its memory.
        virtual void clear();

        /// Returns true if the given model id is in use.
        virtual bool isFree(ModelId id);
        /// Returns true if the given model id points to a model.
        virtual bool isValid(ModelId id);

        /// Initialize new models according to data in the json serialization.
        virtual std::vector<ModelId> fromJson(Json::Value &models);
        /**
         * Initialize a single new model according to data in the json serialization.
         * If mid == INVALID_ID, a new id is found and mid is set to it. Otherwise,
         * the model will be set the given id.
         * Returns false is anything went wrong.
         */
        virtual bool fromSingleJson(Json::Value &model, ModelId &id);

        /// Dump all models' json representation into the given object.
        virtual void toJson(Json::Value &object);

        /// modelType associated with this Manager
        static ModelType modelType()
        {
            return M::modelType();
        }

        virtual Ogre::String logName()
        {
            return "ModelManager<" + modelTypesAsString[this->modelType()] + ">";
        }

        virtual bool onAgentLinkedToModel(Agent *agent, ModelId mid);

        /// Returns the model tags, or an empty set if the given id is not valid.
        virtual std::set<Tag> modelTags(ModelId mid);

        inline Level *level() {return mLevel;}

    protected:
        /// Returns the lowest Id currently in use
        inline ModelId firstId()
        {
            //TODO: actually track the right value
            return 0;
        }
        /// Returns the lowest Id currently in use
        inline ModelId lastId()
        {
            //TODO: actually track the right value
            return mModels.size();
        }
        /**
         * Allocates a new model for the given id and returns its id.
         * If id is INVALID_ID, finds a free id to place the given model at.
         * If allocation was not possible (or id was already taken),
         * returns Steel::INVALID_ID and sets id to it.
         */
        ModelId allocateModel(ModelId &id);
        ModelId allocateModel();

        /**
         * Cleans up the model and marks it as allocable.
         */
        void deallocateModel(ModelId id);

        // not owned
        Level *mLevel;
        //owned
        /// Contains models.
        std::vector<M> mModels;
        std::list<ModelId> mModelsFreeList;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

