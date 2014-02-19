#ifndef SIGNALLISTENERMOCK_H
#define SIGNALLISTENERMOCK_H

#include "steeltypes.h"
#include "SignalListener.h"

namespace Steel
{
    class SignalEmitter;
    
    class SignalListenerMock : public SignalListener
    {
    public:
        SignalListenerMock();

        virtual void onSignal(Signal signal, SignalEmitter *const source);
        void clearSignals();
        
        bool hasOnlyReceived(Signal const signal) const;
        bool hasReceived(Signal const signal) const;

    private:
        typedef std::pair<Signal, SignalEmitter *> Entry;
        std::vector<Entry> mSignals;
    };
}
#endif // SIGNALLISTENERMOCK_H
