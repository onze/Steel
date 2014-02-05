#include "UI/SelectionBox.h"
#include <Level.h>
#include <tools/OgreUtils.h>
#include <Engine.h>

#include <OgreCamera.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>

namespace Steel
{

    SelectionBox::SelectionBox(const Ogre::String name, Engine *engine):
        Ogre::ManualObject(name),
        mEngine(engine),
        mLeft(.0f), mTop(.0f), mRight(.0f), mBottom(.0f),
        mVolQuery(nullptr)
    {
        /*The first function sets the render queue for the object to be the Overlay queue.
         The next two functions set the projection and view matrices to be the identity. Projection and
         view matrices are used by many rendering systems (such as OpenGL and DirectX) to define where
         objects go in the world. Since Ogre abstracts this away for us, we won't go into detail about
         what these matrices actually are or what they do. Instead what you need to know is that if you
         set the projection and view matrix to be the identity, as we have here, we will basically create a
         2D object. When defining this object, the coordinate system changes a bit. We no longer deal with
         the Z axis (if you are asked for the Z axis, set the value to -1). Instead we have a new coordinate
         system with X and Y running from -1 to 1 inclusive. Lastly, we will set the query flags for the object
         to be 0, which will prevent the selection rectangle from being included in the query results.*/
        setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY); // when using this, ensure Depth Check is Off in the material
        setUseIdentityProjection(true);
        setUseIdentityView(true);
        setQueryFlags(0);

        if(nullptr != mEngine->level())
        {
            mEngine->level()->levelRoot()->createChildSceneNode()->attachObject(this);
            mVolQuery = mEngine->level()->sceneManager()->createPlaneBoundedVolumeQuery(Ogre::PlaneBoundedVolumeList());
        }
    }

    SelectionBox::~SelectionBox()
    {
        clear();

        if(nullptr != mVolQuery)
        {
            mEngine->level()->sceneManager()->destroyQuery(mVolQuery);
            mVolQuery = nullptr;
        }
    }

    bool SelectionBox::operator==(const SelectionBox &o) const
    {
        return mLeft == o.mLeft && mTop == o.mTop && mRight == o.mRight && mBottom == o.mBottom;
    }

    void SelectionBox::setCorners(float left, float top, float right, float bottom)
    {

        /*Now that the object is set up, we need to actually build the rectangle. We have one small snag before we
         get started. We are going to be calling this function with mouse locations. That is, we will be given a
         number between 0 and 1 for the x and y coordinates, yet we need to convert these to numbers in the range
         -1, 1. To make matters slightly more complicated, the y coordinate is backwards too. The mouse cursor in
         CEGUI defines the top of the screen at 0, the bottom at 1. In our new coordinate system, the top of the
         screen is +1, the bottom is -1. Thankfully, a few quick conversions will take care of this problem. */
        mLeft = left * 2 - 1;
        mRight = right * 2 - 1;
        mTop = 1 - top * 2;
        mBottom = 1 - bottom * 2;

        if(mLeft > mRight)
            std::swap(mLeft, mRight);

        if(mTop > mBottom)
            std::swap(mTop, mBottom);

        /*Now the positions are in the new coordinate system. Next we need to actually build the object. To do this,
         * we first call the begin() method. It takes in two parameters, the name of the material to use for this section of the
         * object, and the render operation to be used to draw it. Since we are not putting a texture on this, we will leave
         * the material blank. The second parameter is the RenderOperation. We can render the mesh using points, lines, or
         * triangles. We would use triangles if we were rendering a full mesh, but since we want an empty rectangle, we will
         * use the line strip. The line strip draws a line to each vertex from the previous vertex you defined.*/
        clear();
        //Every time you call ManualObject::clear(), the bounding box is reset
        setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
        begin("", Ogre::RenderOperation::OT_LINE_STRIP);
        position(mLeft, mTop, -1);
        position(mRight, mTop, -1);
        position(mRight, mBottom, -1);
        position(mLeft, mBottom, -1);
        position(mLeft, mTop, -1);
        end();
    }

    void SelectionBox::setCorners(const Ogre::Vector2 &topLeft, const Ogre::Vector2 &bottomRight)
    {
        setCorners(topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
    }

    void SelectionBox::performSelection(std::list<AgentId> &selection, Ogre::Camera *mCamera)
    {
        if((mRight - mLeft) * (mBottom - mTop) < 0.0001)
            return;

        float left = (mLeft + 1.f) / 2.f;
        float right = (mRight + 1.f) / 2.f;
        float top = (1.f - mBottom) / 2.f;
        float bottom = (1.f - mTop) / 2.f;
        Ogre::Ray topLeft = mCamera->getCameraToViewportRay(left, top);
        Ogre::Ray topRight = mCamera->getCameraToViewportRay(right, top);
        Ogre::Ray bottomLeft = mCamera->getCameraToViewportRay(left, bottom);
        Ogre::Ray bottomRight = mCamera->getCameraToViewportRay(right, bottom);

        // These planes have now defined an "open box" which extends to infinity in front of the camera. You can think of
        // the rectangle we drew with the mouse as being the termination point of the box just in front of the camera.
        Ogre::PlaneBoundedVolume vol;
        const Ogre::Real min = .1, max = 500;
        vol.planes.push_back(Ogre::Plane(topLeft.getPoint(min), topRight.getPoint(min), bottomRight.getPoint(min)));         // front plane
        vol.planes.push_back(Ogre::Plane(topLeft.getOrigin(), topLeft.getPoint(max), topRight.getPoint(max)));         // top plane
        vol.planes.push_back(Ogre::Plane(topLeft.getOrigin(), bottomLeft.getPoint(max), topLeft.getPoint(max)));       // left plane
        vol.planes.push_back(Ogre::Plane(bottomLeft.getOrigin(), bottomRight.getPoint(max), bottomLeft.getPoint(max)));   // bottom plane
        vol.planes.push_back(Ogre::Plane(topRight.getOrigin(), topRight.getPoint(max), bottomRight.getPoint(max)));     // right plane

        Ogre::PlaneBoundedVolumeList volList;
        volList.push_back(vol);

        mVolQuery->setVolumes(volList);
        Ogre::SceneQueryResult result = mVolQuery->execute();

        // Finally we need to handle the results of the query. First we will deselect all previously selected objects,
        // then we will select all objects which were found by the query.
        std::list<Ogre::SceneNode *> nodes;
        Ogre::SceneQueryResultMovableList::iterator iter;

        for(iter = result.movables.begin(); iter != result.movables.end(); ++iter)
        {
            Ogre::MovableObject *movable = *iter;

            if(movable->getMovableType().compare("Entity") == 0)
            {
                Ogre::Entity *pentity = static_cast<Ogre::Entity *>(movable);
                nodes.push_back(pentity->getParentSceneNode());
            }
        }

        mEngine->level()->getAgentsIdsFromSceneNodes(nodes, selection);
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 


