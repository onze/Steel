#include "UI/FileSystemDataSource.h"
#include <Debug.h>

namespace Steel
{

    FileSystemDataSource::FileSystemDataSource(Ogre::String datasourceName,File rootDir)
        :Rocket::Controls::DataSource(datasourceName.c_str()),Rocket::Controls::DataFormatter(datasourceName.c_str()),
         mDatasourceName(datasourceName),mRootDir(rootDir),mCurrentDir("")
    {
    }

    FileSystemDataSource::FileSystemDataSource(const FileSystemDataSource& o)
        :Rocket::Controls::DataSource(o),Rocket::Controls::DataFormatter(o.mDatasourceName.c_str()),
         mDatasourceName(o.mDatasourceName),mRootDir(o.mRootDir),mCurrentDir("")
    {
    }

    FileSystemDataSource::~FileSystemDataSource()
    {

    }

    FileSystemDataSource& FileSystemDataSource::operator=(const FileSystemDataSource& o)
    {
        mDatasourceName=o.mDatasourceName;
        mRootDir=o.mRootDir;
        mCurrentDir=o.mCurrentDir;
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
//         Debug::log.endl()("GetRow(): table:")(table)(" row_index: ")(row_index)(" columns:")(columns).endl();

        File cwd=(table=="$root")?mRootDir:table.CString();
//         Debug::log("table: ")(cwd).endl();

        File subfile=cwd.ls(File::ANY)[row_index];
//         Debug::log("subfile: ")(subfile).endl();

        for(auto it=columns.begin(); it<columns.end(); ++it)
        {
            Rocket::Core::String colName=*it;
            Ogre::String cell="";
            if(colName=="ext")
            {
                if(subfile.isDir())
                    cell="$dir";
                else
                    cell=subfile.extension();
            }
            else if(colName=="filename")
            {
                cell=subfile.fileName();
            }
            else if(colName=="fullpath")
            {
                cell=mRootDir.subfile(mCurrentDir.fileName()).subfile(subfile.fileName()).path();
            }
            else if(colName==Rocket::Controls::DataSource::CHILD_SOURCE)
            {
                if(subfile.isDir())
                    cell="resources."+subfile.fullPath();
                else
                    cell="resources.$leaf";
            }
//             Debug::log(colName)(" : ")(cell).endl();
//                 if(!cell.empty())
            row.push_back(cell.c_str());
        }
//         Debug::log("returning: ")(row).endl();
//             Debug::warning("FileSystemDataSource<")(mDatasourceName)(", ")(mRootDir.fullPath())(">::GetRow(): unknown table ")(table).endl();
    }

    int FileSystemDataSource::GetNumRows(const Rocket::Core::String& table)
    {
        int n;
        if(table=="$root")
            n=mRootDir.ls(File::ANY).size();
        else if (table=="$leaf")
            n=0;
        else
            n=File(table.CString()).ls(File::ANY).size();
//         Debug::log.endl()("cwd/table:")(table)(" -> ")(n)(" rows").endl();
        return n;
    }

    void FileSystemDataSource::FormatData(Rocket::Core::String& formatted_data,const Rocket::Core::StringList& raw_data)
    {
//         Debug::log.endl()("Formatting ")(raw_data);
        // raw_data==[ext,filename,fullpath,#num_children]
        if (raw_data.size() == 4)
        {
            //TODO: escape double quotes in path
            formatted_data += "<resourceitem path=\""+raw_data[2]+"\">";
            
            if (raw_data[3] != "0")
                formatted_data += "<datagridexpand/>";
            else
                formatted_data += "<spacer/>";
                
            if(raw_data[0]=="$dir")
                formatted_data+="<directory_inode/>";
            else
                formatted_data+="<regular_inode/>";
                
            formatted_data += "<spacer/>";
            formatted_data+=raw_data[1];
        }
        else
        {
            formatted_data += "<resourceitem path=\""+raw_data[2]+"\">";
            formatted_data+="??";
        }
        formatted_data += "</resourceitem>";
//         Debug::log(" as: ")(formatted_data).endl();
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
