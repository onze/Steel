#include <algorithm>
#include <MyGUI.h>

#include "steelassert.h"

#include "tools/3dParties/MyGUI/TreeControl.h"
#include "tools/3dParties/MyGUI/MyGUIFileTreeDataSource.h"
#include "tools/File.h"
#include "tools/ConfigFile.h"
#include "tools/OgreUtils.h"
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
        mControl->eventTreeNodeSelected += MyGUI::newDelegate(this, &MyGUITreeControlDataSource::notifyTreeNodeSelected);
    }

    ////////////////////////////////////////////////////////////////////

    MyGUIFileSystemDataSource::MyGUIFileSystemDataSource() : MyGUITreeControlDataSource(),
        mDataRoot(StringUtils::BLANK), mConfFileBaseName(StringUtils::BLANK)
    {
    }

    MyGUIFileSystemDataSource::~MyGUIFileSystemDataSource()
    {
    }

    void MyGUIFileSystemDataSource::init(MyGUI::TreeControl *const control)
    {
        STEEL_WRONG_CODE_PATH(STEEL_METH_INTRO);
    }

    void MyGUIFileSystemDataSource::init(MyGUI::TreeControl *const control, Ogre::String const &dataRoot, Ogre::String const &confFileBaseName/* = StringUtils::BLANK*/)
    {
        this->MyGUITreeControlDataSource::init(control);

        mDataRoot = dataRoot;
        mConfFileBaseName = confFileBaseName;
        mControl->getRoot()->setData(File(mDataRoot));
    }

    Steel::ConfigFile MyGUIFileSystemDataSource::confFile(File const &dir)
    {
        if(StringUtils::BLANK == mConfFileBaseName)
            return ConfigFile("");

        return ConfigFile(dir / ("." + mConfFileBaseName + ".conf"));
    }

    bool MyGUIFileSystemDataSource::isDirectoryExpanded(File const &file)
    {
        ConfigFile conf = confFile(file);
        bool expandAttribute;
        conf.getSetting("expand", expandAttribute, false);
        return expandAttribute;
    }

    void MyGUIFileSystemDataSource::notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node)
    {
        node->removeAll();

        File const &cwd = *(node->getData<ControlNodeDataType>());
        auto subFiles = cwd.ls(File::ANY);
        std::sort(subFiles.begin(), subFiles.end());

        for(File const & subFile : subFiles)
        {
            Ogre::String nodeType = subFile.isDir() ? "Folder" : "Unknown";
            MyGUI::TreeControlNode *child = new MyGUI::TreeControlNode(subFile.fileName(), nodeType);
            child->setPrepared(subFile.isFile());
            child->setExpanded(isDirectoryExpanded(subFile));
            child->setData(subFile);
            node->add(child);
        }
    }

    void MyGUIFileSystemDataSource::notifyTreeNodeSelected(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node)
    {
        // save ancestors expansion-ness
        MyGUI::TreeControlNode *parent = node->getParent();

        while(parent != nullptr && treeControl->getRoot() != parent)
        {
            saveNodeState(parent);
            parent = parent->getParent();
        }

        if(node->hasChildren())
            saveNodeState(node);
    }
    
    void MyGUIFileSystemDataSource::saveNodeState(MyGUI::TreeControlNode const *const node)
    {
        File const &file = *(node->getData<ControlNodeDataType>());
        // save node expansion-ness
        ConfigFile conf = confFile(file);
        conf.setSetting("expand", Ogre::StringConverter::toString(node->isExpanded()));
        conf.save();
    }

}
