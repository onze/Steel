/*
 * Debug.cpp
 *
 *  Created on: 2011-06-21
 *      Author: onze
 */

#include <Ogre.h>
#include <json/json.h>

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

        void init(Ogre::String defaultLogName, Ogre::LogListener *logListener, bool useColors/* = true*/)
        {
            Ogre::LogManager *olm = new Ogre::LogManager();
            Ogre::Log *defaultLog = olm->createLog(defaultLogName, true, true, false);

            log = DebugObject(defaultLog);

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

        DebugObject::DebugObject()
        {
            mLog = nullptr;
            mMsg = Ogre::StringUtil::BLANK;
            mIndents = 0;
        }

        DebugObject::DebugObject(Ogre::Log *log)
        {
            mLog = log;
            mIndents = 0;
        }

        DebugObject::DebugObject(const DebugObject &o)
        {
            mLog = o.mLog;
            mMsg = o.mMsg;
            mIndents = o.mIndents;
        }

        DebugObject &DebugObject::operator=(const DebugObject &o)
        {
            mLog = o.mLog;
            mMsg = o.mMsg;
            mIndents = o.mIndents;
            return *this;
        }

        Ogre::String DebugObject::getFileName()
        {
            return mLog->getName();
        }

        void DebugObject::setColors(Ogre::String pre/* = Ogre::StringUtil::BLANK*/, Ogre::String post/* = Ogre::StringUtil::BLANK*/)
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
            mMsg.append(msg.toStyledString());
            return *this;
        }

        DebugObject &DebugObject::operator()(Rocket::Core::String const &msg)
        {
            return (*this)(Ogre::String(msg.CString()));
        }

        DebugObject &DebugObject::operator()(Ogre::StringVectorPtr const vec)
        {
            if(vec.isNull())
                return (*this)(*vec);
            else
                return (*this)("nullptr ptr !");
        }

        DebugObject &DebugObject::operator()(Ogre::StringVector const &vec)
        {
            return (*this)(vec);
        }

        DebugObject &DebugObject::operator()(BTShapeToken const &token)
        {
            (*this)("BTShapeToken{type: ")(toString(token.type));
            (*this)(", begin: ")(token.begin);
            (*this)(", end: ")(token.end);
            (*this)(", contentFile: ")(token.contentFile);
            return (*this)("}");
        }

        DebugObject &DebugObject::operator()(BTShapeStream *const shapeStream)
        {
            this->operator()("BTShapeStream[").endl().indent();

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
                (*this)("BTNode{").endl().indent();
                (*this)("begin:")(node->begin())(", ").endl();
                (*this)("end:")(node->end())(", ").endl();
                (*this)("state:")(node->state())(", ").endl();
                (*this)("token:")(node->token())(", ").endl();
                (*this)("}").unIndent();
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
            this->operator()("Ogre::ResourceGroupManager::LocationList[");

            for(auto it = list.begin(); it != list.end(); ++it)
                this->operator()((*it)->archive->getName())(", ");

            return (*this)("]");
        }

        DebugObject &DebugObject::operator()(Ogre::ResourceGroupManager::ResourceDeclarationList const &list)
        {
            this->operator()("Ogre::ResourceGroupManager::ResourceDeclarationList[");

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
            if(nullptr == mLog)
            {
                std::cout << "[RAW]" << mPre << mMsg << mPost;

                if(breakline)
                    std::cout << std::endl;
                else
                    std::cout.flush();
            }
            else
            {
                mLog->logMessage(mPre + mMsg + mPost);
                mMsg.clear();

                for(int i = 0; i < mIndents; ++i)
                    mMsg.append(DebugObject::sIndentString);
            }

            return *this;
        }

        DebugObject &DebugObject::indent()
        {
            mIndents += 1;

            if(mMsg == Ogre::StringUtil::BLANK)
                for(int i = 0; i < mIndents; ++i)
                    mMsg.append(DebugObject::sIndentString);

            return *this;
        }

        DebugObject &DebugObject::unIndent()
        {
            mIndents = mIndents > 0 ? mIndents - 1 : 0;

            if(mIndents > 0 && mMsg == Ogre::StringUtil::BLANK)
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
