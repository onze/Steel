/*
 * Debug.cpp
 *
 *  Created on: 2011-06-21
 *      Author: onze
 */

#include <Ogre.h>
#include <json/json.h>
#include <MyGUI_Widget.h>

#include "Debug.h"
#include "SignalManager.h"
#include "InputSystem/InputBuffer.h"
#include "InputSystem/Action.h"
#include "InputSystem/ActionCombo.h"
#include "tools/StringUtils.h"
#include "BT/BTNode.h"

namespace Steel
{
    namespace Debug
    {
        bool isInit = false;
        DebugObject log;
        DebugObject warning;
        DebugObject error;

        Ogre::String DebugObject::sIndentString = Ogre::String("    ");

        void init(Ogre::String defaultLogName, Ogre::LogListener *logListener, bool useColors/* = true*/, bool ogreConsoleOutput/* = true*/)
        {
            Ogre::LogManager *olm = new Ogre::LogManager();
            Ogre::Log *defaultLog = olm->createLog(defaultLogName, true, ogreConsoleOutput, false);

            log = DebugObject(defaultLog, ogreConsoleOutput);

            if(logListener)
                defaultLog->addListener(logListener);

            Ogre::Log *wlog = olm->createLog("steel_warnings.log", false, true, false);

            if(logListener)
                wlog->addListener(logListener);

            warning = DebugObject(wlog);

            //yellow
            if(useColors)
                warning.setColors("\033[1;33m", "\033[1;m");

            Ogre::Log *elog = olm->createLog("steel_errors.log", false, true, false);

            if(logListener)
                elog->addListener(logListener);

            error = DebugObject(elog);

            //red
            if(useColors)
                error.setColors("\033[1;31m", "\033[1;m");

            isInit = true;
        }

        void ignoreNextErrorMessage()
        {
            warning("next error can be safely ignored").endl();
        }

        DebugObject::DebugObject(): mLog(nullptr), mMsg(StringUtils::blank()), mIndents(0), mOgreConsoleOutput(true)
        {
        }

        DebugObject::DebugObject(Ogre::Log *log, bool ogreConsoleOutput): mLog(log), mMsg(StringUtils::blank()), mIndents(0), mOgreConsoleOutput(ogreConsoleOutput)
        {
        }

        DebugObject::DebugObject(const DebugObject &o): mLog(o.mLog), mMsg(o.mMsg), mIndents(o.mIndents), mOgreConsoleOutput(o.mOgreConsoleOutput)
        {
        }

        DebugObject &DebugObject::operator=(const DebugObject &o)
        {
            if(&o != this)
            {
                mLog = o.mLog;
                mMsg = o.mMsg;
                mIndents = o.mIndents;
                mOgreConsoleOutput = o.mOgreConsoleOutput;
            }

            return *this;
        }

        Ogre::String DebugObject::getFileName()
        {
            return mLog->getName();
        }

        void DebugObject::setColors(Ogre::String pre/* = StringUtils::BLANK*/, Ogre::String post/* = StringUtils::BLANK*/)
        {
            mPre = pre;
            mPost = post;
        }

        DebugObject &DebugObject::operator()()
        {
            return *this;
        }

