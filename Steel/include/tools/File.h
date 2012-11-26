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

#include <OgreString.h>

#include "Debug.h"

namespace Steel
{
//TODO: make this windows friendly
/**
 * This is a helper class to abstract and ease some common filesystem related tasks.
 */
class File
{
public:
    File();
    File ( Ogre::String fullpath );
    /**
     * Copy constructor.
     * Note:
     * Copying a file does not mean copying its content. File instances are
     * meant to be seen as pointers to actual files, so copying F1 to F2 means
     * that F2 will have the same properties as F1 (path, open stream or not,
     * etc), but it does not modifies underlying files.
     */
    File ( File const &f );
    virtual ~File();
    File &operator= ( const File &f );

    /**
     * return the file content.
     */
    Ogre::String read();

    /**
     * write the given string into the file, replacing what's already in.
     */
    File &write ( Ogre::String s );

    /**
     * return the application's current path.
     */
    static File getCurrentDirectory();

    /**
     * return true is the file exists.
     */
    bool exists();

    /**
     * return true if the path points to a directory;
     */
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
    Ogre::String fullPath();

    /**
     * return a file pointing at a subdirectory which name is given.
     * Note that the subdirectory might not exist.
     */
    File subdir ( Ogre::String dirname );

    /**
     * return a file pointing at a subfile which name is given.
     * Note that the subfile might not exist.
     */
    File subfile ( Ogre::String filename );

    /**
     * convert the File instance to a string of the path it's pointing to.
     */
    operator Ogre::String()
    {
        return fullPath();
    }
protected:
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
};

} /* namespace Steel */
#endif /* FILE_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
