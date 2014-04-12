#include "tools/DynamicLines.h"

#include <Ogre.h>
#include <cassert>
#include <cmath>

namespace Steel
{

    enum
    {
        POSITION_BINDING,
        TEXCOORD_BINDING
    };

    u32 DynamicLines::sNextId = 0;

    DynamicLines::DynamicLines(): DynamicRenderable(),
        mId(sNextId++),
        mPoints(), mColors(),
        mDirty(false), mUse2D(false)
    {
    }

    DynamicLines::~DynamicLines()
    {
        clear();
        {
            auto name = mMaterial->getName();
            mMaterial.setNull();
            Ogre::MaterialManager::getSingleton().remove(name);
        }
    }

    void DynamicLines::init(Ogre::String const &resourceGroup,
                            Ogre::RenderOperation::OperationType opType /*= Ogre::RenderOperation::OT_LINE_STRIP*/,
                            bool use2d /*= false*/)
    {
        mPoints.clear();
        mColors.clear();
        //dynamic renderable
        initialize(opType, false);

        {
            Ogre::MaterialPtr const whiteNoLignthing = Ogre::MaterialManager::getSingleton().getByName("BaseWhiteNoLighting");
            Ogre::String const resName = "DynamicLines_" + Ogre::StringConverter::toString(mId) + "_BaseWhiteNoLighting_" + resourceGroup;
            mMaterial = Ogre::MaterialManager::getSingleton().getByName(resName);

            if(mMaterial.isNull())
            {
                mMaterial = whiteNoLignthing->clone(resName, true, resourceGroup);
                mMaterial->load();
            }

            mMaterial->setDepthCheckEnabled(!use2d);
        }
        setMaterial(mMaterial->getName());

        setUse2D(use2d);
        mDirty = false;
    }

    void DynamicLines::setColor(Ogre::ColourValue color)
    {
        // color-coded id
        unsigned int matIndex = ((unsigned int)(color.r) << 24) +
                                ((unsigned int)(color.g) << 16) +
                                ((unsigned int)(color.b) << 8) +
                                ((unsigned int)(color.a));
        Ogre::String matName = "DL" + Ogre::StringConverter::toString(matIndex);
        Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().createOrRetrieve(matName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME).first.staticCast<Ogre::Material>();
        materialPtr->setAmbient(color);
        setMaterial(matName);
    }

    void DynamicLines::setUse2D(bool flag)
    {
        setUseIdentityProjection(flag);
        setUseIdentityView(flag);

        mUse2D = flag;
        mDirty = true;

        update();
    }

    void DynamicLines::setOperationType(Ogre::RenderOperation::OperationType const opType)
    {
        if(opType != mRenderOp.operationType)
        {
            update();
            mRenderOp.operationType = opType;
        }
    }

    Ogre::RenderOperation::OperationType DynamicLines::getOperationType() const
    {
        return mRenderOp.operationType;
    }

    void DynamicLines::addPoint(const Ogre::Vector3 &p)
    {
        mPoints.push_back(p);//position
        mColors.push_back( {}); //color
        mDirty = true;
    }

    void DynamicLines::addPoint(Ogre::Real x, Ogre::Real y, Ogre::Real z)
    {
        addPoint( {x, y, z});
    }

    void DynamicLines::addPoint(const Ogre::Vector2 &p)
    {
        addPoint( {p.x, p.y, 0});
    }

    void DynamicLines::addPoint(Ogre::Real x, Ogre::Real y)
    {
        addPoint( {x, y, .0f});
    }

    const Ogre::Vector3 &DynamicLines::getPoint(unsigned short index) const
    {
        assert(index < mPoints.size() && "Point index is out of bounds!!");
        return mPoints[index];
    }

    unsigned short DynamicLines::getNumPoints(void) const
    {
        return (unsigned short)mPoints.size();
    }

    void DynamicLines::setPoint(unsigned short index, const Ogre::Vector3 &value)
    {
        assert(index < mPoints.size() && "Point index is out of bounds!!");

        mPoints[index] = value;
        mDirty = true;
    }

    void DynamicLines::clear()
    {
        mPoints.clear();
        mDirty = true;
    }

    void DynamicLines::update()
    {
        if(!mDirty) return;

        setRenderQueueGroup(mUse2D ? Ogre::RENDER_QUEUE_OVERLAY : Ogre::RENDER_QUEUE_MAIN);
        fillHardwareBuffers();
    }

    void DynamicLines::createVertexDeclaration()
    {
        // we define the meaning of each batch of vertices added to the buffer
        //http://grahamedgecombe.com/blog/custom-meshes-in-ogre3d
        Ogre::VertexDeclaration *decl = mRenderOp.vertexData->vertexDeclaration;
        size_t offset = 0;

        decl->addElement(POSITION_BINDING, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
        // dynamic renderable only has 1 buffer
//         decl->addElement(POSITION_BINDING, offset, Ogre::VET_FLOAT4, Ogre::VES_POSITION);
//         offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
    }

    void DynamicLines::fillHardwareBuffers()
    {
        unsigned int size = (unsigned int)mPoints.size();

        prepareHardwareBuffers(size, 0);

        if(!size)
        {
            mBox.setExtents(Ogre::Vector3::ZERO, Ogre::Vector3::ZERO);
            mDirty = false;
            return;
        }

        Ogre::Vector3 vaabMin = mPoints[0];
        Ogre::Vector3 vaabMax = mPoints[0];

        Ogre::HardwareVertexBufferSharedPtr vbuf = mRenderOp.vertexData->vertexBufferBinding->getBuffer(0);

        Ogre::Real *prPos = static_cast<Ogre::Real *>(vbuf->lock(Ogre::HardwareBuffer::HBL_DISCARD));
        {
            for(unsigned int i = 0; i < size; i++)
            {
                *prPos++ = mPoints[i].x;
                *prPos++ = mPoints[i].y;
                *prPos++ = mPoints[i].z;

                if(!mUse2D)
                {
                    if(mPoints[i].x < vaabMin.x)
                        vaabMin.x = mPoints[i].x;

                    if(mPoints[i].y < vaabMin.y)
                        vaabMin.y = mPoints[i].y;

                    if(mPoints[i].z < vaabMin.z)
                        vaabMin.z = mPoints[i].z;

                    if(mPoints[i].x > vaabMax.x)
                        vaabMax.x = mPoints[i].x;

                    if(mPoints[i].y > vaabMax.y)
                        vaabMax.y = mPoints[i].y;

                    if(mPoints[i].z > vaabMax.z)
                        vaabMax.z = mPoints[i].z;
                }
            }
        }

        vbuf->unlock();

        if(mUse2D)
            mBox.setInfinite();
        else
            mBox.setExtents(vaabMin, vaabMax);

        mDirty = false;
    }

    /*
     void DynamicLines::getWorldTransforms(Ogre::*Matrix4 *xform) const
    {
    // return identity matrix to prevent parent transforms
        *xform = Ogre::Matrix4::IDENTITY;
    }
    */
    /*
     const Ogre::*Quaternion &DynamicLines::getWorldOrientation(void) const
    {
        return Ogre::Quaternion::IDENTITY;
    }

    const Ogre::Vector3 &DynamicLines::getWorldPosition(void) const
    {
        return Ogre::Vector3::ZERO;
    }
    */
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

