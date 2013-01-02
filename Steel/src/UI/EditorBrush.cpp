
#include <stdexcept>
#include <list>

#include <OgreRoot.h>
#include <OgreSceneNode.h>
#include <OgreSceneManager.h>
#include <OgreEntity.h>

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
    EditorBrush::EditorBrush():Ogre::FrameListener(),
        mEngine(NULL),mEditor(NULL),
        mMode(TRANSLATE),
        mSelectionPosBeforeTransformation(Ogre::Vector3::ZERO),mSelectionRotBeforeTransformation(std::vector<Ogre::Quaternion>()),
        mIsDraggingSelection(false),mIsDraggingSelectionCancelled(false),
        mTerraBrushVisual(NULL),mTerraScale(1.5f),mModeStack(std::vector<BrushMode>()),
        mModifiedTerrains(std::set<Ogre::Terrain *>())
    {
    }

    EditorBrush::EditorBrush(const EditorBrush& other)
    {
        throw std::runtime_error("EditorBrush::EditorBrush(const EditorBrush&) not implemented");
    }

    EditorBrush::~EditorBrush()
    {
        shutdown();
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
        setMode(NONE);
    }

    void EditorBrush::shutdown()
    {
        setMode(NONE);
        if(mTerraBrushVisual!=NULL)
        {
            mTerraBrushVisual->removeAllChildren();
            mTerraBrushVisual->detachAllObjects();
            mTerraBrushVisual->getParent()->removeChild(mTerraBrushVisual);
            delete mTerraBrushVisual;
            mTerraBrushVisual = NULL;
        }
    }

    bool EditorBrush::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        std::list<ModelId> selection;
        switch(mMode)
        {
            case TRANSLATE:
            case ROTATE:
            case SCALE:
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
                break;
            case TERRAFORM:
            default:
                break;
        }
        // not used
        return true;
    }


    bool EditorBrush::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        switch(mMode)
        {
            case TRANSLATE:
            case ROTATE:
            case SCALE:
                if(id==OIS::MB_Left)
                {
                    mIsDraggingSelectionCancelled=false;
                    mIsDraggingSelection=false;
                    mSelectionPosBeforeTransformation = mEngine->selectionPosition();
                    mSelectionRotBeforeTransformation = mEngine->selectionRotations();
                }
                break;
            case TERRAFORM:
            {
                //now might be the right time to update blendmaps
                auto level=mEngine->level();
                if(level)
                {
                    for(auto it=mModifiedTerrains.begin(); it!=mModifiedTerrains.end(); ++it)
                    {
                        Ogre::Terrain *terrain=*it;
                        terrain->update(true);
                        level->terrainManager()->updateBlendMaps(terrain);
                        continue;
                        for(int index=1; index<terrain->getLayerCount(); ++index)
                            terrain->getLayerBlendMap(index)->dirty();
                        for(int index=1; index<terrain->getLayerCount(); ++index)
                            terrain->getLayerBlendMap(index)->update();
                        terrain->updateDerivedData(true);
                    }
                }
                break;
            }
            case NONE:
            default:
                break;
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

        // resize
        if(mMode==TERRAFORM)
        {
            if(mTerraBrushVisual==NULL)
                getBrush(TERRAFORM);
            if(mTerraBrushVisual!=NULL)
            {
                if(evt.state.Z.rel>0)
                    mTerraBrushVisual->setScale(mTerraBrushVisual->getScale()*mTerraScale);
                else if(evt.state.Z.rel<0)
                    mTerraBrushVisual->setScale(mTerraBrushVisual->getScale()/mTerraScale);
            }
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

    bool EditorBrush::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        auto state=mInputMan->mouse()->getMouseState();
        Ogre::Real x = float(state.X.abs);
        Ogre::Real y = float(state.Y.abs);
//         Ogre::Real _x = x-float(state.X.rel);
//         Ogre::Real _y = y-float(state.Y.rel);
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
                // move
                auto ray=mEngine->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                auto hitTest=level->terrainManager()->intersectRay(ray);
                if(hitTest.hit)
                {
                    mTerraBrushVisual->setVisible(true);
                    mTerraBrushVisual->setPosition(hitTest.position);
                    // action !
                    Ogre::Real radius=mTerraBrushVisual->getScale().length()*mTerraBrushVisual->getAttachedObject("cylinderEntity")->getBoundingRadius();
                    Ogre::TerrainGroup::TerrainList newTids;
                    if(mInputMan->mouse()->getMouseState().buttonDown(OIS::MB_Left))
                        newTids=level->terrainManager()->raiseTerrainAt(hitTest.position,radius/3.f,radius);
                    else if(mInputMan->mouse()->getMouseState().buttonDown(OIS::MB_Right))
                        newTids=level->terrainManager()->raiseTerrainAt(hitTest.position,-radius/3.f,radius);
                    mModifiedTerrains.insert(newTids.begin(),newTids.end());
                }
                else
                    mTerraBrushVisual->setVisible(false);
            }
            else
                Debug::warning("EditorBrush::frameRenderingQueued() no level, really ?").endl();
        }
        return true;
    }

    void EditorBrush::processCommand(std::vector<Ogre::String> command)
    {
        if(command[0]=="mode")
        {
            command.erase(command.begin());
            // drop terraform brush model
            if(command[0]=="translate")
                setMode(TRANSLATE);
            else if(command[0]=="rotate")
                setMode(ROTATE);
            else if(command[0]=="scale")
                setMode(SCALE);
            else if(command[0]=="terraform")
                setMode(TERRAFORM);
        }
        else
        {
            Debug::warning("EditorBrush::processCommand(): unknown command: ")(command).endl();
        }
    }

    void EditorBrush::setMode(BrushMode mode)
    {
        if(mode==mMode)
            return;

        // now we know we are setting a new mode
        dropBrush();
        if(mMode==TERRAFORM)
        {
            Ogre::Root::getSingletonPtr()->removeFrameListener(this);
        }

        switch(mode)
        {
            case TRANSLATE:
                mMode=TRANSLATE;
                break;
            case ROTATE:
                mMode=ROTATE;
                break;
            case SCALE:
                mMode=SCALE;
                break;
            case TERRAFORM:
                mMode=TERRAFORM;
                getBrush(TERRAFORM);
                Ogre::Root::getSingletonPtr()->addFrameListener(this);
                break;
            case NONE:
            default:
                mMode=NONE;
                break;
        }
    }

    void EditorBrush::getBrush(BrushMode mode)
    {
        switch(mMode)
        {
            case TERRAFORM:
                if(mTerraBrushVisual==NULL)
                {
                    auto level=mEngine->level();
                    if(level!=NULL)
                    {
                        auto sm=level->sceneManager();
                        Ogre::MeshPtr mesh=Cylinder::getMesh(sm,1,1);
                        Ogre::Entity* entity;
                        if(sm->hasEntity("cylinderEntity"))
                            entity=sm->getEntity("cylinderEntity");
                        else
                            entity = sm->createEntity("cylinderEntity",mesh);
                        mTerraBrushVisual = level->levelRoot()->createChildSceneNode("terraBrushVisual");
                        //         sphereEntity->setMaterialName("material_name_goes_here");
                        mTerraBrushVisual->attachObject(entity);
                    }
                }
                else
                    mTerraBrushVisual->setVisible(true);
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
                if(mTerraBrushVisual!=NULL)
                    mTerraBrushVisual->setVisible(false);
                mModifiedTerrains.clear();
                break;
            default:
                break;
        }
    }

    void EditorBrush::onShow()
    {
        popMode();
    }

    void EditorBrush::onHide()
    {
        pushMode();
        setMode(NONE);
    }

    void EditorBrush::popMode()
    {
        if(mModeStack.size()==0)
            return;
        BrushMode mode=mModeStack.back();
        mModeStack.pop_back();
        setMode(mode);
    }

    void EditorBrush::pushMode()
    {
        mModeStack.push_back(mMode);
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

