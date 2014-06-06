#include "tests/UnitTestManager.h"
#include <Debug.h>

#include "InputSystem/InputBuffer.h"
#include <InputSystem/Action.h>
#include <InputSystem/ActionCombo.h>
#include "tools/ConfigFile.h"
#include "tools/StringUtils.h"
#include "BT/BTShapeManager.h"
#include "BT/BTStateStream.h"
#include "models/BTModel.h"

namespace Steel
{
    UnitTestManager *UnitTestManager::sInstance = nullptr;

    UnitTestManager &UnitTestManager::instance()
    {
        if(nullptr == UnitTestManager::sInstance)
            UnitTestManager::sInstance = new UnitTestManager();

        return *UnitTestManager::sInstance;
    }

    UnitTestManager::UnitTestManager(): mCallbackRegister()
    {
        addTest(&utest_ConfigFile, "Steel.init", "ConfigFile");
        addTest(&utest_StringUtils, "Steel.init", "StringUtils");
        addTest(&utest_Action, "Steel.init", "Action");
        addTest(&utest_ActionCombo, "Steel.init", "ActionCombo");
        addTest(&utest_InputBufferMain, "Steel.init", "InputBuffer");
        addTest(&utest_BTShapeStream, "Steel.init", "BTShapeStream");
        addTest(&utest_BTStateStream, "Steel.init", "BTStateStream");
        
        addTest(&utest_BTrees, "Steel.debugLevel", "BTrees");
    }

    UnitTestManager::~UnitTestManager()
    {
    }

    void UnitTestManager::addTest(UnitTestManager::Callback callback, const UnitTestManager::Category &category, const Ogre::String &name)
    {
        mCallbackRegister.emplace(category, std::set<Entry>()).first->second.insert(Entry(name, callback));
    }

    bool UnitTestManager::execute(UnitTestManager::Category category, UnitTestExecutionContext &context, bool abortOnFail)
    {
        auto it = mCallbackRegister.find(category);
        bool passed = true;

        if(mCallbackRegister.end() != it)
        {
            for(Entry const & entry : it->second)
            {
                Debug::log("testing ").quotes(entry.name)("...").flush();
                passed &= (entry.callback)(&context);

                if(passed)
                {
                    Debug::log(" passed").endl();
                }
                else
                {
                    Debug::error(" failed").endl();

                    if(abortOnFail)
                        return false;
                }
            }
        }
        else
            passed = false;

        return passed;
    }
}
