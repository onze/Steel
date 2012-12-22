/*
 * File.cpp
 *
 *  Created on: Dec 13, 2011
 *      Author: onze
 */

#include <fstream>

#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <iostream>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <ftw.h>

#include "tools/File.h"
#include "Debug.h"
#include "Poco/DirectoryIterator.h"

namespace Steel
{
    // the syscall read is hidden by File::read in Steel::File's scope :p
    auto SYSCALL_ALIAS_read=read;

#if defined(_WIN32)
    auto SYSCALL_ALIAS_mkdir=_mkdir;
#else
    auto SYSCALL_ALIAS_mkdir=mkdir;
#endif

    // init static values
    int File::sInotifyFD=-1;
    std::mutex File::sStaticLock;
    std::vector<int> File::sWatches;
    std::vector<File *> File::sFiles;
    std::list<File *> File::sNotificationList;
    //std::thread File::sNotifier(File::poolAndNotify);


    File::File() :
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" ), mListeners(std::set<FileEventListener *>())
    {
    }

    File::File(const char *fullpath):
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" ), mListeners(std::set<FileEventListener *>())
    {
        Ogre::StringUtil::splitFullFilename ( Ogre::String(fullpath), mBaseName, mExtension, mPath );
        // #ifdef __unix
        //         if(mPath.at(0)!="/")
        //             mPath=File::getCurrentDirectory()+mPath;
        // #else
        // #warning bool File::File(const char *fullpath) is not implemented for your platform.
        //         throw std::runtime_exception("File::File(const char *fullpath) is not implemented for your platform.");
        // #endif
    }

    File::File ( Ogre::String fullPath ) :
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" ), mListeners(std::set<FileEventListener *>())
    {
        //mPath will end with a slash
        Ogre::StringUtil::splitFullFilename ( fullPath, mBaseName, mExtension, mPath );
        // #ifdef __unix
        //         if(mPath.at(0)!="/")
        //             mPath=File::getCurrentDirectory()+mPath;
        // #else
        // #warning bool File::File(const char *fullpath) is not implemented for your platform.
        //         throw std::runtime_exception("File::File(const char *fullpath) is not implemented for your platform.");
        // #endif
    }

    File::File ( File const &f ) :
        mPath ( f.mPath ), mBaseName ( f.mBaseName ), mExtension ( f.mExtension ),mListeners(std::set<FileEventListener *>())
    {
    }

    File::~File()
    {
    }

    File &File::operator= ( File const &f )
    {
        mPath = f.mPath;
        mBaseName = f.mBaseName;
        mExtension = f.mExtension;
        return *this;
    }

    bool File::operator!= ( File const &o )
    {
        return fullPath()!=o.fullPath();
    }

    int File::init()
    {
        if(sInotifyFD>=0)
            return 0;
        sInotifyFD=inotify_init();
        if(sInotifyFD<0)
            return sInotifyFD;
        std::thread t(File::poolAndNotify);
        t.detach();
        return 0;
    }

    Ogre::String File::extension()
    {
        return mExtension;
    }

    Ogre::String File::path()
    {
        return mPath;
    }

    void File::shutdown()
    {
        //TODO: stop static thread ?
    }

    File File::parentDir()
    {
        return File(mPath);
    }

    void File::mkdir()
    {
        Ogre::String path=fullPath();
        if(isFile())
            path=parentDir().fullPath();
// http://stackoverflow.com/a/10356780
#if defined(_WIN32)
        SYSCALL_ALIAS_mkdir(path.c_str());
#else
        SYSCALL_ALIAS_mkdir(path.c_str(), 0777); // notice that 777 is different than 0777
#endif
    }

    void File::poolAndNotify()
    {
        //http://stackoverflow.com/questions/5211993/using-read-with-inotify
        while(true)
        {
            unsigned int avail;
            ioctl(sInotifyFD, FIONREAD,&avail);

            char buffer[avail];
            SYSCALL_ALIAS_read(sInotifyFD,buffer,avail);

            int offset=0;
            int pos;
            File *file;
            while(offset<(int)avail)
            {
                struct inotify_event *event=(inotify_event *)(buffer+offset);
                sStaticLock.lock();
                pos=std::distance(sWatches.begin(),std::find(sWatches.begin(),sWatches.end(),event->wd));
                if(pos==(int)sWatches.size())
                {
                    Debug::warning("File::poolAndNotify(): found no File watching ")(event->name)(". Ignoring notification.").endl();
                    continue;
                }
                file=sFiles.at(pos);
                // TODO: use file md5 to make sure it actually changed
                if(file->fileName()==event->name && (event->mask & IN_MODIFY))
                {
                    sNotificationList.push_back(file);
                }
                sStaticLock.unlock();
//                 Debug::log("File::poolAndNotify(): wd: ")(event->wd)(" file: ")(event->name)("(")(file->fullPath())("), mask: ")(event->mask).endl();

                offset+=sizeof(inotify_event)+event->len;
            }
        }
    }

    /*Ogre::String File::relpath(File comp)
    {
        return "";
    }*/

    void File::dispatchToFiles()
    {
        while (!sNotificationList.empty())
        {
            sStaticLock.lock();
            auto file=sNotificationList.front();
            sNotificationList.pop_front();
            sStaticLock.unlock();
            file->dispatchToListeners();
        }
    }

    void File::dispatchToListeners()
    {
        for(auto it=mListeners.begin(); it!=mListeners.end(); ++it)
            (*it)->onFileChangeEvent(this);
    }

