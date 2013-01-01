#ifndef EDITORBRUSH_H
#define EDITORBRUSH_H

#include <vector>

#include <OIS.h>
#include <OgreQuaternion.h>
#include <OgreVector3.h>

namespace Ogre
{
    class SceneNode;
}

namespace Steel
{
    class Engine;
    class Editor;
    class InputManager;

    class EditorBrush
    {
            /**
             * This class is meant to instanced once, by the editor. It handles editing
             * interactions between the mouse and Steel.
             * Such interactions include:
             * - translation/rotation/scaling of models
             * - terrain edition
             */
        protected:
            enum BrushMode {TRANSLATE, ROTATE, SCALE,TERRAFORM };
        public:
            EditorBrush();
            EditorBrush(const EditorBrush& other);
            virtual ~EditorBrush();
            virtual EditorBrush& operator=(const EditorBrush& other);
            virtual bool operator==(const EditorBrush& other) const;

            void init(Engine *engine,Editor *editor,InputManager *mInputMan);
            void shutdown();

            void getBrush(BrushMode mode);
            void dropBrush();
            
            // mouse events called by the editor
            bool mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id);
            bool mouseMoved(const OIS::MouseEvent& evt);

            /// command interface
            void processCommand(std::vector<Ogre::String> command);

        protected:
            //not owned
            Engine *mEngine;
            Editor *mEditor;
            InputManager *mInputMan;

            //owned
            /// holds the current way dragging selection will affect it
            BrushMode mMode;

            /// holds the position to put the selection back to, if edition is cancelled
            Ogre::Vector3 mSelectionPosBeforeTransformation;
            std::vector<Ogre::Quaternion> mSelectionRotBeforeTransformation;

            /// true when selection is begin dragged (translated/rotated/scaled)
            bool mIsDraggingSelection;
            /// set to true when cancelling a selection drag movement.
            bool mIsDraggingSelectionCancelled;
            
            /// disk used as visual terraforming brush
            Ogre::SceneNode *mTerraBrushVisual;
            /// factor by which the terraforming brush is scaled up (down: 1/mTerraScale)
            float mTerraScale;
    };
}
#endif // EDITORBRUSH_H
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
