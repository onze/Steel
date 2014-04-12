#include "steeltypes.h"
#include "UI/EditorBrush.h"
#include "Engine.h"
#include "UI/Editor.h"
#include "UI/SelectionBox.h"
#include "UI/UI.h"
#include "Camera.h"
#include "Level.h"
#include "tools/Cylinder.h"
#include "tools/OgreUtils.h"
#include "tools/DynamicLines.h"
#include "models/Agent.h"
#include "SelectionManager.h"
#include "models/AgentManager.h"
#include "models/LocationModelManager.h"
#include "OgreManualObject.h"
#include "OgreMeshManager.h"
#include "tools/StringUtils.h"
#include "Debug.h"

namespace Steel
{
    Ogre::SceneNode *EditorBrush::sTerraBrushVisual = nullptr;

    const Ogre::String EditorBrush::MODE = "EditorBrush::mode";
    const Ogre::String EditorBrush::TERRA_SCALE = "EditorBrush::terraScale";
    const Ogre::String EditorBrush::TERRA_SCALE_FACTOR = "EditorBrush::terraScaleFactor";

    EditorBrush::EditorBrush()
        : Ogre::FrameListener(), mEngine(nullptr), mEditor(nullptr), mInputMan(nullptr),
          mMode(BrushMode::TRANSLATE), mContinuousModeActivated(false),
          mSelectionPosBeforeTransformation(std::vector<Ogre::Vector3>()),
          mSelectionRotBeforeTransformation(std::vector<Ogre::Quaternion>()),
          mSelectionScaleBeforeTransformation(std::vector<Ogre::Vector3>()),
          mIsDraggingSelection(false), mIsDraggingSelectionCancelled(false),
          mTerraScaleFactor(1.1f), mTerraScale(1.f, 1.f, 1.f), mSelectedTerrainHeight(.0f),
          mRaiseShape(TerrainManager::RaiseShape::UNIFORM), mModeStack(std::vector<BrushMode>()),
          mModifiedTerrains(std::set<Ogre::Terrain *>()), mIsSelecting(false), mSelectionBox(nullptr),
          mLinkingLine(),
          mLinkingSourceAgentValidationFn(nullptr), mLinkingDestinationAgentValidationFn(nullptr),
          mLinkingValidatedCallbackFn(nullptr), mLinkingValidatedAlternateCallbackFn(nullptr)
    {
    }

    EditorBrush::EditorBrush(const EditorBrush &other)
    {
        Debug::error("EditorBrush::EditorBrush(const EditorBrush&) not implemented").endl().breakHere();
    }

    EditorBrush::~EditorBrush()
    {
        shutdown();
    }

    void EditorBrush::loadConfig(ConfigFile const &config)
    {
        mTerraScale = Ogre::StringConverter::parseVector3(config.getSetting(EditorBrush::TERRA_SCALE), mTerraScale);
        mTerraScaleFactor = config.getSettingAsFloat(EditorBrush::TERRA_SCALE_FACTOR, mTerraScaleFactor);
        setMode((EditorBrush::BrushMode) config.getSettingAsUnsignedLong(EditorBrush::MODE, (unsigned int) mMode));
    }

    void EditorBrush::saveConfig(ConfigFile &config) const
    {
        config.setSetting(EditorBrush::TERRA_SCALE, Ogre::StringConverter::toString(mTerraScale));
        config.setSetting(EditorBrush::TERRA_SCALE_FACTOR, Ogre::StringConverter::toString(mTerraScaleFactor));
        config.setSetting(EditorBrush::MODE, Ogre::StringConverter::toString((unsigned int)mMode));
    }

    EditorBrush &EditorBrush::operator=(const EditorBrush &other)
    {
        Debug::error("EditorBrush::operator=() not implemented").endl().breakHere();
        return *this;
    }

    bool EditorBrush::operator==(const EditorBrush &other) const
    {
        Debug::error("EditorBrush::operator==() not implemented").endl().breakHere();
        return false;
    }

