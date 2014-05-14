#ifndef STEEL_DEBUG_H_
#define STEEL_DEBUG_H_

#include "steeltypes.h"
#include "tools/StringUtils.h"
#include "UI/UI.h"

// #define STEEL_ASSERT(EXPRESSION, ...)
// if(!EXPRESSION)
// {
//     Debug::error("STEEL_ASSERT: ", ##EXPRESSION, __VA_ARGS__).endl().breakHere();
// }

#if OGRE_COMPILER == OGRE_COMPILER_GNUC
#define __CLASS_NAME__ Steel::className(__PRETTY_FUNCTION__)
#define __METHOD_NAME__ Steel::methodName(__PRETTY_FUNCTION__)
#define STEEL_METH_INTRO (Ogre::String(__METHOD_NAME__)+": ")
#else
#define STEEL_METH_INTRO (Ogre::String(typeid(*this).name())+"::"+Ogre::String(__func__)+"(): ")
#endif

#define STEEL_FUNC_INTRO (Ogre::String(__func__)+"(): ")

namespace MyGUI
{
    class Widget;
}

namespace Steel
{
    //http://stackoverflow.com/a/15775519/2909588
    inline std::string methodName(const std::string &prettyFunction)
    {
        size_t colons = prettyFunction.find("::");
        size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
        size_t end = prettyFunction.rfind("(") - begin;

        return prettyFunction.substr(begin, end) + "()";
    }
    inline std::string className(const std::string &prettyFunction)
    {
        size_t colons = prettyFunction.find("::");

        if(colons == std::string::npos)
            return "::";

        size_t begin = prettyFunction.substr(0, colons).rfind(" ") + 1;
        size_t end = colons - begin;

        return prettyFunction.substr(begin, end);
    }

    class Action;
    class ActionCombo;
    class BTNode;
    class BTShapeStream;
    class SignalBufferEntry;
    class SignalManager;

    namespace Debug
    {

        /// Initialise the debug system, linking it to 3 Ogre::Log instances (default, warnings, errors).
        void init(Ogre::String defaultLogName, Ogre::LogListener *logListener, bool useColors = true, bool ogreConsoleOutput = true);
        /// Displays a message telling an error message will appear and can be ignored (used in utests).
        void ignoreNextErrorMessage();

        ///false as long as Debug::init has not been called.
        extern bool isInit;

        class DebugObject
        {
        public:
            static Ogre::String sIndentString;
            DebugObject();
            DebugObject(Ogre::Log *log, bool ogreConsoleOutput = true);
            DebugObject(const DebugObject &o);
            DebugObject &operator=(const DebugObject &o);

            Ogre::String getFileName();
            void setColors(Ogre::String pre = StringUtils::BLANK, Ogre::String post = StringUtils::BLANK);

            DebugObject &operator()(); // usefull in macro arg expansions (__VA_ARGS__)
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
            DebugObject &operator()(MyGUI::Widget const *const widget);
            DebugObject &operator()(Ogre::StringVectorPtr const vec);
            DebugObject &operator()(Ogre::StringVector const &vec);
            DebugObject &operator()(File const &file);
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
                this->operator()("[");

                for(auto it = container.begin(); it != container.end(); ++it)
                    this->operator()(*it)(", ");

                return (*this)("]");
            }

            template<class T>
            DebugObject &operator()(std::set<T> const &container)
            {
                this->operator()("{");

                for(auto it = container.begin(); it != container.end(); ++it)
                    this->operator()(*it)(", ");

                return (*this)("}").unIndent();
            }

            template<class T>
            DebugObject &operator()(std::list<T> const &container)
            {
                this->operator()("[");

                for(auto it = container.begin(); it != container.end(); ++it)
                    this->operator()(*it)(", ");

                return (*this)("]");
            }

            template<class T, class U>
            DebugObject &operator()(std::pair<T, U> const &pair)
            {
                return this->operator()("(")(pair.first)(", ")(pair.second)(")");
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
            bool mOgreConsoleOutput;
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

