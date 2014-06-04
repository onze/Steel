#include "UI/PropertyGrid/PropertyGridAdapter.h"
#include <SignalManager.h>
#include <Debug.h>

namespace Steel
{

    Ogre::String toString(PropertyGridPropertyValueType valueType)
    {
#define STEEL_TOSTRING_CASE(NAME) case NAME:return #NAME

        switch(valueType)
        {
                STEEL_TOSTRING_CASE(PropertyGridPropertyValueType::Bool);
                STEEL_TOSTRING_CASE(PropertyGridPropertyValueType::String);
                STEEL_TOSTRING_CASE(PropertyGridPropertyValueType::Range);
                STEEL_TOSTRING_CASE(PropertyGridPropertyValueType::StringVectorSelection);
                STEEL_TOSTRING_CASE(PropertyGridPropertyValueType::None);
        }

#undef STEEL_TOSTRING_CASE
        return "PropertyGridPropertyValueType::None";
    }

    PropertyGridProperty::PropertyGridProperty(PropertyGridPropertyId id): SignalEmitter(),
        mId(id), mValueType(PropertyGridPropertyValueType::None)
    {

    }

    PropertyGridProperty::~PropertyGridProperty()
    {

    }

////////////////////////////// <PropertyGridPropertyValueType::Bool>
    void PropertyGridProperty::setCallbacks(BoolReadCallback readCallback, BoolWriteCallback writeCallback)
    {
        mValueType = PropertyGridPropertyValueType::Bool;
        mBoolReadCallback = readCallback;
        mBoolWriteCallback = writeCallback;
        mReadOnly = nullptr == writeCallback;
    }

    void PropertyGridProperty::read(bool &value)
    {
        if(nullptr != mBoolReadCallback)
            value = mBoolReadCallback();
    }

    void PropertyGridProperty::write(bool value)
    {
        if(nullptr != mBoolWriteCallback)
        {
            mBoolWriteCallback(value);
            emit(getSignal(PublicSignal::changed));
        }
        else
            Debug::warning(*this, " is a read-only property.").endl();
    }
////////////////////////////// </PropertyGridPropertyValueType::Bool>

////////////////////////////// <PropertyGridPropertyValueType::String>
    void PropertyGridProperty::setCallbacks(StringReadCallback readCallback, StringWriteCallback writeCallback)
    {
        mValueType = PropertyGridPropertyValueType::String;
        mStringReadCallback = readCallback;
        mStringWriteCallback = writeCallback;
        mReadOnly = nullptr == writeCallback;
    }

    void PropertyGridProperty::read(Ogre::String &value)
    {
        if(nullptr != mStringReadCallback)
            value = mStringReadCallback();
    }

    void PropertyGridProperty::write(Ogre::String const &value)
    {
        if(nullptr != mStringWriteCallback)
        {
            mStringWriteCallback(value);
            emit(getSignal(PublicSignal::changed));
        }
        else
            Debug::warning(*this, " is a read-only property.").endl();
    }
////////////////////////////// </PropertyGridPropertyValueType::String>

////////////////////////////// <PropertyGridPropertyValueType::StringVectorSelection>
    void PropertyGridProperty::setCallbacks(StringVectorSelectionReadCallback readCallback, StringVectorSelectionWriteCallback writeCallback)
    {
        mValueType = PropertyGridPropertyValueType::StringVectorSelection;
        mStringVectorSelectionReadCallback = readCallback;
        mStringVectorSelectionWriteCallback = writeCallback;
        mReadOnly = nullptr == writeCallback;
    }

    void PropertyGridProperty::read(StringVectorSelection &value)
    {
        if(nullptr != mStringVectorSelectionReadCallback)
            value = mStringVectorSelectionReadCallback();
    }

    void PropertyGridProperty::write(StringVectorSelection::selection_type value)
    {
        if(nullptr != mStringVectorSelectionWriteCallback)
        {
            mStringVectorSelectionWriteCallback(value);
            emit(getSignal(PublicSignal::changed));
        }
        else
            Debug::warning(*this, " is a read-only property.").endl();
    }
////////////////////////////// </PropertyGridPropertyValueType::StringVectorSelection>

////////////////////////////// <PropertyGridPropertyValueType::Range>
    void PropertyGridProperty::setCallbacks(RangeReadCallback readCallback, RangeWriteCallback writeCallback)
    {
        mValueType = PropertyGridPropertyValueType::Range;
        mRangeReadCallback = readCallback;
        mRangeWriteCallback = writeCallback;
        mReadOnly = nullptr == writeCallback;
    }

    void PropertyGridProperty::read(Range &value)
    {
        if(nullptr != mRangeReadCallback)
            value = mRangeReadCallback();
    }

    void PropertyGridProperty::write(Range const &value)
    {
        if(nullptr != mRangeWriteCallback)
        {
            mRangeWriteCallback(value);
            emit(getSignal(PublicSignal::changed));
        }
        else
            Debug::warning(*this, " is a read-only property.").endl();
    }
////////////////////////////// </PropertyGridPropertyValueType::Range>

    Signal PropertyGridProperty::getSignal(PropertyGridProperty::PublicSignal signal) const
    {
#define STEEL_PROPERTYGRIDPROPERTY_GETSIGNAL_CASE(NAME) case NAME:return SignalManager::instance().toSignal("Steel::PropertyGridProperty::"#NAME)

        switch(signal)
        {
                STEEL_PROPERTYGRIDPROPERTY_GETSIGNAL_CASE(PublicSignal::changed);
        }

#undef STEEL_PROPERTYGRIDPROPERTY_GETSIGNAL_CASE
        return INVALID_SIGNAL;
    }

///////////////////////////////////////////////////////////////////

    PropertyGridAdapter::PropertyGridAdapter():
        mProperties()
    {
    }

    PropertyGridAdapter::~PropertyGridAdapter()
    {
        removeProperties();
    }

    Signal PropertyGridAdapter::getSignal(PropertyGridAdapter::PublicSignal signal) const
    {
#define STEEL_PROPERTYGRIDADAPTER_GETSIGNAL_CASE(NAME) case NAME:return SignalManager::instance().toSignal("Steel::PropertyGridAdapter::"#NAME)

        switch(signal)
        {
            STEEL_PROPERTYGRIDADAPTER_GETSIGNAL_CASE(PublicSignal::newProperty);
        }

#undef STEEL_PROPERTYGRIDADAPTER_GETSIGNAL_CASE
        return INVALID_SIGNAL;
    }

    void PropertyGridAdapter::removeProperties()
    {
        while(mProperties.size() > 0)
        {
            auto &prop = mProperties.back();
            mProperties.pop_back();
            STEEL_DELETE(prop);
        }
    }

    PropertyGridPropertyVector const &PropertyGridAdapter::properties()
    {
        if(mProperties.size() == 0)
            buildProperties();

        return mProperties;
    }
    
    void PropertyGridAdapter::onSignal(Signal signal, SignalEmitter *const source)
    {}

}
