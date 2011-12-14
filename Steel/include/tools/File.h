/*
 * File.h
 *
 *  Created on: Dec 13, 2011
 *      Author: onze
 */

#ifndef FILE_H_
#define FILE_H_

#include <OgreString.h>

#include "Debug.h"

namespace Steel
{
//TODO: make this windows friendly
class File
{
private:
	File();
public:
	File(Ogre::String fullpath);
	File(File const &f);
	virtual ~File();
	File &operator=(const File &f);
	static File getCurrentDirectory();
	bool exists();
	bool isDir();
	Ogre::String fullPath();
	File subdir(Ogre::String dirname);
	File subfile(Ogre::String filename);
	operator Ogre::String()
	{
		return fullPath();
	}
protected:
	Ogre::String mBaseName;
	Ogre::String mExtension;
	Ogre::String mPath;
};

} /* namespace Steel */
#endif /* FILE_H_ */
