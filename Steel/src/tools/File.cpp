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

#include "tools/File.h"
#include "Debug.h"

namespace Steel
{

File::File() :
		mBaseName(""), mExtension(""), mPath("")
{

}

File::File(Ogre::String fullPath)
{
	//mPath will end with a slash
	Ogre::StringUtil::splitFullFilename(fullPath, mBaseName, mExtension, mPath);
}

File::~File()
{
}

File::File(File const &f) :
		mBaseName(f.mBaseName), mExtension(f.mExtension), mPath(f.mPath)
{

}

File &File::operator=(File const &f)
{
	mBaseName = f.mBaseName;
	mExtension = f.mExtension;
	mPath = f.mPath;
	return *this;
}

File File::getCurrentDirectory()
{
	char _cwd[1024];
	if (getcwd(_cwd, sizeof(_cwd)) == 0)
	{
		std::cout << "File::getCurrentDirectory(): getcwd() error:"
				<< errno<<std::endl;
		assert(false);
	}
	return File(Ogre::String(_cwd));
}

bool File::exists()
{
	std::ifstream ifile(fullPath().c_str());
	return ifile;
}

Ogre::String File::fullPath()
{
	Ogre::String s = mPath + mBaseName;
	if (mExtension.size() > 0)
		s.append("." + mExtension);
	return s;
}

bool File::isDir()
{
	//http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
	Debug::log("File<")(fullPath())(">::isDir()").endl();
	int status;
	struct stat st_buf;

	status = stat(fullPath().c_str(), &st_buf);

	if (status != 0)
	{
		Debug::error("File<")(fullPath())(">::isDir():");
		Debug::error(" can't status the file. errno:")(errno).endl();
	}

	if (S_ISREG (st_buf.st_mode))
		return false;

	if (S_ISDIR (st_buf.st_mode))
		return true;

	return false;
}

File File::subdir(Ogre::String dirname)
{
	return File(fullPath() + "/" + dirname);
}

File File::subfile(Ogre::String filename)
{
	return File(fullPath() + "/" + filename);
}
} /* namespace Steel */
