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
private:
	File();
public:
	File(Ogre::String fullpath);
	/**
	 * Copy constructor.
	 * Note:
	 * Copying a file does not mean copying its content. File instances are
	 * meant to be seen as pointers to actual files, so copying F1 to F2 means
	 * that F2 will have the same properties as F1 (path, open stream or not,
	 * etc), but it does not modifies underlying files.
	 */
	File(File const &f);
	virtual ~File();
	File &operator=(const File &f);

	/**
	 * return the file content.
	 */
	Ogre::String read();

	/**
	 * write the given string into the file.
	 */
	File &write(Ogre::String s);

	/**
	 * close the file if it was open.
	 */
	void close();
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
	 * return true is the file has been open already.
	 */
	bool isOpen();
	/**
	 * Return the path pointed to by this file.
	 */
	Ogre::String fullPath();

	/**
	 * return a file pointing at a subdirectory which name is given.
	 */
	File subdir(Ogre::String dirname);

	/**
	 * return a file pointing at a subfile which name is given.
	 */
	File subfile(Ogre::String filename);

	/**
	 * convert the File instance to a string of the path it's pointing to.
	 */
	operator Ogre::String()
	{
		return fullPath();
	}

	/**
	 * open the file if possibleand return true if it there was no error.
	 */
	bool open();
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

	/**
	 * stream used for IOs with the file.
	 */
	std::fstream *mFileStream;
	/**
	 * File copies share the same stream.
	 */
	int *mFileStreamRefCount;

	/**
	 *
	 */
};

} /* namespace Steel */
#endif /* FILE_H_ */
