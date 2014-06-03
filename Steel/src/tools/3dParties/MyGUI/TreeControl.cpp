/*!
    @file
    @author     Pavel Turin
    @date       08/2009
*/

#include "tools/3dParties/MyGUI/TreeControl.h"
#include "tools/3dParties/MyGUI/TreeControlItem.h"

namespace MyGUI
{
    TreeControlNode::TreeControlNode(TreeControl *pOwner) :
        GenericNode<TreeControlNode, TreeControl>(pOwner),
        mbIsPrepared(false),
        mbIsExpanded(true),
        mstrImage("Folder")
    {
    }

    TreeControlNode::TreeControlNode(const UString &strText, TreeControlNode *pParent) :
        GenericNode<TreeControlNode, TreeControl>(strText, pParent),
        mbIsPrepared(false),
        mbIsExpanded(false),
        mstrImage("Folder")
    {
    }

    TreeControlNode::TreeControlNode(const UString &strText, const UString &strImage, TreeControlNode *pParent) :
        GenericNode<TreeControlNode, TreeControl>(strText, pParent),
        mbIsPrepared(false),
        mbIsExpanded(false),
        mstrImage(strImage)
    {
    }

    TreeControlNode::~TreeControlNode()
    {
    }

    void TreeControlNode::prepare()
    {
        if(mbIsPrepared || !mpOwner)
            return;

        mpOwner->eventTreeNodePrepare(mpOwner, this);
        mbIsPrepared = true;
    }

    size_t TreeControlNode::prepareChildren()
    {
        prepare();

        size_t nResult = 0;

        for(VectorNodePtr::iterator Iterator = getChildren().begin(); Iterator != getChildren().end(); ++Iterator)
        {
            TreeControlNode *pChild = *Iterator;

            nResult++;

            pChild->prepare();

            if(pChild->isExpanded())
                nResult += pChild->prepareChildren();
        }

        return nResult;
    }

    void TreeControlNode::setExpanded(bool bIsExpanded)
    {
        if(mbIsExpanded == bIsExpanded)
            return;

        mbIsExpanded = bIsExpanded;

        invalidate();
    }

    void TreeControlNode::setImage(const UString &strImage)
    {
        mstrImage = strImage;

        invalidate();
    }

    TreeControl::TreeControl() :
        mpWidgetScroll(nullptr),
        mbScrollAlwaysVisible(true),
        mbInvalidated(false),
        mbRootVisible(false),
        mnItemHeight(1),
        mnScrollRange(-1),
        mnTopIndex(0),
        mnTopOffset(0),
        mnFocusIndex(ITEM_NONE),
        mpSelection(nullptr),
        mpRoot(nullptr),
        mnExpandedNodes(0),
        mnLevelOffset(0),
        mClient(nullptr)
    {
    }

    void TreeControl::initialiseOverride()
    {
        Base::initialiseOverride();

        // FIXME ����������� �� ������������, ��������� ����� �����
        mpRoot = new TreeControlNode(this);

        //FIXME
        setNeedKeyFocus(true);

        assignWidget(mpWidgetScroll, "VScroll");

        if(mpWidgetScroll != nullptr)
        {
            mpWidgetScroll->eventScrollChangePosition += newDelegate(this, &TreeControl::notifyScrollChangePosition);
            mpWidgetScroll->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
        }

        assignWidget(mClient, "Client");

        if(mClient != nullptr)
        {
            mClient->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
            setWidgetClient(mClient);
        }

        MYGUI_ASSERT(nullptr != mpWidgetScroll, "Child VScroll not found in skin (TreeControl must have VScroll)");
        MYGUI_ASSERT(nullptr != mClient, "Child Widget Client not found in skin (TreeControl must have Client)");

        if(isUserString("SkinLine"))
            mstrSkinLine = getUserString("SkinLine");

        if(isUserString("HeightLine"))
            mnItemHeight = utility::parseValue<int>(getUserString("HeightLine"));

        if(isUserString("LevelOffset"))
            mnLevelOffset = utility::parseValue<int>(getUserString("LevelOffset"));

        MYGUI_ASSERT(!mstrSkinLine.empty(), "SkinLine property not found (TreeControl must have SkinLine property)");

        if(mnItemHeight < 1)
            mnItemHeight = 1;

        mpWidgetScroll->setScrollPage((size_t)mnItemHeight);
        mpWidgetScroll->setScrollViewPage((size_t)mnItemHeight);

        invalidate();
    }

