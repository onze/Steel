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

#include <json/json.h>

#include "tools/File.h"
#include "tools/StringUtils.h"
#include "tools/FileEventListener.h"
#include "Debug.h"
#include "Poco/DirectoryIterator.h"

namespace Steel
{
    auto SYSCALL_ALIAS_read = read;

#if defined(_WIN32)
    Ogre::String const File::Separator = "\\";
#else
    Ogre::String const File::Separator = "/";
#endif

#if defined(_WIN32)
    auto SYSCALL_ALIAS_mkdir = _mkdir;
#else
    auto SYSCALL_ALIAS_mkdir = mkdir;
#endif

// init static values
    int File::sInotifyFD = -1;
    std::mutex File::sStaticLock;
    std::map<File::FileEvent, File::ListenersEntry> File::sListeners;
    std::list<File::FileEvent> File::sNotificationList;
//std::thread File::sNotifier(File::poolAndNotify);

    File::File()
        : mPath(StringUtils::BLANK), mBaseName(StringUtils::BLANK), mExtension(StringUtils::BLANK)
    {
    }

    File::File(const char *fullpath)
    {
        Ogre::StringUtil::splitFullFilename(Ogre::String(fullpath), mBaseName, mExtension, mPath);
        // #ifdef __unix
        //         if(mPath.at(0)!="/")
        //             mPath=File::getCurrentDirectory()+mPath;
        // #else
        // #warning bool File::File(const char *fullpath) is not implemented for your platform.
        //         throw std::runtime_exception("File::File(const char *fullpath) is not implemented for your platform.");
        // #endif
    }

    File::File(Ogre::String fullPath)
    {
        //mPath will end with a slash
        Ogre::StringUtil::splitFullFilename(fullPath, mBaseName, mExtension, mPath);
        // #ifdef __unix
        //         if(mPath.at(0)!="/")
        //             mPath=File::getCurrentDirectory()+mPath;
        // #else
        // #warning bool File::File(const char *fullpath) is not implemented for your platform.
        //         throw std::runtime_exception("File::File(const char *fullpath) is not implemented for your platform.");
        // #endif
    }

    File::File(File const &f)
        : mPath(f.mPath), mBaseName(f.mBaseName), mExtension(f.mExtension)
    {
    }

    File::~File()
    {
    }

    File &File::operator=(File const &f)
    {
        if(this != &f)
        {
            mPath = f.mPath;
            mBaseName = f.mBaseName;
            mExtension = f.mExtension;
        }

        return *this;
    }

    bool File::operator==(File const &o) const
    {
        return fullPath() == o.fullPath();
    }

    bool File::operator!=(File const &o) const
    {
        return !operator==(o);
    }

    bool File::operator<(File const &o) const
    {
        return fileName() < o.fileName();
    }

    int File::init()
    {
        if(sInotifyFD >= 0)
            return 0;

        sInotifyFD = inotify_init();

        if(sInotifyFD < 0)
            return sInotifyFD;

//         std::thread t(File::poolAndNotify);
//         t.detach();
        return 0;
    }

    void File::shutdown()
    {
        // stops the File::poolAndNotify loop
        sInotifyFD = -1;
    }

    Ogre::String File::path() const {return mPath;}
    Ogre::String &File::path() {return mPath;}

    Ogre::String File::fileBaseName() const {return mBaseName;}
    Ogre::String &File::fileBaseName() {return mBaseName;}

    Ogre::String File::extension() const {return mExtension;}
    Ogre::String &File::extension() {return mExtension;}

    File File::parentDir() const {return File(mPath);}

    void File::mkdir() const
    {
        Ogre::String path = fullPath();

        if(isFile())
            path = parentDir().fullPath();

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

        Ogre::String path = fullPath();
#if defined(_WIN32)
        Debug::error("File::touch not implemented").endl().breakHere();
#else
        return open(path.c_str(), O_WRONLY | O_CREAT | O_NONBLOCK) != -1;
#endif
        return false;
    }

    void File::rm()
    {
        if(isDir())
            Debug::error("File::rm not implemented for directories").endl().breakHere();

#if defined(_WIN32)
        Debug::error("File::rm not implemented").endl().breakHere();
#else
        unlink(fullPath().c_str());
#endif
        return;
    }

    void File::poolAndNotify()
    {
        //http://stackoverflow.com/questions/5211993/using-read-with-inotify
        while(sInotifyFD >= 0)
        {
            unsigned int avail;
            ioctl(sInotifyFD, FIONREAD, &avail);

            char buffer[avail];
            SYSCALL_ALIAS_read(sInotifyFD, buffer, avail);

            int offset = 0;

            while(offset < (int) avail)
            {
                struct inotify_event *iEvent = (inotify_event *)(buffer + offset);

                FileEvent event;

                if(iEvent->mask & (IN_MODIFY))
                    event = FileEvent(iEvent->wd, IN_MODIFY);
                else
                    continue;

                sStaticLock.lock();
                auto it = sListeners.find(event);

                if(it == sListeners.end())
                {
                    Debug::warning("File::poolAndNotify(): found no listener for ")(event.first)(". Ignoring notification.").endl();
                    sStaticLock.unlock();
                    continue;
                }

                sNotificationList.push_back(event);
                sStaticLock.unlock();
//                 Debug::log("File::poolAndNotify(): wd: ")(event.first).endl();

                offset += sizeof(inotify_event) + iEvent->len;
            }
        }

        sStaticLock.lock();
        sListeners.clear();
        sStaticLock.unlock();
    }

