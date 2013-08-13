
#include "Debug.h"
#include "BT/BTSignalListener.h"
#include "SignalEmitter.h"

namespace Steel
{

    const char* BTSignalListener::SIGNALS_ATTRIBUTE = "signals";

    BTSignalListener::BTSignalListener(BTShapeToken const &token):BTNode(token)
    {
    }

    BTSignalListener::~BTSignalListener()
    {
    }
    
    BTState BTSignalListener::state()
    {
//         if(mIsSignalReceived)
            return READY;
    }

    bool BTSignalListener::parseNodeContent(Json::Value &root)
    {
        static Ogre::String intro="in BTSignalListener::parseNodeContent(): ";
        Json::Value value;

        value = root[BTSignalListener::SIGNALS_ATTRIBUTE];
        if (value.empty())
        {
            Debug::warning(intro)("Attribute \"")(BTSignalListener::SIGNALS_ATTRIBUTE)("\" is null/empty: node listens to no signal.").endl();
            return false;
        }
        else
        {
            for(Json::ValueIterator it=value.begin(); it!=value.end(); ++it)
            {
                if(!value.isString())
                {
                    Debug::error(intro)().endl();
                    continue;
                }
                registerSignal(value.asString());
            }
        }
        return true;
    }
    
    void BTSignalListener::onSignal(Signal signal, SignalEmitter *src)
    {
        Debug::log("BTSignalListener::onSignal()").endl();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
