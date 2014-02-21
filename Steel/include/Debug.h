#ifndef STEEL_DEBUG_H_
#define STEEL_DEBUG_H_

#include <Rocket/Core/String.h>
#include "steeltypes.h"

namespace Steel
{

    class Action;
    class ActionCombo;
    class BTNode;
    class BTShapeStream;
    class SignalBufferEntry;
    class SignalManager;

    namespace Debug
    {

        /// Initialise the debug system, linking it to 3 Ogre::Log instances (default, warnings, errors).
        void init(Ogre::String defaultLogName, Ogre::LogListener *logListener, bool useColors = true);
        /// Displays a message telling an error message will appear and can be ignored (used in utests).
        void ignoreNextErrorMessage();

        ///false as long as Debug::init has not been called.
        extern bool isInit;

        class DebugObject
        {
        public:
            static Ogre::String sIndentString;
            DebugObject();
            DebugObject(Ogre::Log *log);
            DebugObject(const DebugObject &o);
            DebugObject &operator=(const DebugObject &o);

            Ogre::String getFileName();
            void setColors(Ogre::String pre = Ogre::StringUtil::BLANK, Ogre::String post = Ogre::StringUtil::BLANK);

            DebugObject &operator()();
            DebugObject &operator()(const Ogre::Degree msg);
            DebugObject &operator()(const Ogre::Radian msg);
            DebugObject &operator()(const Ogre::Vector2 msg);
            DebugObject &operator()(const Ogre::Vector3 msg);
            DebugObject &operator()(const Ogre::Quaternion msg);
            DebugObject &operator()(const int msg);
            DebugObject &operator()(const unsigned int msg);
            DebugObject &operator()(const long int msg);
            DebugObject &operator()(const long unsigned int msg);
            DebugObject &operator()(const float msg);
            DebugObject &operator()(const char *msg);
            DebugObject &operator()(Ogre::String const &msg);
            DebugObject &operator()(Json::Value const &msg);
            DebugObject &operator()(Rocket::Core::String const &msg);
            DebugObject &operator()(Ogre::StringVectorPtr const vec);
            DebugObject &operator()(Ogre::StringVector const &vec);
            DebugObject &operator()(BTShapeToken const &token);
            DebugObject &operator()(BTShapeStream *const shapeStream);
            DebugObject &operator()(BTNodeState state);
            DebugObject &operator()(BTNode *const node);
            DebugObject &operator()(Action const &action);
            DebugObject &operator()(ActionCombo const &combo);
            DebugObject &operator()(SignalBufferEntry const &entry);
            DebugObject &operator()(Ogre::ResourceGroupManager::LocationList const &list);
            DebugObject &operator()(Ogre::ResourceGroupManager::ResourceDeclarationList const &list);

            template<class T>
            DebugObject &operator()(std::vector<T> const &container)
            {
                this->operator()("vec[");

                for(auto it = container.begin(); it != container.end(); ++it)
                    this->operator()(*it)(", ");

                return (*this)("]");
            }

            template<class T>
            DebugObject &operator()(std::set<T> const &container)
            {
                this->operator()("set[");

                for(auto it = container.begin(); it != container.end(); ++it)
                    this->operator()(*it)(", ");

                return (*this)("]").unIndent();
            }

            template<class T>
            DebugObject &operator()(std::list<T> const &container)
            {
                this->operator()("list[");

                for(auto it = container.begin(); it != container.end(); ++it)
                    this->operator()(*it)(", ");

                return (*this)("]");
            }

            template<class T, class U>
            DebugObject &operator()(std::pair<T, U> const &pair)
            {
                return this->operator()("pair(")(pair.first)(", ")(pair.second)(")");
            }
            
            /// prints n or more parameters side by side
            template<class First, class Second, class ...Others>
            DebugObject &operator()(First first, Second second, Others... others)
            {
                (*this)(first);
                (*this)(second, others...);
                return *this;
            }

            /// Print the parameter between quotes.
            template<class T>
            DebugObject &quotes(T const &o)
            {
                return (*this)("\"")(o)("\"");
            }

            DebugObject &endl();
            DebugObject &flush(bool breakline = false);
            DebugObject &indent();
            DebugObject &unIndent();
            DebugObject &resetIndent();

            /// In debug, invoke a debugger here.
            DebugObject &breakHere();

        protected:
            Ogre::Log *mLog;
            Ogre::String mMsg;
            Ogre::String mPre;
            Ogre::String mPost;
            int mIndents;
        };

        ///default log in direct access
        extern DebugObject log;

        ///warning  log in direct access
        extern DebugObject warning;

        ///error log in direct access
        extern DebugObject error;
    };
}

#endif /* STEEL_DEBUG_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

