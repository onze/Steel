#ifndef STEEL_SELECTIONBOX_H
#define STEEL_SELECTIONBOX_H

#include <OgreVector2.h>
#include <OgreManualObject.h>
#include <steeltypes.h>

namespace Ogre
{
class Camera;
class PlaneBoundedVolumeListSceneQuery;
}

namespace Steel
{
class Engine;

/**
 * SelectionBox, implements volume selection.
 * Largely inspired by the 4th Ogre3d tutorial
 * (see http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Intermediate+Tutorial+4 )
 */
class SelectionBox: public Ogre::ManualObject
{

public:
    SelectionBox(const Ogre::String name, Engine *engine);
    virtual ~SelectionBox();
    virtual bool operator==(const SelectionBox& other) const;

    void performSelection(std::list<AgentId> &selection, Ogre::Camera *camera);

    void setCorners(float left, float top, float right, float bottom);
    void setCorners(const Ogre::Vector2& topLeft, const Ogre::Vector2& bottomRight);
protected:
    // not owned
    Engine *mEngine;

    // owned
    /// bounding values.
    float mLeft, mTop, mRight, mBottom;
    /// used in the selectionBox query
    Ogre::PlaneBoundedVolumeListSceneQuery *mVolQuery;
};

}

#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
