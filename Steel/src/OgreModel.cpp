/*
 * OgreModel.cpp
 *
 *  Created on: 2011-06-17
 *      Author: onze
 */

#include "OgreModel.h"
#include "Debug.h"
#include <tools/OgreUtils.h>
#include <tools/JsonUtils.h>
#include <OgreSubEntity.h>

namespace Steel
{
    const Ogre::String OgreModel::MISSING_MATERIAL_NAME="_missing_material_";
    const Ogre::String OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE="materialOverride";


    OgreModel::OgreModel() :
        Model(),
        mSceneNode(NULL), mEntity(NULL),mSceneManager(NULL)
    {

    }

    bool OgreModel::init(Ogre::String meshName,
                         Ogre::Vector3 pos, Ogre::Quaternion rot, Ogre::Vector3 scale,
                         Ogre::SceneNode *levelRoot,
                         Ogre::SceneManager *sceneManager,
                         Ogre::String const &resourceGroupName
                        )
    {
        mSceneManager=sceneManager;
        // handle
        Ogre::ResourceGroupManager *rgm=Ogre::ResourceGroupManager::getSingletonPtr();

        if(!rgm->resourceExists(resourceGroupName,meshName))
            rgm->declareResource(meshName, "FileSystem", resourceGroupName);

        mEntity = sceneManager->createEntity(meshName);

        mSceneNode = levelRoot->createChildSceneNode(pos, rot);
        mSceneNode->attachObject(mEntity);
        mSceneNode->setInheritScale(false);
        mSceneNode->setScale(scale);
        return true;
    }

    OgreModel::OgreModel(const OgreModel &m)
    {
        Model::operator=(m);
        (*this) = m;
    }

    OgreModel &OgreModel::operator=(const OgreModel &m)
    {
        Model::operator=(m);
        mEntity = m.mEntity;
        mSceneNode = m.mSceneNode;
        mSceneManager=m.mSceneManager;
        return *this;
    }

    OgreModel::~OgreModel()
    {
    }

    void OgreModel::cleanup()
    {
        if (mEntity != NULL)
        {
            mEntity->detachFromParent();
            mSceneManager->destroyEntity(mEntity);
            mEntity = NULL;
        }
        if (mSceneNode != NULL)
        {
            OgreUtils::destroySceneNode(mSceneNode);
            mSceneNode = NULL;
        }
    }

    Ogre::Vector3 OgreModel::position() const
    {
        return mSceneNode->getPosition();
    }

    Ogre::Quaternion OgreModel::rotation() const
    {
        return mSceneNode->getOrientation();
    }

    Ogre::Vector3 OgreModel::scale() const
    {
        return mSceneNode->getScale();
    }

    void OgreModel::move(const Ogre::Vector3 &dpos)
    {
        mSceneNode->translate(dpos);
    }

