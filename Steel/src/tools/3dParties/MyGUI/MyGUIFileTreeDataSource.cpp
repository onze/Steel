#include <algorithm>
#include <MyGUI.h>

#include "tools/3dParties/MyGUI/TreeControl.h"
#include "tools/3dParties/MyGUI/MyGUIFileTreeDataSource.h"
#include "tools/File.h"
#include <Debug.h>

namespace Steel
{

    MyGUITreeControlDataSource::MyGUITreeControlDataSource() : mControl(nullptr), mDataRoot(StringUtils::BLANK)
    {

    }

    MyGUITreeControlDataSource::~MyGUITreeControlDataSource()
    {

    }

    void MyGUITreeControlDataSource::init(MyGUI::TreeControl *const control, Ogre::String const &dataRoot)
    {
        mControl = control;
        mControl->eventTreeNodePrepare += MyGUI::newDelegate(this, &MyGUITreeControlDataSource::notifyTreeNodePrepare);

        mDataRoot = dataRoot;
        mControl->getRoot()->setData(File(mDataRoot));
    }

    ////////////////////////////////////////////////////////////////////

    MyGUIFileSystemDataSource::MyGUIFileSystemDataSource() : MyGUITreeControlDataSource()
    {
    }

    MyGUIFileSystemDataSource::~MyGUIFileSystemDataSource()
    {

    }

    void MyGUIFileSystemDataSource::notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node)
    {
        node->removeAll();

        File const &cwd = *(node->getData<File>());
        auto subFiles = cwd.ls(File::ANY);
        Debug::log(STEEL_FUNC_INTRO, "cwd: ", cwd, " found ", subFiles.size()," subfiles.").endl();
        std::sort(subFiles.begin(), subFiles.end());

        for(File const & subFile : subFiles)
        {
            Ogre::String nodeType = subFile.isDir() ? "Folder" : "Unknown";
            MyGUI::TreeControlNode *child = new MyGUI::TreeControlNode(subFile.fileName(), nodeType);
            child->setPrepared(subFile.isFile());
            child->setData(subFile);
            node->add(child);
        }
    }



}
