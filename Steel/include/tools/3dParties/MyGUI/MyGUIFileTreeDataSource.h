#ifndef STEEL_MYGUIFILETREEDATASOURCE_H
#define STEEL_MYGUIFILETREEDATASOURCE_H

#include "steeltypes.h"
#include "tools/File.h"

namespace MyGUI
{
    class TreeControl;
}

namespace Steel
{
    /**
     * ABstract base class of MyGUI::TreeControl.
     */
    class MyGUITreeControlDataSource
    {
    public:
        
        MyGUITreeControlDataSource();
        ~MyGUITreeControlDataSource();
        
        virtual void init(MyGUI::TreeControl *const control);
        
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl* pTreeControl, MyGUI::TreeControlNode* pNode) = 0;

    protected:
        MyGUI::TreeControl *mControl;
    };

    class MyGUIFileSystemDataSource : public MyGUITreeControlDataSource
    {
    public:
        typedef File ControlNodeDataType;
        
        MyGUIFileSystemDataSource();
        ~MyGUIFileSystemDataSource();
        
        virtual void init(MyGUI::TreeControl *const control, Ogre::String const &dataRoot);
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node) override;
        
    private:
        // Not enough parameters. Use the public one.
        virtual void init(MyGUI::TreeControl *const control);
        Ogre::String mDataRoot;
    };
}

#endif // STEEL_MYGUIFILETREEDATASOURCE_H
