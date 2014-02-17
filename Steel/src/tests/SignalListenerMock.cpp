#include "tests/SignalListenerMock.h"

namespace Steel
{
    SignalListenerMock::SignalListenerMock(): SignalListener(),
        mSignals()
    {

    }

    void SignalListenerMock::onSignal(Signal signal, SignalEmitter *source)
    {
        mSignals.push_back(std::make_pair(signal, source));
    }


    void SignalListenerMock::clearSignals()
    {
        mSignals.clear();
    }

    bool SignalListenerMock::hasOnlyReceived(Signal signal) const
    {
        return 1 == mSignals.size() && signal == mSignals[0].first;
    }

}
