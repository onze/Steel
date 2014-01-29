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
    const Ogre::String OgreModel::POSITION_ATTRIBUTE = "position";
    const Ogre::String OgreModel::ROTATION_ATTRIBUTE = "rotation";
    const Ogre::String OgreModel::SCALE_ATTRIBUTE = "scale";
    const Ogre::String OgreModel::ENTITY_MESH_NAME_ATTRIBUTE = "entityMeshName";

    const Ogre::String OgreModel::MISSING_MATERIAL_NAME = "_missing_material_";
    const Ogre::String OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE = "materialOverride";

    OgreModel::OgreModel(): Model(),
        mSceneManager(nullptr), mSceneNode(nullptr), mEntity(nullptr), mHasMaterialOverride(false)
    {

    }

    bool OgreModel::init(Ogre::String meshName,
                         Ogre::Vector3 pos, Ogre::Quaternion rot, Ogre::Vector3 scale,
                         Ogre::SceneNode *levelRoot,
                         Ogre::SceneManager *sceneManager,
                         Ogre::String const &resourceGroupName
                        )
    {
        mSceneManager = sceneManager;
        // handle
        Ogre::ResourceGroupManager *rgm = Ogre::ResourceGroupManager::getSingletonPtr();

        if(!rgm->resourceExists(resourceGroupName, meshName))
            rgm->declareResource(meshName, "FileSystem", resourceGroupName);

        const bool withRestore = nullptr != mSceneNode;
        Ogre::Any any;

        if(withRestore)
        {
            any = mSceneNode->getUserAny();

            if(mEntity != nullptr)
            {
                mEntity->detachFromParent();
                mSceneManager->destroyEntity(mEntity);
                mEntity = nullptr;
            }

            if(mSceneNode != nullptr)
            {
                OgreUtils::destroySceneNode(mSceneNode);
                mSceneNode = nullptr;
            }
        }

        mEntity = sceneManager->createEntity(meshName);

        mSceneNode = levelRoot->createChildSceneNode(pos, rot);
        mSceneNode->attachObject(mEntity);
        mSceneNode->setInheritScale(false);
        mSceneNode->setScale(scale);

        if(withRestore)
        {
            mSceneNode->setUserAny(any);
        }

        return true;
    }

    OgreModel::OgreModel(const OgreModel &o): Model(o),
        mSceneManager(o.mSceneManager), mSceneNode(o.mSceneNode), mEntity(o.mEntity), mHasMaterialOverride(o.mHasMaterialOverride)
    {
    }

    OgreModel &OgreModel::operator=(const OgreModel &o)
    {
        Model::operator=(o);
        mSceneManager = o.mSceneManager;
        mSceneNode = o.mSceneNode;
        mEntity = o.mEntity;
        mHasMaterialOverride = o.mHasMaterialOverride;
        return *this;
    }

    OgreModel::~OgreModel()
    {
    }

    void OgreModel::cleanup()
    {
        if(mEntity != nullptr)
        {
            mEntity->detachFromParent();
            mSceneManager->destroyEntity(mEntity);
            mEntity = nullptr;
        }

        if(mSceneNode != nullptr)
        {
            OgreUtils::destroySceneNode(mSceneNode);
            mSceneNode = nullptr;
        }

        mHasMaterialOverride = false;

        Model::cleanup();
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
        mSceneNode->rotate(q, Ogre::Node::TS_WORLD);
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
        if(mSceneNode == nullptr)
        {
            Debug::error("OgreModel::toJson() called while mSceneNode is nullptr !");
            return;
        }

        //TODO: use abbreviated keys for release
        node[OgreModel::POSITION_ATTRIBUTE] = JsonUtils::toJson(mSceneNode->getPosition());
        node[OgreModel::ROTATION_ATTRIBUTE] = JsonUtils::toJson(mSceneNode->getOrientation());
        node[OgreModel::SCALE_ATTRIBUTE] = JsonUtils::toJson(mSceneNode->getScale());
        node[OgreModel::ENTITY_MESH_NAME_ATTRIBUTE] = Json::Value(mEntity->getMesh()->getName());

        if(mHasMaterialOverride)
            node[OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE] = Json::Value(mEntity->getSubEntity(0)->getMaterialName());

        serializeTags(node);
    }

    bool OgreModel::fromJson(const Json::Value &mode)
    {
        Debug::error("OgreModel::fromJson(): wrong deserialization method called.").endl();
        // TODO: implement this method for inplace modification
        return false;
    }

    bool OgreModel::fromJson(const Json::Value &node, Ogre::SceneNode *levelRoot, Ogre::SceneManager *sceneManager, const Ogre::String &resourceGroupName)
    {
        Ogre::String intro = "in OgreModel::fromJson(): ";
        // data to gather
        Ogre::String meshName;
        Ogre::Vector3 pos(Ogre::Vector3::ZERO);
        Ogre::Quaternion rot(Ogre::Radian(0), -Ogre::Vector3::UNIT_Z);
        Ogre::Vector3 scale(Ogre::Vector3::UNIT_SCALE);

        Json::Value value;
        bool allWasFine = true;

        // gather it
        value = node[OgreModel::POSITION_ATTRIBUTE];

        if(value.isNull())
            Debug::error(intro)("position is null: no translation applied.").endl();
        else
            pos = Ogre::StringConverter::parseVector3(value.asString());

        value = node[OgreModel::ROTATION_ATTRIBUTE];

        if(value.isNull())
            Debug::warning(intro)("rotation is null: no rotation applied.").endl();
        else
            rot = Ogre::StringConverter::parseQuaternion(value.asString());

        value = node[OgreModel::SCALE_ATTRIBUTE];

        if(value.isNull())
            Debug::warning(intro)("scale is null: no scaling applied.").endl();
        else
            scale = Ogre::StringConverter::parseVector3(value.asString());

        value = node[OgreModel::ENTITY_MESH_NAME_ATTRIBUTE];

        if(value.isNull() && !(allWasFine = false))
            Debug::error(intro)("field ").quotes(OgreModel::ENTITY_MESH_NAME_ATTRIBUTE)(" is null.").endl();
        else
            meshName = Ogre::String(value.asString());

        Ogre::String materialName = Ogre::StringUtil::BLANK;

        if(node.isMember(OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE))
        {
            value = node[OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE];

            if(!value.isString())
                Debug::error(intro)("field ").quotes(OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE)(" is not a string.").endl();
            else if(value.isNull())
                Debug::error(intro)("field ").quotes(OgreModel::MATERIAL_OVERRIDE_ATTRIBUTE)(" is null.").endl();
            else
                materialName = Ogre::String(value.asString());
        }

        // agentTags
        allWasFine &= deserializeTags(node);

        if(!allWasFine)
        {
            Debug::error("json was:").endl()(node.toStyledString()).endl();
            Debug::error("model deserialisation aborted.").endl();
            return false;
        }

        // now whether we have minor changes (and we apply them directly), or major ones (cleanup, then init).
        // lets start with major ones
        if(mEntity == nullptr || meshName != mEntity->getMesh()->getName())
        {
            // make sure we've been called with all arguments, because they're all needed now
            if(levelRoot == nullptr || sceneManager == nullptr)
            {
                Debug::error(intro)("a new mesh is required, but sceneManager or levelRoot are nullptr.");
                Debug::error(" Aborting.").endl();
                return false;
            }

            // make sure the new meshName is valid
            if(meshName.length() == 0 || !Ogre::ResourceGroupManager::getSingletonPtr()->resourceExistsInAnyGroup(meshName))
            {
                Debug::error(intro)("could not find resource ").quotes(meshName)(" in any group. Aborting.").endl();
                return false;
            }

            if(!init(meshName, pos, rot, scale, levelRoot, sceneManager, resourceGroupName))
            {
                return false;
            }
        }
        else
        {
            setPosition(pos);
            setRotation(rot);
            setScale(scale);
        }

        if(Ogre::StringUtil::BLANK != materialName)
        {
            setMaterial(materialName, resourceGroupName);
        }

        return true;
    }

    void OgreModel::setMaterial(Ogre::String resName, Ogre::String const &resourceGroupName)
    {
        Ogre::MaterialManager *mm = Ogre::MaterialManager::getSingletonPtr();
        Ogre::MaterialPtr mat;

        if(!mm->resourceExists(resName))
        {
            Debug::error("in OgreModel::setMaterial(): material ").quotes(resName)(" does not exist. Using default.").endl();
            resName = OgreModel::MISSING_MATERIAL_NAME;
        }

        mEntity->setMaterial(mm->getByName(resName));
        mHasMaterialOverride = true;
    }

    void OgreModel::setVisible(bool flag)
    {
        if(nullptr != mEntity)
        {
            mEntity->setVisible(flag);
        }
    }


}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

