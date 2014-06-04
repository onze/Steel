#ifndef STEEL_PROPERTYGRIDMANAGER_H
#define STEEL_PROPERTYGRIDMANAGER_H

#include "steeltypes.h"
#include "SignalListener.h"
#include "PropertyGridAdapter.h"

namespace MyGUI
{
    class ComboBox;
    class EditBox;
    class ScrollBar;
    class ScrollView;
    class TextBox;
    class Widget;
}

namespace Steel
{
    /**
     * Build a set of MyGUI controls to modify a PropertyGridAdapter's properties.
     * 
     * The adapter is mostly seen as data (rather than logic):
     * - it contains the properties to display. It does not display them.
     * - it can warn about a property's value change. It does not update the corresponding control.
     */
    class PropertyGridManager: public SignalListener
    {
    private:
        /// within a control
        static int const WIDGET_SPACING = 2;
    public:
        PropertyGridManager() = delete;
        
        PropertyGridManager(MyGUI::ScrollView *const propertyGrid);
        virtual ~PropertyGridManager();

        /// Returns a pointer to the current adapter.
        PropertyGridAdapter *adapter() const {return mAdapter;}
        /// Set the new adapter. Unsets anyprevious one if needed.
        void setAdapter(PropertyGridAdapter *const adapter);
        /// Unsets the current adapter.
        void unsetAdapter();
        
        /// SignalListener interface
        virtual void onSignal(Signal signal, SignalEmitter *const src) override;

    private:
        /// Parse the adapter properties and creates corresponding controls.
        void buildNewControls();
        /// Creates a control (ie set of widgets) dedicated to a property
        bool buildNewControl(Steel::PropertyGridProperty *const property, int &height);
        /// Removes all widgets from the container.
        void cleanControls();
        
        //specialized builder methods
        /// Builds an empty control that only shows the property id
        void buildDummyControl(MyGUI::Widget *control, PropertyGridProperty *const property);
        
        /// Builds a checkbox control bound to a property of type Bool
        void buildBoolControl(MyGUI::Widget *control, PropertyGridProperty *const property);
        void updateBoolControlValue(MyGUI::Widget *control, PropertyGridProperty *const property);
        /// Toggle a checkbox
        void onMyGUIMouseButtonClickForCheckboxToggle(MyGUI::Widget *button);
        
        /// Builds a textbox control bound to a property of type String
        void buildStringControl(MyGUI::Widget *control, PropertyGridProperty *const property);
        void updateStringControlValue(MyGUI::Widget *control, PropertyGridProperty *const property);
        /// textBox content changed
        void onMyGUIEditSelectAccept(MyGUI::EditBox *editBox);
        
        /// Builds a scrollbar control bound to a property of type Range
        void buildRangeControl(MyGUI::Widget *control, PropertyGridProperty *const property);
        void updateRangeControlValue(MyGUI::Widget *control, PropertyGridProperty *const property);
        void onMyGUIScrollChangePosition(MyGUI::ScrollBar *scrollBar, size_t index);
        
        /// Builds a combobox control bound to a property of type StringVectorSelection
        void buildStringVectorSelectionControl(MyGUI::Widget *control, PropertyGridProperty *const property);
        void updateStringVectorSelectionControlValue(MyGUI::Widget *control, PropertyGridProperty *const property);
        /// Combobox event callback
        void onMyGUIComboAccept(MyGUI::ComboBox* sender, size_t index);
        
        // helper methods
        int insertLabel(MyGUI::Widget *const control, Ogre::String const&labelName, Ogre::String const&labelCaption, int x)
        {
            MyGUI::TextBox *ptr = nullptr;;
            return insertLabel(control, labelName, labelCaption, x, ptr);
        }
        int insertLabel(MyGUI::Widget *const control, const Ogre::String &labelName, const Ogre::String &labelCaption, int x, MyGUI::TextBox *&labelPtr);
        /// Creates a left aligned Textbox with the property id as label. Returns the label.right value.
        int insertPropertyIdLabel(MyGUI::Widget *const control, Steel::PropertyGridProperty *const property);
        /// Read the property value into the given control.
        void updateControlValue(MyGUI::Widget *control, PropertyGridProperty *const property);
        
        
        // not owned
        MyGUI::ScrollView *mContainerWidget = nullptr;
        PropertyGridAdapter *mAdapter = nullptr;

        // owned
        struct Signals
        {
            Signal newAdapterProperty = INVALID_SIGNAL;
            SignalSet propertyChanged;
        };
        Signals mSignals;
        
        typedef std::map<PropertyGridPropertyId, MyGUI::Widget *> ControlsMap;
        /// Maps a property id to its dedicated widget
        ControlsMap mControlsMap;
        
        /// Properties Ranges have to be saved in between callbacks, and deleted afterwards
        std::map<PropertyGridPropertyId, PropertyGridProperty::Range *> mRanges;
    };
}

#endif // STEEL_PROPERTYGRIDMANAGER_H
