
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
#include <UI/SelectionBox.h>
#include <Camera.h>
#include <Level.h>
#include <tools/Cylinder.h>
#include <tools/OgreUtils.h>
#include <Agent.h>
#include <OgreManualObject.h>
#include <OgreMeshManager.h>

namespace Steel
{
    Ogre::SceneNode *EditorBrush::sTerraBrushVisual=NULL;

    EditorBrush::EditorBrush():Ogre::FrameListener(),
        mEngine(NULL),mEditor(NULL),
        mMode(TRANSLATE),mContinuousModeActivated(false),
        mSelectionPosBeforeTransformation(std::vector<Ogre::Vector3>()),
        mSelectionRotBeforeTransformation(std::vector<Ogre::Quaternion>()),
        mSelectionScaleBeforeTransformation(std::vector<Ogre::Vector3>()),
        mIsDraggingSelection(false),mIsDraggingSelectionCancelled(false),
        mTerraScaleFactor(1.1f),mTerraScale(1.f,1.f,1.f),mSelectedTerrainHeight(.0f),mRaiseShape(TerrainManager::RaiseShape::UNIFORM),
        mModeStack(std::vector<BrushMode>()),mModifiedTerrains(std::set<Ogre::Terrain *>()),
        mIsSelecting(false),mSelectionBox(NULL)
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
        if(NULL!=mSelectionBox)
        {
            OgreUtils::destroySceneNode(mSelectionBox->getParentSceneNode());
            delete mSelectionBox;
            mSelectionBox=NULL;
        }
    }

    bool EditorBrush::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
        mContinuousModeActivated=false;
        switch(mMode)
        {
            case TRANSLATE:
            case ROTATE:
            case SCALE:
                switch (id)
                {
                    case OIS::MB_Left:
                    {
                        std::list<ModelId> selection;
                        auto mPos=mInputMan->mousePos();
                        mEngine->pickAgents(selection, mPos.x, mPos.y);

                        // click on nothing
                        if(selection.size()==0 && !mInputMan->isKeyDown(OIS::KC_LCONTROL))
                            mEngine->clearSelection();
                        else if(mEngine->hasSelection())
                        {
                            // clicked a new agent
                            if(mEngine->isSelected(selection.front()))
                            {
                                if(mInputMan->isKeyDown(OIS::KC_LCONTROL))
                                    mEngine->removeFromSelection(selection);
                            }
                            else
                                mEngine->setSelectedAgents(selection,!mInputMan->isKeyDown(OIS::KC_LCONTROL));
                        }
                        else
                        {
                            mEngine->setSelectedAgents(selection);
                            Debug::log("EditorBrush::mousePressed(): selection position: ")(mEngine->selectionPosition()).endl();
                        }

                        if (mEngine->hasSelection())
                        {
                            mIsDraggingSelection=true;
                            // saved so that we know what to reset properties to
                            //TODO: save complete serialisations
                            mSelectionPosBeforeTransformation = mEngine->selectionPositions();
                            mSelectionRotBeforeTransformation = mEngine->selectionRotations();
                            mSelectionScaleBeforeTransformation= mEngine->selectionScales();
                        }
                        else
                        {
                            mIsSelecting=true;
                            mSelectionBox->setVisible(true);
                            Ogre::Vector2 screenSize(float(mEditor->width()),float(mEditor->height()));
                            mSelectionBox->setCorners(mInputMan->mousePosAtLastMousePressed()/screenSize,mPos/screenSize);
                        }
                        break;
                    }
                    case OIS::MB_Right:
                        // cancel current selection dragging (translate, etc)
                        if(mIsDraggingSelection)
                        {
                            mIsDraggingSelectionCancelled=true;
                            mEngine->setSelectionPositions(mSelectionPosBeforeTransformation);
                            mEngine->setSelectionRotations(mSelectionRotBeforeTransformation);
                            mEngine->setSelectionScales(mSelectionScaleBeforeTransformation);
                        }
                        break;
                    default:
                        break;
                }
                break;
            case TERRAFORM:
                mContinuousModeActivated=true;
                switch (id)
                {
                    case OIS::MB_Middle:
                    {
                        auto level=mEngine->level();
                        if(level)
                        {
                            Ogre::Terrain *terrain;
                            auto pos=level->terrainManager()->terrainGroup()->getHeightAtWorldPosition(sTerraBrushVisual->getPosition(),&terrain);
                            // keep last valid value
                            if(terrain!=NULL)
                                mSelectedTerrainHeight=pos;
                        }
                        break;
                    }
                    default:
                        break;
                }
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
                    if(mIsSelecting)
                    {
                        Ogre::Vector2 screenSize(float(mEditor->width()),float(mEditor->height()));
                        mSelectionBox->setCorners(mInputMan->mousePosAtLastMousePressed()/screenSize,mInputMan->mousePos()/screenSize);

                        std::list<AgentId> selection;
                        mSelectionBox->performSelection(selection, mEngine->level()->camera()->cam());
                        if(selection.size()>0)
                            mEngine->setSelectedAgents(selection,!mInputMan->isKeyDown(OIS::KC_LCONTROL));
                        mSelectionBox->setVisible(false);
                        mIsSelecting=false;
                    }
                    if(mIsDraggingSelection)
                    {
                        mIsDraggingSelection=false;
                        mIsDraggingSelectionCancelled=false;
                        mSelectionPosBeforeTransformation = mEngine->selectionPositions();
                        mSelectionRotBeforeTransformation = mEngine->selectionRotations();
                    }
                }
                break;
            case TERRAFORM:
            {
                if(mContinuousModeActivated)
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
//                             level->terrainManager()->updateHeightmap(terrain);
                            continue;
                            for(int index=1; index<terrain->getLayerCount(); ++index)
                                terrain->getLayerBlendMap(index)->dirty();
                            for(int index=1; index<terrain->getLayerCount(); ++index)
                                terrain->getLayerBlendMap(index)->update();
                            terrain->updateDerivedData(true);
                        }
                    }
                }
                break;
            }
            case NONE:
            default:
                break;
        }
        mContinuousModeActivated=false;
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
            if(sTerraBrushVisual==NULL)
                getBrush(TERRAFORM);
            if(sTerraBrushVisual!=NULL)
            {
                if (mInputMan->isKeyDown(OIS::KC_LSHIFT))
                {
                    // height
                    if(evt.state.Z.rel>0)
                        mTerraScale.y*=mTerraScaleFactor;
                    else if(evt.state.Z.rel<0)
                        mTerraScale.y/=mTerraScaleFactor;
                }
                else if (mInputMan->isKeyDown(OIS::KC_LCONTROL))
                {
                    //surface
                    if(evt.state.Z.rel>0)
                    {
                        mTerraScale.x*=mTerraScaleFactor;
                        mTerraScale.z*=mTerraScaleFactor;
                    }
                    else if(evt.state.Z.rel<0)
                    {
                        mTerraScale.x/=mTerraScaleFactor;
                        mTerraScale.z/=mTerraScaleFactor;
                    }
                }
                else
                {
                    // height+surface
                    if(evt.state.Z.rel>0)
                        mTerraScale*=mTerraScaleFactor;
                    else if(evt.state.Z.rel<0)
                        mTerraScale/=mTerraScaleFactor;
                }
                sTerraBrushVisual->setScale(mTerraScale);
            }
        }

        if(evt.state.buttonDown(OIS::MB_Left))
        {
            if(mIsDraggingSelection)
            {
                if (mEngine->hasSelection())
                {
                    std::list<ModelId> selection;
                    mEngine->pickAgents(selection, evt.state.X.abs, evt.state.Y.abs);
                    if(mIsDraggingSelectionCancelled)
                        return true;
                    Ogre::Vector3 selectionPos = mEngine->selectionPosition();
                    Ogre::Vector3 src,dst;
                    Ogre::Vector3 camPos=mEngine->level()->camera()->camNode()->getPosition();
                    // wall facing the camera, placed between cam and selection
                    auto dirToCam=camPos-selectionPos;
                    Ogre::Plane vPlane=Ogre::Plane(dirToCam, selectionPos);
                    Ogre::Plane hPlane= Ogre::Plane(Ogre::Vector3::UNIT_Y, selectionPos);

                    switch (mMode)
                    {
                        case TRANSLATE:
                        {
                            if (mInputMan->isKeyDown(OIS::KC_LSHIFT))
                            {
                                if(mousePlaneProjection(vPlane,_x / w, _y / h,src) && mousePlaneProjection(vPlane,x / w, y / h,dst))
                                {
                                    mEngine->moveSelection(Ogre::Vector3(.0f,(dst-src).y,.0f));
                                }
                            }
                            else
                            {
                                if(mousePlaneProjection(hPlane,_x / w, _y / h,src) && mousePlaneProjection(hPlane,x / w, y / h,dst))
                                {
                                    auto dpos=dst-src;
                                    dpos.y=.0f;
                                    mEngine->moveSelection(dpos);
                                }
                            }
                        }
                        break;
                        case ROTATE:
                        {
                            if(mInputMan->isKeyDown(OIS::KC_LSHIFT))
                            {
                                Ogre::Plane plane = Ogre::Plane(dirToCam, (camPos+selectionPos)/2.f);
                                if(mousePlaneProjection(plane,_x / w, _y / h,src) && mousePlaneProjection(plane,x / w, y / h,dst))
                                {
                                    src-=selectionPos;
                                    dst-=selectionPos;
                                    Ogre::Radian r=src.angleBetween(dst);
                                    mEngine->rotateSelectionRotationAroundCenter(r,src.crossProduct(dst));
                                }
                            }
                            else
                            {
                                if(mousePlaneProjection(hPlane,_x / w, _y / h,src) && mousePlaneProjection(hPlane,x / w, y / h,dst))
                                {
                                    src-=selectionPos;
                                    dst-=selectionPos;
                                    // take the angle between them, in the direction given by the sign of their crossProduct (cw/ccw)
                                    Ogre::Radian r=src.angleBetween(dst)*Ogre::Math::Sign(src.crossProduct(dst).y);
                                    // rotate selection around Y on its center of the same amount
                                    mEngine->rotateSelectionRotationAroundCenter(r,Ogre::Vector3::UNIT_Y);
                                    //// simple heuristic - early ages
                                    //Ogre::Vector3 r = Ogre::Vector3(.0f, 180.f * (x - _x) / (w / 2.f), .0f);
                                    //mEngine->rotateSelection(r);
                                }
                            }
                        }
                        break;
                        case SCALE:
                        {
                            if(mousePlaneProjection(hPlane,_x / w, _y / h,src) && mousePlaneProjection(hPlane,x / w, y / h,dst))
                            {
                                Ogre::Real factor=selectionPos.distance(dst)/selectionPos.distance(src);
                                mEngine->rescaleSelection(Ogre::Vector3(factor,factor,factor));
                            }
                        }
                        break;
                        case TERRAFORM:
                            // not only done when the mouse moves, but also when it stays at the same place with a button held down,
                            // hence, implemented in frameRenderingQueued
                            break;
                        default:
                            break;
                    } //end of switch (mMode)
                } //end of if(mEngine->hasSelection())
            }// end of if(mIsDraggingSelection)
            if(mIsSelecting)
            {
                Ogre::Vector2 screenSize(float(mEditor->width()),float(mEditor->height()));
                mSelectionBox->setCorners(mInputMan->mousePosAtLastMousePressed()/screenSize,mInputMan->mousePos()/screenSize);
            }
        } //end of if(evt.state.buttonDown(OIS::MB_Left))
        // not used
        return true;
    }

    bool EditorBrush::mousePlaneProjection(const Ogre::Plane &plane,float x, float y,Ogre::Vector3 &pos)
    {
        Ogre::Ray ray = mEngine->level()->camera()->cam()->getCameraToViewportRay(x, y);
        std::pair<bool, Ogre::Real> result = ray.intersects(plane);
        if (result.first)
        {
            pos=ray.getPoint(result.second);
            return true;
        }
        return false;
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
        //TODO: put this in a proper terraform method
        if(mMode==TERRAFORM)
        {
            if(sTerraBrushVisual==NULL)
                getBrush(TERRAFORM);
            auto level=mEngine->level();
            if(level!=NULL && sTerraBrushVisual!=NULL)
            {
                // move
                auto ray=mEngine->level()->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                auto hitTest=level->terrainManager()->intersectRay(ray);
                if(hitTest.hit)
                {
                    sTerraBrushVisual->setVisible(true);
                    sTerraBrushVisual->setPosition(hitTest.position);
                    if(mContinuousModeActivated)
                    {
                        // action !
                        Ogre::TerrainGroup::TerrainList newTids;
                        // values for OIS::MB_Left down
                        Ogre::Real _intensity=intensity();
                        Ogre::Real _radius=radius();
                        TerrainManager::RaiseMode rMode=TerrainManager::RELATIVE;
                        if(mInputMan->mouse()->getMouseState().buttonDown(OIS::MB_Right))
                            _intensity=-_intensity;
                        if(mInputMan->mouse()->getMouseState().buttonDown(OIS::MB_Middle))
                        {
                            rMode=TerrainManager::ABSOLUTE;
                            hitTest.position.y=mSelectedTerrainHeight;
                        }
                        newTids=level->terrainManager()->raiseTerrainAt(hitTest.position,_intensity,_radius,rMode,mRaiseShape);
                        mModifiedTerrains.insert(newTids.begin(),newTids.end());
                    }
                }
                else
                    sTerraBrushVisual->setVisible(false);
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
        else if(command[0]=="terrabrush")
        {
            command.erase(command.begin());
            if(command.size()>0 && command[0]=="distribution")
            {
                command.erase(command.begin());
                if(command.size()==0)
                {
                    Debug::warning("EditorBrush::processCommand(): terrabrush needs a distribution").endl();
                    return;
                }
                if(command[0]=="uniform")
                    mRaiseShape=TerrainManager::RaiseShape::UNIFORM;
                else if(command[0]=="round")
                    mRaiseShape=TerrainManager::RaiseShape::ROUND;
                else if(command[0]=="sinh")
                    mRaiseShape=TerrainManager::RaiseShape::SINH;
                else if(command[0]=="triangular")
                    mRaiseShape=TerrainManager::RaiseShape::TRANGULAR;
                else
                {
                    Debug::warning("EditorBrush::processCommand(): unknown terrabrush distribution").endl();
                    return;
                }
            }
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
            Ogre::Root::getSingletonPtr()->removeFrameListener(this);

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
                if(sTerraBrushVisual==NULL)
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
                        sTerraBrushVisual = level->levelRoot()->createChildSceneNode("terraBrushVisual");

                        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create("terraBrushVisual_material","UI");
                        material->setReceiveShadows(false);
                        material->getTechnique(0)->setLightingEnabled(true);

//                         material->getTechnique(0)->getPass(0)->setDiffuse(1,1,1,.5);
//                         material->getTechnique(0)->getPass(0)->setAmbient(1,1,1);
//                         material->getTechnique(0)->getPass(0)->setSelfIllumination(.5,.5,.5);
                        material->getTechnique(0)->getPass(0)->setAmbient(1,1,1);
                        material->getTechnique(0)->getPass(0)->setDiffuse(1,1,1,.5);
                        material->getTechnique(0)->getPass(0)->setSelfIllumination(1,1,1);
                        material->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
                        material->setDepthWriteEnabled(false);
                        entity->setMaterial(material);
                        sTerraBrushVisual->attachObject(entity);
                    }
                }
                else
                    sTerraBrushVisual->setVisible(true);
                sTerraBrushVisual->setScale(mTerraScale);
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
                if(sTerraBrushVisual!=NULL)
                {
                    OgreUtils::destroySceneNode(sTerraBrushVisual);
                    sTerraBrushVisual = NULL;
                }
                mModifiedTerrains.clear();
                break;
            default:
                break;
        }
    }

    void EditorBrush::onShow()
    {
        popMode();
        mSelectionBox=new SelectionBox("EditorBrushSelectionBox",mEngine);
        mSelectionBox->setVisible(false);
    }

    void EditorBrush::onHide()
    {
        if(NULL!=mSelectionBox)
        {
            OgreUtils::destroySceneNode(mSelectionBox->getParentSceneNode());
            delete mSelectionBox;
            mSelectionBox=NULL;
        }

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





