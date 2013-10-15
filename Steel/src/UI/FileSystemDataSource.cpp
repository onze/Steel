#include <list>
#include <algorithm>

#include <OgreConfigFile.h>
#include <Rocket/Controls/ElementDataGridRow.h>
#include <Rocket/Controls/ElementDataGrid.h>
#include <Rocket/Controls/ElementDataGridExpandButton.h>

#include "UI/FileSystemDataSource.h"
#include <Debug.h>
#include <tools/StringUtils.h>
#include <tools/ConfigFile.h>

namespace Steel
{
    const Ogre::String FileSystemDataSource::EXPAND_ATTRIBUTE = "expand";

    FileSystemDataSource::FileSystemDataSource(Ogre::String datasourceName, File rootDir):
        Rocket::Controls::DataSource(datasourceName.c_str()),
        Rocket::Core::EventListener(),
        Rocket::Controls::DataFormatter(datasourceName.c_str()),
        mDatagrid(nullptr),
        mDatasourceName(datasourceName), mRootDir(rootDir)
    {
    }

    FileSystemDataSource::FileSystemDataSource(const FileSystemDataSource &o):
        Rocket::Controls::DataSource(o),
        Rocket::Core::EventListener(o),
        Rocket::Controls::DataFormatter(o.mDatasourceName.c_str()),
        mDatagrid(o.mDatagrid),
        mDatasourceName(o.mDatasourceName), mRootDir(o.mRootDir)
    {
    }

    FileSystemDataSource::~FileSystemDataSource()
    {
        if(nullptr != mDatagrid)
        {
            mDatagrid->RemoveEventListener("click", this);
        }
    }

    FileSystemDataSource &FileSystemDataSource::operator=(const FileSystemDataSource &o)
    {
        if(&o == this)
            return *this;

        mDatagrid = o.mDatagrid;
        mDatasourceName = o.mDatasourceName;
        mRootDir = o.mRootDir;
        return *this;
    }

    bool FileSystemDataSource::operator==(const FileSystemDataSource &other) const
    {
        return false;
    }

    void FileSystemDataSource::GetRow(Rocket::Core::StringList &row,
                                      const Rocket::Core::String &table,
                                      int row_index,
                                      const Rocket::Core::StringList &columns)
    {
//         Debug::log.endl()("GetRow(): table:")(table)(" row_index: ")(row_index)(" columns:")(columns).endl();

        File cwd = (table == "$root") ? mRootDir : table.CString();
//         Debug::log("table: ")(cwd).endl();

        auto subfiles = cwd.ls(File::ANY);
        std::sort(subfiles.begin(), subfiles.end());
        File subfile = subfiles[row_index];
//         Debug::log("subfile: ")(subfile).endl();

        for(auto it = columns.begin(); it < columns.end(); ++it)
        {
            Rocket::Core::String colName = *it;
            Ogre::String cell = "";

            if(colName == "ext")
            {
                if(subfile.isDir())
                    cell = "$dir";
                else
                    cell = subfile.extension();
            }
            else if(colName == "filename")
            {
                cell = subfile.fileBaseName();
            }
            else if(colName == "fullpath")
            {
                cell = subfile.fullPath();
            }
            else if(colName == Rocket::Controls::DataSource::CHILD_SOURCE)
            {
                if(subfile.isDir())
                    cell = mDatasourceName + "." + subfile.fullPath();
                else
                    cell = mDatasourceName + ".$leaf";
            }

//             Debug::log(colName)(" : ")(cell).endl();
//                 if(!cell.empty())
            row.push_back(cell.c_str());
        }

//         Debug::log("returning: ")(row).endl();
//             Debug::warning("FileSystemDataSource<")(mDatasourceName)(", ")(mRootDir.fullPath())(">::GetRow(): unknown table ")(table).endl();
    }

    Ogre::String FileSystemDataSource::confFileName()
    {
        return "." + mDatasourceName + ".conf";
    }

    int FileSystemDataSource::GetNumRows(const Rocket::Core::String &table)
    {
        int n;

        if(table == "$root")
            n = mRootDir.ls(File::ANY).size();
        else if(table == "$leaf")
            n = 0;
        else
        {
            // ignore hidden files
            File file(table.CString());
            n = file.ls(File::DIR | File::FILE, false).size();
        }

//         Debug::log.endl()("cwd/table:")(table)(" -> ")(n)(" rows").endl();
        return n;
    }

    void FileSystemDataSource::FormatData(Rocket::Core::String &formatted_data, const Rocket::Core::StringList &raw_data)
    {
//         Debug::log.endl()("Formatting ")(raw_data);
        // raw_data==[ext,filename,fullpath,#num_children]
        if(raw_data.size() == 4)
        {
            bool isDir = raw_data[0] == "$dir";
            bool hasKids = raw_data[3] != "0";
            Rocket::Core::String filename = raw_data[1];
            Rocket::Core::String fullpath = raw_data[2];

            //TODO: escape double quotes in path
            formatted_data += "<resourceitem id=\"";
            formatted_data += mDatasourceName.c_str();
            formatted_data += "\"";

            if(isDir)
            {
                // dirs only contain their path (so that we can save whether to expand them)
                formatted_data += " fullpath=\"" + fullpath + "\"";
            }
            else
            {
                formatted_data += " ondragdrop=\"";
                formatted_data += fullpath;
                formatted_data += "\" style=\"drag:clone;\"";

                formatted_data += " onclick=\"";
                formatted_data += fullpath;
                formatted_data += "\" ";
            }

            formatted_data += ">";

            if(isDir && hasKids)
            {
                formatted_data += "<datagridexpand />";
            }
            else
                formatted_data += "<spacer/>";

            if(isDir)
                formatted_data += "<directory_inode/>";
            else
                formatted_data += "<regular_inode/>";

            formatted_data += "<spacer/>";
            formatted_data += filename;
        }
        else
        {
            formatted_data += "<resourceitem>";
            formatted_data += "??";
        }

        formatted_data += "</resourceitem>";
//         Debug::log(" as: ")(formatted_data).endl();
    }

