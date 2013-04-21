#ifndef STEEL_BTMODEL_H_
#define STEEL_BTMODEL_H_

#include <json/json.h>

#include "BT/btnodetypes.h"
#include "BT/BTStateStream.h"

#include "Model.h"

namespace Steel
{
    /**
     * instances of this class hold agent-specific data related to BTrees. One can see them as a blackboard on which
     * BTNodes can write stuff other nodes could need. It is like a short/long term memory for an agent.
     */
    class BTModel: public Model
    {
        public:
            BTModel();
            BTModel(const BTModel &m);
            BTModel &operator=(const BTModel &m);
            virtual ~BTModel();
            
            bool init(BTShapeStream *shapeStream);

            virtual ModelType modelType();

            /// Deserialize itself from the given Json object
            virtual bool fromJson(Json::Value &node);

            /// Serialize itself into the given Json object
            virtual void toJson(Json::Value &node);
            
            /// Sets the current shape to the given one.
            bool switchShapeTo(BTShapeStream* shapeStream);

        protected:
            virtual void cleanup();
            
            // not owned
            // owned
            /// states, aligned with the shape stream.
            BTStateStream mStateStream;
    };

}
#endif /* STEEL_BTMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
