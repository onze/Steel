#include "UI/PropertyGrid/PropertyGridManager.h"

#include "steeltypes.h"
#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>
#include <MyGUI_Delegate.h>
#include <MyGUI_Widget.h>
#include <MyGUI_Button.h>
#include <MyGUI_ComboBox.h>
#include <MyGUI_ScrollBar.h>
#include <MyGUI_TextBox.h>

#include "Debug.h"
#include "UI/PropertyGrid/PropertyGridAdapter.h"
#include "SignalManager.h"
#include "tools/MyGUIUtils.h"

namespace Steel
{
    PropertyGridManager::PropertyGridManager(MyGUI::ScrollView *const propertyGrid):
        mContainerWidget(propertyGrid), mAdapter(nullptr), mSignals(), mControlsMap()
    {
    }

    PropertyGridManager::~PropertyGridManager()
    {
        unsetAdapter();
    }

    void PropertyGridManager::setAdapter(PropertyGridAdapter *const adapter)
    {
        if(nullptr != mAdapter)
            unsetAdapter();

        mAdapter = adapter;

        if(nullptr != mAdapter)
        {
            buildNewControls();

            mSignals.newAdapterProperty = mAdapter->getSignal(PropertyGridAdapter::PublicSignal::newProperty);
            registerSignal(mSignals.newAdapterProperty);
        }
    }

    void PropertyGridManager::unsetAdapter()
    {
        unregisterSignal(mSignals.newAdapterProperty);
        mSignals.newAdapterProperty = INVALID_SIGNAL;

        cleanControls();

        mAdapter = nullptr;
    }

    void PropertyGridManager::onSignal(Signal signal, SignalEmitter *const src)
    {
        if(signal == mSignals.newAdapterProperty)
            buildNewControls(); //should we clean beforehands to respect properties order ?
        else if(mSignals.propertyChanged.end() != mSignals.propertyChanged.find(signal))
        {
            // a property changed, we update its control
            PropertyGridProperty *const property = static_cast<PropertyGridProperty *const>(src);
            auto it = mControlsMap.find(property->id());

            if(mControlsMap.end() == it)
            {
                Debug::error(STEEL_METH_INTRO, "could not find control for property ", *property, ". Aborting.").endl();
                return;
            }

            MyGUI::Widget *const control = it->second;
            updateControlValue(control, property);
        }
    }

    void PropertyGridManager::cleanControls()
    {
        while(mRanges.size())
        {
            auto it = mRanges.begin();
            STEEL_DELETE(it->second);
            mRanges.erase(it);
        }

        // unregister properties signals
        for(Signal signal : mSignals.propertyChanged)
            unregisterSignal(signal);

        mSignals.propertyChanged.clear();

        // remove controls
        if(nullptr != mContainerWidget)
        {
            auto enumerator = mContainerWidget->getEnumerator();
            MyGUI::Gui::getInstance().destroyWidgets(enumerator);
        }

        mControlsMap.clear();
    }

    void PropertyGridManager::buildNewControls()
    {
        if(nullptr == mAdapter)
        {
            cleanControls();
            return;
        }

        PropertyGridPropertyVector const &properties = mAdapter->properties();
        int height = 0;

        for(PropertyGridProperty * const property : properties)
        {
            auto it = mControlsMap.find(property->id());

            if(mControlsMap.end() == it)
            {
                if(!buildNewControl(property, height))
                    Debug::error(STEEL_METH_INTRO, "could not create new widget for ", *property).endl();
            }
            else
            {
                //we already have a control for this property, so we can skip it
                height = it->second->getBottom();
            }
        }
    }

    bool PropertyGridManager::buildNewControl(PropertyGridProperty *const property, int &height)
    {
        if(nullptr == mContainerWidget)
        {
            Debug::error(STEEL_METH_INTRO, "no container for controls. Aborting.").endl();
            return false;
        }

        // build the control matching the property value type
        Ogre::String const skin = "PanelEmpty";
        Ogre::String const controlName = "PropertyGridManager_Control_" + property->id();

        // original control size is set here, but the method that build the control can totally resize it.
        MyGUI::IntCoord coords(0, height, mContainerWidget->getClientWidget()->getWidth(), 20);
        MyGUI::Widget *control = mContainerWidget->createWidget<MyGUI::Widget>(skin, coords, MyGUI::Align::HCenter | MyGUI::Align::Top, controlName);

        if(nullptr == control)
        {
            Debug::error(STEEL_METH_INTRO, "could not build control for property: ", *property, ". Skipping.").endl();
            return false;
        }

        switch(property->valueType())
        {
            case PropertyGridPropertyValueType::Bool:
                buildBoolControl(control, property);
                break;

            case PropertyGridPropertyValueType::String:
                buildStringControl(control, property);
                break;

            case PropertyGridPropertyValueType::Range:
                buildRangeControl(control, property);
                break;

            case PropertyGridPropertyValueType::StringVectorSelection:
                buildStringVectorSelectionControl(control, property);
                break;

            case PropertyGridPropertyValueType::None:
            default:
                buildDummyControl(control, property);
        }

        height = control->getBottom();
        mControlsMap.insert(std::make_pair(property->id(), control));

        // subscribe to property changes
        Signal propertyChanged = property->getSignal(PropertyGridProperty::PublicSignal::changed);
        registerSignal(propertyChanged);
        mSignals.propertyChanged.insert(propertyChanged);

        // update control value
        updateControlValue(control, property);

        return true;
    }

