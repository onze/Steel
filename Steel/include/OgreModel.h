#ifndef STEEL_OGREMODEL_H_
#define STEEL_OGREMODEL_H_

#include "steeltypes.h"
#include "Model.h"
#include "SignalListener.h"

namespace Steel
{

    class OgreModel: public Model, SignalListener
    {
    public:
        static const Ogre::String POSITION_ATTRIBUTE;
        static const Ogre::String ROTATION_ATTRIBUTE;
        static const Ogre::String SCALE_ATTRIBUTE;
        static const Ogre::String ENTITY_MESH_NAME_ATTRIBUTE;

        /// Default material name. This name is set by blender's OgreExporter to meshes with no material.
        static const Ogre::String MISSING_MATERIAL_NAME;
        /// Attribute used to set a mesh model (overrites the material the mesh originally links to).
        static const Ogre::String MATERIAL_OVERRIDE_ATTRIBUTE;

        OgreModel();
        bool init(Ogre::String meshName,
                  Ogre::Vector3 pos, Ogre::Quaternion rot, Ogre::Vector3 scale,
                  Ogre::SceneNode *levelRoot,
                  Ogre::SceneManager *sceneManager,
                  Ogre::String const &resourceGroupName);
        OgreModel(const OgreModel &o);
        OgreModel &operator=(const OgreModel &o);
        virtual ~OgreModel();

        static ModelType modelType()
        {
            return ModelType::OGRE;
        }

        Ogre::Vector3 position() const;
        Ogre::Quaternion rotation() const;
        Ogre::Vector3 scale() const;

        /// Translates the model's scenenode by the given vector.
        void move(const Ogre::Vector3 &dpos);
        /// Rotate the model's scenenode by r.x in the x axis, etc.
        void rotate(const Ogre::Vector3 &r);
        /// Rotate the model's scenenode by the given quaternion
        void rotate(const Ogre::Quaternion &q);
        /// Rescale the model's scenenode by the given factor (current_scale*given_scale).
        void rescale(const Ogre::Vector3 &scale);

        void setPosition(const Ogre::Vector3 &pos);
        void setRotation(const Ogre::Quaternion &rot);
        void setScale(const Ogre::Vector3 &sca);

        void setSelected(bool selected);
        void setVisible(bool flag);

        /// Used to store the owner agent.
        void setNodeAny(AgentId aid);

        /// Not mean to be used.
        virtual bool fromJson(const Json::Value &mode);
        /// Deserialize itself from the given Json object
        virtual bool fromJson(const Json::Value &node,
                              Ogre::SceneNode *levelRoot,
                              Ogre::SceneManager *sceneManager,
                              Ogre::String const &resourceGroupName);
        /// Serialize itself into the given Json object
        virtual void toJson(Json::Value &node);

        /// Dynamically change the material used by the underlying model.
        void setMaterial(Ogre::String resName, const Ogre::String &resourceGroupName);

        // getters
        inline Ogre::Entity *entity() {return mEntity;}
        inline Ogre::SceneNode *sceneNode() {return mSceneNode;}
        bool isVisible() const {return nullptr == mEntity ? false : mEntity->isVisible();}

        virtual void cleanup();
        virtual void onSignal(Signal signal, SignalEmitter *const src = nullptr);

    protected:
        // not owned
        Ogre::SceneManager *mSceneManager;

        // owned
        Ogre::SceneNode *mSceneNode;
        Ogre::Entity *mEntity;
        bool mHasMaterialOverride;
    };

}

#endif /* STEEL_OGREMODEL_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
