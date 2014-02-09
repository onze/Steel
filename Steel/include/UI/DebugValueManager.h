#ifndef STEEL_DEBUGVALUEMANAGER_H
#define STEEL_DEBUGVALUEMANAGER_H

#include <map>
#include <vector>
#include <functional>

#include <OgreHeaderSuffix.h>
#include <steeltypes.h>

namespace Rocket
{
    namespace Core
    {
        class Element;
        class ElementDocument;
    }

    namespace Controls
    {
        class ElementFormControlDataSelect;
        class ElementFormControlInput;
    }
}


namespace Steel
{
    class Editor;
    class DebugValueManager
    {
    public:
        typedef std::function<void(float)> CallbackFunction;

        DebugValueManager();
        DebugValueManager(const DebugValueManager &o);
        ~DebugValueManager();
        DebugValueManager &operator=(const DebugValueManager &o);

        void init(Ogre::String const selectControlId, Rocket::Core::ElementDocument *document);
        void shutdown();
        void refresh(Rocket::Core::ElementDocument *const document);

        bool processCommand(std::vector<Ogre::String> command);

        /**
         * Adds the given entry in the editor's DebugValues panel. The given function will be called when the UI control gets updated
         * with a float value between 0 and 1..
         */
        void addDebugValue(Ogre::String const &entryName, CallbackFunction callback, float min = .0f, float max = 1.f);
        void removeDebugValue(Ogre::String const &entryName);

    private:
        class Entry
        {
        public:
            Entry(): callback(nullptr), min(.0f), max(1.f) {}
            Entry(CallbackFunction _callback, float _min, float _max): callback(_callback), min(_min), max(_max) {}
            CallbackFunction callback;
            float min;
            float max;
        };
        
        Rocket::Controls::ElementFormControlDataSelect *findSelectControl(Rocket::Core::ElementDocument *document);
        Rocket::Controls::ElementFormControlInput *findSliderControl(Rocket::Core::ElementDocument *document);
        /// Actually adds an entry
        int addOption(const Ogre::String &entryName);

        // not owned
        Rocket::Core::ElementDocument *mDocument;

        //owned

        std::map<Ogre::String, Entry> mCallbacks;
        Ogre::String mSelectControlId;
        Entry mCurrentEntry;
    };
}
#endif // DEBUGVALUEMANAGER_H