    int PropertyGridManager::insertPropertyIdLabel(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        return insertLabel(control, "IdLabel", property->id() + ":", control->getLeft());
    }

    int PropertyGridManager::insertLabel(MyGUI::Widget *const control, const Ogre::String &labelName, const Ogre::String &labelCaption, int x, MyGUI::TextBox *&labelPtr)
    {
        MyGUI::IntCoord coords(control->getClientCoord());
        coords.left = x;
        labelPtr = (MyGUI::TextBox *)control->createWidget<MyGUI::TextBox>("TextBox", coords, MyGUI::Align::Left | MyGUI::Align::VCenter, labelName);
        labelPtr->setColour(MyGUI::Colour::White);
        labelPtr->setCaption(labelCaption);
        labelPtr->setSize(labelPtr->getTextSize());
        return labelPtr->getRight();
    }

    void PropertyGridManager::updateControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        switch(property->valueType())
        {
            case PropertyGridPropertyValueType::Bool:
                updateBoolControlValue(control, property);
                break;

            case PropertyGridPropertyValueType::String:
                updateStringControlValue(control, property);
                break;

            case PropertyGridPropertyValueType::Range:
                updateRangeControlValue(control, property);
                break;

            case PropertyGridPropertyValueType::StringVectorSelection:
                updateStringVectorSelectionControlValue(control, property);
                break;

            case PropertyGridPropertyValueType::None:
            default:
                break;
        }
    }

/////////////////// <DUMMY CONTROL>
    void PropertyGridManager::buildDummyControl(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        MyGUI::TextBox *label = (MyGUI::TextBox *)control->createWidget<MyGUI::TextBox>("TextBox", control->getClientCoord(), MyGUI::Align::HCenter | MyGUI::Align::Top, "Label");
        label->setColour(MyGUI::Colour::White);
        label->setCaption(property->id());
    }
    /////////////////// </DUMMY CONTROL>

    /////////////////// <BOOL CONTROL>
    void PropertyGridManager::buildBoolControl(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        int left = insertPropertyIdLabel(control, property) + WIDGET_SPACING;

        int size = control->getClientCoord().height;
        MyGUI::IntCoord coord(left, 0, size, size);
        MyGUI::Button *checkbox = (MyGUI::Button *)control->createWidget<MyGUI::Button>("CheckBox", coord, MyGUI::Align::Left | MyGUI::Align::VCenter, "CheckBox");
        checkbox->setEnabled(!property->isReadOnly());

        checkbox->setUserData(property);
        checkbox->eventMouseButtonClick += MyGUI::newDelegate(this, &PropertyGridManager::onMyGUIMouseButtonClickForCheckboxToggle);
    }

    void PropertyGridManager::updateBoolControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        MyGUI::Button *checkbox = (MyGUI::Button *)control->findWidget("CheckBox");

        if(nullptr == checkbox)
        {
            Debug::error(STEEL_METH_INTRO, "could not find checkbox button in bool propertyControl. Control: ", control, " property: ", *property, ". Aborting.").endl();
            return;
        }

        bool checked = false;
        property->read(checked);
        checkbox->setStateSelected(checked);
    }

    void PropertyGridManager::onMyGUIMouseButtonClickForCheckboxToggle(MyGUI::Widget *button)
    {
        MyGUI::Button *checkbox = button->castType<MyGUI::Button>();

        PropertyGridProperty *const property = *(button->getUserData<PropertyGridProperty *>());

        if(nullptr == property)
        {
            Debug::error(STEEL_METH_INTRO, "no property linked to control ", button->getParent()).endl().breakHere();
            return;
        }

        bool const currentState = checkbox->getStateSelected();

        if(PropertyGridPropertyValueType::Bool == property->valueType())
            property->write(!currentState);
        else
            Debug::error(STEEL_METH_INTRO, "got a checkbox event for property ", *property, ". Skipping event.").endl();
    }
/////////////////// </BOOL CONTROL>

