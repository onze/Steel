#ifndef STEEL_DEBUGVALUEMANAGER_H
#define STEEL_DEBUGVALUEMANAGER_H

#include <functional>

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
        void addDebugValue(Ogre::String const &entryName,
                           CallbackFunction callback,
                           float min = .0f, float max = 1.f, float init = -1.f);
        void removeDebugValue(Ogre::String const &entryName);

    private:
        class Entry
        {
        public:
            Entry(): callback(nullptr), min(.0f), max(1.f), init(-1.f)
            {
                if(init < .0f)
                    init = (min + max) / 2.f;
            }
            Entry(CallbackFunction _callback, float _min, float _max, float _init): callback(_callback), min(_min), max(_max), init(_init)
            {
                if(init < .0f)
                    init = (min + max) / 2.f;
            }
            CallbackFunction callback;
            float min;
            float max;
            float init;
        };

        Rocket::Controls::ElementFormControlDataSelect *findSelectControl(Rocket::Core::ElementDocument *document);
        Rocket::Controls::ElementFormControlInput *findSliderControl(Rocket::Core::ElementDocument *document);
        void setFeedback(Ogre::String const& value);
        void setSliderActive(bool flag);
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
