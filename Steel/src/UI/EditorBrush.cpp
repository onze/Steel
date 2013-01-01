
#include <stdexcept>
#include <list>

#include <steeltypes.h>
#include "UI/EditorBrush.h"
#include "Engine.h"
#include "UI/Editor.h"
#include <Camera.h>
#include <Level.h>
#include <tools/Cylinder.h>
#include <OgreManualObject.h>
#include <OgreMeshManager.h>

namespace Steel
{
    EditorBrush::EditorBrush():
        mEngine(NULL),mEditor(NULL),
        mMode(TRANSLATE),
        mSelectionPosBeforeTransformation(Ogre::Vector3::ZERO),mSelectionRotBeforeTransformation(std::vector<Ogre::Quaternion>()),
        mIsDraggingSelection(false),mIsDraggingSelectionCancelled(false),
        mTerraBrushVisual(NULL),mTerraScale(1.5f)
    {
#ifdef DEBUG
        mMode=TERRAFORM;
#endif
    }

    EditorBrush::EditorBrush(const EditorBrush& other)
    {
        throw std::runtime_error("EditorBrush::EditorBrush(const EditorBrush&) not implemented");
    }

    EditorBrush::~EditorBrush()
    {

    }

    EditorBrush& EditorBrush::operator=(const EditorBrush& other)
    {
        throw std::runtime_error("EditorBrush::operator=() not implemented");
        return *this;
    }

    bool EditorBrush::operator==(const EditorBrush& other) const
    {
        return false;
    }

    void EditorBrush::init(Engine *engine,Editor *editor,InputManager *inputMan)
    {
        mEngine=engine;
        mEditor=editor;
        mInputMan=inputMan;
        mIsDraggingSelectionCancelled=mIsDraggingSelection=false;
        auto level=mEngine->level();
        if(level!=NULL)
        {
            mTerraBrushVisual=Cylinder::getSceneNode(level->sceneManager(),level->levelRoot(),100,10);
            mTerraBrushVisual->setVisible(false);
        }
        else
        {
            Debug::warning("EditorBrush::init() no level, so no mTerraBrushVisual.").endl();
            mTerraBrushVisual=NULL;
        }
    }

    void EditorBrush::shutdown()
    {
        if(mTerraBrushVisual!=NULL)
        {
            mTerraBrushVisual->removeAndDestroyAllChildren();
            mTerraBrushVisual->detachAllObjects();
            mTerraBrushVisual->getParent()->removeChild(mTerraBrushVisual);
            delete mTerraBrushVisual;
            mTerraBrushVisual = NULL;
        }
    }

