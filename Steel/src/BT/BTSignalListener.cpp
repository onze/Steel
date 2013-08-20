
#include "Debug.h"
#include "BT/BTSignalListener.h"
#include "SignalEmitter.h"
#include <tools/JsonUtils.h>

namespace Steel
{

    const char* BTSignalListener::SIGNALS_ATTRIBUTE = "signals";
    const char* BTSignalListener::NON_BLOCKING_ATTRIBUTE = "nonBlocking";

    BTSignalListener::BTSignalListener(BTShapeToken const &token):BTNode(token),SignalListener(),
        mSignalReceived(false),mIsNonBlocking(false)
    {
        switchClosed();
    }

    BTSignalListener::~BTSignalListener()
    {
    }

    bool BTSignalListener::parseNodeContent(Json::Value &root)
    {
        static Ogre::String intro="in BTSignalListener::parseNodeContent(): ";
        bool allGood=true;
        Json::Value value;

        value = root[BTSignalListener::NON_BLOCKING_ATTRIBUTE];
        if(!value.isBool())
            Debug::warning(intro)("Invalid attribute ").quotes(BTSignalListener::NON_BLOCKING_ATTRIBUTE)(". Skipped.").endl();
        else
        {
            mIsNonBlocking=value.asBool();
            switchClosed();
        }

        value = root[BTSignalListener::SIGNALS_ATTRIBUTE];
        if(value.empty() || !value.isArray())
        {
            Debug::warning(intro)("Attribute \"")(BTSignalListener::SIGNALS_ATTRIBUTE)("\" is null/empty: node listens to no signal.").endl();
            allGood=false;
        }
        else
        {
            for(Json::ValueIterator it=value.begin(); it!=value.end(); ++it)
            {
                Json::Value item=*it;
                if(!item.isString())
                {
                    Debug::error(intro)("invalid signal")(item).endl();
                    allGood=false;
                    break;
                }
                registerSignal(item.asString());
            }
        }

        if(!allGood)
        {
            unregisterAllSignals();
            Debug::error(intro)("could not parse content: ")(root).endl();
        }
        return allGood;
    }

    void BTSignalListener::onParentNotified()
    {
        // out work is done, wait for next
        switchClosed();
    }

    void BTSignalListener::onSignal(Signal signal, SignalEmitter *src)
    {
        Debug::log("BTSignalListener::onSignal(): ")(signal)("/").quotes(SignalManager::instance().fromSignal(signal)).endl();
        switchOpened();
    }

    void BTSignalListener::switchClosed()
    {
        mSignalReceived=false;
        if(mIsNonBlocking)
            mState=FAILURE;
        else
            mState=RUNNING;
    }

    void BTSignalListener::switchOpened()
    {
        mSignalReceived=true;
        // goto child
        mState=SKIPT_TO;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
