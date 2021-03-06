/*
 * RayCaster.cpp
 *
 *  Created on: 2011-06-19
 *      Author: onze
 */

#include <iostream>

#include <Ogre.h>

#include "Debug.h"
#include "tools/RayCaster.h"
#include <Level.h>
#include <Engine.h>

namespace Steel
{

    RayCaster::RayCaster(Engine *engine):EngineEventListener(),
        mEngine(engine),mSceneManager(nullptr),mRaySceneQuery(nullptr)
    {
        engine->addEngineEventListener(this);
    }

    RayCaster::~RayCaster()
    {
        if(nullptr!=mSceneManager && nullptr!=mRaySceneQuery)
            mSceneManager->destroyQuery(mRaySceneQuery);
        if(nullptr!=mEngine)
        {
            mEngine->removeEngineEventListener(this);
            mEngine=nullptr;
        }
        mRaySceneQuery=nullptr;
        mSceneManager=nullptr;
    }

    void RayCaster::onLevelUnset(Level *level)
    {
        if(nullptr==level)
            return;
        mSceneManager->destroyQuery(mRaySceneQuery);
        mRaySceneQuery=nullptr;
        mSceneManager=nullptr;
    }

    void RayCaster::onLevelSet(Level *level)
    {
        if(nullptr==level)
        {
            mSceneManager=nullptr;
            mRaySceneQuery=nullptr;
            return;
        }
        mSceneManager=level->sceneManager();
        if(nullptr == mSceneManager)
            return;
        // create the ray scene query object
        mRaySceneQuery = mSceneManager->createRayQuery(Ogre::Ray(), Ogre::SceneManager::WORLD_GEOMETRY_TYPE_MASK);
        if (nullptr == mRaySceneQuery)
            Debug::log("Raycaster::Raycaster(): Failed to create Ogre::RaySceneQuery instance").endl();
        else
            mRaySceneQuery->setSortByDistance(true);
    }

    bool RayCaster::fromRay(Ogre::Ray &ray, std::list<Ogre::SceneNode *> &nodes)
    {
//         Debug::log("RayCaster::fromRay()").endl();

        // check we are initialised
        if (mRaySceneQuery != nullptr)
        {
            // create a query object
            mRaySceneQuery->setRay(ray);
            // execute the query, returns a vector of hits
            if (mRaySceneQuery->execute().size() <= 0)
            {
                // raycast did not hit an objects bounding box
//                 Debug::log("query executed but got nothing").endl();
                return false;
            }
        }
        else
        {
            Debug::error("Raycaster::fromRay(): Cannot raycast without RaySceneQuery instance.").endl();
            return false;
        }
        // at this point we have raycast to a series of different objects bounding boxes.
        // we need to test these different objects to see which is the first polygon hit.
        // there are some minor optimizations (distance based) that mean we wont have to
        // check all of the objects most of the time, but the worst case scenario is that
        // we need to test every triangle of every object.

        Ogre::Real closest_distance = -1.0f;
        Ogre::Vector3 closest_result;
        Ogre::RaySceneQueryResult &query_result = mRaySceneQuery->getLastResults();
//         Debug::log("ok, got ")(query_result.size())(" results.").endl();
        for (size_t qr_idx = 0; qr_idx < query_result.size(); ++qr_idx)
        {
//             Debug::log("query_result[")(qr_idx)("]: ")(query_result[qr_idx].movable->getMovableType()).endl();
            // stop checking if we have found a raycast hit that is closer
            // than all remaining entities
            if ((closest_distance >= 0.0f) && (closest_distance < query_result[qr_idx].distance))
            {
//                 break;
            }
            // only check this result if it's a hit against an entity
            if (query_result[qr_idx].movable != nullptr)
            {
                if ((query_result[qr_idx].movable->getMovableType().compare("Entity") == 0))
                {
                    // get the entity to check
                    Ogre::Entity *pentity = static_cast<Ogre::Entity*> (query_result[qr_idx].movable);
                    // mesh data to retrieve
                    size_t vertex_count;
                    size_t index_count;
                    Ogre::Vector3 *vertices;
                    Ogre::uint32 *indices;
                    // get the mesh information
                    getMeshInformation(	pentity,
                                        vertex_count,
                                        vertices,
                                        index_count,
                                        indices,
                                        pentity->getParentNode()->_getDerivedPosition(),
                                        pentity->getParentNode()->_getDerivedOrientation(),
                                        pentity->getParentNode()->_getDerivedScale());
                    // test for hitting individual triangles on the mesh
                    bool new_closest_found = false;
                    for (int i = 0; i < static_cast<int> (index_count); i += 3)
                    {
                        // check for a hit against this triangle
                        std::pair<bool, Ogre::Real> hit = Ogre::Math::intersects(ray,
                                                          vertices[indices[i]],
                                                          vertices[indices[i + 1]],
                                                          vertices[indices[i + 2]],
                                                          true,
                                                          false);
                        // if it was a hit check if its the closest
                        if (hit.first)
                        {
                            if ((closest_distance < 0.0f) || (hit.second < closest_distance))
                            {
                                // this is the closest so far, save it off
                                closest_distance = hit.second;
                                new_closest_found = true;
                            }
                        }
                    }

                    // free the verticies and indices memory
                    delete[] vertices;
                    delete[] indices;
                    // if we found a new closest raycast for this object, update the
                    // closest_result before moving on to the next object.
                    if (new_closest_found)
                    {
                        closest_result = ray.getPoint(closest_distance);
//                         Debug::log("adding node ")(pentity->getParentSceneNode()->getName()).endl();
                        nodes.push_back(pentity->getParentSceneNode());
                    }
                }
                else
                {
                    // not en entity, keep searching
                    continue;
                }
            }
            // return the result
            if (closest_distance >= 0.0f)
            {
                // raycast success
                return true;
            }
            else
            {
                // raycast failed
                return false;
            }
        }
        return false;
    }

