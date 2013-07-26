#ifndef STEEL_EDITORBRUSH_H
#define STEEL_EDITORBRUSH_H

#include <vector>

#include <OIS.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>
#include <OgreFrameListener.h>

#include "tools/ConfigFile.h"
#include "TerrainManager.h"

namespace Ogre
{
    class SceneNode;
    class Terrain;
}

namespace Steel
{

    class Engine;
    class Editor;
    class InputManager;
    class SelectionBox;

    class EditorBrush: public Ogre::FrameListener
    {
            /**
             * This class is meant to instanced once, by the editor. It handles editing
             * interactions between the mouse and Steel.
             * Such interactions include:
             * - translation/rotation/scaling of models
             * - terrain edition
             */
        public:
            static const Ogre::String MODE;
            static const Ogre::String TERRA_SCALE;
            static const Ogre::String TERRA_SCALE_FACTOR;

            enum BrushMode
            {
                NONE = 0, TRANSLATE, ROTATE, SCALE, TERRAFORM
            };
            EditorBrush();
            EditorBrush(const EditorBrush& other);
            virtual ~EditorBrush();
            virtual EditorBrush& operator=(const EditorBrush& other);
            virtual bool operator==(const EditorBrush& other) const;

            void init(Engine *engine, Editor *editor, InputManager *mInputMan);
            void shutdown();

            /// called by Ogre once per frame. Used during terraforming.
            bool frameRenderingQueued(const Ogre::FrameEvent &evt);

            void getBrush(BrushMode mode);
            void dropBrush();

            // mouse events called by the editor
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseMoved(const OIS::MouseEvent& evt);

            /** Tries to project a ray starting at the current camera and passing through the
             * given screen position, through the given plane. If the ray does, the intersection
             * is set into the given pos (in coordinates local), and the method returns true; It eturns false otherwise */
            bool mousePlaneProjection(const Ogre::Plane &plane, float x, float y, Ogre::Vector3 &pos);

            void onShow();
            void onHide();

            /// command interface
            void processCommand(std::vector<Ogre::String> command);

            /// saves the current editing mode
            void pushMode();

            ///restore the last saved editing mode
            void popMode();

            void loadConfig(ConfigFile const &config);
            void saveConfig(ConfigFile &config) const;

            //getters
            inline BrushMode mode()
            {
                return mMode;
            }
            //setters
            void setMode(BrushMode mode);

            inline float intensity()
            {
                if (sTerraBrushVisual == NULL)
                    return .0f;
                return sTerraBrushVisual->getScale().y / 3.f;
            }

            inline float radius()
            {
                if (sTerraBrushVisual == NULL)
                    return .0f;
                return (sTerraBrushVisual->getScale().x + sTerraBrushVisual->getScale().z) / 2.f;
            }

        protected:
            /// Warns, in case some values make using the brush impossible.
            void checkTerraScaleFactorValue();
            
            //not owned
            Engine *mEngine;
            Editor *mEditor;
            InputManager *mInputMan;

            //owned
            /// holds the current way dragging selection will affect it
            BrushMode mMode;
            /// true between a mousePressed and its following mouseRelease when the brush is in a continuous mode (->terraforming)
            bool mContinuousModeActivated;

            /// holds the position to put the selection back to, if edition is cancelled
            std::vector<Ogre::Vector3> mSelectionPosBeforeTransformation;
            std::vector<Ogre::Quaternion> mSelectionRotBeforeTransformation;
            std::vector<Ogre::Vector3> mSelectionScaleBeforeTransformation;

            /// true when selection is begin dragged (translated/rotated/scaled)
            bool mIsDraggingSelection;
            /// set to true when cancelling a selection drag movement.
            bool mIsDraggingSelectionCancelled;

            /// disk used as visual terraforming brush
            static Ogre::SceneNode *sTerraBrushVisual;
            /// factor by which the terraforming brush is scaled up (down: 1/mTerraScale)
            float mTerraScaleFactor;
            /// scale of the brush
            Ogre::Vector3 mTerraScale;
            /// terrain height at mousePressed (used in terrascale equalize mode)
            float mSelectedTerrainHeight;
            /// current terrabrush shape
            TerrainManager::RaiseShape mRaiseShape;

            std::vector<BrushMode> mModeStack;
            std::set<Ogre::Terrain *> mModifiedTerrains;

            /// True while the mouse is being dragged, but drag did not start on a selected agent.
            bool mIsSelecting;
            /// Helper for selecting many agents at once
            SelectionBox *mSelectionBox;
    };
}
#endif // STEEL_EDITORBRUSH_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