    void TreeControl::shutdownOverride()
    {
        if(mbInvalidated)
            Gui::getInstance().eventFrameStart -= newDelegate(this, &TreeControl::notifyFrameEntered);
        
        mpWidgetScroll = nullptr;
        mClient = nullptr;
        // FIXME ����������� �� �����������, ��������� ����� �����
        delete mpRoot;

        Base::shutdownOverride();
    }

    void TreeControl::setRootVisible(bool bValue)
    {
        if(mbRootVisible == bValue)
            return;

        mbRootVisible = bValue;
        invalidate();
    }

    void TreeControl::setSelection(TreeControlNode *pSelection, bool updateScroll/* = false*/)
    {
        if(mpSelection == pSelection)
            return;

        mpSelection = pSelection;

        while(pSelection)
        {
            pSelection->setExpanded(true);
            pSelection = pSelection->getParent();
        }
        
        if(updateScroll)
            ;//scrollTo();TODO: scroll to selected node

        invalidate();
        eventTreeNodeSelected(this, mpSelection);
    }

    void TreeControl::onMouseWheel(int nValue)
    {
        notifyMouseWheel(nullptr, nValue);

        Widget::onMouseWheel(nValue);
    }

    void TreeControl::onKeyButtonPressed(KeyCode Key, Char Character)
    {
        // TODO: handle multi selection

        Widget::onKeyButtonPressed(Key, Character);
    }

    void TreeControl::setSize(const IntSize &Size)
    {
        Widget::setSize(Size);

        invalidate();
    }

    void TreeControl::setCoord(const IntCoord &Bounds)
    {
        Widget::setCoord(Bounds);

        invalidate();
    }

    void TreeControl::notifyFrameEntered(float nTime)
    {
        if(!mbInvalidated)
            return;

        mnExpandedNodes = mpRoot->prepareChildren();

        if(mbRootVisible)
            mnExpandedNodes++;

        updateScroll();
        updateItems();

        validate();

        mbInvalidated = false;
        Gui::getInstance().eventFrameStart -= newDelegate(this, &TreeControl::notifyFrameEntered);
    }

    void TreeControl::updateScroll()
    {
        mnScrollRange = (mnItemHeight * (int)mnExpandedNodes) - mClient->getHeight();

        if(!mbScrollAlwaysVisible || mnScrollRange <= 0 || mpWidgetScroll->getLeft() <= mClient->getLeft())
        {
            if(mpWidgetScroll->getVisible())
            {
                mpWidgetScroll->setVisible(false);
                mClient->setSize(mClient->getWidth() + mpWidgetScroll->getWidth(), mClient->getHeight());
            }
        }
        else if(!mpWidgetScroll->getVisible())
        {
            mClient->setSize(mClient->getWidth() - mpWidgetScroll->getWidth(), mClient->getHeight());
            mpWidgetScroll->setVisible(true);
        }

        mpWidgetScroll->setScrollRange(mnScrollRange + 1);

        if(mnExpandedNodes)
            mpWidgetScroll->setTrackSize(mpWidgetScroll->getLineSize() * mClient->getHeight() / mnItemHeight / mnExpandedNodes);
    }

