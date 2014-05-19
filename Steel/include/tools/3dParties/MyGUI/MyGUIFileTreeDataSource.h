#ifndef STEEL_MYGUIFILETREEDATASOURCE_H
#define STEEL_MYGUIFILETREEDATASOURCE_H

#include "steeltypes.h"
#include "tools/File.h"
#include <tools/ConfigFile.h>

namespace MyGUI
{
    class TreeControl;
    class TreeControlNode;
}

namespace Steel
{
    /**
     * Abstract base class of MyGUI::TreeControl.
     */
    class MyGUITreeControlDataSource
    {
    public:

        MyGUITreeControlDataSource();
        virtual ~MyGUITreeControlDataSource();

        virtual void init(MyGUI::TreeControl *const control);
        virtual void shutdown(){};
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl *pTreeControl, MyGUI::TreeControlNode *pNode) = 0;
        virtual void notifyTreeNodeSelected(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node){};
    protected:
        MyGUI::TreeControl *mControl;
    };

    class MyGUIFileSystemDataSource : public MyGUITreeControlDataSource
    {
    public:
        typedef File ControlNodeDataType;

        MyGUIFileSystemDataSource();
        virtual ~MyGUIFileSystemDataSource();

        /// Control is the TreeControl fed with items, dataRoot is the path to the root (relative to )
        void init(MyGUI::TreeControl *const control, Ogre::String const &dataRoot, Ogre::String const &confFileBaseName = StringUtils::BLANK);
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node) override;
        virtual void notifyTreeNodeSelected(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node);

    private:
        // Not enough parameters. Use the public one.
        virtual void init(MyGUI::TreeControl *const control) override;
        Steel::ConfigFile confFile(File const &dir);
        bool isDirectoryExpanded(File const &file);

        Ogre::String mDataRoot;
        Ogre::String mConfFileBaseName;
    };
}

#endif // STEEL_MYGUIFILETREEDATASOURCE_H
