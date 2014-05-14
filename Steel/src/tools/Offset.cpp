#include "tools/Offset.h"
#include <tools/JsonUtils.h>
#include <models/OgreModel.h>
#include <UI/Editor.h>
#include <SignalManager.h>
#include <json/value.h>

namespace Steel
{

    Offset::Offset(): mEditor(nullptr), mPrefix(StringUtils::BLANK),
        mIsEditable(false), mPosition(Ogre::Vector3::ZERO), mRotation(Ogre::Quaternion::IDENTITY),
        mChangedSignal(INVALID_SIGNAL)
    {
    }

    Offset::Offset(const Offset &o): mEditor(o.mEditor), mPrefix(o.mPrefix),
        mIsEditable(o.mIsEditable), mPosition(o.mPosition), mRotation(o.mRotation),
        mChangedSignal(o.mChangedSignal)
    {
    }

    Offset::~Offset()
    {
        if(mIsEditable)
            setEditable(false, nullptr, StringUtils::BLANK);
    }

    Offset &Offset::operator=(const Offset &o)
    {
        if(&o != this)
        {
            mEditor = o.mEditor;
            mPrefix = o.mPrefix;
            mPosition = o.mPosition;
            mRotation = o.mRotation;
            mChangedSignal = o.mChangedSignal;

            if(mIsEditable != o.mIsEditable)
                setEditable(o.mIsEditable, nullptr, StringUtils::BLANK);
        }

        return *this;
    }

    bool Offset::operator==(const Offset &o) const
    {
        return mPrefix == o.mPrefix && mPosition == o.mPosition && mRotation == o.mRotation && mChangedSignal == o.mChangedSignal;
    }

    void Offset::fromJson(Json::Value const &node)
    {
        Json::Value value;

        //position
        value = node[OgreModel::POSITION_ATTRIBUTE];

        if(!value.isNull())
            mPosition = Ogre::StringConverter::parseVector3(value.asString());

        // rotation
        value = node[OgreModel::ROTATION_ATTRIBUTE];

        if(!value.isNull())
            mRotation = Ogre::StringConverter::parseQuaternion(value.asString());
    }

    void Offset::toJson(Json::Value &node)
    {
        if(mPosition != Ogre::Vector3::ZERO)
            node[OgreModel::POSITION_ATTRIBUTE] = JsonUtils::toJson(mPosition);

        if(mRotation != Ogre::Quaternion::IDENTITY)
            node[OgreModel::ROTATION_ATTRIBUTE] = JsonUtils::toJson(mRotation);
    }
    
    void Offset::setEditable(bool flag, Editor *const editor, Ogre::String const &prefix)
    {
        if(!prefix.empty())
            mPrefix = prefix;

        if(nullptr != editor)
            mEditor = editor;

        if(flag)
        {
            // can happen
            if(nullptr != mEditor)
            {
                #ifdef DEBUGVALUEMANAGER
                float delta = 2.f;
                mEditor->addDebugValue(mPrefix + ".offset.position.x",
                [this](float v) {setPosition(v, mPosition.y, mPosition.z);},
                mPosition.x - delta, mPosition.x + delta);

                mEditor->addDebugValue(mPrefix + ".offset.position.y",
                [this](float v) {setPosition(mPosition.x, v, mPosition.z);},
                mPosition.y - delta, mPosition.y + delta);

                mEditor->addDebugValue(mPrefix + ".offset.position.z",
                [this](float v) {setPosition(mPosition.x, mPosition.y, v);},
                mPosition.z - delta, mPosition.z + delta);
                #endif
                
                mIsEditable = true;
            }
        }
        else
        {
            if(nullptr != mEditor)
            {
                #ifdef DEBUGVALUEMANAGER
                mEditor->removeDebugValue(mPrefix + ".offset.position.x");
                mEditor->removeDebugValue(mPrefix + ".offset.position.y");
                mEditor->removeDebugValue(mPrefix + ".offset.position.z");
                #endif
                mIsEditable = false;
            }
        }
    }

    void Offset::setPosition(float x, float y, float z)
    {
        mPosition.x = x;
        mPosition.y = y;
        mPosition.z = z;
        SignalManager::instance().emit(mChangedSignal);
    }

    void Offset::setRotation(float x, float y, float z, float w)
    {
        mRotation.x = x;
        mRotation.y = y;
        mRotation.z = z;
        mRotation.w = w;
        SignalManager::instance().emit(mChangedSignal);
    }


    Signal Offset::getChangeSignal()
    {
        if(INVALID_SIGNAL == mChangedSignal)
            mChangedSignal = SignalManager::instance().anonymousSignal();

        return mChangedSignal;
    }
}
