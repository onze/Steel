#ifndef STEEL_BTSIGNALLISTENER_H
#define STEEL_BTSIGNALLISTENER_H

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
            /// If true, node will pause the tree until a signal is received
            static const char* NON_BLOCKING_ATTRIBUTE;

            inline static BTShapeTokenType tokenType()
            {
                return BTShapeTokenType::BTSignalListenerToken;
            }

            BTSignalListener(BTShapeToken const &token);
            virtual ~BTSignalListener();

            // SignalListener interface
            void onSignal(Signal signal, SignalEmitter *const src);

            void onParentNotified();

        private:
            /// Next run will execute child.
            void switchOpened();
            /// Will wait for a signal before opening.
            void switchClosed();
            /// See BTNode::parseNodeContent.
            virtual bool parseNodeContent(Json::Value &root);
            //owned
            bool mSignalReceived;
            /// See NON_BLOCKING_ATTRIBUTE
            bool mIsNonBlocking;
    };
}

#endif // STEEL_BTSIGNALLISTENER_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
