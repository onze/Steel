#ifndef STEEL_OFFSET_H
#define STEEL_OFFSET_H

#include <OgreVector3.h>
#include <steeltypes.h>
#include <json/value.h>

namespace Steel
{
    class Editor;

    class Offset
    {
    public:
        Offset();
        Offset(const Offset &other);
        ~Offset();
        Offset &operator=(const Offset &other);
        bool operator==(const Offset &other) const;

        void fromJson(Json::Value const &node);
        void toJson(Json::Value &node);

        void setEditable(bool flag, Editor *const editor, Ogre::String const &prefix);

        inline Ogre::Vector3 const &position() const {return mPosition;}
        void setPosition(float x, float y, float z);

        inline Ogre::Quaternion const &rotation() const {return mRotation;}
        void setRotation(float x, float y, float z, float w);

        /// Returns the signbal emitted when the offset changes
        Signal getChangeSignal();

        inline bool isEditable() const {return mIsEditable;};
    private:
        // not owned
        Editor *mEditor;
        // owned
        Ogre::String mPrefix;
        bool mIsEditable;
        Ogre::Vector3 mPosition;
        Ogre::Quaternion mRotation;
        Signal mChangedSignal;
    };
}

#endif // STEEL_OFFSET_H
