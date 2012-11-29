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
                    DebugObject()
                    {
                        mLog = NULL;
                        mMsg = "";
                    }
                    DebugObject(Ogre::Log *log)
                    {
                        mLog = log;
                    }
                    DebugObject(const DebugObject &o)
                    {
                        mLog = o.mLog;
                        mMsg = o.mMsg;
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
//		DebugObject &log(Ogre::String const &msg)
//		{
//			return (*this)(msg);
//		}
//
//		DebugObject &log(const char * msg)
//		{
//			return (*this)(msg);
//		}
//
//		DebugObject &log(Ogre::Vector3 const &msg)
//		{
//			return (*this)(msg);
//		}
//
//		DebugObject &log(Ogre::Quaternion const &msg)
//		{
//			return (*this)(msg);
//		}
//
//		DebugObject &log(long unsigned int msg)
//		{
//			return (*this)(msg);
//		}

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

                    DebugObject &operator()(const char * msg)
                    {
                        return (*this)(Ogre::String(msg));
                    }

                    DebugObject &operator()(Ogre::Vector3 msg)
                    {
                        return (*this)(Ogre::StringConverter::toString(msg));
                    }

                    DebugObject &operator()(Ogre::Quaternion msg)
                    {
                        return (*this)(Ogre::StringConverter::toString(msg));
                    }

                    DebugObject &operator()(long unsigned int msg)
                    {
                        return (*this)(Ogre::StringConverter::toString(msg));
                    }

                    DebugObject &endl()
                    {
                        mLog->logMessage(mPre + mMsg + mPost);
                        mMsg.clear();
                        return *this;
                    }
                protected:
                    Ogre::Log *mLog;
                    Ogre::String mMsg;
                    Ogre::String mPre;
                    Ogre::String mPost;
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
