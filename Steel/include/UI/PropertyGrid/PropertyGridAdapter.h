#ifndef STEEL_PROPERTYGRIDADAPTER_H
#define STEEL_PROPERTYGRIDADAPTER_H

#include <functional>

#include "steeltypes.h"
#include <SignalEmitter.h>

namespace Steel
{
    enum class PropertyGridPropertyValueType : u32
    {
        None = 0,
        Bool,
        Float,
        StringVectorSelection
    };

    Ogre::String toString(PropertyGridPropertyValueType valueType);

    class PropertyGridProperty : public SignalEmitter
    {
    public:
        struct StringVectorSelection
        {
            typedef u32 selection_type;
            StringVector selectableValues;
            selection_type selectedIndex;
        };
        
        // read callbacks
        typedef std::function<bool(void)> BoolReadCallback;
        typedef std::function<StringVectorSelection(void)> StringVectorSelectionReadCallback;
        
        // write callbacks
        typedef std::function<void(bool)> BoolWriteCallback;
        typedef std::function<void(StringVectorSelection::selection_type)> StringVectorSelectionWriteCallback;

        PropertyGridProperty() = delete;
        PropertyGridProperty(PropertyGridPropertyId id);
        virtual ~PropertyGridProperty();

        PropertyGridPropertyId id() const {return mId;}
        PropertyGridPropertyValueType valueType() const {return mValueType;}
        
        /// Reads the property value into the given parameter.
        void read(bool &value);
        /// Writes the given parameter to the property value.
        void write(bool value);
        void setCallbacks(BoolReadCallback readCallback, BoolWriteCallback writeCallback);
        
        void read(StringVectorSelection &value);
        void write(StringVectorSelection::selection_type value);
        void setCallbacks(StringVectorSelectionReadCallback readCallback, StringVectorSelectionWriteCallback writeCallback);
        
        
        enum class PublicSignal : u32
        {
            /// Emitted when the property value changes
            changed = 0,
        };
        Signal getSignal(PropertyGridProperty::PublicSignal signal) const;
    private:
        // owned
        PropertyGridPropertyId mId;
        /// the type of value this property represents
        PropertyGridPropertyValueType mValueType;
        
        BoolReadCallback mBoolReadCallback;
        BoolWriteCallback mBoolWriteCallback;
        
        StringVectorSelectionReadCallback mStringVectorSelectionReadCallback;
        StringVectorSelectionWriteCallback mStringVectorSelectionWriteCallback;
        
    };

    typedef std::vector<PropertyGridProperty *> PropertyGridPropertyVector;

    /**
     * Feeds properties to a PropertyGridManager. A property is a 2 ways binding
     * between a property grid control and "whatever the adapter is adapting" (say, an object's
     * member value).
     * Said differently, it provides an interface that the PropertyGridManager can understand and build
     * controls to (1)reflect and (2)affect the property's state.
     *
     * Since the adaptee (what the adapater is adapting) is not necessarily equiped with all the reflection
     * to be directly plugged into the editor's UI, adapters fills this gap.
     */
    class PropertyGridAdapter
    {
    public:
        PropertyGridAdapter();
        virtual ~PropertyGridAdapter();

        virtual PropertyGridPropertyVector const &properties();


        enum class PublicSignal : u32
        {
            newProperty = 1
        };
        Signal getSignal(PropertyGridAdapter::PublicSignal signal) const;

    protected:
        /// Meant to be overriden by subclass. Fills mProperties according to adaptee.
        virtual void buildProperties() {};
        /// Removes and deletes all properties from mProperties.
        void removeProperties();

        // owned
        PropertyGridPropertyVector mProperties;
    };
}

#endif // STEEL_PROPERTYGRIDADAPTER_H
