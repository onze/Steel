#include "tools/3dParties/MyGUI/TreeControlItemDecorator.h"
#include "tools/3dParties/MyGUI/TreeControl.h"
#include "tools/3dParties/MyGUI/TreeControlItem.h"

namespace MyGUI
{

    TreeControlItemDecorator::TreeControlItemDecorator(MyGUI::TreeControl *treeControl)
        : mTreeControl(treeControl), mDraggedItem(nullptr),
          mTextColour(Colour::Black),
          mEnableDragAndDropWithNodes(false), mEnableDragAndDropWithLeaves(false)
    {
        mTreeControl->requestDecorateTreeControlItem = newDelegate(this, &TreeControlItemDecorator::requestDecorateTreeControlItem);
    }

    TreeControlItemDecorator::TreeControlItemDecorator(const TreeControlItemDecorator &o)
    {

    }

    TreeControlItemDecorator::~TreeControlItemDecorator()
    {
        mTreeControl->requestDecorateTreeControlItem = nullptr;
        mTreeControl = nullptr;
        mDraggedItem = nullptr;
    }

    MyGUI::TreeControlItemDecorator &TreeControlItemDecorator::setEnableDragAndDrop(bool flag)
    {
        return setEnableDragAndDrop(flag, flag);
    }

    MyGUI::TreeControlItemDecorator &TreeControlItemDecorator::setEnableDragAndDrop(bool nodes, bool leaves)
    {
        mEnableDragAndDropWithNodes = nodes;
        mEnableDragAndDropWithLeaves = leaves;
        return *this;
    }

    MyGUI::TreeControlItemDecorator &TreeControlItemDecorator::setTextColour(MyGUI::Colour const &textColour)
    {
        mTextColour = textColour;
        return *this;
    }

    void TreeControlItemDecorator::requestDecorateTreeControlItem(MyGUI::TreeControl *_sender, MyGUI::TreeControlItem *_item, MyGUI::TreeControlNode *_node)
    {
        _item->setTextColour(mTextColour);
        _item->setCaption("");

        MyGUI::TextBox *draggable = (MyGUI::TextBox *)_item->findWidget("draggable");

        if(nullptr == draggable)
        {
            MyGUI::IntCoord const itemCoords = _item->getClientCoord();
            MyGUI::IntCoord const buttonCoords = _item->getButtonExpandCollapse()->getClientCoord();
            MyGUI::IntCoord const iconCoords = _item->getIcon()->getClientCoord();
            const int left = buttonCoords.right() + iconCoords.right();
            const int width = itemCoords.width - left;
            const int top = 0;
            const int height = itemCoords.bottom() ;
            const IntCoord coords(left, top, width, height);

            draggable = _item->createWidget<MyGUI::TextBox>("TextBox", coords, MyGUI::Align::Default, "draggable");
            draggable->setAlign(Align::VCenter | Align::Left);

            // look and feel: forward events to parents
            draggable->eventMouseButtonPressed += newDelegate(this, &TreeControlItemDecorator::treeItemMouseButtonPressed);
            draggable->eventMouseButtonReleased += newDelegate(this, &TreeControlItemDecorator::treeItemMouseButtonReleased);
            draggable->eventMouseSetFocus += newDelegate(this, &TreeControlItemDecorator::treeItemMouseSetFocus);
            draggable->eventMouseLostFocus += newDelegate(this, &TreeControlItemDecorator::treeItemMouseLostFocus);
            draggable->eventMouseWheel += newDelegate(this, &TreeControlItemDecorator::treeItemMouseWheel);

            // actually useful
            draggable->eventMouseDrag += newDelegate(this, &TreeControlItemDecorator::treeItemMouseDragEvent);
            draggable->eventMouseButtonReleased += newDelegate(this, &TreeControlItemDecorator::treeItemMouseButtonReleasedEvent);
        }

        draggable->setCaption(_node->getText());
        draggable->setUserData(_item);
    }
    
    void TreeControlItemDecorator::treeItemMouseSetFocus(MyGUI::Widget *_sender, MyGUI::Widget *_old)
    {
        (*_sender->getUserData<MyGUI::TreeControlItem *>())->_riseMouseSetFocus(_old);
    }

    void TreeControlItemDecorator::treeItemMouseLostFocus(MyGUI::Widget *_sender, MyGUI::Widget *_new)
    {
        (*_sender->getUserData<MyGUI::TreeControlItem *>())->_riseMouseLostFocus(_new);
    }

    void TreeControlItemDecorator::treeItemMouseWheel(MyGUI::Widget *_sender, int _rel)
    {
        mTreeControl->_riseMouseWheel(_rel);
    }

    void TreeControlItemDecorator::treeItemMouseButtonPressed(MyGUI::Widget *_sender, int _left, int _top, MyGUI::MouseButton _id)
    {
        (*_sender->getUserData<MyGUI::TreeControlItem *>())->_riseMouseButtonPressed(_left, _top, _id);
    }

    void TreeControlItemDecorator::treeItemMouseButtonReleased(MyGUI::Widget *_sender, int _left, int _top, MyGUI::MouseButton _id)
    {
        (*_sender->getUserData<MyGUI::TreeControlItem *>())->_riseMouseButtonReleased(_left, _top, _id);
    }

    void TreeControlItemDecorator::treeItemMouseDragEvent(MyGUI::Widget *_sender, int _left, int _top, MyGUI::MouseButton _id)
    {
        TreeControlItem *const _item = *_sender->getUserData<TreeControlItem *>();
        TreeControlNode *const _node = *_item->getUserData<TreeControlNode *>();

        if(nullptr != mDraggedItem)
        {
            IntPoint offset(0,  - mDraggedItem->getClientCoord().height / 2);
            mDraggedItem->setPosition(_left + offset.left, _top + offset.top);
        }
        // Yo dawg, I heard you like conditions so I put a condition in your condition, so you can branch while you branch =)
        else if(_node->hasChildren() ? mEnableDragAndDropWithNodes : mEnableDragAndDropWithLeaves)
        {
            IntCoord const itemCoords = _item->getClientCoord();
            IntCoord const coords(_left, _top, itemCoords.width, itemCoords.height);
            mDraggedItem = Gui::getInstance().createWidget<TextBox>("TextBox", coords, MyGUI::Align::Default, "DragAndDrop", "draggedTreeItemCopy");

            IntPoint offset(0,  - mDraggedItem->getClientCoord().height / 2);
            mDraggedItem->setPosition(_left + offset.left, _top + offset.top);
            mDraggedItem->setCaption(_node->getText());
            mDraggedItem->setTextColour(mTextColour);
            mDraggedItem->setVisible(true);
        }
    }

    void TreeControlItemDecorator::treeItemMouseButtonReleasedEvent(MyGUI::Widget *_sender, int _left, int _top, MyGUI::MouseButton _id)
    {
        if(nullptr != mDraggedItem) // is null if wthe item was not draggable
        {
            TreeControlItem *_item = *_sender->getUserData<TreeControlItem *>();
            TreeControlNode *_node = *_item->getUserData<TreeControlNode *>();

            eventItemDropped(this, _node, IntPoint(_left, _top));
            Gui::getInstance().destroyChildWidget(mDraggedItem);
            mDraggedItem = nullptr;
        }
    }

}