    void OgreModel::rotate(const Ogre::Vector3 &rotation)
    {
        mSceneNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Degree(rotation.x), Ogre::Node::TS_WORLD);
        mSceneNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Degree(rotation.y), Ogre::Node::TS_WORLD);
        mSceneNode->rotate(Ogre::Vector3::UNIT_Z, Ogre::Degree(rotation.z), Ogre::Node::TS_WORLD);
    }

    void OgreModel::rescale(const Ogre::Vector3 &scale)
    {
        mSceneNode->scale(scale);
    }

    void OgreModel::rotate(const Ogre::Quaternion &q)
    {
        mSceneNode->rotate(q,Ogre::Node::TS_WORLD);
    }

    void OgreModel::setNodeAny(Steel::AgentId aid)
    {
        mSceneNode->getUserObjectBindings().setUserAny(Ogre::Any(aid));
    }

    void OgreModel::setPosition(const Ogre::Vector3 &pos)
    {
        mSceneNode->setPosition(pos);
    }

    void OgreModel::setRotation(const Ogre::Quaternion &rot)
    {
        mSceneNode->setOrientation(rot);
    }

    void OgreModel::setScale(const Ogre::Vector3 &sca)
    {
        mSceneNode->setScale(sca);
    }

    void OgreModel::setSelected(bool selected)
    {
#ifdef DEBUG
        mSceneNode->showBoundingBox(selected);
#endif
    }

    void OgreModel::toJson(Json::Value &node)
    {
        if (mSceneNode == NULL)
        {
            Debug::error("OgreModel::toJson() called while mSceneNode is NULL !");
            return;
        }
        //TODO: use abbreviated keys for release
        node["position"] = JsonUtils::toJson(mSceneNode->getPosition());
        node["rotation"] = JsonUtils::toJson(mSceneNode->getOrientation());
        node["scale"] = JsonUtils::toJson(mSceneNode->getScale());
        node["entityMeshName"] = Json::Value(mEntity->getMesh()->getName());
    }

    bool OgreModel::fromJson(Json::Value &mode)
    {
        Debug::error("OgreModel::fromJson(): wrong deserialization method called.").endl();
        // TODO: implement this method for inplace modification
        return false;
    }

    bool OgreModel::fromJson(Json::Value& node,
                             Ogre::SceneNode* levelRoot,
                             Ogre::SceneManager* sceneManager,
                             const Ogre::String& resourceGroupName)
    {
        Ogre::String intro="in OgreModel::fromJson(): ";
        // data to gather
        Ogre::String meshName;
        Ogre::Vector3 pos;
        Ogre::Quaternion rot;
        Ogre::Vector3 scale=Ogre::Vector3::UNIT_SCALE;

        Json::Value value;
        bool allWasFine = true;

        // gather it
        value = node["position"];
        if (value.isNull())
            Debug::error(intro)("position is null: no translation applied.").endl();
        else
            pos = Ogre::StringConverter::parseVector3(value.asString());

        value = node["rotation"];
        if (value.isNull())
            Debug::warning(intro)("rotation is null: no rotation applied.").endl();
        else
            rot = Ogre::StringConverter::parseQuaternion(value.asString());

        value = node["scale"];
        if (value.isNull())
            Debug::warning(intro)("scale is null: no scaling applied.").endl();
        else
            scale = Ogre::StringConverter::parseVector3(value.asString());

        value = node["entityMeshName"];
        if (value.isNull() && !(allWasFine = false))
            Debug::error(intro)("field 'entityMeshName' is null.").endl();
        else
            meshName = Ogre::String(value.asString());

        value = node[OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE];
        Ogre::String materialName="";
        if (value.isNull())
            Debug::error(intro)("field ").quotes(OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE)(" is null.").endl();
        else
            materialName = Ogre::String(value.asString());

        if (!allWasFine)
        {
            Debug::error("json was:").endl()(node.toStyledString()).endl();
            Debug::error("model deserialisation aborted.").endl();
            return false;
        }

        // now whether we have minor changes (and we apply them directly), or major ones (cleanup, then init).
        // lets start with major ones
        if (mEntity == NULL || meshName != mEntity->getMesh()->getName())
        {
            // make sure we've been called with all arguments, because they're all needed now
            if (levelRoot == NULL || sceneManager == NULL)
            {
                Debug::error(intro)("a new mesh is required, but sceneManager or levelRoot are NULL.");
                Debug::error(" Aborting.").endl();
                return false;
            }
            // make sure the new meshName is valid
            if (meshName.length()==0 || !Ogre::ResourceGroupManager::getSingletonPtr()->resourceExistsInAnyGroup(meshName))
            {
                Debug::error(intro)("new mesh name is not valid:")(meshName).endl();
                return false;
            }
            Ogre::Any any;
            if(NULL!=mSceneNode)
                any = mSceneNode->getUserAny();
            cleanup();
            if(!init(meshName, pos, rot, scale, levelRoot, sceneManager, resourceGroupName))
            {
                return false;
            }
            mSceneNode->setUserAny(any);
        }
        else
        {
            setPosition(pos);
            setRotation(rot);
            setScale(scale);
        }

        if(""!=materialName)
        {
            setMaterial(materialName);
        }
        return true;
    }
    
    void OgreModel::setMaterial(Ogre::String resName)
    {
        Ogre::MaterialManager *mm=Ogre::MaterialManager::getSingletonPtr();
        Ogre::MaterialPtr mat;
        if(!mm->resourceExists(resName))
        {
            Debug::error("in OgreModel::setMaterial(): material ").quotes(resName)(" does not exist. Using default.").endl();
            resName=OgreModel::MISSING_MATERIAL_NAME;
        }
        mEntity->setMaterial(mm->getByName(resName));
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