    void File::dispatchEvents()
    {
        while(!sNotificationList.empty())
        {
            sStaticLock.lock();
            //pair wd, filepath
            FileEvent event = sNotificationList.front();

            // find listeners
            auto it_event = sListeners.find(event);

            if(it_event == sListeners.end())
            {
                sNotificationList.pop_front();
                sStaticLock.unlock();
                continue;
            }

            ListenersEntry entry = (*it_event).second;
            File file(entry.first);
            auto listeners = std::set<FileEventListener *>(entry.second.begin(), entry.second.end());
            sNotificationList.pop_front();
            sStaticLock.unlock();

            for(auto it = listeners.begin(); it != listeners.end(); ++it)
                (*it)->onFileChangeEvent(file);
        }
    }

    void File::addFileListener(FileEventListener *listener)
    {
        File::addFileListener(this, listener);
    }

    void File::removeFileListener(FileEventListener *listener)
    {
        File::removeFileListener(this, listener);
    }

// static
    void File::addFileListener(File *file, FileEventListener *listener)
    {
        sStaticLock.lock();

//         if(!file->isDir())
//             file->setPath(file->parentDir());
//         Poco::DirectoryWatcher watcher(file->fullPath());

        if(!file->exists())
        {
            sStaticLock.unlock();
            Debug::error("File::addFileListener(): file not found: ")(*file).endl();
            return;
        }

        int wd = inotify_add_watch(sInotifyFD, file->fullPath().c_str(), IN_MODIFY);

        if(-1 == wd)
        {
            sStaticLock.unlock();
            Debug::error("File::addFileListener(): inotify error with ")(*file).endl();
            return;
        }

        FileEvent fileEvent(wd, IN_MODIFY);
//         Debug::log("File::addFileListener(): getting wd ")(wd)(" for path ")(file->fullPath()).endl();
        auto it = sListeners.find(fileEvent);

        if(it == sListeners.end())
        {
            auto entry = ListenersEntry(file->fullPath(), std::set<FileEventListener *>());
            sListeners.insert(std::pair<FileEvent, ListenersEntry>(fileEvent, entry));
            it = sListeners.find(fileEvent);
        }

        std::set<FileEventListener *> *record = &((*it).second.second);
        record->insert(listener);
        sStaticLock.unlock();
    }

// static
    void File::removeFileListener(File *file, FileEventListener *listener)
    {
        sStaticLock.lock();
        int wd = inotify_add_watch(sInotifyFD, file->fullPath().c_str(), IN_MODIFY);
//         Debug::log("File::removeFileListener(): getting wd ")(wd)(" for path ")(file->fullPath()).endl();
        FileEvent fileEvent(wd, IN_MODIFY);
        auto it = sListeners.find(fileEvent);

        if(it != sListeners.end())
        {
            std::set<FileEventListener *> *record = &((*it).second.second);
            record->erase(listener);

            if(record->size() == 0)
                sListeners.erase(fileEvent);
        }

        sStaticLock.unlock();
    }

    File File::getCurrentDirectory()
    {
        char _cwd[1024];

        if(getcwd(_cwd, sizeof(_cwd)) == 0)
        {
            Debug::error("File::getCurrentDirectory(): getcwd() error:")(errno).endl();
            assert(false);
        }

        Ogre::String s(_cwd);
        return File(s);
    }

    bool File::exists() const
    {
        std::ifstream ifile(fullPath().c_str());
        return ifile;
    }

    Ogre::String File::fileName() const
    {
        Ogre::String s = mBaseName;

        if(mExtension.size() > 0)
            s.append("." + mExtension);

        return s;
    }

    Ogre::String File::fullPath() const {return mPath + fileName();}
    Ogre::String File::absPath() const
    {
        if(isPathAbsolute())
            return fullPath();

        return File::getCurrentDirectory()/fullPath();
    }

    bool File::isPathAbsolute() const
    {
        return Poco::Path(mPath.c_str()).isAbsolute();
    }

