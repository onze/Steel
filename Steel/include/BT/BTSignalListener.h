#ifndef STEEL_BTSIGNALLISTENER_H
#define STEEL_BTSIGNALLISTENER_H

#include <json/json.h>

#include "steeltypes.h"
#include "BT/BTNode.h"
#include "SignalListener.h"

namespace Steel
{
    class SignalEmitter;
    
    /**
     * This node listens to a set of signals.
     * Yields READY until one of the target signals is fired, then yields the subnode result.
     */
    class BTSignalListener:public BTNode, SignalListener
    {
        public:
            /// Name of the attribute listing signals the node listens to.
            static const char* SIGNALS_ATTRIBUTE;

            inline static BTShapeTokenType tokenType()
            {
                return BTSignalListenerToken;
            }

            BTSignalListener(BTShapeToken const &token);
            virtual ~BTSignalListener();
            
            // SignalListener interface
            virtual void onSignal(Signal signal, SignalEmitter* src);
            
            BTState state();

        private:
            /// See BTNode::parseNodeContent.
            virtual bool parseNodeContent(Json::Value &root);
    };
}

#endif // STEEL_BTSIGNALLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
