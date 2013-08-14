#ifndef STEEL_BTMODEL_H_
#define STEEL_BTMODEL_H_

#include <stack>
#include <json/json.h>

#include "BT/btnodetypes.h"
#include "BT/BTStateStream.h"

#include "Model.h"

namespace Steel
{
    /**
     * Instances of this class hold agent-specific data related to their BTree.
     * One can see them as a blackboard on which
     * BTNode subclasses can write stuff other nodes could need.
     * It is like a short&long term memory for an agent.
     * An agent's BTModel is also responsible for running the Btree it holds.
     */
    class BTModel: public Model
    {
        public:
            BTModel();
            BTModel(const BTModel &m);
            BTModel &operator=(const BTModel &m);
            virtual ~BTModel();

            /**
             * Initialize a BTModel from a shapestream:
             * - creates the states stream matching the shape stream
             * - do other wonders
             */
            bool init(BTShapeStream *shapeStream);

            virtual ModelType modelType();

            /// Deserialize itself from the given Json object
            virtual bool fromJson(Json::Value &node);

            /// Serialize itself into the given Json object
            virtual void toJson(Json::Value &node);

            /// Sets the current shape to the given one.
            bool switchShapeTo(BTShapeStream* shapeStream);

            /// Runs the tree until its end or a node yields RUNNING.
            void update(float timestep);

            virtual void cleanup();
        protected:

            // not owned
            // owned
            /// states, aligned with the shape stream.
            BTStateStream mStateStream;
            
            /// Index of the currently running node, within the StateStream.
            BTStateIndex mCurrentStateIndex;
            std::stack<BTStateIndex> mStatesStack;
    };

}
#endif /* STEEL_BTMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