    void EditorBrush::init(Engine *engine, Editor *editor, InputManager *inputMan)
    {
        mEngine = engine;
        mEditor = editor;
        mInputMan = inputMan;
        mIsDraggingSelectionCancelled = mIsDraggingSelection = false;
    }

    void EditorBrush::shutdown()
    {
        if(nullptr != mSelectionBox)
        {
            OgreUtils::destroySceneNode(mSelectionBox->getParentSceneNode());
            delete mSelectionBox;
            mSelectionBox = nullptr;
        }

        mLinkingSourceAgentValidationFn = nullptr;
        mLinkingDestinationAgentValidationFn = nullptr;
        mLinkingValidatedCallbackFn = nullptr;
        mLinkingValidatedAlternateCallbackFn = nullptr;
    }

    float EditorBrush::intensity()
    {
        if(sTerraBrushVisual == nullptr)
            return .0f;

        return sTerraBrushVisual->getScale().y / 3.f;
    }

    float EditorBrush::radius()
    {
        if(sTerraBrushVisual == nullptr)
            return .0f;

        return (sTerraBrushVisual->getScale().x + sTerraBrushVisual->getScale().z) / 2.f;
    }

    Selection EditorBrush::mousePressedSelectionUpdate(Input::Code button, Input::Event const &evt)
    {
        auto level = mEngine->level();
        SelectionManager *selectionMan = level->selectionMan();

        switch(button)
        {
            case Input::Code::MC_LEFT:
            {
                std::list<ModelId> selection;
                mEngine->pickAgents(selection, evt.position.x, evt.position.y);

                // click on nothing
                if(selection.size() == 0 && !mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                    level->selectionMan()->clearSelection();
                else if(selectionMan->hasSelection())
                {
                    // clicked a new agent
                    if(selectionMan->isSelected(selection.front()))
                    {
                        if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                            selectionMan->removeFromSelection(selection);
                    }
                    else
                        selectionMan->setSelectedAgents(selection, !mInputMan->isKeyDown(Input::Code::KC_LCONTROL));
                }
                else
                {
                    selectionMan->setSelectedAgents(selection);
                    //Debug::log("EditorBrush::mousePressed(): selection position: ")(selectionMan->selectionPosition()).endl();
                }

                if(selectionMan->hasSelection())
                {
                    mIsDraggingSelection = true;
                    // saved so that we know what to reset properties to
                    //TODO: save complete serialisations
                    mSelectionPosBeforeTransformation = selectionMan->selectionPositions();
                    mSelectionRotBeforeTransformation = selectionMan->selectionRotations();
                    mSelectionScaleBeforeTransformation = selectionMan->selectionScales();
                }
                else
                {
                    mIsSelecting = true;
                    mSelectionBox->setVisible(true);
                    Ogre::Vector2 screenSize(float(mEditor->width()), float(mEditor->height()));
                    mSelectionBox->setCorners(mInputMan->mousePosAtLastMousePressed() / screenSize, evt.position / screenSize);
                }

                break;
            }

            case Input::Code::MC_RIGHT:

                // cancel current selection dragging (translate, etc)
                if(mIsDraggingSelection)
                {
                    mIsDraggingSelectionCancelled = true;
                    selectionMan->setSelectionPositions(mSelectionPosBeforeTransformation);
                    selectionMan->setSelectionRotations(mSelectionRotBeforeTransformation);
                    selectionMan->setSelectionScales(mSelectionScaleBeforeTransformation);
                }

                break;

            default:
                break;
        }

        return selectionMan->selection();
    }

    bool EditorBrush::mouseWheeled(int delta, Input::Event const &evt)
    {
        switch(mMode)
        {
            case BrushMode::TERRAFORM:
            {
                // resize
                if(sTerraBrushVisual == nullptr)
                    getBrush(BrushMode::TERRAFORM);

                if(sTerraBrushVisual != nullptr)
                {
                    if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    {
                        checkTerraScaleFactorValue();

                        // height
                        if(delta > 0)
                            mTerraScale.y *= mTerraScaleFactor;
                        else if(delta < 0)
                            mTerraScale.y /= mTerraScaleFactor;
                    }
                    else if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
                    {
                        checkTerraScaleFactorValue();

                        //surface
                        if(delta > 0)
                        {
                            mTerraScale.x *= mTerraScaleFactor;
                            mTerraScale.z *= mTerraScaleFactor;
                        }
                        else if(delta < 0)
                        {
                            mTerraScale.x /= mTerraScaleFactor;
                            mTerraScale.z /= mTerraScaleFactor;
                        }
                    }
                    else
                    {
                        // height+surface
                        if(delta > 0)
                            mTerraScale *= mTerraScaleFactor;
                        else if(delta < 0)
                            mTerraScale /= mTerraScaleFactor;
                    }

                    sTerraBrushVisual->setScale(mTerraScale);
                }
            }

            default:
                break;
        }

        return true;
    }

    bool EditorBrush::mousePressed(Input::Code button, Input::Event const &evt)
    {
        mContinuousModeActivated = false;
        auto level = mEngine->level();

//         SelectionManager *selectionMan=level->selectionMan();
        switch(mMode)
        {
            case BrushMode::TRANSLATE:
            case BrushMode::ROTATE:
            case BrushMode::SCALE:
                mousePressedSelectionUpdate(button, evt);
                break;

            case BrushMode::TERRAFORM:
                mContinuousModeActivated = true;

                switch(button)
                {
                    case Input::Code::MC_MIDDLE:
                    {
                        Ogre::Terrain *terrain;
                        auto pos = level->terrainManager()->terrainGroup()->getHeightAtWorldPosition(sTerraBrushVisual->getPosition(), &terrain);

                        // keep last valid value
                        if(terrain != nullptr)
                            mSelectedTerrainHeight = pos;

                        break;
                    }

                    default:
                        break;
                }

                break;

            case BrushMode::LINK:
            {
                AgentId aid = mEngine->level()->agentIdUnderMouse();

                if(nullptr != mLinkingSourceAgentValidationFn)
                    if(!mLinkingSourceAgentValidationFn(aid))
                        break;

                mFirstLinkedAgent = aid;
                mContinuousModeActivated = true;
                break;
            }

            default:
                break;
        }

        // not used
        return true;
    }

    bool EditorBrush::mouseReleased(Input::Code button, const Input::Event &evt)
    {
        static const Ogre::String intro = "in EditorBrush::mouseReleased(): ";
        auto level = mEngine->level();
        SelectionManager *selectionMan = level->selectionMan();

        switch(mMode)
        {
            case BrushMode::TRANSLATE:
            case BrushMode::ROTATE:
            case BrushMode::SCALE:
                if(button == Input::Code::MC_LEFT)
                {
                    if(mIsSelecting)
                    {
                        Ogre::Vector2 screenSize(float(mEditor->width()), float(mEditor->height()));
                        mSelectionBox->setCorners(mInputMan->mousePosAtLastMousePressed() / screenSize,
                                                  mInputMan->mousePos() / screenSize);

                        std::list<AgentId> selection;
                        mSelectionBox->performSelection(selection, mEngine->level()->camera()->cam());

                        if(selection.size() > 0)
                            selectionMan->setSelectedAgents(selection, !mInputMan->isKeyDown(Input::Code::KC_LCONTROL));

                        mSelectionBox->setVisible(false);
                        mIsSelecting = false;
                    }

                    if(mIsDraggingSelection)
                    {
                        mIsDraggingSelection = false;
                        mIsDraggingSelectionCancelled = false;
                        mSelectionPosBeforeTransformation = selectionMan->selectionPositions();
                        mSelectionRotBeforeTransformation = selectionMan->selectionRotations();
                    }
                }

                break;

            case BrushMode::TERRAFORM:
            {
                if(mContinuousModeActivated)
                {
                    //now might be the right time to update blendmaps
                    for(auto it = mModifiedTerrains.begin(); it != mModifiedTerrains.end(); ++it)
                    {
                        Ogre::Terrain *terrain = *it;
                        terrain->update(true);
                        level->terrainManager()->updateBlendMaps(terrain);
//                             level->terrainManager()->updateHeightmap(terrain);
                        continue;

                        for(int index = 1; index < terrain->getLayerCount(); ++index)
                            terrain->getLayerBlendMap(index)->dirty();

                        for(int index = 1; index < terrain->getLayerCount(); ++index)
                            terrain->getLayerBlendMap(index)->update();

                        terrain->updateDerivedData(true);
                    }
                }

                break;
            }

            case BrushMode::LINK:
                if(mContinuousModeActivated)
                {
                    AgentId aid = mEngine->level()->agentIdUnderMouse();

                    if(INVALID_ID == aid)
                        break;

                    if(aid == mFirstLinkedAgent)
                        break;

                    if(nullptr != mLinkingDestinationAgentValidationFn)
                        if(!mLinkingDestinationAgentValidationFn(aid))
                            break;

                    if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                    {
                        if(nullptr != mLinkingValidatedAlternateCallbackFn)
                        {
                            if(mLinkingValidatedAlternateCallbackFn(mFirstLinkedAgent, aid))
                                Debug::log(intro)("alternate link operation succeeded.").endl();
                            else
                                Debug::log(intro)("alternate link operation failed.").endl();
                        }
                    }
                    else
                    {
                        if(nullptr != mLinkingValidatedCallbackFn)
                        {
                            if(mLinkingValidatedCallbackFn(mFirstLinkedAgent, aid))
                                Debug::log(intro)("link operation succeeded.").endl();
                            else
                                Debug::log(intro)("link operation failed.").endl();
                        }
                    }
                }

                break;

            case BrushMode::NONE:
            default:
                break;
        }

        // clean linking mode state
        mFirstLinkedAgent = INVALID_ID;
        mLinkingLine.clear();
        mLinkingLine.update();

        mContinuousModeActivated = false;
        // not used
        return true;
    }

    bool EditorBrush::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        Level *level = mEngine->level();
        SelectionManager *selectionMan = level->selectionMan();
        Ogre::Real x = float(position.x);
        Ogre::Real y = float(position.y);
        Ogre::Real _x = x - float(evt.speed.x);
        Ogre::Real _y = y - float(evt.speed.y);
        Ogre::Real w = float(mEditor->width());
        Ogre::Real h = float(mEditor->height());

        if(mInputMan->isKeyDown(Input::Code::MC_LEFT))
        {
            if(mContinuousModeActivated)
            {
                switch(mMode)
                {
                    case BrushMode::LINK:
                    {
                        if(level->agentMan()->isIdFree(mFirstLinkedAgent))
                            break;

                        // draw a link between source and cursor
                        mLinkingLine.clear();
                        Agent *agent = level->agentMan()->getAgent(mFirstLinkedAgent);
                        Ogre::Vector2 srcPos = level->camera()->screenPosition(agent->position());
                        // ortho view -> invert y axis
                        srcPos.y = 1.f - srcPos.y;
                        // then rescale from [0,1] to [-1,1]
                        srcPos = srcPos * 2.f - 1.f;
                        mLinkingLine.addPoint(srcPos);
                        Ogre::Vector2 dstPos(x / w, 1.f - y / h);
                        dstPos = dstPos * 2.f - 1.f;
                        mLinkingLine.addPoint(dstPos);
                        mLinkingLine.update();
                        break;
                    }

                    default:
                        break;
                }
            }

            if(mIsDraggingSelection)
            {
                if(selectionMan->hasSelection())
                {
                    std::list<ModelId> selection;
                    mEngine->pickAgents(selection, evt.position.x, evt.position.y);

                    if(mIsDraggingSelectionCancelled)
                        return true;

                    Ogre::Vector3 selectionPos = selectionMan->selectionPosition();
                    Ogre::Vector3 src, dst;
                    Ogre::Vector3 camPos = mEngine->level()->camera()->camNode()->getPosition();
                    // wall facing the camera, placed between cam and selection
                    auto dirToCam = camPos - selectionPos;
                    Ogre::Plane vPlane = Ogre::Plane(dirToCam, selectionPos);
                    Ogre::Plane hPlane = Ogre::Plane(Ogre::Vector3::UNIT_Y, selectionPos);

                    switch(mMode)
                    {
                        case BrushMode::TRANSLATE:
                        {
                            if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                            {
                                if(mousePlaneProjection(vPlane, _x / w, _y / h, src) && mousePlaneProjection(vPlane, x / w, y / h, dst))
                                {
                                    selectionMan->moveSelection(Ogre::Vector3(.0f, (dst - src).y, .0f));
                                }
                            }
                            else
                            {
                                if(mousePlaneProjection(hPlane, _x / w, _y / h, src) && mousePlaneProjection(hPlane, x / w, y / h, dst))
                                {
                                    auto dpos = dst - src;
                                    dpos.y = .0f;
                                    selectionMan->moveSelection(dpos);
                                }
                            }
                        }
                        break;

                        case BrushMode::ROTATE:
                        {
                            if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                            {
                                Ogre::Plane plane = Ogre::Plane(dirToCam, (camPos + selectionPos) / 2.f);

                                if(mousePlaneProjection(plane, _x / w, _y / h, src) && mousePlaneProjection(plane, x / w, y / h, dst))
                                {
                                    src -= selectionPos;
                                    dst -= selectionPos;
                                    Ogre::Radian r = src.angleBetween(dst);
                                    selectionMan->rotateSelectionRotationAroundCenter(r, src.crossProduct(dst));
                                }
                            }
                            else
                            {
                                if(mousePlaneProjection(hPlane, _x / w, _y / h, src) && mousePlaneProjection(hPlane, x / w, y / h, dst))
                                {
                                    src -= selectionPos;
                                    dst -= selectionPos;
                                    // take the angle between them, in the direction given by the sign of their crossProduct (cw/ccw)
                                    Ogre::Radian r = src.angleBetween(dst) * Ogre::Math::Sign(src.crossProduct(dst).y);
                                    // rotate selection around Y on its center of the same amount
                                    selectionMan->rotateSelectionRotationAroundCenter(r, Ogre::Vector3::UNIT_Y);
                                    //// simple heuristic - early ages
                                    //Ogre::Vector3 r = Ogre::Vector3(.0f, 180.f * (x - _x) / (w / 2.f), .0f);
                                    //selectionMan->rotateSelection(r);
                                }
                            }
                        }
                        break;

                        case BrushMode::SCALE:
                        {
                            if(mousePlaneProjection(hPlane, _x / w, _y / h, src) && mousePlaneProjection(hPlane, x / w, y / h, dst))
                            {
                                Ogre::Real factor = selectionPos.distance(dst) / selectionPos.distance(src);

                                if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
                                    selectionMan->expandSelection(selectionPos.distance(dst) - selectionPos.distance(src));
                                else
                                    selectionMan->rescaleSelection(Ogre::Vector3(factor, factor, factor));
                            }
                        }
                        break;

                        case BrushMode::TERRAFORM:
                            // not only done when the mouse moves, but also when it stays at the same place with a button held down,
                            // hence, implemented in frameRenderingQueued
                            break;

                        default:
                            break;
                    } //end of switch (mMode)
                } //end of if(selectionMan->hasSelection())
            } // end of if(mIsDraggingSelection)

            if(mIsSelecting)
            {
                Ogre::Vector2 screenSize(float(mEditor->width()), float(mEditor->height()));
                mSelectionBox->setCorners(mInputMan->mousePosAtLastMousePressed() / screenSize, mInputMan->mousePos() / screenSize);
            }// end of if(mIsSelecting)
        } //end of if(evt.state.buttonDown(OIS::MB_Left))

        // not used
        return true;
    }

