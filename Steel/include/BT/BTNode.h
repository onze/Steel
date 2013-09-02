#ifndef STEEL_BTNODE_H
#define STEEL_BTNODE_H

#include <json/json.h>

#include "BT/btnodetypes.h"
#include "AgentSpec.h"

namespace Steel
{
    class BTNode
    {

        public:
            BTNode(const Steel::BTShapeToken& token);
            BTNode(const BTNode& other);
            virtual ~BTNode();
            virtual BTNode& operator=(const BTNode& other);
            virtual bool operator==(const BTNode& other) const;

            /// Sets the node to its original content file settings.
            virtual bool reset();

            // getters
            inline const unsigned begin()
            {
                return mToken.begin;
            }

            inline const unsigned end()
            {
                return mToken.end;
            }

            inline const BTShapeToken token() const
            {
                return mToken;
            }

            /// Current state of the node. Can be overwritten by subclasses if needed.
            virtual BTState state();

            BTStateIndex firstChildIndex();
            BTStateIndex lastChildIndex();

            ////////////
            // interface in contact to BTModel
            /**
             * Called as long as the node shows itself as READY.
             */
            virtual void run(float timestep);

            /**
             * Returns what's the next node to run. Defaults to parent node.
             * Meant to be overwritten.
             */
            virtual BTStateIndex nodeSkippedTo();

            /**
             * Tells the node what state its currently running child just returned.
             * Defaults to updating the node's state.
             * Meant to be overwritten.
             */
            virtual void childReturned(BTNode const * const node, BTState state);

            /**
             * Called after run, once the parent has been notified of the node's state.
             * Defaults to resetting the state to READY;
             * Can typically be used to reset the node's state.
             */
            virtual void onParentNotified();
            ////////////

        protected:
            static const char * AGENT_SPEC_ATTRIBUTE;

            /// Actual parsing. Meant to be overloaded by subclasses.
            virtual bool parseNodeContent(Json::Value &root);

            // owned
            BTState mState;
            BTShapeToken mToken;
    };
}
#endif

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
