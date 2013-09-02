#ifndef STEEL_CAMERA_H_
#define STEEL_CAMERA_H_

#include <json/json.h>

#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

namespace Steel
{
    class Level;
    class Camera
    {
            /// Wraps ogre camera and add features to ease its manipulation.
        private:
            Camera();
            Camera(const Camera &camera);
            Camera &operator=(const Camera &camera);
        public:
            Camera(Level *level);
            virtual ~Camera();

            /**
             * deserialization method;
             * returns true if all required fields were found.
             */
            bool fromJson(Json::Value &value);

            /// orient the camera center towards the given screen position
            void lookTowards(float x, float y, float roll = .0f, float factor = 1.f);

            /// Serialization method
            Json::Value toJson();

            /// Returns the position of the drop target in front of the camera.
            Ogre::Vector3 dropTargetPosition();

            /// Returns the rotation of the drop target in front of the camera.
            Ogre::Quaternion dropTargetRotation();

            Ogre::Vector2 screenPosition(const Ogre::Vector3& worldPosition);

            /// Moves the camera according to the given coordinates.
            void translate(float dx, float dy, float dz, float speed = 1.f);

            /// Returns a pointer to the internal ogre camera instance.
            inline Ogre::Camera *cam()
            {
                return mCamera;
            }

            /// Returns a pointer to the node the internal camera instance is attached to.
            inline Ogre::SceneNode *camNode()
            {
                return mCameraNode;
            }

        protected:
            // not owned
            Level *mLevel;

            // owned
            Ogre::Camera *mCamera;
            Ogre::SceneNode *mCameraNode;
            Ogre::SceneManager *mSceneManager;
    };

}

#endif /* STEEL_CAMERA_H_ */
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
