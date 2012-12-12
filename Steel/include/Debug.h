/*
 * Debug.h
 *
 *  Created on: 2011-06-21
 *      Author: onze
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <iostream>

#include <OgreLog.h>
#include <OgreLogManager.h>
#include <OgreString.h>
#include <OgreStringConverter.h>
#include <Rocket/Core/String.h>

namespace Steel
{

    class Debug
    {
        public:
            Debug();
            virtual ~Debug();
        public:
            class DebugObject
            {
                public:
                    static Ogre::String sIndentString;
                    DebugObject()
                    {
                        mLog = NULL;
                        mMsg = "";
                        mIndents=0;
                    }
                    DebugObject(Ogre::Log *log)
                    {
                        mLog = log;
                        mIndents=0;
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
                    void setColors(Ogre::String pre = "", Ogre::String post = "")
                    {
                        mPre = pre;
                        mPost = post;
                    }

                    /**
                     * equivalent to myDebugObject.log(Ogre::String msg)
                     */

                    DebugObject &operator()(Ogre::String const &msg)
                    {
                        mMsg.append(msg);
                        return *this;
                    }

                    DebugObject &operator()(Rocket::Core::String const &msg)
                    {
                        return (*this)(Ogre::String(msg.CString()));
                    }

                    template<class T>
                    DebugObject &operator()(std::vector<T> const &container)
                    {
                        this->operator()("vec[");
                        for(auto it=container.begin(); it!=container.end(); ++it)
                        {
                            this->operator()(*it);
                            if(((T)(*it))!=((T)container.back()))
                                this->operator()(", ");
                        }
                        this->operator()("]");
                        return *this;
                    }

                    template<class T>
                    DebugObject &operator()(std::list<T> const &container)
                    {
                        this->operator()("list[");
                        for(auto it=container.begin(); it!=container.end(); ++it)
                        {
                            this->operator()(*it);
                            if(((T)(*it))!=((T)container.back()))
                                this->operator()(", ");
                        }
                        this->operator()("]");
                        return *this;
                    }

                    DebugObject &operator()(const char * msg)
                    {
                        return (*this)(Ogre::String(msg));
                    }

                    DebugObject &operator()(const Ogre::Vector3 msg)
                    {
                        return (*this)(Ogre::StringConverter::toString(msg));
                    }

                    DebugObject &operator()(const Ogre::Quaternion msg)
                    {
                        return (*this)(Ogre::StringConverter::toString(msg));
                    }

                    DebugObject &operator()(const long unsigned int msg)
                    {
                        return (*this)(Ogre::StringConverter::toString(msg));
                    }

                    DebugObject &endl()
                    {
                        mLog->logMessage(mPre + mMsg + mPost);
                        mMsg.clear();
                        for(int i=0; i<mIndents; ++i)
                            mMsg.append(DebugObject::sIndentString);
                        return *this;
                    }

                    DebugObject &indent()
                    {
                        mIndents+=1;
                        if(mMsg=="")
                            for(int i=0; i<mIndents; ++i)
                                mMsg.append(DebugObject::sIndentString);
                        return *this;
                    }

                    DebugObject &unIndent()
                    {
                        mIndents=mIndents>0?mIndents-1:0;
                        if(mIndents>0 && mMsg=="")
                            for(int i=0; i<mIndents; ++i)
                                mMsg.append(DebugObject::sIndentString);
                        return *this;
                    }

                    DebugObject &resetIndent()
                    {
                        mIndents=0;
                        return *this;
                    }
                protected:
                    Ogre::Log *mLog;
                    Ogre::String mMsg;
                    Ogre::String mPre;
                    Ogre::String mPost;
                    int mIndents;
            }; //end of class DebugObject

        public:
            ///default log in direct access
            static DebugObject log;

            ///warning  log in direct access
            static DebugObject warning;

            ///error log in direct access
            static DebugObject error;

            /**
             * Initialise the debug system, linking it to 3 Ogre::Log instances (default, warnings, errors).
             */
            static void init(Ogre::String defaultLogName, Ogre::LogListener *logListener)
            {
                Ogre::LogManager *olm = new Ogre::LogManager();
                Ogre::Log *defaultLog = olm->createLog(defaultLogName, true, true, false);

                log = DebugObject(defaultLog);
                if (logListener)
                    defaultLog->addListener(logListener);

                Ogre::Log *wlog = olm->createLog("steel_warnings.log", false, true, false);
                if (logListener)
                    wlog->addListener(logListener);
                warning = DebugObject(wlog);
                //yellow
                warning.setColors("\033[1;33m", "\033[1;m");

                Ogre::Log *elog = olm->createLog("steel_errors.log", false, true, false);
                if (logListener)
                    elog->addListener(logListener);
                error = DebugObject(elog);
                //red
                error.setColors("\033[1;31m", "\033[1;m");
            }
    };

}

#endif /* DEBUG_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

