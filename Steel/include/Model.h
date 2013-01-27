/*
 * Model.h
 *
 *  Created on: 2011-06-16
 *      Author: onze
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <json/json.h>

#include "Debug.h"
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
            Model();
            Model(const Model &m);
            virtual ~Model();

            virtual Model &operator=(const Model &m);
            inline void incRef()
            {
                ++mRefCount;
            }
            inline void decRef()
            {
                if (--mRefCount <= 0L)
                    cleanup();
            }
            inline bool isFree()
            {
                return mRefCount <= 0L;
            }

            /// Serialize itself into the given Json object
            virtual void toJson(Json::Value &object)
            {};

            /// Deserialize itself from the given Json object. return true is successful.
            virtual bool fromJson(Json::Value &object)
            {return true;};

            //getters
            inline unsigned long refCount()
            {
                return mRefCount;
            }
            
            /// Returns the ModelType associated with this model.
            virtual ModelType modelType()=0;
            
        protected:
            /// Called by decRef() when the ref count gets below 0.
            virtual void cleanup() {};
            /// Number of agents referencing it.
            unsigned long mRefCount;
    };

}

#endif /* MODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
