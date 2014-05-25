#include "UI/PropertyGrid/PropertyGridManager.h"

#include "steeltypes.h"
#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>
#include <MyGUI_Delegate.h>
#include <MyGUI_Widget.h>
#include <MyGUI_Button.h>
#include <MyGUI_TextBox.h>

#include "UI/PropertyGrid/PropertyGridAdapter.h"
#include <Debug.h>
#include <SignalManager.h>

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
        MyGUI::IntCoord coords(0, height, mContainerWidget->getWidth(), 20);
        MyGUI::Widget *control = mContainerWidget->createWidget<MyGUI::Widget>(skin, coords, MyGUI::Align::HCenter | MyGUI::Align::Top, controlName);

        if(nullptr == control)
        {
            Debug::error(STEEL_METH_INTRO, "could not build control for property: ", *property, ". Skipping.").endl();
            return false;
        }

        switch(property->valueType())
        {
            case PropertyGridPropertyValueType::Float:
                buildFloatControl(control, property);
                break;

            case PropertyGridPropertyValueType::Bool:
                buildBoolControl(control, property);
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
        MyGUI::TextBox *label = (MyGUI::TextBox *)control->createWidget<MyGUI::TextBox>("TextBox", control->getClientCoord(), MyGUI::Align::Left | MyGUI::Align::Top, "Label");
        label->setColour(MyGUI::Colour::White);
        label->setCaption(property->id() + ":");
        label->setSize(label->getTextSize());
        return label->getRight() + sSpaceBetweenWidgets;
    }

    void PropertyGridManager::updateControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {
        switch(property->valueType())
        {
            case PropertyGridPropertyValueType::Float:
                updateFloatControlValue(control, property);
                break;

            case PropertyGridPropertyValueType::Bool:
                updateBoolControlValue(control, property);
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
        int left = insertPropertyIdLabel(control, property);
        int size = control->getClientCoord().height;
        MyGUI::IntCoord coord(left, 0, size, size);
        MyGUI::Button *checkbox = (MyGUI::Button *)control->createWidget<MyGUI::Button>("CheckBox", coord, MyGUI::Align::Left | MyGUI::Align::Top, "CheckBox");
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

/////////////////// <FLOAT CONTROL>
    void PropertyGridManager::buildFloatControl(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {

    }
    void PropertyGridManager::updateFloatControlValue(MyGUI::Widget *const control, PropertyGridProperty *const property)
    {

    }

/////////////////// </FLOAT CONTROL>

}
