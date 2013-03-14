#ifndef STEEL_FILE_H_
#define STEEL_FILE_H_

#include <iostream>
#include <fstream>
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <set>
#include <thread>

#include <OgreString.h>

#include "Debug.h"
#include "FileEventListener.h"

namespace Steel
{
    //TODO: make this windows friendly
    /**
     * This is a helper class to abstract and ease some common filesystem related tasks.
     */
    class File
    {
        public:

            typedef std::pair<int,uint32_t> FileEvent;
            typedef std::pair<Ogre::String,std::set<FileEventListener *> >  ListenersEntry;

            /// initialise the File subsystem. Return 0 if success, error code otherwise.
            static int init();

            /// stops the File subsystem
            static void shutdown();

            /// batch call to file change listeners.
            static void dispatchEvents();

            /// return the application's current path.
            static File getCurrentDirectory();

            /// register an event listener. It will be notified of file events.
            static void addFileListener(File *file, FileEventListener *listener);

            /// register an event listener. It will be notified of file events.
            static void removeFileListener(File *file, FileEventListener *listener);

            enum NodeType
            {
                FILE=1<<1,
                DIR=1<<2,
                /// filename starts with a dot.
                HIDDEN=1<<3,
                /// unlikely, unless the file does not exist.
                ANY=~0
            };

            typedef std::ios_base::openmode OpenMode;
            static const OpenMode OM_OVERWRITE=std::ios_base::trunc;
            static const OpenMode OM_APPEND=std::ios_base::app;
            static const OpenMode OM_BINARY=std::ios_base::binary;

            File();
            File ( const char *fullpath );
            File ( Ogre::String fullpath );
            /**
             * Copy constructor.
             * Note:
             * Copying a file does not mean copying its content (!!! nor its listeners !!!).
             * File instances are meant to be seen as pointers to actual files, so copying
             * F1 to F2 means that F2 will have the same properties as F1 (path, open stream
             * or not, etc), but it does not modifies underlying files.
             */
            File ( File const &f );
            virtual ~File();
            File &operator= ( const File &f );
            bool operator== ( const File &f ) const;
            bool operator!= ( const File &f ) const;
            /// Compare 2 files' fileName() output.
            bool operator<  ( const File &f ) const;

            /// register an event listener. It will be notified of file events. If the file path change, it need to be removed and reset.
            void addFileListener(FileEventListener *listener);

            /// unregister an event listener. It won't be notified of files events anymore.
            void removeFileListener(FileEventListener *listener);

            /// return the file content.
            Ogre::String read();

            /// write the given string into the file, replacing what's already in.
            File &write (Ogre::String s, OpenMode mode=OM_APPEND);

            /// returns the file name (extension is part of the name)
            Ogre::String fileBaseName() const;

            /// returns the file name (extension is part of the name)
            Ogre::String fileName() const;

            /// return true is the file exists.
            bool exists();

            /// return true if the path points to a directory;
            bool isDir();

            /**
             * return true if the path points to a regular file;
             */
            bool isFile();

            /**
             * return true is the file exists, and is readable and writable.
             */
            bool isValid();

            /**
             * Return the path pointed to by this file.
             */
            Ogre::String fullPath() const;

            /**
             * return a file pointing at a subfile which name is given.
             * Note that the subfile might not exist.
             */
            File subfile ( Ogre::String const filename ) const;

            /**
             * the file/dir containing this file instance.
             */
            File parentDir();

            /**
             * convert the File instance to a string of the path it's pointing to.
             */
            operator Ogre::String() const
            {
                return fullPath();
            }

            /**
             * change the path the file instance is poiting to. If the file has change listeners,
             * it will popup events about its new path.
             */
            void setPath(Ogre::String path);

            /**
             * creates the directory mPath is poiting to. If mPath points to a file,
             * this file's folder (and possible ancestors) is created.
             * Does not crash if the path could not be created, so please check.
             */
            void mkdir();

            /**
             * creates the file fullpath() is poiting to.
             * Returns false if the file could not be created.
             */
            bool touch();
            
            /// Deletes the file. Silent if the file does not exist or on a directory.
            void rm();

            /**
             * list the current directory and return a list of files.
             * Only instances pointing to an existing directory return a non-empty list.
             * NodeType filter indicates whether to return folders, files, or both (default).
             */
            std::vector<File> ls(NodeType filter=ANY,bool include_hidden=false);

            /**
             * Returns the type of the node as a NodeType compatible value.
             * Returns File::ANY if the File instance points neither to a file nor to a dir.
             */
            NodeType nodeType();

            /// returns the file path (!! excludes its name).
            Ogre::String path();

            /// returns the file extension.
            Ogre::String extension();

        protected:

            /// fd of the file getting files event notifications from the kernel
            static int sInotifyFD;

            /// for modification of static data
            static std::mutex sStaticLock;

            /// paths and listeners associated with tokens in sWatches.
            static std::map<FileEvent,ListenersEntry> sListeners;

            /// wd of files that have changed since init/last call to dispatchToFiles (FIFO)
            static std::list<FileEvent> sNotificationList;

            /// tells inotify to monitor this file
            void startWatching();

            /// tells inotify to stop monitoring this file
            void stopWatching();

            /// inotify callback method. Dispatches event to File instances.
            static void poolAndNotify();

            /// Standardized path to the file.
            Ogre::String mPath;

            /// Node name.
            Ogre::String mBaseName;

            /// Extension of the file. Empty string if the file is a directory.
            Ogre::String mExtension;

    };
    /// type safe enum bitwise OR operator
    inline File::NodeType operator|(File::NodeType a, File::NodeType b)
    {
        return static_cast<File::NodeType>(static_cast<int>(a) | static_cast<int>(b));
    }
    /// type safe enum bitwise AND operator
    inline File::NodeType operator&(File::NodeType a, File::NodeType b)
    {
        return static_cast<File::NodeType>(static_cast<int>(a) & static_cast<int>(b));
    }
    /// type safe enum bitwise NOT operator
    inline File::NodeType operator~(File::NodeType a)
    {
        return static_cast<File::NodeType>(~static_cast<int>(a));
    }
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
