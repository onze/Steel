#ifndef STEEL_PROPERTYGRIDADAPTER_H
#define STEEL_PROPERTYGRIDADAPTER_H

#include <functional>

#include "steeltypes.h"
#include "SignalEmitter.h"
#include "SignalListener.h"

namespace Steel
{
    enum class PropertyGridPropertyValueType : u32
    {
        None = 0,
        Bool,
        String,
        Range,
        StringVectorSelection
    };

    Ogre::String toString(PropertyGridPropertyValueType valueType);

    /**
     * A property is a 2 ways binding
     * between a property grid control and "whatever the adapter is adapting" (say, an object's
     * member value).
     * Said differently, it provides an interface that the PropertyGridManager can understand and build
     * controls to (1)reflect and (2)affect the property's state.
     */
    class PropertyGridProperty : public SignalEmitter
    {
    public:
        struct StringVectorSelection
        {
            typedef u32 selection_type;
            StringVector selectableValues;
            selection_type selectedIndex;
        };

        struct Range
        {
            typedef f32 value_type;
            value_type min;
            value_type max;
            value_type value;
            // TODO: enum class RangeType : u32 {Linear = 1, Exponential = 2, LogLinear = 3, etc}
        };

        // read callbacks
        typedef std::function<bool(void)> BoolReadCallback;
        typedef std::function<Ogre::String(void)> StringReadCallback;
        typedef std::function<StringVectorSelection(void)> StringVectorSelectionReadCallback;
        typedef std::function<Range(void)> RangeReadCallback;

        // write callbacks
        typedef std::function<void(bool)> BoolWriteCallback;
        typedef std::function<void(Ogre::String const &)> StringWriteCallback;
        typedef std::function<void(StringVectorSelection::selection_type)> StringVectorSelectionWriteCallback;
        typedef std::function<void(Range const &)> RangeWriteCallback;

        PropertyGridProperty() = delete;
        PropertyGridProperty(PropertyGridPropertyId id);
        virtual ~PropertyGridProperty();

        PropertyGridPropertyId id() const {return mId;}
        PropertyGridPropertyValueType valueType() const {return mValueType;}

        bool isReadOnly()const {return mReadOnly;}

        // UI bindings

        void read(bool &value);/// Reads the property value into the given parameter.
        void write(bool value);/// Writes the given parameter to the property value.
        void setCallbacks(BoolReadCallback readCallback, BoolWriteCallback writeCallback);

        void read(Ogre::String &value);/// Reads the property value into the given parameter.
        void write(Ogre::String const &value);/// Writes the given parameter to the property value.
        void setCallbacks(StringReadCallback readCallback, StringWriteCallback writeCallback);

        void read(StringVectorSelection &value);/// Reads the property value into the given parameter.
        void write(StringVectorSelection::selection_type value);/// Writes the given parameter to the property value.
        void setCallbacks(StringVectorSelectionReadCallback readCallback, StringVectorSelectionWriteCallback writeCallback);

        void read(Range &value);/// Reads the property value into the given parameter.
        void write(Range const &value);/// Writes the given parameter to the property value.
        void setCallbacks(RangeReadCallback readCallback, RangeWriteCallback writeCallback);

        enum class PublicSignal : u32
        {
            /// Emitted when the property value changes (should be emitted by adapter that created the property).
            changed = 0,
        };
        Signal getSignal(PropertyGridProperty::PublicSignal signal) const;
    private:
        // owned
        PropertyGridPropertyId mId;
        /// the type of value this property represents
        PropertyGridPropertyValueType mValueType;
        bool mReadOnly = true;

        BoolReadCallback mBoolReadCallback;
        BoolWriteCallback mBoolWriteCallback;

        StringReadCallback mStringReadCallback;
        StringWriteCallback mStringWriteCallback;

        StringVectorSelectionReadCallback mStringVectorSelectionReadCallback;
        StringVectorSelectionWriteCallback mStringVectorSelectionWriteCallback;

        RangeReadCallback mRangeReadCallback;
        RangeWriteCallback mRangeWriteCallback;
    };

    typedef std::vector<PropertyGridProperty *> PropertyGridPropertyVector;

    /**
     * Feeds PropertyGridProperty to a PropertyGridManager.
     *
     * Since the adaptee (what the adapater is adapting) is not necessarily equiped with all the reflection
     * to be directly plugged into the editor's UI, adapters fills this gap.
     */
    class PropertyGridAdapter : public SignalListener, public SignalEmitter
    {
    public:
        PropertyGridAdapter();
        virtual ~PropertyGridAdapter();

        virtual PropertyGridPropertyVector const &properties();

        virtual void onSignal(Signal signal, SignalEmitter *const source) override;

        enum class PublicSignal : u32
        {
            /// Emitted when the adapter has a new property to adapt
            newProperty = 1
        };

        /// to be overloaded by subclasses if needed (in order to listen to adaptee changes for instance...)
        virtual Signal getSignal(PropertyGridAdapter::PublicSignal signal) const;

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