    void RayCaster::getMeshInformation(	Ogre::Entity *entity,
                                        size_t &vertex_count,
                                        Ogre::Vector3* &vertices,
                                        size_t &index_count,
                                        Ogre::uint32* &indices,
                                        const Ogre::Vector3 &position,
                                        const Ogre::Quaternion &orient,
                                        const Ogre::Vector3 &scale)
    {
        bool added_shared = false;
        size_t current_offset = 0;
        size_t shared_offset = 0;
        size_t next_offset = 0;
        size_t index_offset = 0;
        vertex_count = index_count = 0;
        Ogre::MeshPtr mesh = entity->getMesh();
        bool useSoftwareBlendingVertices = entity->hasSkeleton();
        if (useSoftwareBlendingVertices)
        {
            entity->_updateAnimation();
        }
        // Calculate how many vertices and indices we're going to need
        for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh* submesh = mesh->getSubMesh(i);
            // We only need to add the shared vertices once
            if (submesh->useSharedVertices)
            {
                if (!added_shared)
                {
                    vertex_count += mesh->sharedVertexData->vertexCount;
                    added_shared = true;
                }
            }
            else
            {
                vertex_count += submesh->vertexData->vertexCount;
            }
            // Add the indices
            index_count += submesh->indexData->indexCount;
        }
        // Allocate space for the vertices and indices
        vertices = new Ogre::Vector3[vertex_count];
        indices = new Ogre::uint32[index_count];
        added_shared = false;
        // Run through the submeshes again, adding the data into the arrays
        for (unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh* submesh = mesh->getSubMesh(i);
            //----------------------------------------------------------------
            // GET VERTEXDATA
            //----------------------------------------------------------------
            Ogre::VertexData* vertex_data;
            //When there is animation:
            if (useSoftwareBlendingVertices)
                vertex_data = submesh->useSharedVertices ? entity->_getSkelAnimVertexData()
                              : entity->getSubEntity(i)->_getSkelAnimVertexData();
            else
                vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
            if ((!submesh->useSharedVertices) || (submesh->useSharedVertices && !added_shared))
            {
                if (submesh->useSharedVertices)
                {
                    added_shared = true;
                    shared_offset = current_offset;
                }
                const Ogre::VertexElement* posElem =
                    vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
                Ogre::HardwareVertexBufferSharedPtr vbuf =
                    vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
                unsigned char* vertex = static_cast<unsigned char*> (vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

                // There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
                // as second argument. So make it float, to avoid trouble when Ogre::Real will
                // be comiled/typedefed as double:
                // Ogre::Real* pReal;
                float* pReal;
                for (size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
                {
                    posElem->baseVertexPointerToElement(vertex, &pReal);
                    Ogre::Vector3 pt(pReal[0], pReal[1], pReal[2]);
                    vertices[current_offset + j] = (orient * (pt * scale)) + position;
                }
                vbuf->unlock();
                next_offset += vertex_data->vertexCount;
            }
            Ogre::IndexData* index_data = submesh->indexData;
            size_t numTris = index_data->indexCount / 3;
            Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
            bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
            void* hwBuf = ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
            size_t offset = (submesh->useSharedVertices) ? shared_offset : current_offset;
            size_t index_start = index_data->indexStart;
            size_t last_index = numTris * 3 + index_start;
            if (use32bitindexes)
            {
                Ogre::uint32* hwBuf32 = static_cast<Ogre::uint32*> (hwBuf);
                for (size_t k = index_start; k < last_index; ++k)
                {
                    indices[index_offset++] = hwBuf32[k] + static_cast<Ogre::uint32> (offset);
                }
            }
            else
            {
                Ogre::uint16* hwBuf16 = static_cast<Ogre::uint16*> (hwBuf);
                for (size_t k = index_start; k < last_index; ++k)
                {
                    indices[index_offset++] = static_cast<Ogre::uint32> (hwBuf16[k]) + static_cast<Ogre::uint32> (offset);
                }
            }
            ibuf->unlock();
            current_offset = next_offset;
        }
    }

}

// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
