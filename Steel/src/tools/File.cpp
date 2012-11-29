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
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" ), mListeners(std::set<FileEventListener *>())
    {
    }

    File::File(const char *fullpath ) :
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" ), mListeners(std::set<FileEventListener *>())
    {
    }

    File::File ( Ogre::String fullPath ) :
        mPath ( "" ), mBaseName ( "" ), mExtension ( "" ), mListeners(std::set<FileEventListener *>())
    {
        //mPath will end with a slash
        Ogre::StringUtil::splitFullFilename ( fullPath, mBaseName, mExtension, mPath );
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
        mBaseName = f.mBaseName;
        mExtension = f.mExtension;
        mPath = f.mPath;
        return *this;
    }

    void File::addFileListener(FileEventListener *listener)
    {
        mListeners.insert(listener);
    }

    File File::getCurrentDirectory()
    {
        char _cwd[1024];
        if ( getcwd ( _cwd, sizeof ( _cwd ) ) == 0 )
        {
            std::cout << "File::getCurrentDirectory(): getcwd() error:" << errno<<std::endl;
            assert ( false );
        }
        Ogre::String s ( _cwd );
//	std::cout<<"File::getCurrentDir(): "<<s<<std::endl;
        return File ( s );
    }

    bool File::exists()
    {
        std::ifstream ifile ( fullPath().c_str() );
        return ifile;
    }

    Ogre::String File::fullPath()
    {
        Ogre::String s = mPath + mBaseName;
        if ( mExtension.size() > 0 )
            s.append ( "." + mExtension );
        return s;
    }

    bool File::isDir()
    {
        //http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//	Debug::log("File<")(fullPath())(">::isDir()").endl();
        int status;
        struct stat st_buf;

        status = stat ( fullPath().c_str(), &st_buf );

        if ( status != 0 )
        {
            Debug::error ( "File<" ) ( fullPath() ) ( ">::isDir():" );
            Debug::error ( " can't status the file. errno:" ) ( errno ).endl();
        }

        return S_ISDIR ( st_buf.st_mode );
    }

    bool File::isValid()
    {
        std::fstream f ( fullPath().c_str() );
        return f.good();
    }

    bool File::isFile()
    {
        //http://stackoverflow.com/questions/1036625/differentiate-between-a-unix-directory-and-file-in-c
//	Debug::log("File<")(fullPath())(">::isFile()").endl();
        int status;
        struct stat st_buf;

        status = stat ( fullPath().c_str(), &st_buf );

        if ( status != 0 )
        {
            Debug::error ( "File<" ) ( fullPath() ) ( ">::isDir():" );
            Debug::error ( " can't status the file. errno:" ) ( errno ).endl();
        }

        return S_ISREG ( st_buf.st_mode );
    }

    Ogre::String File::read()
    {

        std::ifstream s;
        s.open ( fullPath().c_str() );

        s.seekg ( 0, std::ios::end );
        std::ios::pos_type fileLength = s.tellg();
        s.seekg ( 0, std::ios::beg );

        char *fileData = new char[fileLength];

        s.read ( fileData, ( int ) fileLength );
        return Ogre::String ( fileData );
    }

    File &File::write ( Ogre::String buffer )
    {
        std::ofstream s;
        s.open ( fullPath().c_str() );
        s.write ( buffer.c_str(), buffer.length() * sizeof ( char ) );
        return *this;
    }

    File File::subdir ( Ogre::String dirname )
    {
        return File ( fullPath() + "/" + dirname );
    }

    File File::subfile ( Ogre::String filename )
    {
        return File ( fullPath() + "/" + filename );
    }
} /* namespace Steel */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
