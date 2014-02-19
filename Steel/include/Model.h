#ifndef STEEL_MODEL_H_
#define STEEL_MODEL_H_

#include "steeltypes.h"

namespace Steel
{
    class Agent;
    /**
     * Base class for models.
     * Implements reference counting. Subclass is in charge of  everything else.
     */
    class Model
    {
        public:
            static const char *AGENT_TAGS_ATTRIBUTES;

            Model();
            Model(const Model &o);
            virtual ~Model();

            virtual Model &operator=(const Model &o);
            virtual bool operator==(const Model &o) const;

            inline void incRef()
            {
                ++mRefCount;
            }

            inline void decRef()
            {
                if (mRefCount > 0)
                {
                    --mRefCount;
                    if (mRefCount == 0)
                        this->cleanup();
                }
            }
            
            inline bool isFree() const
            {
                return mRefCount == 0L;
            }

            /// Serialize itself into the given Json object
            virtual void toJson(Json::Value &object)
            {
            }

            /// Deserialize itself from the given Json object. return true is successful.
            virtual bool fromJson(Json::Value &object)
            {
                return true;
            }

            //getters
            inline unsigned long refCount()
            {
                return mRefCount;
            }

            /// Returns the ModelType associated with this model.
            static ModelType modelType()
            {
                return ModelType::LAST;
            };

            /// Cleans any model specific data. SHould not be called directly, except by dedicated manager.
            virtual void cleanup();

            inline std::set<Tag> tags()
            {
                return mTags;
            }

            /// write tags to serialization.
            void serializeTags(Json::Value &value);

            /// reads tags from serialization. Returns true if all went ok.
            bool deserializeTags(Json::Value const &value);

        protected:
            /// Number of agents referencing it.
            unsigned long mRefCount;

            /// Tags the model delegates t its agents
            std::set<Tag> mTags;
    };

}

#endif /* STEEL_MODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