        /**
         * equivalent to myDebugObject.log(Ogre::String msg)
         */
        DebugObject &DebugObject::operator()(const Ogre::Degree msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg.valueDegrees()) + "deg");
        }

        DebugObject &DebugObject::operator()(const Ogre::Radian msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg.valueRadians()) + "rad");
        }

        DebugObject &DebugObject::operator()(const Ogre::Vector2 msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const Ogre::Vector3 msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const Ogre::Quaternion msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const int msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const unsigned int msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const long int msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const long unsigned int msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const float msg)
        {
            return (*this)(Ogre::StringConverter::toString(msg));
        }

        DebugObject &DebugObject::operator()(const char *msg)
        {
            return (*this)(Ogre::String(msg));
        }

        DebugObject &DebugObject::operator()(Ogre::String const &msg)
        {
            mMsg.append(msg);
            return *this;
        }

        DebugObject &DebugObject::operator()(Json::Value const &msg)
        {
            return (*this)(msg.toStyledString());
        }

        DebugObject &DebugObject::operator()(MyGUI::Widget const *const widget)
        {
            return (*this)("MyGUI::Widget{",
                           "typeName: ", widget->getTypeName(), ", ",
                           "name: ", widget->getName(),
                           "}");
        }

        DebugObject &DebugObject::operator()(Rocket::Core::String const &msg)
        {
            return (*this)(Ogre::String(msg.CString()));
        }

        DebugObject &DebugObject::operator()(Ogre::StringVectorPtr const vec)
        {
            return vec.isNull() ? (*this)("nullptr ptr !") : (*this)(*vec);
        }

        DebugObject &DebugObject::operator()(Ogre::StringVector const &vec)
        {
            return (*this)(vec);
        }

        DebugObject &DebugObject::operator()(File const &file)
        {
            Ogre::String const abspath = file.absPath();
            if(file.isPathAbsolute())
                return (*this)(abspath);

            Ogre::String const fullpath = file.fullPath();
            return (*this)("(", abspath.substr(0, abspath.length() - fullpath.length()), ")", fullpath);
        }

        DebugObject &DebugObject::operator()(BTShapeToken const &token)
        {
            return (*this)("BTShapeToken{type: ", toString(token.type),
                           ", begin: ", token.begin,
                           ", end: ", token.end,
                           ", contentFile: ", token.contentFile,
                           "}");
        }

        DebugObject &DebugObject::operator()(BTShapeStream *const shapeStream)
        {
            (*this)("BTShapeStream[").endl().indent();

            for(auto it = shapeStream->mData.begin(); it != shapeStream->mData.end(); ++it)
                (*this)(*it)(", ").endl();

            return (*this)("]").unIndent();
        }

        DebugObject &DebugObject::operator()(BTNodeState state)
        {
            return (*this)(BTNodeStateAsString[static_cast<int>(state)]);
        }

        DebugObject &DebugObject::operator()(BTNode *const node)
        {
            if(nullptr == node)
            {
                (*this)("BTNode{ // nullptr pointer !}");
            }
            else
            {
                (*this)("BTNode{").endl().indent()
                ("begin: ", node->begin(), ", ").endl()
                ("end: ", node->end(), ", ").endl()
                ("state: ", node->state(), ", ").endl()
                ("token: ", node->token(), ", ").endl()
                ("}").unIndent();
            }

            return *this;
        }

        DebugObject &DebugObject::operator()(Action const &action)
        {
            Json::Value value;
            action.toJson(value);
            return (*this)("Action")(value);
        }

        DebugObject &DebugObject::operator()(ActionCombo const &combo)
        {
            Json::Value value;
            combo.toJson(value);
            return (*this)("ActionCombo")(value);
        }

        DebugObject &DebugObject::operator()(SignalBufferEntry const &entry)
        {
            return (*this)("(")(SignalManager::instance().fromSignal(entry.signal))(", ")(entry.timestamp)(")");
        }

        DebugObject &DebugObject::operator()(Ogre::ResourceGroupManager::LocationList const &list)
        {
            (*this)("Ogre::ResourceGroupManager::LocationList[");

            for(auto it = list.begin(); it != list.end(); ++it)
                this->operator()((*it)->archive->getName())(", ");

            return (*this)("]");
        }

        DebugObject &DebugObject::operator()(Ogre::ResourceGroupManager::ResourceDeclarationList const &list)
        {
            (*this)("Ogre::ResourceGroupManager::ResourceDeclarationList[");

            for(auto it = list.begin(); it != list.end(); ++it)
                this->operator()((*it).resourceName)(", ");

            return (*this)("]");
        }

        DebugObject &DebugObject::endl()
        {
            return flush(true);
        }

        DebugObject &DebugObject::flush(bool breakline/* = false*/)
        {
            //                         std::replace(mMsg.begin(),mMsg.end(),"\n","\n\t");
            if(mOgreConsoleOutput && nullptr != mLog)
            {
                mLog->logMessage(mPre + mMsg + mPost);
            }
            else
            {
                std::cout << mPre << mMsg << mPost;

                if(breakline)
                    std::cout << std::endl;
                else
                    std::cout.flush();
            }

            mMsg.clear();

            for(int i = 0; i < mIndents; ++i)
                mMsg.append(DebugObject::sIndentString);

            return *this;
        }

        DebugObject &DebugObject::indent()
        {
            mIndents += 1;

            if(mMsg == StringUtils::BLANK)
                for(int i = 0; i < mIndents; ++i)
                    mMsg.append(DebugObject::sIndentString);

            return *this;
        }

        DebugObject &DebugObject::unIndent()
        {
            mIndents = mIndents > 0 ? mIndents - 1 : 0;

            if(mIndents > 0 && mMsg == StringUtils::BLANK)
                for(int i = 0; i < mIndents; ++i)
                    mMsg.append(DebugObject::sIndentString);

            return *this;
        }

        DebugObject &DebugObject::resetIndent()
        {
            mIndents = 0;
            return *this;
        }

        DebugObject &DebugObject::breakHere()
        {
#ifdef DEBUG
            /**
             * From here: http://hg.mozilla.org/mozilla-central/file/98fa9c0cff7a/js/src/jsutil.cpp#l66
             * Found there: http://stackoverflow.com/questions/4326414/set-breakpoint-in-c-or-c-code-programmatically-for-gdb-on-linux
             */
#if defined(WIN32)
            /*
             * We used to call DebugBreak() on Windows, but amazingly, it causes
             * the MSVS 2010 debugger not to be able to recover a call stack.
             */
            *((int *) NULL) = 0;
            *   exit(3);
#elif defined(__APPLE__)
            /*
             * On Mac OS X, Breakpad ignores signals. Only real Mach exceptions are
             * trapped.
             */
            *((int *) NULL) = 0;  /* To continue from here in GDB: "return" then "continue". */
            raise(SIGABRT);  /* In case above statement gets nixed by the optimizer. */
#else
            raise(SIGABRT);  /* To continue from here in GDB: "signal 0". */
#endif
#endif
            return *this;
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