    void FileSystemDataSource::refresh(Rocket::Core::Element *docRoot)
    {
        localizeDatagridBody(docRoot);

        if(nullptr == mDatagrid)
            return;

        NotifyRowChange("$root");
        mDatagrid->Update();
        expandRows();
    }

    void FileSystemDataSource::localizeDatagridBody(Rocket::Core::Element *docRoot)
    {
        Ogre::String intro = "FileSystemDataSource<" + mDatasourceName + ">::localizeDatagridBody(): ";

        // get our root node
        if(nullptr != docRoot)
        {
            Rocket::Core::ElementList dgElements;
            docRoot->GetElementsByTagName(dgElements, "datagrid");
            Rocket::Core::String sourceName = (mDatasourceName + ".$root").c_str();

            if(dgElements.size() > 0)
            {
                auto it = dgElements.begin();

                for(; it != dgElements.end(); ++it)
                {
                    auto elem = static_cast<Rocket::Core::Element *>(*it);

                    if(elem->GetAttribute<Rocket::Core::String>("source", "") == sourceName)
                    {
                        mDatagrid = elem;
                        // ask to be notified when rows get clicked
                        static_cast<Rocket::Controls::ElementDataGrid *>(mDatagrid)->AddEventListener("click", this);
                        break;
                    }
                }

                if(it == dgElements.end() && nullptr == mDatagrid)
                {
                    Debug::warning(intro)("associated document does not ");
                    Debug::warning("have a <datagrid> element with 'source' tag set to ")(sourceName)(". Aborting.").endl();
                }
            }
            else
            {
                Debug::warning(intro)("associated document does not have a <datagrid> child. Aborting.").endl();
            }
        }
        else
            Debug::warning(intro)("was given a nullptr main document. Aborting.").endl();
    }

    void FileSystemDataSource::ProcessEvent(Rocket::Core::Event &event)
    {
        Ogre::String intro = "FileSystemDataSource::ProcessEvent(): ";

        // make sure we're looking at an intersting event
        if(event.GetType() != "click")
            return;

        // and it's our business to deal with it
        if(event.GetCurrentElement() != mDatagrid)
            return;

        // elem is actually a Rocket::Controls::ElementDataGridExpandButton *
        Rocket::Core::Element *elem = event.GetTargetElement();

        if(nullptr == elem || elem->GetTagName() != "datagridexpand")
            return;

        // get the path of the row via the parent resourceitem
        Rocket::Core::Element *parent = elem->GetParentNode();
        Ogre::String fullpath = parent->GetAttribute<Rocket::Core::String>("fullpath", "").CString();

        if(fullpath.empty())
            return;

        File file(fullpath);

        if(!file.isDir())
            return;

        // rewind the hierarchy until datagridrow parent
        while(elem->GetTagName() != "datagridrow")
        {
            elem = elem->GetParentNode();

            if(elem == mDatagrid)
            {
                Debug::error(intro)(event.GetTargetElement()->GetTagName())(" element has no Datagrid parent. Aborting.").endl();
                break;
            }
        }

        if(elem != mDatagrid)
        {
            auto parentRow = static_cast<Rocket::Controls::ElementDataGridRow *>(elem);
            // save setting
            ConfigFile cf(file.subfile(confFileName()));
            cf.setSetting(FileSystemDataSource::EXPAND_ATTRIBUTE, Ogre::StringConverter::toString(parentRow->IsRowExpanded()));
            cf.save();
        }
    }

    void FileSystemDataSource::expandRows()
    {
        if(nullptr == mDatagrid)
            return;

        // iterate through rows
        Rocket::Core::ElementList rows;
        mDatagrid->GetElementsByTagName(rows, "datagridrow");

        for(auto it = rows.begin(); it != rows.end(); ++it)
        {
            // get to the resourceitem fullpath value...
            auto row = static_cast<Rocket::Controls::ElementDataGridRow *>(*it);
            Rocket::Core::ElementList cells;
            row->GetElementsByTagName(cells, "datagridcell");

            if(cells.size() == 0)
                continue;

            Rocket::Core::ElementList elems;
            cells[0]->GetElementsByTagName(elems, "resourceitem");

            if(elems.size() == 0)
                continue;

            // if it exists
            auto fullpath = elems[0]->GetAttribute<Rocket::Core::String>("fullpath", "");
            // and we want to expand it
            ConfigFile cf(File(fullpath.CString()).subfile(confFileName()));

            if(cf.getSettingAsBool(FileSystemDataSource::EXPAND_ATTRIBUTE, false))
            {
                row->ExpandRow();
                // don't forget to also expand the button
                elems.clear();
                cells[0]->GetElementsByTagName(elems, "datagridexpand");

                if(elems.size() == 0)
                    continue;

                auto btn = static_cast<Rocket::Controls::ElementDataGridExpandButton *>(elems[0]);
                btn->SetClass("expanded", true);
                btn->SetClass("collapsed", false);
            }
        }

        mDatagrid->UpdateLayout();
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

