#ifndef STEEL_DEBUG_H_
#define STEEL_DEBUG_H_

#include <iostream>
#include <json/value.h>

#include <OgreLog.h>
#include <OgreLogManager.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <OgreResourceManager.h>
#include <Rocket/Core/String.h>

#include "steeltypes.h"
#include "BT/btnodetypes.h"
#include "BT/BTNode.h"
#include "tools/StringUtils.h"

namespace Steel
{

    class Debug
    {
    public:
        Debug();
        virtual ~Debug();
        class DebugObject
        {
        public:
            static Ogre::String sIndentString;
            DebugObject()
            {
                mLog = nullptr;
                mMsg = Ogre::StringUtil::BLANK;
                mIndents = 0;
            }
            DebugObject(Ogre::Log *log)
            {
                mLog = log;
                mIndents = 0;
            }
            DebugObject(const DebugObject &o)
            {
                mLog = o.mLog;
                mMsg = o.mMsg;
                mIndents = o.mIndents;
            }
            DebugObject &operator=(const DebugObject &o)
            {
                mLog = o.mLog;
                mMsg = o.mMsg;
                mIndents = o.mIndents;
                return *this;
            }
            Ogre::String getFileName()
            {
                return mLog->getName();
            }
            void setColors(Ogre::String pre = Ogre::StringUtil::BLANK, Ogre::String post = Ogre::StringUtil::BLANK)
            {
                mPre = pre;
                mPost = post;
            }

            /// Empty call. Allows build-safe typos.
            DebugObject &operator()()
            {
                return *this;
            }

            /**
             * equivalent to myDebugObject.log(Ogre::String msg)
             */
            DebugObject &operator()(const Ogre::Degree msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg.valueDegrees()) + "deg");
            }

