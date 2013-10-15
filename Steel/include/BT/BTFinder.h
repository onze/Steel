#ifndef STEEL_BTFINDER_H_
#define STEEL_BTFINDER_H_

#include <steeltypes.h>
#include <BT/btnodetypes.h>
#include <BT/BTNode.h>
#include <AgentSpec.h>

namespace Steel
{
    /**
     * BTFinder looks for a spec(ification) and if the spec is matched,
     * saves the result in the BTModel under the specified dest(ination).
     */
    class BTFinder: public BTNode
    {
        public:

            BTFinder(BTShapeToken const &token);
            virtual ~BTFinder();

            inline static BTShapeTokenType tokenType()
            {
                return BTFinderToken;
            }

        private:

            /// See BTNode::parseNodeContent
            bool parseNodeContent(Json::Value &root);
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