    void TreeControl::updateItems()
    {
        int nPosition = mnTopIndex * mnItemHeight + mnTopOffset;

        int nHeight = (int)mItemWidgets.size() * mnItemHeight - mnTopOffset;

        while((nHeight <= (mClient->getHeight() + mnItemHeight)) && mItemWidgets.size() < mnExpandedNodes)
        {
            TreeControlItem *pItem = mClient->createWidget<TreeControlItem>(
                                         mstrSkinLine,
                                         0,
                                         nHeight,
                                         mClient->getWidth(),
                                         mnItemHeight,
                                         Align::Top | Align::HStretch);

            pItem->eventMouseButtonPressed += newDelegate(this, &TreeControl::notifyMousePressed);
            pItem->eventMouseButtonDoubleClick += newDelegate(this, &TreeControl::notifyMouseDoubleClick);
            pItem->eventMouseWheel += newDelegate(this, &TreeControl::notifyMouseWheel);
            pItem->eventMouseSetFocus += newDelegate(this, &TreeControl::notifyMouseSetFocus);
            pItem->eventMouseLostFocus += newDelegate(this, &TreeControl::notifyMouseLostFocus);
            pItem->_setInternalData((size_t)mItemWidgets.size());
            pItem->getButtonExpandCollapse()->eventMouseButtonClick += newDelegate(this, &TreeControl::notifyExpandCollapse);

            mItemWidgets.push_back(pItem);

            nHeight += mnItemHeight;
        };

        if(nPosition >= mnScrollRange)
        {
            if(mnScrollRange <= 0)
            {
                if(nPosition || mnTopOffset || mnTopIndex)
                {
                    nPosition = 0;
                    mnTopIndex = 0;
                    mnTopOffset = 0;
                }
            }
            else
            {
                int nCount = mClient->getHeight() / mnItemHeight;
                mnTopOffset = mnItemHeight - (mClient->getHeight() % mnItemHeight);

                if(mnTopOffset == mnItemHeight)
                {
                    mnTopOffset = 0;
                    nCount--;
                }

                mnTopIndex = ((int)mnExpandedNodes) - nCount - 1;
                nPosition = mnTopIndex * mnItemHeight + mnTopOffset;
            }
        }

        mpWidgetScroll->setScrollPosition(nPosition);
    }

    void TreeControl::validate()
    {
        typedef std::pair<VectorNodePtr::iterator, VectorNodePtr::iterator> PairNodeEnumeration;
        typedef std::list<PairNodeEnumeration> ListNodeEnumeration;
        ListNodeEnumeration EnumerationStack;
        PairNodeEnumeration Enumeration;
        VectorNodePtr vectorNodePtr;

        if(mbRootVisible)
        {
            vectorNodePtr.push_back(mpRoot);
            Enumeration = PairNodeEnumeration(vectorNodePtr.begin(), vectorNodePtr.end());
        }
        else
            Enumeration = PairNodeEnumeration(mpRoot->getChildren().begin(), mpRoot->getChildren().end());

        size_t nLevel = 0;
        size_t nIndex = 0;
        size_t nItem = 0;
        int nOffset = 0 - mnTopOffset;

        while(true)
        {
            if(Enumeration.first == Enumeration.second)
            {
                if(EnumerationStack.empty())
                    break;

                Enumeration = EnumerationStack.back();
                EnumerationStack.pop_back();
                nLevel--;
                continue;
            }

            TreeControlNode *pNode = *Enumeration.first;
            Enumeration.first++;

            if(nIndex >= (size_t)mnTopIndex)
            {
                // FIXME �������� ��������� ��� ��� ������ ������ ������ ������ ��� ����� �������check inserted as index falls more than just aytema aytemov
                if(nItem >= mItemWidgets.size())
                    break;

                if(nIndex >= mnExpandedNodes || mItemWidgets[nItem]->getTop() > mClient->getHeight())
                    break;

                TreeControlItem *pItem = mItemWidgets[nItem];
                pItem->setVisible(true);
                pItem->setCaption(pNode->getText());
                pItem->setPosition(IntPoint(nLevel * mnLevelOffset, nOffset));
                pItem->setStateSelected(pNode == mpSelection);
                pItem->setUserData(pNode);

                Button *pButtonExpandCollapse = pItem->getButtonExpandCollapse();
                pButtonExpandCollapse->setVisible(pNode->hasChildren());
                pButtonExpandCollapse->setStateSelected(!pNode->isExpanded());

                ImageBox *pIcon = pItem->getIcon();

                if(pIcon)
                {
                    ResourceImageSetPtr pIconResource = pIcon->getItemResource();

                    if(pIconResource)
                    {
                        UString strIconType(pNode->isExpanded() ? "Expanded" : "Collapsed");
                        ImageIndexInfo IconInfo = pIconResource->getIndexInfo(pNode->getImage(), strIconType);

                        if(IconInfo.size.empty())
                            pIcon->setItemResourceInfo(pIconResource->getIndexInfo(pNode->getImage(), "Common"));
                        else
                            pIcon->setItemResourceInfo(IconInfo);
                    }
                }

                requestDecorateTreeControlItem(this, pItem, pNode);

                nOffset += mnItemHeight;
                nItem++;
            }

            nIndex++;

            if(pNode->hasChildren() && pNode->isExpanded())
            {
                EnumerationStack.push_back(Enumeration);
                Enumeration.first = pNode->getChildren().begin();
                Enumeration.second = pNode->getChildren().end();
                nLevel++;
            }
        }

        if(nItem < mItemWidgets.size())
        {
            for(; nItem < mItemWidgets.size(); ++nItem)
            {
                mItemWidgets[nItem]->setStateSelected(false);
                mItemWidgets[nItem]->setVisible(false);
            }
        }
    }