    void EditorBrush::checkTerraScaleFactorValue()
    {
        if(Ogre::Math::Abs(mTerraScaleFactor - 1.f) < 0.001)
        {
            Debug::warning("EditorBrush::TerraScaleFactor value is very close to 1 (");
            Debug::warning(mTerraScaleFactor)("). Terraforming might prove hard. Set it to x>1.1 in the conf.").endl();
        }
    }

    bool EditorBrush::mousePlaneProjection(const Ogre::Plane &plane, float x, float y, Ogre::Vector3 &pos)
    {
        Ogre::Ray ray = mEngine->level()->camera()->cam()->getCameraToViewportRay(x, y);
        std::pair<bool, Ogre::Real> result = ray.intersects(plane);

        if(result.first)
        {
            pos = ray.getPoint(result.second);
            return true;
        }

        return false;
    }

    bool EditorBrush::frameRenderingQueued(const Ogre::FrameEvent &evt)
    {
        auto state = mInputMan->mouse()->getMouseState();
        Ogre::Real x = float(state.X.abs);
        Ogre::Real y = float(state.Y.abs);
//         Ogre::Real _x = x-float(state.X.rel);
//         Ogre::Real _y = y-float(state.Y.rel);
        Ogre::Real w = float(mEditor->width());
        Ogre::Real h = float(mEditor->height());

        // terraform does not need a selection
        //TODO: put this in a proper terraform method
        if(mMode == BrushMode::TERRAFORM)
        {
            if(sTerraBrushVisual == nullptr)
                getBrush(BrushMode::TERRAFORM);

            auto level = mEngine->level();

            if(level != nullptr && sTerraBrushVisual != nullptr)
            {
                // move
                auto ray = mEngine->level()->camera()->cam()->getCameraToViewportRay(x / w, y / h);
                auto hitTest = level->terrainManager()->intersectRay(ray);

                if(hitTest.hit)
                {
                    sTerraBrushVisual->setVisible(true);
                    sTerraBrushVisual->setPosition(hitTest.position);

                    if(mContinuousModeActivated)
                    {
                        Ogre::TerrainGroup::TerrainList newTids;
                        Ogre::Real _intensity;
                        Ogre::Real _radius = radius();
                        TerrainManager::RaiseMode rMode = TerrainManager::RaiseMode::RELATIVE;

                        if(mInputMan->isKeyDown(Input::Code::MC_LEFT))
                        {
                            if(mInputMan->isKeyDown(Input::Code::MC_RIGHT))
                            {
                                rMode = TerrainManager::RaiseMode::ABSOLUTE;
                                hitTest.position.y = mSelectedTerrainHeight;
                            }
                            else
                            {
                                _intensity = intensity();
                            }
                        }
                        else if(mInputMan->isKeyDown(Input::Code::MC_RIGHT))
                        {
                            _intensity = -intensity();
                        }

                        newTids = level->terrainManager()->raiseTerrainAt(hitTest.position, _intensity, _radius, rMode, mRaiseShape);
                        mModifiedTerrains.insert(newTids.begin(), newTids.end());
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

    bool EditorBrush::processCommand(std::vector<Ogre::String> command)
    {
        static const Ogre::String intro = "EditorBrush::processCommand(): ";

        if(command[0] == "mode")
        {
            command.erase(command.begin());

            // drop terraform brush model
            if(command[0] == "translate")
                setMode(BrushMode::TRANSLATE);
            else if(command[0] == "rotate")
                setMode(BrushMode::ROTATE);
            else if(command[0] == "scale")
                setMode(BrushMode::SCALE);
            else if(command[0] == "terraform")
                setMode(BrushMode::TERRAFORM);
            else if(command[0] == "link")
            {
                command.erase(command.begin());
                processLinkCommand(command);
            }
            else
            {
                Debug::warning(intro)("unknown mode ").quotes(StringUtils::join(command, ".")).endl();
                return false;
            }
        }
        else if(command[0] == "terrabrush")
        {
            command.erase(command.begin());

            if(command.size() > 0 && command[0] == "distribution")
            {
                command.erase(command.begin());

                if(command.size() == 0)
                {
                    Debug::warning(intro)("terrabrush needs a distribution").endl();
                    return false;
                }

                if(command[0] == "uniform")
                    mRaiseShape = TerrainManager::RaiseShape::UNIFORM;
                else if(command[0] == "round")
                    mRaiseShape = TerrainManager::RaiseShape::ROUND;
                else if(command[0] == "sinh")
                    mRaiseShape = TerrainManager::RaiseShape::SINH;
                else if(command[0] == "triangular")
                    mRaiseShape = TerrainManager::RaiseShape::TRANGULAR;
                else
                {
                    Debug::warning("EditorBrush::processCommand(): unknown terrabrush distribution").endl();
                    return false;
                }
            }
            else
            {
                Debug::warning("EditorBrush::processCommand(): unknown terrabrush command: ")(command).endl();
                return false;
            }
        }
        else
        {
            Debug::warning("EditorBrush::processCommand(): unknown command: ")(command).endl();
            return false;
        }

        return true;
    }

    bool EditorBrush::processLinkCommand(std::vector<Ogre::String> command)
    {

        static const Ogre::String intro = "EditorBrush::processCommand(): ";

        if(command.size() == 0)
        {
            Debug::error(intro)("empty link command").endl();
            return false;
        }

        if(command[0] == "build_path")
        {
            setLinkingMode(nullptr,//std::bind(&AgentManager::agentCanBePathSource, mEngine->level()->agentMan(), std::placeholders::_1),
                           nullptr,//std::bind(&AgentManager::agentCanBePathDestination, mEngine->level()->agentMan(), std::placeholders::_1),
                           std::bind(&LocationModelManager::linkAgents, mEngine->level()->locationModelMan(), std::placeholders::_1, std::placeholders::_2),
                           std::bind(&LocationModelManager::unlinkAgents, mEngine->level()->locationModelMan(), std::placeholders::_1, std::placeholders::_2)
                          );
        }
        else if(command[0] == "assign_path")
        {
            setLinkingMode(nullptr,//std::bind(&AgentManager::agentCanBeAssignedBTPath, mEngine->level()->agentMan(), std::placeholders::_1),
                           nullptr,//std::bind(&AgentManager::agentHasLocationPath, mEngine->level()->agentMan(), std::placeholders::_1),
                           std::bind(&AgentManager::assignBTPath, mEngine->level()->agentMan(), std::placeholders::_1, std::placeholders::_2),
                           std::bind(&AgentManager::unassignBTPath, mEngine->level()->agentMan(), std::placeholders::_1, std::placeholders::_2)
                          );
        }
        else
        {
            Debug::error(intro)("unknown linking command ").quotes(command).endl();
            return false;
        }

        return true;
    }

    void EditorBrush::setLinkingMode(std::function<bool(AgentId srcAid)> sourceAgentValidation,
                                     std::function<bool(AgentId dstAid)> destinationAgentValidation,
                                     std::function<bool(AgentId srcAid, AgentId const dstAid)> linkingValidatedCallbackFn,
                                     std::function<bool(AgentId srcAid, AgentId const dstAid)> linkingValidatedAlternateCallbackFn)
    {
        setMode(BrushMode::LINK);
        mLinkingSourceAgentValidationFn = sourceAgentValidation;
        mLinkingDestinationAgentValidationFn = destinationAgentValidation;
        mLinkingValidatedCallbackFn = linkingValidatedCallbackFn;
        mLinkingValidatedAlternateCallbackFn = linkingValidatedAlternateCallbackFn;
    }

    void EditorBrush::setMode(BrushMode mode)
    {
        if(mode == mMode)
            return;

        // now we know we are setting a new mode
        dropBrush();

        if(mMode == BrushMode::TERRAFORM)
            Ogre::Root::getSingletonPtr()->removeFrameListener(this);

        mMode = mode;

        switch(mMode)
        {
            case BrushMode::TERRAFORM:
                getBrush(BrushMode::TERRAFORM);
                Ogre::Root::getSingletonPtr()->addFrameListener(this);
                break;

            case BrushMode::TRANSLATE:
            case BrushMode::ROTATE:
            case BrushMode::SCALE:
            case BrushMode::LINK:
                break;

            case BrushMode::NONE:
            default:
                Debug::warning("in EditorBrush::setMode(): ")("unknown mode ").quotes(toString(mode)).endl();
                mMode = BrushMode::NONE;
                break;
        }

        if(BrushMode::LINK != mMode)
        {
            mLinkingDestinationAgentValidationFn = nullptr;
            mLinkingSourceAgentValidationFn = nullptr;
            mLinkingValidatedCallbackFn = nullptr;
            mLinkingValidatedAlternateCallbackFn = nullptr;
        }
    }

    void EditorBrush::getBrush(BrushMode mode)
    {
        switch(mMode)
        {
            case BrushMode::TERRAFORM:
                if(sTerraBrushVisual == nullptr)
                {
                    auto level = mEngine->level();

                    if(level != nullptr)
                    {
                        auto sm = level->sceneManager();
                        Ogre::MeshPtr mesh = Cylinder::getMesh(sm, 1, 1);
                        Ogre::Entity *entity;

                        if(sm->hasEntity("cylinderEntity"))
                            entity = sm->getEntity("cylinderEntity");
                        else
                            entity = sm->createEntity("cylinderEntity", mesh);

                        sTerraBrushVisual = level->levelRoot()->createChildSceneNode("terraBrushVisual");

                        Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create("terraBrushVisual_material",
                                                     "UI");
                        material->setReceiveShadows(false);
                        material->getTechnique(0)->setLightingEnabled(true);

//                         material->getTechnique(0)->getPass(0)->setDiffuse(1,1,1,.5);
//                         material->getTechnique(0)->getPass(0)->setAmbient(1,1,1);
//                         material->getTechnique(0)->getPass(0)->setSelfIllumination(.5,.5,.5);
                        material->getTechnique(0)->getPass(0)->setAmbient(1, 1, 1);
                        material->getTechnique(0)->getPass(0)->setDiffuse(1, 1, 1, .5);
                        material->getTechnique(0)->getPass(0)->setSelfIllumination(1, 1, 1);
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
            case BrushMode::TERRAFORM:
                if(sTerraBrushVisual != nullptr)
                {
                    OgreUtils::destroySceneNode(sTerraBrushVisual);
                    sTerraBrushVisual = nullptr;
                }

                mModifiedTerrains.clear();
                break;

            default:
                break;
        }
    }

    void EditorBrush::onShow()
    {
        mSelectionBox = new SelectionBox("EditorBrushSelectionBox", mEngine);
        mSelectionBox->setVisible(false);

        mLinkingLine.init(mEngine->ui()->resourceGroup(), Ogre::RenderOperation::OT_LINE_LIST, true);
        mEngine->level()->levelRoot()->attachObject(&mLinkingLine);

        popMode();
    }

    void EditorBrush::onHide()
    {
        pushMode();
        setMode(BrushMode::NONE);

        if(nullptr != mSelectionBox)
        {
            OgreUtils::destroySceneNode(mSelectionBox->getParentSceneNode());
            delete mSelectionBox;
            mSelectionBox = nullptr;
        }

        mLinkingLine.clear();
        mEngine->level()->levelRoot()->detachObject(&mLinkingLine);
    }

    void EditorBrush::popMode()
    {
        if(mModeStack.size() == 0)
            return;

        BrushMode mode = mModeStack.back();
        mModeStack.pop_back();
        setMode(mode);
    }

    void EditorBrush::pushMode()
    {
        mModeStack.push_back(mMode);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////
    // ENUM SERIALIZATION

    Ogre::String toString(EditorBrush::BrushMode e)
    {
        STEEL_ENUM_TO_STRING_START(EditorBrush::BrushMode)
        {
            STEEL_ENUM_TO_STRING_CASE(EditorBrush::BrushMode::TRANSLATE);
            STEEL_ENUM_TO_STRING_CASE(EditorBrush::BrushMode::ROTATE);
            STEEL_ENUM_TO_STRING_CASE(EditorBrush::BrushMode::SCALE);
            STEEL_ENUM_TO_STRING_CASE(EditorBrush::BrushMode::TERRAFORM);
            STEEL_ENUM_TO_STRING_CASE(EditorBrush::BrushMode::LINK);
            STEEL_ENUM_TO_STRING_CASE(EditorBrush::BrushMode::NONE);
        }
        return StringUtils::BLANK;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
