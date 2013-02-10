#ifndef STEEL_SELECTIONBOX_H
#define STEEL_SELECTIONBOX_H

#include <OgreVector2.h>
#include <OgreManualObject.h>

namespace Steel
{
    /**
     * SelectionBox, largely inspired by the 4th Ogre3d tutorial
     * (see http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Intermediate+Tutorial+4 )
     */
    class SelectionBox:public Ogre::ManualObject
    {

        public:
            SelectionBox(const Ogre::String name);
            SelectionBox(const SelectionBox& other);
            virtual ~SelectionBox();
            virtual SelectionBox& operator=(const SelectionBox& other);
            virtual bool operator==(const SelectionBox& other) const;

            void setCorners(float left, float top, float right, float bottom);
            void setCorners(const Ogre::Vector2& topLeft, const Ogre::Vector2& bottomRight);
        protected:
            /// bounding values.
            float mLeft, mTop, mRight, mBottom;
    };

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
