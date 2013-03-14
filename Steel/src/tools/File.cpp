/*
 * File.cpp
 *
 *  Created on: Dec 13, 2011
 *      Author: onze
 */

#include <fstream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
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
    std::map<File::FileEvent,File::ListenersEntry> File::sListeners;
    std::list<File::FileEvent> File::sNotificationList;
    //std::thread File::sNotifier(File::poolAndNotify);


    File::File() :
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" )
    {
    }

    File::File(const char *fullpath):
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" )
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
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" )
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
        mPath ( f.mPath ), mBaseName ( f.mBaseName ), mExtension ( f.mExtension )
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

    bool File::operator== ( File const &o ) const
    {
        return fullPath()==o.fullPath();
    }
    
    bool File::operator!= ( File const &o ) const
    {
        return !operator==(o);
    }
    
    bool File::operator< ( File const &o ) const
    {
        return fileName()<o.fileName();
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

    void File::shutdown()
    {
        sInotifyFD=-1;
    }

    Ogre::String File::extension()
    {
        return mExtension;
    }

    Ogre::String File::path()
    {
        return mPath;
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
        SYSCALL_ALIAS_mkdir(path.c_str(), 0777); // notice that 777 is different from 0777
#endif
    }

    bool File::touch()
    {
        if(isDir())
        {
            if(!exists())
                mkdir();
            return true;
        }
        Ogre::String path=fullPath();
#if defined(_WIN32)
        throw std::runtime_error("File::touch not implemented");
#else
        return open(path.c_str(), O_WRONLY|O_CREAT|O_NONBLOCK)!=-1;
#endif
        return false;
    }

    void File::rm()
    {
        if(isDir())
            throw std::runtime_error("File::rm not implemented for directories");
#if defined(_WIN32)
            throw std::runtime_error("File::rm not implemented");
#else
        unlink(fullPath().c_str());
#endif
        return;
    }

    void File::poolAndNotify()
    {
        //http://stackoverflow.com/questions/5211993/using-read-with-inotify
        while(sInotifyFD>=0)
        {
            unsigned int avail;
            ioctl(sInotifyFD, FIONREAD,&avail);

            char buffer[avail];
            SYSCALL_ALIAS_read(sInotifyFD,buffer,avail);

            int offset=0;
            while(offset<(int)avail)
            {
                struct inotify_event *iEvent=(inotify_event *)(buffer+offset);

                FileEvent event;
                sStaticLock.lock();
                if(iEvent->mask & IN_MODIFY)
                    event=FileEvent(iEvent->wd,IN_MODIFY);
                else
                    continue;

                auto it=sListeners.find(event);
                if(it==sListeners.end())
                {
                    Debug::warning("File::poolAndNotify(): found no listener for ")(event.first)(". Ignoring notification.").endl();
                    sStaticLock.unlock();
                    continue;
                }

                sNotificationList.push_back(event);
                sStaticLock.unlock();
//                 Debug::log("File::poolAndNotify(): wd: ")(event.first).endl();

                offset+=sizeof(inotify_event)+iEvent->len;
            }
        }
        sStaticLock.lock();
        sListeners.clear();
        sStaticLock.unlock();
    }

    /*Ogre::String File::relpath(File comp)
    {
        return "";
    }*/

    void File::dispatchEvents()
    {
        while (!sNotificationList.empty())
        {
            sStaticLock.lock();
            //pair wd, filepath
            FileEvent event=sNotificationList.front();

            // find listeners
            auto it_event=sListeners.find(event);
            if(it_event==sListeners.end())
            {
                sStaticLock.unlock();
                sNotificationList.pop_front();
                continue;
            }

            ListenersEntry entry=(*it_event).second;
            File file(entry.first);
            auto listeners=std::set<FileEventListener *>(entry.second.begin(),entry.second.end());
            sNotificationList.pop_front();
            sStaticLock.unlock();

            for(auto it=listeners.begin(); it!=listeners.end(); ++it)
                (*it)->onFileChangeEvent(file);
        }
    }

    void File::addFileListener(FileEventListener *listener)
    {
        File::addFileListener(this,listener);
    }

    void File::removeFileListener(FileEventListener *listener)
    {
        File::removeFileListener(this,listener);
    }

    // static
    void File::addFileListener(File *file,FileEventListener *listener)
    {
        sStaticLock.lock();
        int wd=inotify_add_watch(sInotifyFD, file->fullPath().c_str(), IN_MODIFY);
        FileEvent fileEvent(wd,IN_MODIFY);
//         Debug::log("File::addFileListener(): getting wd ")(wd)(" for path ")(file->fullPath()).endl();
        auto it=sListeners.find(fileEvent);
        if(it==sListeners.end())
        {
            auto entry=ListenersEntry(file->fullPath(),std::set<FileEventListener *>());
            sListeners.insert(std::pair<FileEvent,ListenersEntry>(fileEvent,entry));
            it=sListeners.find(fileEvent);
        }
        std::set<FileEventListener *> *record=&((*it).second.second);
        record->insert(listener);
        sStaticLock.unlock();
    }

    // static
    void File::removeFileListener(File *file,FileEventListener *listener)
    {
        sStaticLock.lock();
        int wd=inotify_add_watch(sInotifyFD, file->fullPath().c_str(), IN_MODIFY);
//         Debug::log("File::removeFileListener(): getting wd ")(wd)(" for path ")(file->fullPath()).endl();
        FileEvent fileEvent(wd,IN_MODIFY);
        auto it=sListeners.find(fileEvent);
        if(it!=sListeners.end())
        {
            std::set<FileEventListener *> *record=&((*it).second.second);
            record->erase(listener);
            if(record->size()==0)
                sListeners.erase(fileEvent);
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

    Ogre::String File::fileBaseName() const
    {
        return mBaseName;
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
        throw std::runtime_error("bool File::isDir() is not implemented for your platform.");
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
        throw std::runtime_error("bool File::isFile() is not implemented for your platform.");
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

    File &File::write(Ogre::String buffer, std::ios_base::openmode mode)
    {
        std::ofstream s;
        s.open(fullPath().c_str(), mode);
        s.write(buffer.c_str(), buffer.length()*sizeof(char));
        return *this;
    }

    File File::subfile(Ogre::String const filename) const
    {
#ifdef __unix
        return File ( fullPath() + "/" + filename );
#else
        return File ( fullPath() + "\\" + filename );
#endif
    }

    void File::setPath(Ogre::String path)
    {
        mPath=path;
    }
} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
