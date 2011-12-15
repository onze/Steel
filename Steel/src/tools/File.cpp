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
		mPath(""), mBaseName(""), mExtension(""), mFileStream(NULL), mFileStreamRefCount(0)
{
}

File::File(Ogre::String fullPath) :
		mPath(""), mBaseName(""), mExtension(""), mFileStream(NULL), mFileStreamRefCount(0)
{
	//mPath will end with a slash
	Ogre::StringUtil::splitFullFilename(fullPath, mBaseName, mExtension, mPath);

}

File::File(File const &f) :
		mPath(f.mPath), mBaseName(f.mBaseName), mExtension(f.mExtension)
{
	// if we had our file open, close it.
	if (mFileStreamRefCount != NULL && mFileStream->is_open())
	{
		assert(*mFileStreamRefCount >= 1);
		assert(mFileStream!=NULL);
		close();
	}
	//then add ourself as a ref to f.mFileStream
	if (f.mFileStream != NULL)
	{
		mFileStreamRefCount = f.mFileStreamRefCount;
		++(*mFileStreamRefCount);
		mFileStream = f.mFileStream;
	}
}

File::~File()
{
	close();
	if (mFileStreamRefCount != NULL)
	{
		(*mFileStreamRefCount)--;
		if ((*mFileStreamRefCount) <= 0)
		{
			delete mFileStream;
			delete mFileStreamRefCount;
		}
	}
}

bool File::open()
{
	if (mFileStreamRefCount == NULL)
	{
		mFileStreamRefCount = new int(1);
		mFileStream = new std::fstream();
	}
	mFileStream->open(fullPath().c_str());
	return !mFileStream->fail();
}

void File::close()
{
	if (mFileStreamRefCount != 0 && *mFileStreamRefCount <= 0
			&& mFileStream != NULL && mFileStream->is_open())
	{
		mFileStream->close();
	}
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
	Ogre::String s(_cwd);
//	std::cout<<"File::getCurrentDir(): "<<s<<std::endl;
	return File(s);
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
//	Debug::log("File<")(fullPath())(">::isDir()").endl();
	int status;
	struct stat st_buf;

	status = stat(fullPath().c_str(), &st_buf);

	if (status != 0)
	{
		Debug::error("File<")(fullPath())(">::isDir():");
		Debug::error(" can't status the file. errno:")(errno).endl();
	}

	return S_ISDIR (st_buf.st_mode);
}

bool File::isFile()
{
	//http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//	Debug::log("File<")(fullPath())(">::isFile()").endl();
	int status;
	struct stat st_buf;

	status = stat(fullPath().c_str(), &st_buf);

	if (status != 0)
	{
		Debug::error("File<")(fullPath())(">::isDir():");
		Debug::error(" can't status the file. errno:")(errno).endl();
	}

	return S_ISREG (st_buf.st_mode);
}

bool File::isOpen()
{
	return mFileStreamRefCount != NULL && *mFileStreamRefCount > 0
			&& mFileStream != NULL;
}

Ogre::String File::read()
{
	if (!isOpen())
		open();
	mFileStream->seekg(0, std::ios::end);
	std::ios::pos_type fileLength = mFileStream->tellg();

	char *fileData = new char[fileLength];

	mFileStream->seekg(0, std::ios::beg);

	mFileStream->read(fileData, (int) fileLength);
	return Ogre::String(fileData);
}

File &File::write(Ogre::String s)
{
	if (!isOpen())
		open();
	mFileStream->write(s.c_str(), s.size());
	return *this;
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