    bool EditorBrush::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        std::list<ModelId> selection;
        switch (id)
        {
            case OIS::MB_Left:
                if (mEngine->hasSelection())
                {
                    mEngine->clearSelection();
                }
                mEngine->pickAgents(selection, evt.state.X.abs, evt.state.Y.abs);
                mEngine->setSelectedAgents(selection, true);
                if (mEngine->hasSelection())
                {
                    // saved so that we know what to reset properties to
                    //TODO: save complete serialisations
                    mSelectionPosBeforeTransformation = mEngine->selectionPosition();
                }
                break;
            case OIS::MB_Right:
                // cancel current selection dragging (translate, etc)
                if(mIsDraggingSelection)
                {
                    mIsDraggingSelectionCancelled=true;
                    mEngine->setSelectionPosition(mSelectionPosBeforeTransformation);
                    mEngine->setSelectionRotations(mSelectionRotBeforeTransformation);
                }
                break;
            default:
                break;
        }
        // not used
        return true;
    }


    bool EditorBrush::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        if(id==OIS::MB_Left)
        {
            mIsDraggingSelectionCancelled=false;
            mIsDraggingSelection=false;
            mSelectionPosBeforeTransformation = mEngine->selectionPosition();
            mSelectionRotBeforeTransformation = mEngine->selectionRotations();
        }
        // not used
        return true;
    }

    bool EditorBrush::mouseMoved(const OIS::MouseEvent& evt)
    {
        Ogre::Real x = float(evt.state.X.abs);
        Ogre::Real y = float(evt.state.Y.abs);
        Ogre::Real _x = x-float(evt.state.X.rel);
        Ogre::Real _y = y-float(evt.state.Y.rel);
        Ogre::Real w = float(mEditor->width());
        Ogre::Real h = float(mEditor->height());
        // terraform does not need a selection
        if(mMode==TERRAFORM)
        {
            if(mTerraBrushVisual==NULL)
                getBrush(TERRAFORM);
            auto level=mEngine->level();
            if(level!=NULL && mTerraBrushVisual!=NULL)
            {
                // resize
                if(evt.state.Z.rel>0)
                    mTerraBrushVisual->setScale(mTerraBrushVisual->getScale()*mTerraScale);
                else if(evt.state.Z.rel<0)
                    mTerraBrushVisual->setScale(mTerraBrushVisual->getScale()/mTerraScale);
                    
                // move
                auto ray=mEngine->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                auto hitTest=level->terrainManager()->intersectRay(ray);
                if(hitTest.hit)
                {
                    mTerraBrushVisual->setVisible(true);
                    mTerraBrushVisual->setPosition(hitTest.position);
                }
                else
                    mTerraBrushVisual->setVisible(false);
            }
            else
                Debug::warning("EditorBrush::mouseMoved() no level, really ?").endl();
            
        }
        if(evt.state.buttonDown(OIS::MB_Left))
        {
            if (mEngine->hasSelection())
            {
                if(mIsDraggingSelectionCancelled)
                    return true;
                mIsDraggingSelection=true;
                switch (mMode)
                {
                    case TRANSLATE:
                    {
                        Ogre::Vector3 selectionPos = mEngine->selectionPosition();
                        // translating with shift held: along Y axis
                        if (mInputMan->isKeyDown(OIS::KC_LSHIFT))
                        {
                            // I have not found a faster way to do this:
                            // first I build a plan with the camera orientation as normal (but vertical).
                            Ogre::Vector3 normal = mEngine->camera()->camNode()->getOrientation()* Ogre::Vector3::UNIT_Z;
                            normal.y = .0f;
                            Ogre::Plane plane = Ogre::Plane(normal, Ogre::Vector3::ZERO);
                            // and since I don't know how to compute the distance to feed it, I let it at (0,0,0).
                            // ask IT its distance to where I wanted to put it (the selection),
                            Ogre::Real dist = plane.getDistance(selectionPos);
                            // and build a new one there. suboptimal =/
                            plane = Ogre::Plane(normal, dist);
                            plane.normalise();
                            // the idea here is to move the selection on a plane perpendicular to the camera view.
                            // we want a vector of the translation from src to dst, where src is where a ray cast from the camera
                            // to the last mouse coordinates hits the mentionned plane, and dst is the same with a ray
                            // passing through the current mouse coordinates.
                            Ogre::Ray ray = mEngine->camera()->cam()->getCameraToViewportRay(_x / w, _y / h);
                            std::pair<bool, Ogre::Real> result = ray.intersects(plane);
                            if (result.first)
                            {
                                Ogre::Vector3 src = ray.getPoint(result.second);
                                // then we do the same with the new coordinates on the viewport
                                ray = mEngine->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                                result = ray.intersects(plane);
                                if (result.first)
                                {
                                    Ogre::Vector3 dst = ray.getPoint(result.second);
                                    // finally, we translate the selection according to the vector given by substracting two points.
                                    Ogre::Vector3 t = dst - src;
                                    t.y = t.y > 10.f ? .0f : t.y < -10.f ? 0.f : t.y;
                                    mEngine->translateSelection(Ogre::Vector3::UNIT_Y * t);
                                }
                            }
                        }
                        else
                        {
                            // normal translation: on the x/z plane
                            Ogre::Plane plane = Ogre::Plane(Ogre::Vector3::UNIT_Y, selectionPos.y);
                            plane.normalise();
                            // what we want is a vector of translation from the selection's position to a new one.
                            // first we see where falls a ray that we cast from the cam to the last coordinates on the viewport
                            // (the idea is to cast a ray from the camera to a horizontal plane at the base of the selection)
                            Ogre::Ray ray = mEngine->camera()->cam()->getCameraToViewportRay(_x / w, _y / h);
                            std::pair<bool, Ogre::Real> result = ray.intersects(plane);
                            if (result.first)
                            {
                                Ogre::Vector3 src = ray.getPoint(result.second);
                                // then we do the same with the new coordinates on the viewport
                                ray = mEngine->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                                result = ray.intersects(plane);
                                if (result.first)
                                {
                                    Ogre::Vector3 dst = ray.getPoint(result.second);
                                    // finally, we translate the selection according to the vector given by substracting two points.
                                    Ogre::Vector3 t = dst - src;
                                    // just making sure we have an horizontal translation (should be useless since plane is horizontal)
                                    t.y = 0.f;
                                    mEngine->translateSelection(t);
                                }
                            }
                        }
                    }
                    break;
                    case ROTATE:
                    {
                        Ogre::Vector3 r = Ogre::Vector3(.0f, 180.f * (x - _x) / (w / 2.f), .0f);
                        mEngine->rotateSelection(r);
                    }
                    break;
                    case SCALE:
                    case TERRAFORM:
                    default:
                        break;
                } //end of switch (mMode)
            } //end of if(mEngine->hasSelection())

        } //end of if(evt.state.buttonDown(OIS::MB_Left))
        // not used
        return true;
    }

    void EditorBrush::processCommand(std::vector<Ogre::String> command)
    {
        if(command[0]=="mode")
        {
            command.erase(command.begin());
            // drop terraform brush model
            if(mMode==TERRAFORM && command[0]!="terraform")
            {
                dropBrush();
            }
            if(command[0]=="translate")
                mMode=TRANSLATE;
            else if(command[0]=="rotate")
                mMode=ROTATE;
            else if(command[0]=="scale")
                mMode=SCALE;
            else if(command[0]=="terraform")
            {
                if(mMode!=TERRAFORM)
                {
                    mMode=TERRAFORM;
                    getBrush(TERRAFORM);
                }
            }
        }
        else
        {
            Debug::warning("EditorBrush::processCommand(): unknown command: ")(command).endl();
        }
    }

    void EditorBrush::getBrush(BrushMode mode)
    {
        switch(mMode)
        {
            case TERRAFORM:
                if(mTerraBrushVisual==NULL)
                {
                    Debug::warning("EditorBrush::getBrush() building mTerraBrushVisual").endl();
                    auto level=mEngine->level();
                    if(level!=NULL)
                    {
                        mTerraBrushVisual=Cylinder::getSceneNode(level->sceneManager(),level->levelRoot(),5,1);
                        mTerraBrushVisual->setVisible(true);
                    }
                    else
                    {
                        Debug::warning("EditorBrush::getBrush() level still NULL").endl();
                        mTerraBrushVisual=NULL;
                    }
                }
                else
                    Debug::warning("EditorBrush::getBrush(): could not instanciate mTerraBrushVisual.").endl();
                break;
            default:
                break;
        }
    }

    void EditorBrush::dropBrush()
    {
        switch(mMode)
        {
            case TERRAFORM:
                if(mTerraBrushVisual==NULL)
                    Debug::warning("EditorBrush::dropBrush(): can drop NULL mTerraBrushVisual.").endl();
                else
                    mTerraBrushVisual->setVisible(false);
                break;
            default:
                break;
        }
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