    bool File::isDir() const
    {
#ifdef __unix
        //http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//  Debug::log("File<")(fullPath())(">::isDir()").endl();
        int status;
        struct stat st_buf;

        status = stat(fullPath().c_str(), &st_buf);

        if(status != 0)
        {
            return false;
            Debug::error("File<")(fullPath())(">::isDir():");
            Debug::error(" can't status the file. errno:")(errno).endl();
        }

        return S_ISDIR(st_buf.st_mode);
#else
#warning bool File::isDir() is not implemented for your platform.
        Debug::error("bool File::isDir() is not implemented for this platform.").endl().breakHere();
#endif
    }

    bool File::isValid() const
    {
        std::fstream f(fullPath().c_str());
        return f.good();
    }

    bool File::isFile() const
    {
#ifdef __unix
        //http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//  Debug::log("File<")(fullPath())(">::isFile()").endl();
        int status;
        struct stat st_buf;

        status = stat(fullPath().c_str(), &st_buf);

        if(status != 0)
        {
            return false;
            Debug::error("File<")(fullPath())(">::isDir():");
            Debug::error(" can't status the file. errno:")(errno).endl();
        }

        return S_ISREG(st_buf.st_mode);
#else
#warning bool File::isFile() is not implemented for this platform.
        Debug::error("File::isFile() is not implemented for this platform").endl().breakHere();
#endif
    }

    std::vector<File> File::ls(File::NodeType filter, bool include_hidden) const
    {
        if(!(exists() && isDir()))
            return std::vector<File>(0);

        std::vector<std::string> files;
        Poco::File(fullPath()).list(files);

        std::list<File> nodes;

        for(auto it = files.begin(); it != files.end(); ++it)
        {
            File file = subfile(*it);
            NodeType nodeType = file.nodeType();

            if(nodeType & filter)
            {
                if(!include_hidden && (nodeType & HIDDEN))
                    continue;

                nodes.push_back(file);
            }
        }

        std::vector<File> vecnodes;

        for(auto it = nodes.begin(); it != nodes.end(); ++it)
            vecnodes.push_back(*it);

        return vecnodes;
    }

    File::NodeType File::nodeType()
    {
        NodeType type = static_cast<NodeType>(0);

        if(exists())
        {
            if(isDir())
                type = type | File::DIR;

            if(isFile())
                type = type | File::FILE;

            if(fileName().at(0) == '.')
                type = type | File::HIDDEN;
        }
        else
            return File::ANY;

        return type;
    }

    Ogre::String File::read(bool skiptEmtpyLines) const
    {
        if(!exists())
            return StringUtils::BLANK;

        std::ifstream s;
        s.open(fullPath().c_str());

        s.seekg(0, std::ios::end);
        std::ios::pos_type fileLength = s.tellg();
        s.seekg(0, std::ios::beg);

        char *fileData = new char[fileLength + (std::ios::pos_type) 1];

        s.read(fileData, (int) fileLength);
        fileData[fileLength] = '\0';
        Ogre::String rawContent(fileData);

        if(skiptEmtpyLines)
        {
            auto lines = StringUtils::split(rawContent, StringUtils::LINE_SEP);
            decltype(lines) filteredLines(lines.size());
            auto it = std::copy_if(lines.begin(), lines.end(), filteredLines.begin(), [](Ogre::String line)
            {
                auto _line = line;

                if(_line.length() > 0)
                    Ogre::StringUtil::trim(_line);

                return _line.length() > 0;
            });
            filteredLines.resize(std::distance(filteredLines.begin(), it));
            return StringUtils::join(filteredLines, StringUtils::LINE_SEP);
        }

        return rawContent;
    }

    bool File::readInto(Json::Value &root, bool keepComments) const
    {
        if(!exists())
        {
            Ogre::String msg = "could not open :" + fullPath() + ": file not found.";

            if(Debug::isInit)
                Debug::error(STEEL_METH_INTRO, msg).endl();
            else
                std::cerr << STEEL_METH_INTRO.c_str() << msg.c_str() << std::endl;

            return false;
        }

        Json::Reader reader;
        Ogre::String content = read(false);
        root.clear();

        if(!reader.parse(content, root, keepComments))
        {
            Ogre::String msg = "Could not parse content:" + reader.getFormatedErrorMessages() + "\non :" + content;

            if(Debug::isInit)
                Debug::error(STEEL_METH_INTRO, msg).endl();
            else
                std::cerr << STEEL_METH_INTRO.c_str() << msg.c_str() << std::endl;

            return false;
        }

        return true;
    }

    File &File::write(Ogre::String buffer, std::ios_base::openmode mode)
    {
        std::ofstream s;
        s.open(fullPath().c_str(), mode);
        s.write(buffer.c_str(), buffer.length() * sizeof(char));
        return *this;
    }

    File File::subfile(Ogre::String const filename) const
    {
        Ogre::String path = fullPath();

        if(!Ogre::StringUtil::endsWith(path, File::Separator))
            path += File::Separator;

        return File(path + filename);
    }
    
    File File::operator/(Ogre::String const filename) const
    {
        return subfile(filename);
    }

    void File::setPath(Ogre::String path)
    {
        mPath = path;
    }
} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

