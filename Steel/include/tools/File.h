/*
 * File.h
 *
 *  Created on: Dec 13, 2011
 *      Author: onze
 */

#ifndef FILE_H_
#define FILE_H_

#include <iostream>
#include <fstream>
#include <mutex>
#include <vector>
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
            /// initialise the File subsystem. Return 0 if success, error code otherwise.
            static int init();

            /// stops the File subsystem
            static void shutdown();

            /// batch call to file change listeners.
            static void dispatchToFiles();

            /// return the application's current path.
            static File getCurrentDirectory();

            enum NodeType
            {
                FILE=1<<1,
                DIR=1<<2,
                /// filename starts with a dot.
                HIDDEN=1<<3,
                /// unlikely, unless the file does not exist.
                ANY=~0
            };

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
            bool operator!= ( const File &f );


            /// register an event listener. It will be notified of file events.
            void addFileListener(FileEventListener *listener);

            /// unregister an event listener. It won't be notified of files events anymore.
            void removeFileListener(FileEventListener *listener);

            /// call listeners' callback methods
            void dispatchToListeners();

            /// return the file content.
            Ogre::String read();

            /// write the given string into the file, replacing what's already in.
            File &write ( Ogre::String s );
            
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

            /*
            Not trivial to implement. There is no definite way to differencite between a directory and a file.
            /// returns the path relative to the given File
            Ogre::String relpath(File comp);
            */


        protected:
            /// fd of the file getting files event notifications from the kernel
            static int sInotifyFD;

            /// for modification of static data
            static std::mutex sStaticLock;

            /// inotify watch tokens
            static std::vector<int> sWatches;

            /// instances associated with tokens in sWatches
            static std::vector<File *> sFiles;

            /// files that have changed since init/last call to notifyListeners (FIFO)
            static std::list<File *> sNotificationList;

            /// threads that gets blocked by the blocking event pooling inotify function
//             static std::thread sNotifier;

            /// inotify callback method. Disatches event to File instances.
            static void poolAndNotify();

            /**
             * standardized path to the file.
             */
            Ogre::String mPath;

            /**
             * node name.
             */
            Ogre::String mBaseName;

            /**
             * extension of the file. Empty string if the file is a directory.
             */
            Ogre::String mExtension;

            /**
             * set of all current listeners;
             */
            std::set<FileEventListener *> mListeners;

            /// tells inotify to monitor this file
            void startWatching();

            /// tells inotify to stop monitoring this file
            void stopWatching();

            /// personal watch descriptor
            int mWD;

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
