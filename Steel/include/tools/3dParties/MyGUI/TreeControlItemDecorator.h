#ifndef MYGUI_TREECONTROLITEMDECORATOR_H
#define MYGUI_TREECONTROLITEMDECORATOR_H

#include <MyGUI_Colour.h>
#include <MyGUI_MouseButton.h>
#include <MyGUI_Delegate.h>

namespace MyGUI
{
    
    class TreeControl;
    class TreeControlItem;
    class TreeControlNode;
    class DDContainer;
    class DDItemInfo;
    class ItemBox;
    class IBNotifyItemData;
    class Widget;
    class IBDrawItemInfo;
    
    namespace types
    {
        template<typename T>
        struct TCoord;
    }
    typedef types::TCoord<int> IntCoord;
    
    class TreeControlItemDecorator
    {
    private:
        TreeControlItemDecorator(){};
        
    public:
        typedef delegates::CMultiDelegate3<TreeControlItemDecorator *, TreeControlNode *, IntPoint const&> EventHandle_TreeControlItemDecoratorPtrTreeControlNodePtrIntPointRef;
        /** Event : Called when a dragged item is dropped.\n
         * Note: get the TreeControl with _sender->treeControl()
         *      signature : void method(MyGUI::TreeControlItemDecorator *_sender, MyGUI::TreeControlNode *_node, MyGUI::IntPoint const& _pos)
         *      @param _sender decorator whose treeControl has a dropped item
         *      @param _item item widget being dropped
         *      @param _pos the position the item was dropped at
         */
        EventHandle_TreeControlItemDecoratorPtrTreeControlNodePtrIntPointRef eventItemDropped;
        
        TreeControlItemDecorator(MyGUI::TreeControl *treeControl);
        TreeControlItemDecorator(MyGUI::TreeControlItemDecorator const&o);
        virtual ~TreeControlItemDecorator();
        
        TreeControlItemDecorator &setEnableDragAndDrop(bool nodesAndLeaves);
        TreeControlItemDecorator &setEnableDragAndDrop(bool nodes, bool leaves);
        TreeControlItemDecorator &setTextColour(MyGUI::Colour const& textColour);
        
        void requestDecorateTreeControlItem(MyGUI::TreeControl *_sender, MyGUI::TreeControlItem* _item, MyGUI::TreeControlNode *_node);
        
        TreeControl *const treeControl() const {return mTreeControl;};
    private:
        void treeItemMouseDragEvent(MyGUI::Widget *_sender, int _left, int _top, MyGUI::MouseButton _id);
        void treeItemMouseButtonReleasedEvent(MyGUI::Widget *_sender, int _left, int _top, MyGUI::MouseButton _id);
        void treeItemMouseButtonPressed(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
        void treeItemMouseButtonReleased(MyGUI::Widget* _sender, int _left, int _top, MyGUI::MouseButton _id);
        void treeItemMouseSetFocus(MyGUI::Widget* _sender, MyGUI::Widget* _old);
        void treeItemMouseLostFocus(MyGUI::Widget* _sender, MyGUI::Widget* _new);
        void treeItemMouseWheel(MyGUI::Widget* _sender, int _rel);
        
        TreeControl *mTreeControl;
        TextBox *mDraggedItem;
        
        // settings
        Colour mTextColour;
        bool mEnableDragAndDropWithNodes;
        bool mEnableDragAndDropWithLeaves;
    };
}

#endif // MYGUI_TREECONTROLITEMDECORATOR_H