/////////////////// <STRING CONTROL>
    void PropertyGridManager::buildStringControl(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        int left = insertPropertyIdLabel(control, property);

        MyGUI::IntCoord coord(left + WIDGET_SPACING, 0, control->getRight() - left - 2 * WIDGET_SPACING, control->getHeight());
        MyGUI::EditBox *editBox = (MyGUI::EditBox *)control->createWidget<MyGUI::EditBox>("EditBox", coord, MyGUI::Align::Left | MyGUI::Align::VCenter, "EditBox");
        editBox->setEditReadOnly(property->isReadOnly());

        editBox->setUserData(property);
        editBox->eventEditSelectAccept += MyGUI::newDelegate(this, &PropertyGridManager::onMyGUIEditSelectAccept);
    }

    void PropertyGridManager::updateStringControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        MyGUI::EditBox *editBox = (MyGUI::EditBox *)control->findWidget("EditBox");

        if(nullptr == editBox)
        {
            Debug::error(STEEL_METH_INTRO, "could not find editBox in String propertyControl. Control: ", control, " property: ", *property, ". Aborting.").endl();
            return;
        }

        Ogre::String value = StringUtils::BLANK;
        property->read(value);
        editBox->setCaption(value);
    }

    void PropertyGridManager::onMyGUIEditSelectAccept(MyGUI::EditBox *editBox)
    {
        PropertyGridProperty *const property = *(editBox->getUserData<PropertyGridProperty *>());

        if(nullptr == property)
        {
            Debug::error(STEEL_METH_INTRO, "no property linked to control ", editBox->getParent()).endl().breakHere();
            return;
        }

        if(PropertyGridPropertyValueType::String == property->valueType())
            property->write(editBox->getCaption());
        else
            Debug::error(STEEL_METH_INTRO, "got an editBox event for property ", *property, ". Skipping event.").endl();
    }
/////////////////// </STRING  CONTROL>

/////////////////// <RANGE CONTROL>
    void PropertyGridManager::buildRangeControl(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        int left = insertPropertyIdLabel(control, property) ;

        // scrollbar in between
        int width = control->getWidth() - left - WIDGET_SPACING * 2;
        MyGUI::IntCoord coord(left + WIDGET_SPACING, 1, width, control->getHeight() - 2);
        MyGUI::ScrollBar *scrollBar = (MyGUI::ScrollBar *)control->createWidget<MyGUI::ScrollBar>("SliderH", coord, MyGUI::Align::Left | MyGUI::Align::Top, "ScrollBar");
        scrollBar->setEnabled(!property->isReadOnly());
        MyGUIUtils::expandWidgetCoord(control, 0, 0, 0, 2);

        scrollBar->setScrollRange(100);
        scrollBar->setUserData(property);
        static_cast<MyGUI::ScrollBar *>(scrollBar)->eventScrollChangePosition += MyGUI::newDelegate(this, &PropertyGridManager::onMyGUIScrollChangePosition);


        MyGUI::TextBox *labelPtr;

        // label for min value
        left = insertLabel(control, "minLabel", "0.000", scrollBar->getLeft() + WIDGET_SPACING, labelPtr);
        labelPtr->setAlign(MyGUI::Align::Left | MyGUI::Align::VCenter);
        labelPtr->setNeedMouseFocus(false);
        labelPtr->setTextAlign(MyGUI::Align::Left);
        labelPtr->setTextColour(MyGUI::Colour::Black);

        // label for max value
        insertLabel(control, "maxLabel", "0.000", scrollBar->getRight() - labelPtr->getWidth() - WIDGET_SPACING, labelPtr); // built right after the minLabel
        labelPtr->setAlign(MyGUI::Align::Right | MyGUI::Align::VCenter);
        labelPtr->setNeedMouseFocus(false);
        labelPtr->setTextAlign(MyGUI::Align::Right);
        labelPtr->setTextColour(MyGUI::Colour::Black);

        // label for value
        insertLabel(control, "valueLabel", "0.000", (scrollBar->getLeft() + scrollBar->getRight()) / 2, labelPtr); // built right after the minLabel
        labelPtr->setAlign(MyGUI::Align::Center);
        labelPtr->setNeedMouseFocus(false);
        labelPtr->setTextAlign(MyGUI::Align::Center);
        labelPtr->setTextColour(MyGUI::Colour::Black);
    }

    void PropertyGridManager::updateRangeControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        MyGUI::ScrollBar *scrollBar = (MyGUI::ScrollBar *)control->findWidget("ScrollBar");

        if(nullptr == scrollBar)
        {
            Debug::error(STEEL_METH_INTRO, "could not find scrollBar in Range propertyControl. Control: ", control, " property: ", *property, ". Aborting.").endl();
            return;
        }

        PropertyGridProperty::Range *range = new PropertyGridProperty::Range();
        property->read(*range);
        auto it = mRanges.insert( {property->id(), range});

        if(!it.second) // true iff already there
        {
            STEEL_DELETE(it.first->second);
            it.first->second = range;
        }

        // min label
        {
            MyGUI::TextBox *label = (MyGUI::TextBox *)control->findWidget("minLabel");

            if(nullptr != label)
                label->setCaption(Ogre::StringConverter::toString(range->min));
        }

        // max label
        {
            MyGUI::TextBox *label = (MyGUI::TextBox *)control->findWidget("maxLabel");

            if(nullptr != label)
                label->setCaption(Ogre::StringConverter::toString(range->max));
        }

        // value label
        {
            MyGUI::TextBox *label = (MyGUI::TextBox *)control->findWidget("valueLabel");

            if(nullptr != label)
                label->setCaption(Ogre::StringConverter::toString(range->value, 6));
        }

        decltype(range->value) percent = (range->value - range->min) / (range->max - range->min) * ((decltype(range->value))99.f); // within [0, 99]
        scrollBar->setScrollPosition((size_t)percent);

    }

    void PropertyGridManager::onMyGUIScrollChangePosition(MyGUI::ScrollBar *scrollBar, size_t index)
    {
        PropertyGridProperty *const property = *(scrollBar->getUserData<PropertyGridProperty *>());

        if(nullptr == property)
        {
            Debug::error(STEEL_METH_INTRO, "no property linked to control ", scrollBar->getParent()).endl().breakHere();
            return;
        }

        if(PropertyGridPropertyValueType::Range == property->valueType())
        {
            auto it = mRanges.find(property->id());

            if(mRanges.end() != it && nullptr != it->second)
            {
                PropertyGridProperty::Range &range = *(it->second);
                PropertyGridProperty::Range::value_type percent = ((PropertyGridProperty::Range::value_type)scrollBar->getScrollPosition()) / 100.f;
                range.value = range.min + percent * (range.max - range.min);
                property->write(range);
            }
            else
            {
                Debug::error(STEEL_METH_INTRO, "got no Range associated with property ", *property, ". Aborting update.").endl();
            }
        }
        else
            Debug::error(STEEL_METH_INTRO, "got a scrollBar event for property ", *property, ". Skipping event.").endl();

    }
