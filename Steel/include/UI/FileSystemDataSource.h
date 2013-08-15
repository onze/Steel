#ifndef STEEL_FILESYSTEMDATASOURCE_H
#define STEEL_FILESYSTEMDATASOURCE_H

#include <Rocket/Core/Element.h>
#include <Rocket/Core/EventListener.h>
#include <Rocket/Controls/DataSource.h>
#include <Rocket/Controls/DataFormatter.h>
#include <OgrePrerequisites.h>
#include <tools/File.h>
#include <vector>

namespace Steel
{
    /**
     * This class operates as a data source for a ui librocket grid element:
     * it reads a file system and "write" to the grid a hierarchy of resources.
     * see http://librocket.com/wiki/documentation/tutorials/Datagrid
     *
     * Each produced item is whether a file or a folder with the following properties:
     * - it emits a click event when clicked, with click value "<mDataSourceName>.<item full path>"
     * - it emits a dragdrop event when dropped, with dragdrop value "<mDataSourceName>.<item full path>"
     *
     * Its stores the state (~config) of a folder node in a file located in that very node. This file's name is
     * given by confFileName(). The file is written via a ConfigFile instance (key=value pairs), and contains:
     * - expand = {true, false}: whether the folder node should appear expanded or collapsed
     */
    class FileSystemDataSource: public Rocket::Controls::DataSource, Rocket::Core::EventListener, Rocket::Controls::DataFormatter
    {
        private:
            FileSystemDataSource() {}
        public:
            /// Whether the node should appear expanded when first displayed.
            static const Ogre::String EXPAND_ATTRIBUTE;

            FileSystemDataSource(Ogre::String datasourceName, File rootDir);
            FileSystemDataSource(const FileSystemDataSource& other);
            virtual ~FileSystemDataSource();
            virtual FileSystemDataSource& operator=(const FileSystemDataSource& other);
            virtual bool operator==(const FileSystemDataSource& other) const;

            void localizeDatagridBody(Rocket::Core::Element *docRoot);
            /** This method expand all rows which have saved such a setting in their ConfigFile.*/
            void expandRows();

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
             * The function is passed in the name of a table, and it returns
             * how many rows are currently in that
             */
            virtual int GetNumRows(const Rocket::Core::String& table);

            /// Regenerate the tree. Nodes open beforehand are reopened afterwards.
            void refresh(Rocket::Core::Element *docRoot);

            /** Formats the raw results of a data source request into RML.
             *  Output resourceitems have their attribute "id" set to mDatasourceName.
             * @param[out] formatted_data The formatted RML.
             * @param[in] raw_data A list of the raw data fields.
             */
            virtual void FormatData(Rocket::Core::String& formatted_data, const Rocket::Core::StringList& raw_data);

            /// Rocket::Core::EventListener interface.
            virtual void ProcessEvent(Rocket::Core::Event& event);

        protected:
            /// Returns the name of the file, in each non-leaf of the subtree, that contains configuration for the row representing it.
            Ogre::String confFileName();
            // not owned
            Rocket::Core::Element *mDatagrid;
            // owned
            Ogre::String mDatasourceName;
            File mRootDir;
    };

}

#endif // STEEL_FILESYSTEMDATASOURCE_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

