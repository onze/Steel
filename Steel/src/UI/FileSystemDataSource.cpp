#include "UI/FileSystemDataSource.h"
#include <Debug.h>

namespace Steel
{

    FileSystemDataSource::FileSystemDataSource(Ogre::String datasourceName,File rootDir)
        :Rocket::Controls::DataSource(datasourceName.c_str())
    {
        mDatasourceName=datasourceName;
        mRootDir=rootDir;
    }

    FileSystemDataSource::FileSystemDataSource(const FileSystemDataSource& o)
        :Rocket::Controls::DataSource(o),mDatasourceName(o.mDatasourceName),mRootDir(o.mRootDir)
    {
    }

    FileSystemDataSource::~FileSystemDataSource()
    {

    }

    FileSystemDataSource& FileSystemDataSource::operator=(const FileSystemDataSource& o)
    {
        mDatasourceName=o.mDatasourceName;
        mRootDir=o.mRootDir;
        return *this;
    }

    bool FileSystemDataSource::operator==(const FileSystemDataSource& other) const
    {
        return false;
    }

    void FileSystemDataSource::GetRow(Rocket::Core::StringList& row,
                                      const Rocket::Core::String& table,
                                      int row_index,
                                      const Rocket::Core::StringList& columns)
    {
        Debug::log("GetRow(): row:")(row)(" table:")(table)(" row_index:")(row_index)(" columns:")(columns).endl();
        if(table=="all_files")
        {
            std::list<File> subfiles=mRootDir.ls(File::ANY);
            Debug::log(subfiles).endl();
        }
        else
            Debug::warning("FileSystemDataSource<")(mDatasourceName)(", ")(mRootDir.fullPath())(">::GetRow(): unknown table ")(table).endl();
    }

    int FileSystemDataSource::GetNumRows(const Rocket::Core::String& table)
    {
        Debug::log("GetNumRow() ")(table).endl();
        return 1;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
