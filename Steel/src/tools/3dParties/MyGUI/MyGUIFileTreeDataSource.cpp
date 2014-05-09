#include <algorithm>
#include <MyGUI.h>

#include "steelassert.h"

#include "tools/3dParties/MyGUI/TreeControl.h"
#include "tools/3dParties/MyGUI/MyGUIFileTreeDataSource.h"
#include "tools/File.h"
#include "Debug.h"

namespace Steel
{

    MyGUITreeControlDataSource::MyGUITreeControlDataSource() : mControl(nullptr)
    {
    }

    MyGUITreeControlDataSource::~MyGUITreeControlDataSource()
    {
    }

    void MyGUITreeControlDataSource::init(MyGUI::TreeControl *const control)
    {
        mControl = control;
        mControl->eventTreeNodePrepare += MyGUI::newDelegate(this, &MyGUITreeControlDataSource::notifyTreeNodePrepare);
    }

    ////////////////////////////////////////////////////////////////////

    MyGUIFileSystemDataSource::MyGUIFileSystemDataSource() : MyGUITreeControlDataSource(),
        mDataRoot(StringUtils::BLANK)
    {
    }

    MyGUIFileSystemDataSource::~MyGUIFileSystemDataSource()
    {
    }

    void MyGUIFileSystemDataSource::init(MyGUI::TreeControl *const control)
    {
        STEEL_WRONG_CODE_PATH();
    }

    void MyGUIFileSystemDataSource::init(MyGUI::TreeControl *const control, Ogre::String const &dataRoot)
    {
        this->MyGUITreeControlDataSource::init(control);

        mDataRoot = dataRoot;
        mControl->getRoot()->setData(File(mDataRoot));
    }

    void MyGUIFileSystemDataSource::notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node)
    {
        node->removeAll();

        File const &cwd = *(node->getData<ControlNodeDataType>());
        auto subFiles = cwd.ls(File::ANY);
        Debug::log(STEEL_FUNC_INTRO, "cwd: ", cwd, " found ", subFiles.size(), " subfiles.").endl();
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
