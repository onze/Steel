#ifndef STEEL_MYGUIFILETREEDATASOURCE_H
#define STEEL_MYGUIFILETREEDATASOURCE_H

#include "steeltypes.h"

namespace MyGUI
{
    class TreeControl;
}

namespace Steel
{
    class MyGUITreeControlDataSource
    {
    public:
        MyGUITreeControlDataSource();
        ~MyGUITreeControlDataSource();
        
        virtual void init(MyGUI::TreeControl *const control, Ogre::String const &dataRoot);
        
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl* pTreeControl, MyGUI::TreeControlNode* pNode) = 0;

    protected:
        MyGUI::TreeControl *mControl;
        Ogre::String mDataRoot;
    };

    class MyGUIFileSystemDataSource : public MyGUITreeControlDataSource
    {
    public:
        MyGUIFileSystemDataSource();
        ~MyGUIFileSystemDataSource();
        
        virtual void notifyTreeNodePrepare(MyGUI::TreeControl *treeControl, MyGUI::TreeControlNode *node) override;
    };
}

#endif // STEEL_MYGUIFILETREEDATASOURCE_H