            DebugObject &operator()(const Ogre::Radian msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg.valueRadians()) + "rad");
            }

            DebugObject &operator()(const Ogre::Vector2 msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const Ogre::Vector3 msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const Ogre::Quaternion msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const int msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const unsigned int msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const long int msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const long unsigned int msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const float msg)
            {
                return (*this)(Ogre::StringConverter::toString(msg));
            }

            DebugObject &operator()(const char *msg)
            {
                return (*this)(Ogre::String(msg));
            }

            DebugObject &operator()(Ogre::String const &msg)
            {
                mMsg.append(msg);
                return *this;
            }

            DebugObject &operator()(Json::Value const &msg)
            {
                mMsg.append(msg.toStyledString());
                return *this;
            }

            DebugObject &operator()(Rocket::Core::String const &msg)
            {
                return (*this)(Ogre::String(msg.CString()));
            }

            DebugObject &operator()(Ogre::StringVectorPtr const vec)
            {
                if(vec.isNull())
                    return (*this)(*vec);
                else
                    return (*this)("nullptr ptr !");
            }

            DebugObject &operator()(Ogre::StringVector const &vec)
            {
                return (*this)(vec);
            }

            DebugObject &operator()(BTShapeToken const &token)
            {
                (*this)("BTShapeToken{type: ")(StringUtils::BTShapeTokenTypeToString(token.type));
                (*this)(", begin: ")(token.begin);
                (*this)(", end: ")(token.end);
                (*this)(", contentFile: ")(token.contentFile);
                return (*this)("}");
            }

            DebugObject &operator()(BTShapeStream *const shapeStream)
            {
                this->operator()("BTShapeStream[").endl().indent();

                for(auto it = shapeStream->mData.begin(); it != shapeStream->mData.end(); ++it)
                    (*this)(*it)(", ").endl();

                this->operator()("]").unIndent();
                return *this;
            }

            DebugObject &operator()(BTNode *const node)
            {
                if(nullptr == node)
                {
                    (*this)("BTNode{ // nullptr pointer !}");
                }
                else
                {
                    this->operator()("BTNode{").endl().indent();
                    (*this)("begin:")(node->begin())(", ").endl();
                    (*this)("end:")(node->end())(", ").endl();
                    (*this)("state:")(node->state())(", ").endl();
                    (*this)("token:")(node->token())(", ").endl();
                    this->operator()("}").unIndent();
                }

                return *this;
            }

            DebugObject &operator()(Ogre::ResourceGroupManager::LocationList const &list)
            {
                this->operator()("list[");

                for(auto it = list.begin(); it != list.end(); ++it)
                {
                    this->operator()((*it)->archive->getName())(", ");
                }

                this->operator()("]");
                return *this;
            }

            DebugObject &operator()(Ogre::ResourceGroupManager::ResourceDeclarationList const &list)
            {
                this->operator()("list[");

                for(auto it = list.begin(); it != list.end(); ++it)
                {
                    this->operator()((*it).resourceName)(", ");
                }

                this->operator()("]");
                return *this;
            }

            template<class T>
            DebugObject &operator()(std::vector<T> const &container)
            {
                this->operator()("vec[");

                for(auto it = container.begin(); it != container.end(); ++it)
                {
                    this->operator()(*it)(", ");
//                             if(((T)(*it))!=((T)container.back()))
//                                 this->operator()(", ");
                }

                this->operator()("]");
                return *this;
            }

            template<class T>
            DebugObject &operator()(std::set<T> const &container)
            {
                this->operator()("set[");

                for(auto it = container.begin(); it != container.end(); ++it)
                {
                    this->operator()(*it)(", ");
                    //                             if(((T)(*it))!=((T)container.back()))
                    //                                 this->operator()(", ");
                }

                this->operator()("]");
                return *this;
            }

            template<class T>
            DebugObject &operator()(std::list<T> const &container)
            {
                this->operator()("list[");

                for(auto it = container.begin(); it != container.end(); ++it)
                {
                    this->operator()(*it)(", ");
//                             if(((T)(*it))!=((T)container.back()))
//                                 this->operator()(", ");
                }

                this->operator()("]");
                return *this;
            }

            /// Print the parameter between quotes.
            template<class T>
            DebugObject &quotes(T const &o)
            {
                (*this)("\"")(o)("\"");
                return *this;
            }

            DebugObject &endl()
            {
//                         std::replace(mMsg.begin(),mMsg.end(),"\n","\n\t");
                if(nullptr == mLog)
                {
                    std::cout << "[RAW]" << mPre << mMsg << mPost << std::endl;
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

            DebugObject &indent()
            {
                mIndents += 1;

                if(mMsg == Ogre::StringUtil::BLANK)
                    for(int i = 0; i < mIndents; ++i)
                        mMsg.append(DebugObject::sIndentString);

                return *this;
            }

            DebugObject &unIndent()
            {
                mIndents = mIndents > 0 ? mIndents - 1 : 0;

                if(mIndents > 0 && mMsg == Ogre::StringUtil::BLANK)
                    for(int i = 0; i < mIndents; ++i)
                        mMsg.append(DebugObject::sIndentString);

                return *this;
            }

            DebugObject &resetIndent()
            {
                mIndents = 0;
                return *this;
            }

        protected:
            Ogre::Log *mLog;
            Ogre::String mMsg;
            Ogre::String mPre;
            Ogre::String mPost;
            int mIndents;
        }; //end of class DebugObject

        ///default log in direct access
        static DebugObject log;

        ///warning  log in direct access
        static DebugObject warning;

        ///error log in direct access
        static DebugObject error;

        ///false as long as Debug::init has not been called.
        static bool isInit;

        /// Initialise the debug system, linking it to 3 Ogre::Log instances (default, warnings, errors).
        static void init(Ogre::String defaultLogName, Ogre::LogListener *logListener, bool useColors = true)
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

            Debug::isInit = true;
        }

        // preset messages
        /// Displays a message telling an error message will appear and can be ignored (used in utests).
        static void ignoreNextErrorMessage();
    };

}

#endif /* STEEL_DEBUG_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