    void TreeControl::invalidate()
    {
        if(mbInvalidated)
            return;

        Gui::getInstance().eventFrameStart += newDelegate(this, &TreeControl::notifyFrameEntered);
        mbInvalidated = true;
    }
    
    void TreeControl::reset()
    {
        if(nullptr != mpRoot)
        {
            mpRoot->removeAll();
            mpRoot->setPrepared(false);
            invalidate();
        }
    }

    void TreeControl::scrollTo(size_t nPosition)
    {
        mnTopOffset = ((int)nPosition % mnItemHeight);
        mnTopIndex = ((int)nPosition / mnItemHeight);

        invalidate();
    }

    void TreeControl::sendScrollingEvents(size_t nPosition)
    {
        eventTreeScrolled(this, nPosition);

        if(mnFocusIndex != ITEM_NONE)
            eventTreeNodeMouseSetFocus(this, mItemWidgets[mnFocusIndex]->getNode());
    }

    void TreeControl::notifyMousePressed(Widget *pSender, int nLeft, int nTop, MouseButton nID)
    {
        if((nID == MouseButton::Left || nID == MouseButton::Right) && pSender != mpWidgetScroll)
        {
            TreeControlNode *pSelection = mpSelection;

            if(pSender == mClient)
                pSelection = nullptr;
            else if(pSender->getVisible())
                pSelection = *pSender->getUserData<TreeControlNode *>();

            setSelection(pSelection);

            if(nID == MouseButton::Right)
                eventTreeNodeContextMenu(this, mpSelection);
        }
    }

    void TreeControl::notifyMouseWheel(Widget *pSender, int nValue)
    {
        if(mnScrollRange <= 0)
            return;

        int nPosition = (int)mpWidgetScroll->getScrollPosition();

        if(nValue < 0)
            nPosition += mnItemHeight;
        else
            nPosition -= mnItemHeight;

        if(nPosition >= mnScrollRange)
            nPosition = mnScrollRange;
        else if(nPosition < 0)
            nPosition = 0;

        if((int)mpWidgetScroll->getScrollPosition() == nPosition)
            return;

        mpWidgetScroll->setScrollPosition(nPosition);

        scrollTo(nPosition);
        sendScrollingEvents(nPosition);
    }

    void TreeControl::notifyMouseDoubleClick(Widget *pSender)
    {
        if(mpSelection)
            eventTreeNodeActivated(this, mpSelection);
    }

    void TreeControl::notifyMouseSetFocus(Widget *pSender, Widget *pPreviousWidget)
    {
        mnFocusIndex = *pSender->_getInternalData<size_t>();
        eventTreeNodeMouseSetFocus(this, mItemWidgets[mnFocusIndex]->getNode());
    }

    void TreeControl::notifyMouseLostFocus(Widget *pSender, Widget *pNextWidget)
    {
        if(!pNextWidget || (pNextWidget->getParent() != mClient))
        {
            mnFocusIndex = ITEM_NONE;
            eventTreeNodeMouseLostFocus(this, nullptr);
        }
    }

    void TreeControl::notifyScrollChangePosition(ScrollBar *pSender, size_t nPosition)
    {
        scrollTo(nPosition);
        sendScrollingEvents(nPosition);
    }

    void TreeControl::notifyExpandCollapse(Widget *pSender)
    {
        TreeControlItem *pItem = pSender->getParent()->castType<TreeControlItem>(false);

        if(!pItem)
            return;

        TreeControlNode *pNode = pItem->getNode();
        pNode->setExpanded(!pNode->isExpanded());
        
        eventTreeNodeExpandCollapse(this, pNode);

        if(!pNode->isExpanded() && mpSelection && mpSelection->hasAncestor(pNode))
        {
            mpSelection = pNode;
            eventTreeNodeSelected(this, mpSelection);
        }

        invalidate();
    }
}
