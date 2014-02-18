#include "tests/SignalListenerMock.h"

namespace Steel
{
    SignalListenerMock::SignalListenerMock(): SignalListener(),
        mSignals()
    {

    }

    void SignalListenerMock::onSignal(Signal signal, SignalEmitter *const source)
    {
        mSignals.push_back(std::make_pair(signal, source));
    }


    void SignalListenerMock::clearSignals()
    {
        mSignals.clear();
    }

    bool SignalListenerMock::hasOnlyReceived(Signal const signal) const
    {
        return 1 == mSignals.size() && signal == mSignals[0].first;
    }

    bool SignalListenerMock::hasReceived(Signal const signal) const
    {
        for(Entry const& entry : mSignals)
            if(signal == entry.first)
                return true;

        return false;
    }


}
