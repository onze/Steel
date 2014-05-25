#ifndef STEEL__MODELMANAGER_H_
#define STEEL__MODELMANAGER_H_

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
        // TODO:
        /*
         ModelType modelType() const{return ManagedModel::modelType()};
         * */
        typedef M ManagedModel;

        /// modelType associated with this Manager
        static ModelType staticModelType()
        {
            return ManagedModel::staticModelType();
        }

        _ModelManager(Level *level);
        virtual ~_ModelManager();
        /**
         * Returns a pointer to the model referenced by the given id, if the model is in use. Return nullptr
         * otherwise. Since model addresses can change without notice, models pointers lifetime is not
         * supposed to be longer than the function they're created in (and watchout when multithreading).
         * ModelIds are meant for this, as seen in Agent.h/cpp.
         */
        virtual ManagedModel *at(ModelId id);

        /// Forward call to allocatedModel, meant to add extra functionality the bare allocation.
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

        /// Initializes new models according to data in the json serialization.
        virtual std::vector<ModelId> fromJson(Json::Value const &models);
        /**
         * Initializes a single model pointed to by mid, according to data in the
         * json serialization.
         * If mid == INVALID_ID, a new id is found and mid is set to it. If mid is free,
         * it becomes the model's id. If mid is valid (ie, in use), updateModel determines
         * whether (if true) the pointed model is updated, or if the operation is a failure.
         * Returns false is the operation fails.
         *
         * The actual model initialization (the call to fromJson) is done in deserializeToModel,
         * so that subclass can pass more parameters to their models withouth reimplementing the
         * allocation.
         */
        virtual bool fromSingleJson(Json::Value const &model, ModelId &mid, bool updateModel = false);

        /// Dumps refenreced models' json representation into the given object.
        virtual void toJson(Json::Value &object, std::list<ModelId> const &modelIds);
        /// Redirects the call to the pointed model. Returns false if the model is invalid.
        virtual bool toSingleJson(ModelId mid, Json::Value &object);

//         virtual void swapModels(ModelId midLeft, ModelId midRight) override;

        virtual Ogre::String logName()
        {
            return "ModelManager<" + toString(ManagedModel::staticModelType()) + ">";
        }

        virtual bool onAgentLinkedToModel(Agent *agent, ModelId mid);
        virtual void onAgentUnlinkedFromModel(Agent *agent, ModelId mid);

        /// Returns the model tags, or an empty set if the given id is not valid.
        virtual std::set<Tag> modelTags(ModelId mid);

        inline Level *level() {return mLevel;}

    protected:
        /**
         * Allocates a new model for the given id and returns its id.
         * If id is INVALID_ID, finds a free id to place the given model at.
         * If allocation was not possible (or id was already taken),
         * returns Steel::INVALID_ID and sets id to it.
         */
        ModelId allocateModel(ModelId &id);
        ModelId allocateModel();

        /// Cleans up the model and marks it as allocable.
        void deallocateModel(ModelId id);

        virtual bool deserializeToModel(Json::Value const &model, ModelId &mid);

        // not owned
        Level *mLevel;
        //owned
        /// Contains models.
        std::vector<ManagedModel> mModels;
        std::list<ModelId> mModelsFreeList;
    };
}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

