#include "UI/DebugValueManager.h"
#include <Debug.h>

#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Controls/ElementFormControlDataSelect.h>
#include <Rocket/Controls/ElementFormControlInput.h>

namespace Steel
{
    DebugValueManager::DebugValueManager():
        mDocument(nullptr), mCallbacks(),
        mSelectControlId(""), mCurrentEntry()
    {
    }

    DebugValueManager::DebugValueManager(const DebugValueManager &o)
        : mDocument(o.mDocument), mCallbacks(o.mCallbacks),
          mSelectControlId(o.mSelectControlId), mCurrentEntry(o.mCurrentEntry)
    {
    }

    DebugValueManager::~DebugValueManager()
    {
        shutdown();
    }

    DebugValueManager &DebugValueManager::operator=(const DebugValueManager &o)
    {
        if(this != &o)
        {
            mDocument = o.mDocument;
            mCallbacks = o.mCallbacks;
            mSelectControlId = o.mSelectControlId;
            mCurrentEntry = o.mCurrentEntry;
        }

        return *this;
    }

    void DebugValueManager::init(const Ogre::String selectControlId, Rocket::Core::ElementDocument *document)
    {
        mSelectControlId = selectControlId;
        mDocument = document;
        mCallbacks.clear();
        mCurrentEntry = Entry();
        setSliderActive(false);
    }

    void DebugValueManager::shutdown()
    {
        mSelectControlId.clear();
        mCallbacks.clear();
        mDocument = nullptr;
        mCurrentEntry = Entry();
    }

    void DebugValueManager::refresh(Rocket::Core::ElementDocument *const document)
    {
        mDocument = document;
        Rocket::Controls::ElementFormControlDataSelect *selectControl = findSelectControl(mDocument);

        if(nullptr == selectControl)
            return;

        selectControl->RemoveAll();

        for(auto & entry : mCallbacks)
            selectControl->Add(entry.first.c_str(), entry.first.c_str());
    }

    void DebugValueManager::addDebugValue(const Ogre::String &entryName, Steel::DebugValueManager::CallbackFunction callback, float min, float max, float init)
    {
        mCallbacks.emplace(entryName, Entry(callback, min, max, init));
        refresh(mDocument);
    }

    Rocket::Controls::ElementFormControlDataSelect *DebugValueManager::findSelectControl(Rocket::Core::ElementDocument *document)
    {
        return nullptr == mDocument ? nullptr : static_cast<Rocket::Controls::ElementFormControlDataSelect *>(mDocument->GetElementById(mSelectControlId.c_str()));
    }

    void DebugValueManager::setSliderActive(bool flag)
    {
        auto slider = findSliderControl(mDocument);

        if(slider)
        {
            slider->SetDisabled(!flag);
            slider->SetProperty("visible", flag ? "true" : "false");
        }
    }

    void DebugValueManager::removeDebugValue(const Ogre::String &entryName)
    {
        static const Ogre::String intro = "DebugValueManager::removeDebugValue(): ";

        auto it = mCallbacks.find(entryName);

        if(mCallbacks.end() != it)
        {
            auto selectControl = findSelectControl(mDocument);

            if(selectControl && selectControl->GetOption(selectControl->GetSelection())->GetValue().CString() == entryName.c_str())
            {
                mCurrentEntry = Entry();
                setSliderActive(false);
            }

            mCallbacks.erase(entryName);
            refresh(mDocument);
        }
    }

    bool DebugValueManager::processCommand(std::vector<Ogre::String> command)
    {
        static const Ogre::String intro = "DebugValueManager::processCommand(): ";

        if(command.size() == 0)
        {
            Debug::log(intro)("empty command.").endl();
            return true;
        }

        // dispatch the command to the right subprocessing function
        if(command[0] == "debugvaluemanager")
        {
            command.erase(command.begin());
            return processCommand(command);
        }

        if(command[0] == "onEntrySelected")
        {
            auto selectControl = findSelectControl(mDocument);
            auto option = selectControl->GetOption(selectControl->GetSelection());
            auto it = mCallbacks.find(option->GetValue().CString());

            if(mCallbacks.end() == it)
                return true;

            mCurrentEntry = it->second;

            // set slider to init position
            setSliderActive(true);
            auto slider = findSliderControl(mDocument);
            float value = 100.f * (mCurrentEntry.init - mCurrentEntry.min) / (mCurrentEntry.max - mCurrentEntry.min);
            slider->SetValue(Ogre::StringConverter::toString(value).c_str());

            Debug::log(intro)("chose ")(option->GetValue()).endl();
        }
        else if(command[0] == "update")
        {
            if(nullptr != mCurrentEntry.callback)
            {
                Rocket::Controls::ElementFormControlInput *slider = findSliderControl(mDocument);
                float value = Ogre::StringConverter::parseReal(slider->GetValue().CString()) * .01;
                mCurrentEntry.callback(mCurrentEntry.min + value * (mCurrentEntry.max - mCurrentEntry.min));
            }
            else
            {
                Debug::log(intro)("entry callback is null in command ").quotes(command)(". Skipping.").endl();
            }
        }
        else
        {
            Debug::warning(intro)("unknown command ").quotes(command).endl();
            return false;
        }

        return true;
    }

    Rocket::Controls::ElementFormControlInput *DebugValueManager::findSliderControl(Rocket::Core::ElementDocument *document)
    {
        auto select = findSelectControl(document);
        return nullptr == select ? nullptr : static_cast<Rocket::Controls::ElementFormControlInput *>(select->GetParentNode()->GetElementById("debugvaluemanager_slider"));
    }

}