/////////////////// </RANGE CONTROL>

/////////////////// <SELECTION FROM STRING VECTOR CONTROL>
    void PropertyGridManager::buildStringVectorSelectionControl(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        int left = insertPropertyIdLabel(control, property) + WIDGET_SPACING;

        int width = control->getWidth() - left - WIDGET_SPACING;
        MyGUI::IntCoord coord(left, 0, width, control->getHeight());
        MyGUI::ComboBox *cbBox = (MyGUI::ComboBox *)control->createWidget<MyGUI::ComboBox>("ComboBox", coord, MyGUI::Align::Left | MyGUI::Align::Top, "ComboBox");
        cbBox->setEnabled(!property->isReadOnly());

        cbBox->setEditReadOnly(true);
        cbBox->setComboModeDrop(true); //a position change also acts as a submit (aka accept) (both events are raised in this order).
        cbBox->setUserData(property);
        cbBox->eventComboAccept += MyGUI::newDelegate(this, &PropertyGridManager::onMyGUIComboAccept);
    }

    void PropertyGridManager::updateStringVectorSelectionControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        MyGUI::ComboBox *cbBox = (MyGUI::ComboBox *)control->findWidget("ComboBox");

        if(nullptr == cbBox)
        {
            Debug::error(STEEL_METH_INTRO, "could not find comboBox in stringVectorSelection propertyControl. Control: ", control, " property: ", *property, ". Aborting.").endl();
            return;
        }

        PropertyGridProperty::StringVectorSelection readItem;
        property->read(readItem);

        cbBox->removeAllItems();

        for(Ogre::String const & value : readItem.selectableValues)
            cbBox->addItem(value);

        cbBox->setIndexSelected(readItem.selectedIndex);
    }

    void PropertyGridManager::onMyGUIComboAccept(MyGUI::ComboBox *sender, size_t index)
    {
        MyGUI::ComboBox *cbBox = sender->castType<MyGUI::ComboBox>();

        PropertyGridProperty *const property = *(cbBox->getUserData<PropertyGridProperty *>());

        if(nullptr == property)
        {
            Debug::error(STEEL_METH_INTRO, "no property linked to control ", cbBox->getParent()).endl().breakHere();
            return;
        }

        if(PropertyGridPropertyValueType::StringVectorSelection == property->valueType())
            property->write((u32)index);
        else
            Debug::error(STEEL_METH_INTRO, "got a combobox event for property ", *property, ". Skipping event.").endl();
    }
/////////////////// </SELECTION FROM STRING VECTOR CONTROL>

}


