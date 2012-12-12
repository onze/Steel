#ifndef STEEL_FILESYSTEMDATASOURCE_H
#define STEEL_FILESYSTEMDATASOURCE_H

#include <Rocket/Controls/DataSource.h>
#include <OgrePrerequisites.h>
#include <tools/File.h>

namespace Steel
{
    /**
     * This class operates as a data source for a ui librocket grid element:
     * it reads a file system and "write" to the grid a hierarchy of resources.
     * see http://librocket.com/wiki/documentation/tutorials/Datagrid
     */
    class FileSystemDataSource:public Rocket::Controls::DataSource
    {
        private:
            FileSystemDataSource() {};
        public:
            FileSystemDataSource(Ogre::String datasourceName,File rootDir);
            FileSystemDataSource(const FileSystemDataSource& other);
            virtual ~FileSystemDataSource();
            virtual FileSystemDataSource& operator=(const FileSystemDataSource& other);
            virtual bool operator==(const FileSystemDataSource& other) const;

            //inherited from DataSource
            /**
             * It takes a table, an index to a row within that table and
             * a list of columns that are being queried. The function must
             * look that row up and, for each column in the list, push the
             * data into the Rocket::Core::StringList row.
             *
             * Fetches the contents of one row of a table within the data source.
             * @param[out] row The list of values in the table.
             * @param[in] table The name of the table to query.
             * @param[in] row_index The index of the desired row.
             * @param[in] columns The list of desired columns within the row.
             **/
            virtual void GetRow(Rocket::Core::StringList& row,
                                const Rocket::Core::String& table,
                                int row_index,
                                const Rocket::Core::StringList& columns);
            /**
             * the function is passed in the name of a table, and it returns
             * how many rows are currently in that
             */
            virtual int GetNumRows(const Rocket::Core::String& table);
        protected:
            Ogre::String mDatasourceName;
            File mRootDir;
    };

}

#endif // STEEL_FILESYSTEMDATASOURCE_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