    void File::addFileListener(FileEventListener *listener)
    {
        mListeners.insert(listener);
        if(mListeners.size()==1)
            startWatching();
    }

    void File::removeFileListener(FileEventListener *listener)
    {
        mListeners.erase(listener);
        if(mListeners.size()==0)
            stopWatching();
    }

    void File::startWatching()
    {
        sStaticLock.lock();
        //IN_MODIFY | IN_CREATE | IN_DELETE
        mWD=inotify_add_watch(sInotifyFD, mPath.c_str(), IN_MODIFY);
        sWatches.push_back(mWD);
        sFiles.push_back(this);
        sStaticLock.unlock();
    }

    void File::stopWatching()
    {
        sStaticLock.lock();
        inotify_rm_watch(sInotifyFD, mWD);
        std::vector<File *>::iterator pos=std::find(sFiles.begin(),sFiles.end(),this);
        if(pos!=sFiles.end())
        {
            int index=std::distance(sFiles.begin(), pos);
            sWatches.erase(sWatches.begin()+index);
            sFiles.erase(sFiles.begin()+index);
        }
        sStaticLock.unlock();
    }

    File File::getCurrentDirectory()
    {
        char _cwd[1024];
        if ( getcwd ( _cwd, sizeof ( _cwd ) ) == 0 )
        {
            Debug::error("File::getCurrentDirectory(): getcwd() error:" )(errno).endl();
            assert ( false );
        }
        Ogre::String s ( _cwd );
        return File ( s );
    }

    bool File::exists()
    {
        std::ifstream ifile ( fullPath().c_str() );
        return ifile;
    }

    Ogre::String File::fileName() const
    {
        Ogre::String s = mBaseName;
        if ( mExtension.size() > 0 )
            s.append ( "." + mExtension );
        return s;
    }

    Ogre::String File::fullPath() const
    {
        return mPath + fileName();
    }

    bool File::isDir()
    {
#ifdef __unix
        //http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//	Debug::log("File<")(fullPath())(">::isDir()").endl();
        int status;
        struct stat st_buf;

        status = stat ( fullPath().c_str(), &st_buf );

        if ( status != 0 )
        {
            return false;
            Debug::error ( "File<" ) ( fullPath() ) ( ">::isDir():" );
            Debug::error ( " can't status the file. errno:" ) ( errno ).endl();
        }

        return S_ISDIR ( st_buf.st_mode );
#else
#warning bool File::isDir() is not implemented for your platform.
        throw std::runtime_exception("bool File::isDir() is not implemented for your platform.");
#endif
    }

    bool File::isValid()
    {
        std::fstream f ( fullPath().c_str() );
        return f.good();
    }

    bool File::isFile()
    {
#ifdef __unix
        //http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//	Debug::log("File<")(fullPath())(">::isFile()").endl();
        int status;
        struct stat st_buf;

        status = stat ( fullPath().c_str(), &st_buf );

        if ( status != 0 )
        {
            return false;
            Debug::error ( "File<" ) ( fullPath() ) ( ">::isDir():" );
            Debug::error ( " can't status the file. errno:" ) ( errno ).endl();
        }

        return S_ISREG ( st_buf.st_mode );
#else
#warning bool File::isFile() is not implemented for your platform.
        throw std::runtime_exception("bool File::isFile() is not implemented for your platform.");
#endif
    }

    std::vector<File> File::ls(File::NodeType filter, bool include_hidden)
    {
        if(!(exists() && isDir()))
            return std::vector<File>(0);

        std::vector<std::string> files;
        Poco::File(fullPath()).list(files);;
        std::list<File> nodes;
        for(auto it=files.begin(); it!=files.end(); ++it)
        {
            File file=subfile(*it);
            NodeType nodeType=file.nodeType();
            if(nodeType&filter)
            {
                if(!include_hidden && (nodeType&HIDDEN))
                    continue;
                nodes.push_back(file);
            }
        }
        std::vector<File> vecnodes;
        for(auto it=nodes.begin(); it!=nodes.end(); ++it)
            vecnodes.push_back(*it);
        return vecnodes;
    }

    File::NodeType File::nodeType()
    {
        NodeType type=static_cast<NodeType>(0);
        if(exists())
        {
            if(isDir())type=type|File::DIR;
            if(isFile())type=type|File::FILE;
            if(fileName().at(0)=='.')type=type|File::HIDDEN;
        }
        else
            return File::ANY;
        return type;
    }

    Ogre::String File::read()
    {

        std::ifstream s;
        s.open ( fullPath().c_str() );

        s.seekg ( 0, std::ios::end );
        std::ios::pos_type fileLength = s.tellg();
        s.seekg ( 0, std::ios::beg );

        char *fileData = new char[fileLength+(std::ios::pos_type)1];

        s.read ( fileData, ( int ) fileLength );
        fileData[fileLength]='\0';
        return Ogre::String ( fileData );
    }

    File &File::write ( Ogre::String buffer )
    {
        std::ofstream s;
        s.open ( fullPath().c_str() );
        s.write ( buffer.c_str(), buffer.length() * sizeof ( char ) );
        return *this;
    }

    File File::subfile ( Ogre::String const filename ) const
    {
#ifdef __unix
        return File ( fullPath() + "/" + filename );
#else
        return File ( fullPath() + "\\" + filename );
#endif
    }

    void File::setPath(Ogre::String path)
    {
        if(!mListeners.empty())
            stopWatching();
        mPath=path;
        if(!mListeners.empty())
            startWatching();
    }
} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
