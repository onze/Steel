#ifndef SIGNALLISTENERMOCK_H
#define SIGNALLISTENERMOCK_H

#include <vector>

#include <SignalListener.h>
#include <SignalEmitter.h>

namespace Steel
{
    class SignalListenerMock : public SignalListener
    {
    public:
        SignalListenerMock();

        virtual void onSignal(Signal signal, SignalEmitter *source);
        void clearSignals();
        
        bool hasOnlyReceived(Signal signal) const;

    private:
        typedef std::pair<Signal, SignalEmitter *> Entry;
        std::vector<Entry> mSignals;
    };
}
#endif // SIGNALLISTENERMOCK_H
