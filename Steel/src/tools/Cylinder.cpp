#include "tools/Cylinder.h"

#include <OgreSceneNode.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreMeshManager.h>

using namespace Ogre;

namespace Steel
{

    MeshPtr Cylinder::getMesh(SceneManager *sceneManager, int radius, int height)
    {
        MeshPtr mesh=MeshPtr(Ogre::MeshManager::getSingletonPtr()->getByName("manualMesh_Cylinder","UI"));
        if(!mesh.isNull())
            return mesh;

        //// this is an adaptation of
        //// http://www.ogre3d.org/tikiwiki/tiki-index.php?page=ManualSphereMeshes&structure=Cookbook
        // horizonal rings
        int nRings=6;
        int nSegments=18;

        ManualObject *manual = sceneManager->createManualObject("manualObject_cylinder");
        manual->begin("BaseWhiteNoLighting", RenderOperation::OT_TRIANGLE_LIST);

        float fDeltaRingAngle = (Math::PI / nRings);
        float fDeltaSegAngle = (2 * Math::PI / nSegments);
        unsigned short wVerticeIndex = 0 ;

        // Generate the group of rings for the sphere
        for( int ring = 0; ring <= nRings; ring++ )
        {
            float r0 = radius * std::sin(ring * fDeltaRingAngle);
            float y0 = radius * std::cos(ring * fDeltaRingAngle);

            // Generate the group of segments for the current ring
            for(int seg = 0; seg <= nSegments; seg++)
            {
                float x0 = r0 * std::sin(seg * fDeltaSegAngle);
                float z0 = r0 * std::cos(seg * fDeltaSegAngle);

                // Add one vertex to the strip which makes up the sphere
                manual->position( x0, y0, z0);
                manual->normal(Vector3(x0, y0, z0).normalisedCopy());
                manual->textureCoord((float) seg / (float) nSegments, (float) ring / (float) nRings);

                if (ring != nRings)
                {
                    // each vertex (except the last) has six indicies pointing to it
                    manual->index(wVerticeIndex + nSegments + 1);
                    manual->index(wVerticeIndex);
                    manual->index(wVerticeIndex + nSegments);
                    manual->index(wVerticeIndex + nSegments + 1);
                    manual->index(wVerticeIndex + 1);
                    manual->index(wVerticeIndex);
                    wVerticeIndex ++;
                }
            }; // end for seg
        } // end for ring
        manual->end();
        mesh = manual->convertToMesh("manualMesh_Cylinder","UI");
        mesh->_setBounds( AxisAlignedBox( Vector3(-radius, -radius, -radius), Vector3(radius, radius, radius) ), false );

        mesh->_setBoundingSphereRadius(radius);
        unsigned short src, dest;
        if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
        {
            mesh->buildTangentVectors(VES_TANGENT, src, dest);
        }
        return mesh;
    }

    SceneNode *Cylinder::getSceneNode(SceneManager *sceneManager,SceneNode *parent, Ogre::String name,int radius, int height)
    {
        Ogre::SceneNode *node;
        try
        {
            node=static_cast<Ogre::SceneNode *>(parent->getChild(name));
        }
        catch(Ogre::Exception e)
        {
            MeshPtr mesh=Cylinder::getMesh(sceneManager,radius,height);
            Entity* entity;
            if(sceneManager->hasEntity("cylinderEntity"))
                entity=sceneManager->getEntity("cylinderEntity");
            else
                entity = sceneManager->createEntity("cylinderEntity",mesh);
            node = parent->createChildSceneNode(name);
//         sphereEntity->setMaterialName("material_name_goes_here");
            node->attachObject(entity);
        }
        return node;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
