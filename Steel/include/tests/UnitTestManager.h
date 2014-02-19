#ifndef STEEL_UNITTESTMANAGER_H
#define STEEL_UNITTESTMANAGER_H

#include "steeltypes.h"

// include all *Mock classes for single include from other classes
#include "tests/SignalListenerMock.h"

namespace Steel
{
    class Engine;

    struct UnitTestExecutionContext
    {
        Engine *engine;
    };

    class UnitTestManager
    {
    private:
        static UnitTestManager *sInstance;
        
    public:
        typedef std::function<bool(UnitTestExecutionContext *const)> Callback;
        typedef Ogre::String Category;

        UnitTestManager();
        ~UnitTestManager();

        static UnitTestManager &instance();

        void addTest(UnitTestManager::Callback function, UnitTestManager::Category const &category, Ogre::String const &name);
        bool execute(Steel::UnitTestManager::Category category, UnitTestExecutionContext &context, bool abortOnFail = false);
    private:
        class Entry
        {
        public:
            Entry(Ogre::String _name, Callback _callback): name(_name), callback(_callback) {};
            Ogre::String name;
            Callback callback;
            bool operator<(Steel::UnitTestManager::Entry &o) const
            {
                return name < o.name;
            }
            bool operator<(Steel::UnitTestManager::Entry const &o) const
            {
                return name < o.name;
            }
        };
        std::map<Category, std::set<Entry>> mCallbackRegister;
    };
}



#endif // STEEL_UNITTESTMANAGER_H
