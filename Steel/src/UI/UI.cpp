
#include <Ogre.h>

#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>
#include <Ogre.h>

#include <MyGUI_OgrePlatform.h>
#include <MyGUI.h>

#include "UI/UI.h"
#include "UI/RenderInterfaceOgre3D.h"
#include "Debug.h"
#include "InputSystem/InputManager.h"
#include "tools/OgreUtils.h"
#include "tools/StringUtils.h"
#include "Level.h"
#include "Engine.h"

#include "UI/RenderInterfaceOgre3D.h"

namespace Steel
{
    UI::UI(): Rocket::Core::SystemInterface(), Ogre::RenderQueueListener(), EngineEventListener(), InputEventListener(),
        mInputMan(nullptr), mWindow(nullptr), mEngine(nullptr), 
        mTimer(),
        mRocketRenderInterface(nullptr), mMainContext(nullptr), mRocketKeyIdentifiers(), mRocketMouseIdentifiers(),
        mMyGUIData(),
        mEditor(*this), mHUD(*this), mUIDataDir(), mEditMode(false)
    {
        buildCodeMaps();
    }

    UI::UI(const UI &o): Rocket::Core::SystemInterface(o), Ogre::RenderQueueListener(o), EngineEventListener(o), InputEventListener(o),
        mInputMan(o.mInputMan), mWindow(o.mWindow), mEngine(o.mEngine),
        mTimer(o.mTimer),
        mRocketRenderInterface(o.mRocketRenderInterface), mMainContext(o.mMainContext),mRocketKeyIdentifiers(o.mRocketKeyIdentifiers), mRocketMouseIdentifiers(o.mRocketMouseIdentifiers),
        mMyGUIData(o.mMyGUIData),
        mEditor(o.mEditor), mHUD(o.mHUD), mUIDataDir(o.mUIDataDir), mEditMode(o.mEditMode)
    {}

    UI::~UI()
    {
        shutdown();
    }

    UI &UI::operator=(const UI &o)
    {
        Debug::error("not implemented").endl().breakHere();
        if(this != &o)
        {
            if(mMainContext != nullptr && mMainContext != o.mMainContext)
            {
                mMainContext->RemoveReference();
                mMainContext = o.mMainContext;
                mMainContext->AddReference();
            }

            mMyGUIData = o.mMyGUIData;

            mEditor = o.mEditor;
            mHUD = o.mHUD;
            mRocketKeyIdentifiers = o.mRocketKeyIdentifiers;
            mEditMode = o.mEditMode;
        }

        return *this;
    }

    void UI::loadConfig(ConfigFile const &config)
    {
        mEditor.loadConfig(config);
        mHUD.loadConfig(config);
    }

    void UI::saveConfig(ConfigFile &config) const
    {
        mEditor.saveConfig(config);
        mHUD.saveConfig(config);
    }

    bool UI::processCommand(std::vector<Ogre::String> command)
    {
        while(command.size() > 0 && command[0] == "ui")
            command.erase(command.begin());
        
        if(0 == command.size())
            return false;
        
        if(command[0] == "reload")
        {
            reload();
        }
        else
        {
            Debug::warning("UI::processCommand(): unknown command ");
            Debug::warning(StringUtils::join(command, ".")).endl();
            return false;
        }

        return true;
    }

    void UI::reload()
    {
        Debug::log("UI::reload()").endl();

        mHUD.reloadContent();
        mEditor.reloadContent();
    }

    float UI::GetElapsedTime()
    {
        return mTimer.getMilliseconds() * 0.001f;
    }

    bool UI::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String &message)
    {
        Ogre::String intro = "[Rocket] ";

        switch(type)
        {
            case Rocket::Core::Log::LT_ERROR:
            case Rocket::Core::Log::LT_ASSERT:
                Debug::error(intro)(message).endl();
                break;

            case Rocket::Core::Log::LT_WARNING:
                Debug::warning(intro)(message).endl();
                break;

            case Rocket::Core::Log::LT_ALWAYS:
            default:
                Debug::log(intro)(message).endl();
                break;
        }

        return false;
    }

    void UI::shutdown()
    {
        stopEditMode();
        mHUD.shutdown();
        mEditor.shutdown();


        // libRocket shutdown
        {
            if(mMainContext != nullptr)
            {
                mMainContext->UnloadAllMouseCursors();
                mMainContext->UnloadAllDocuments();
                mMainContext->RemoveReference();
                mMainContext = nullptr;
            }

            Rocket::Core::Shutdown();

            if(nullptr != mRocketRenderInterface)
            {
                delete mRocketRenderInterface;
                mRocketRenderInterface = nullptr;
            }
        }

        // MyGui shutdown
        {
            if(nullptr != mMyGUIData.gui)
            {
                mMyGUIData.gui->destroyAllChildWidget();
                mMyGUIData.gui->shutdown();
                delete mMyGUIData.gui;
                mMyGUIData.gui = nullptr;
            }

            if(nullptr != mMyGUIData.platform)
            {
                mMyGUIData.platform->shutdown();
                delete mMyGUIData.platform;
                mMyGUIData.platform = nullptr;
            }
        }

        if(mInputMan != nullptr)
            mInputMan = nullptr;

        mEngine->removeEngineEventListener(this);
    }

    void UI::init(File UIDataDir,
                  InputManager *inputMan,
                  Ogre::RenderWindow *window,
                  Engine *engine)
    {
        Debug::log("UI::init()").endl();
        mUIDataDir = UIDataDir.subfile("current");
        mInputMan = inputMan;
        mWindow = window;
        mEngine = engine;

        mEditMode = false;
        mEngine->addEngineEventListener(this);
        mInputMan->addInputEventListener(this);

        auto orm = Ogre::ResourceGroupManager::getSingletonPtr();
        orm->addResourceLocation(mUIDataDir.fullPath(), "FileSystem", resourceGroup(), true);
        orm->initialiseResourceGroup(resourceGroup());
        orm->loadResourceGroup(resourceGroup());

        //rocket init
        {

            mRocketRenderInterface = new Rocket::RenderInterfaceOgre3D(mWindow->getWidth(), mWindow->getHeight(), mEngine);
            Rocket::Core::SetRenderInterface(mRocketRenderInterface);


            Rocket::Core::SetSystemInterface(this);
            Rocket::Core::Initialise();

            Rocket::Controls::Initialise();
            Rocket::Core::FontDatabase::LoadFontFace(mUIDataDir.subfile("fonts").subfile("tahoma.ttf").fullPath().c_str());

            mMainContext = Rocket::Core::CreateContext("UI-main", Rocket::Core::Vector2i(mWindow->getWidth(), mWindow->getHeight()));
//         Rocket::Core::ElementDocument* cursor = mMainContext->LoadMouseCursor(mUIDataDir.subfile("current/cursor.rml").fullPath().c_str());

//             bool firstInit = mRocketRenderInterface == nullptr;
//             if(firstInit)
//                 Rocket::Debugger::Initialise(mMainContext);
//             else
//                 Rocket::Debugger::SetContext(mMainContext);
        }
        
        // MyGui init
        {
            
            auto rgm = Ogre::ResourceGroupManager::getSingletonPtr();
            rgm->addResourceLocation(mUIDataDir/"MyGUI_Media", "FileSystem", resourceGroup(), true);
            rgm->initialiseResourceGroup(resourceGroup());
            
            mMyGUIData.platform = new MyGUI::OgrePlatform();
            mMyGUIData.platform->initialise(mWindow, engine->level()->sceneManager(), resourceGroup(), "steel_gui.log");

            mMyGUIData.gui = new MyGUI::Gui();
            mMyGUIData.gui->initialise("MyGUI_Core.xml");
        }

        //UI init
        mEditor.init(mWindow->getWidth(), mWindow->getHeight(), mEngine);
        mHUD.init(mWindow->getWidth(), mWindow->getHeight(), mEngine);

        OgreUtils::resourceGroupsInfos();
    }

    void UI::onResize(int width, int height)
    {
    }

    void UI::startEditMode()
    {
        if(!mEditMode)
        {
            mEditMode = true;
            mEditor.show();

//             if(!Rocket::Debugger::IsVisible())
//                 Rocket::Debugger::SetVisible(true);
        }
    }

    void UI::stopEditMode()
    {
        if(mEditMode)
        {
            mEditMode = false;
            mEditor.hide();

//             if(Rocket::Debugger::IsVisible())
//                 Rocket::Debugger::SetVisible(false);
        }
    }

    // Called from Ogre before a queue group is rendered.
    void UI::renderQueueStarted(Ogre::uint8 queueGroupId,
                                const Ogre::String &/*invocation*/,
                                bool &skipThisInvocation)
    {
        if(skipThisInvocation)
            return;

        if(Ogre::RENDER_QUEUE_OVERLAY == queueGroupId && Ogre::Root::getSingleton().getRenderSystem()->_getViewport()->getOverlaysEnabled())
        {
            mHUD.context()->Update();
            mMainContext->Update();

            if(mEditMode)
                mEditor.context()->Update();

            configureRenderSystem();

            mHUD.context()->Render();
            mMainContext->Render();

            if(mEditMode)
                mEditor.context()->Render();
        }
    }

    // Called from Ogre after a queue group is rendered.
//     void UI::renderQueueEnded(Ogre::uint8 /*queueGroupId*/,
//                               const Ogre::String &/*invocation*/,
//                               bool &/*repeatThisInvocation*/)
//     {
//     }

    // Configures Ogre's rendering system for rendering RocketUI.
    void UI::configureRenderSystem()
    {
        Ogre::RenderSystem *render_system = Ogre::Root::getSingleton().getRenderSystem();
        // Set up the projection and view matrices.
        Ogre::Matrix4 projection_matrix;
        buildProjectionMatrix(projection_matrix);
        render_system->_setProjectionMatrix(projection_matrix);
        render_system->_setViewMatrix(Ogre::Matrix4::IDENTITY);

        // Disable lighting, as all of Rocket's geometry is unlit.
        render_system->setLightingEnabled(false);
        // Disable depth-buffering; all of the geometry is already depth-sorted.
        render_system->_setDepthBufferParams(false, false);
        // Rocket generates anti-clockwise geometry, so enable clockwise-culling.
        render_system->_setCullingMode(Ogre::CULL_CLOCKWISE);
        // Disable fogging.
        render_system->_setFog(Ogre::FOG_NONE);
        // Enable writing to all four channels.
        render_system->_setColourBufferWriteEnabled(true, true, true, true);
        // Unbind any vertex or fragment programs bound previously by the application.
        render_system->unbindGpuProgram(Ogre::GPT_FRAGMENT_PROGRAM);
        render_system->unbindGpuProgram(Ogre::GPT_VERTEX_PROGRAM);

        // Set texture settings to clamp along both axes.
        Ogre::TextureUnitState::UVWAddressingMode addressing_mode;
        addressing_mode.u = Ogre::TextureUnitState::TAM_CLAMP;
        addressing_mode.v = Ogre::TextureUnitState::TAM_CLAMP;
        addressing_mode.w = Ogre::TextureUnitState::TAM_CLAMP;
        render_system->_setTextureAddressingMode(0, addressing_mode);

        // Set the texture coordinates for unit 0 to be read from unit 0.
        render_system->_setTextureCoordSet(0, 0);
        // Disable texture coordinate calculation.
        render_system->_setTextureCoordCalculation(0, Ogre::TEXCALC_NONE);
        // Enable linear filtering; images should be rendering 1 texel == 1 pixel, so point filtering could be used
        // except in the case of scaling tiled decorators.
        render_system->_setTextureUnitFiltering(0, Ogre::FO_LINEAR, Ogre::FO_LINEAR, Ogre::FO_POINT);
        // Disable texture coordinate transforms.
        render_system->_setTextureMatrix(0, Ogre::Matrix4::IDENTITY);
        // Reject pixels with an alpha of 0.
        render_system->_setAlphaRejectSettings(Ogre::CMPF_GREATER, 0, false);
        // Disable all texture units but the first.
        render_system->_disableTextureUnitsFrom(1);

        // Enable simple alpha blending.
        render_system->_setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);

        // Disable depth bias.
        render_system->_setDepthBias(0, 0);
    }

    // Builds an OpenGL-style orthographic projection matrix.
    void UI::buildProjectionMatrix(Ogre::Matrix4 &projection_matrix)
    {
        float z_near = -1;
        float z_far = 1;

        projection_matrix = Ogre::Matrix4::ZERO;

        // Set up matrices.
        projection_matrix[0][0] = 2.0f / mWindow->getWidth();
        projection_matrix[0][3] = -1.0000000f;
        projection_matrix[1][1] = -2.0f / mWindow->getHeight();
        projection_matrix[1][3] = 1.0000000f;
        projection_matrix[2][2] = -2.0f / (z_far - z_near);
        projection_matrix[3][3] = 1.0000000f;
    }

    bool UI::keyPressed(Input::Code key, Input::Event const &evt)
    {
        MyGUI::InputManager::getInstance().injectKeyPress(getMyGUIKeyIdentifier(evt.code), (MyGUI::Char)evt.text);
        
        Rocket::Core::Input::KeyIdentifier keyIdentifier = getRocketKeyIdentifier(key);

        if(evt.text >= 32)
            mMainContext->ProcessTextInput((Rocket::Core::word) evt.text);
        else if(keyIdentifier == Rocket::Core::Input::KI_RETURN)
            mMainContext->ProcessTextInput((Rocket::Core::word) '\n');
        else
            mMainContext->ProcessKeyDown(keyIdentifier , getRocketKeyModifierState());

        if(mEditMode)
            mEditor.keyPressed(key, evt);

        return true;
    }

    bool UI::keyReleased(Input::Code key, Input::Event const &evt)
    {
        MyGUI::InputManager::getInstance().injectKeyRelease(getMyGUIKeyIdentifier(evt.code));
        
        mMainContext->ProcessKeyUp(getRocketKeyIdentifier(key), getRocketKeyModifierState());

        if(mEditMode)
            mEditor.keyReleased(key, evt);

        return true;
    }

    bool UI::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        auto const mouseState = mInputMan->mouse()->getMouseState();
        MyGUI::InputManager::getInstance().injectMouseMove(mouseState.X.abs, mouseState.Y.abs, mouseState.Z.abs);
        
        mMainContext->ProcessMouseMove(evt.position.x, evt.position.y, getRocketKeyModifierState());

        if(mEditMode)
            mEditor.mouseMoved(position, evt);

        return true;
    }

    Rocket::Core::Input::KeyIdentifier UI::getRocketKeyIdentifier(Input::Code key) const
    {
        auto it = mRocketKeyIdentifiers.find(key);

        if(mRocketKeyIdentifiers.cend() == it)
            return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;

        return it->second;
    }
    
    MyGUI::KeyCode UI::getMyGUIKeyIdentifier(Input::Code key) const
    {
        auto it = mMyGUIData.keyMap.find(key);
        
        if(mMyGUIData.keyMap.cend() == it)
            return MyGUI::KeyCode::None;
        
        return it->second;
    }

    int UI::getRocketMouseIdentifier(Input::Code button) const
    {
        auto it = mRocketMouseIdentifiers.find(button);

        if(mRocketMouseIdentifiers.cend() == it)
            return 0; // left btn

        return it->second;
    }

    bool UI::mousePressed(Input::Code button, Input::Event const &evt)
    {
        auto const mouseState = mInputMan->mouse()->getMouseState();
        MyGUI::InputManager::getInstance().injectMousePress(mouseState.X.abs, mouseState.Y.abs, MyGUI::MouseButton::Enum(getRocketMouseIdentifier(button)));
        
        mMainContext->ProcessMouseButtonDown(getRocketMouseIdentifier(button), getRocketKeyModifierState());

        if(mEditMode)
            mEditor.mousePressed(button, evt);

        return true;
    }

    bool UI::mouseReleased(Input::Code button, Input::Event const &evt)
    {
        auto const mouseState = mInputMan->mouse()->getMouseState();
        MyGUI::InputManager::getInstance().injectMouseRelease(mouseState.X.abs, mouseState.Y.abs, MyGUI::MouseButton::Enum(getRocketMouseIdentifier(button)));
        
        mMainContext->ProcessMouseButtonUp(getRocketMouseIdentifier(button), getRocketKeyModifierState());

        if(mEditMode)
            mEditor.mouseReleased(button, evt);

        return true;
    }

    bool UI::mouseWheeled(int delta, Input::Event const &evt)
    {
        mMainContext->ProcessMouseWheel(delta / -120, getRocketKeyModifierState());

        if(mEditMode)
            mEditor.mouseWheeled(delta, evt);

        return true;
    }

    void UI::onLevelUnset(Level *level)
    {
        // stop rendering as long as there is no level->scenemanager to render from
        level->sceneManager()->removeRenderQueueListener(this);
    }

    void UI::onLevelSet(Level *level)
    {
        level->viewport()->setOverlaysEnabled(true);
        level->sceneManager()->addRenderQueueListener(this);
    }

    void UI::buildCodeMaps()
    {
        mRocketKeyIdentifiers[Input::Code::UNASSIGNED] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_ESCAPE] = Rocket::Core::Input::KI_ESCAPE;
        mRocketKeyIdentifiers[Input::Code::KC_1] = Rocket::Core::Input::KI_1;
        mRocketKeyIdentifiers[Input::Code::KC_2] = Rocket::Core::Input::KI_2;
        mRocketKeyIdentifiers[Input::Code::KC_3] = Rocket::Core::Input::KI_3;
        mRocketKeyIdentifiers[Input::Code::KC_4] = Rocket::Core::Input::KI_4;
        mRocketKeyIdentifiers[Input::Code::KC_5] = Rocket::Core::Input::KI_5;
        mRocketKeyIdentifiers[Input::Code::KC_6] = Rocket::Core::Input::KI_6;
        mRocketKeyIdentifiers[Input::Code::KC_7] = Rocket::Core::Input::KI_7;
        mRocketKeyIdentifiers[Input::Code::KC_8] = Rocket::Core::Input::KI_8;
        mRocketKeyIdentifiers[Input::Code::KC_9] = Rocket::Core::Input::KI_9;
        mRocketKeyIdentifiers[Input::Code::KC_0] = Rocket::Core::Input::KI_0;
        mRocketKeyIdentifiers[Input::Code::KC_MINUS] = Rocket::Core::Input::KI_OEM_MINUS;
        mRocketKeyIdentifiers[Input::Code::KC_EQUALS] = Rocket::Core::Input::KI_OEM_PLUS;
        mRocketKeyIdentifiers[Input::Code::KC_BACK] = Rocket::Core::Input::KI_BACK;
        mRocketKeyIdentifiers[Input::Code::KC_TAB] = Rocket::Core::Input::KI_TAB;
        mRocketKeyIdentifiers[Input::Code::KC_Q] = Rocket::Core::Input::KI_Q;
        mRocketKeyIdentifiers[Input::Code::KC_W] = Rocket::Core::Input::KI_W;
        mRocketKeyIdentifiers[Input::Code::KC_E] = Rocket::Core::Input::KI_E;
        mRocketKeyIdentifiers[Input::Code::KC_R] = Rocket::Core::Input::KI_R;
        mRocketKeyIdentifiers[Input::Code::KC_T] = Rocket::Core::Input::KI_T;
        mRocketKeyIdentifiers[Input::Code::KC_Y] = Rocket::Core::Input::KI_Y;
        mRocketKeyIdentifiers[Input::Code::KC_U] = Rocket::Core::Input::KI_U;
        mRocketKeyIdentifiers[Input::Code::KC_I] = Rocket::Core::Input::KI_I;
        mRocketKeyIdentifiers[Input::Code::KC_O] = Rocket::Core::Input::KI_O;
        mRocketKeyIdentifiers[Input::Code::KC_P] = Rocket::Core::Input::KI_P;
        mRocketKeyIdentifiers[Input::Code::KC_LBRACKET] = Rocket::Core::Input::KI_OEM_4;
        mRocketKeyIdentifiers[Input::Code::KC_RBRACKET] = Rocket::Core::Input::KI_OEM_6;
        mRocketKeyIdentifiers[Input::Code::KC_RETURN] = Rocket::Core::Input::KI_RETURN;
        mRocketKeyIdentifiers[Input::Code::KC_LCONTROL] = Rocket::Core::Input::KI_LCONTROL;
        mRocketKeyIdentifiers[Input::Code::KC_A] = Rocket::Core::Input::KI_A;
        mRocketKeyIdentifiers[Input::Code::KC_S] = Rocket::Core::Input::KI_S;
        mRocketKeyIdentifiers[Input::Code::KC_D] = Rocket::Core::Input::KI_D;
        mRocketKeyIdentifiers[Input::Code::KC_F] = Rocket::Core::Input::KI_F;
        mRocketKeyIdentifiers[Input::Code::KC_G] = Rocket::Core::Input::KI_G;
        mRocketKeyIdentifiers[Input::Code::KC_H] = Rocket::Core::Input::KI_H;
        mRocketKeyIdentifiers[Input::Code::KC_J] = Rocket::Core::Input::KI_J;
        mRocketKeyIdentifiers[Input::Code::KC_K] = Rocket::Core::Input::KI_K;
        mRocketKeyIdentifiers[Input::Code::KC_L] = Rocket::Core::Input::KI_L;
        mRocketKeyIdentifiers[Input::Code::KC_SEMICOLON] = Rocket::Core::Input::KI_OEM_1;
        mRocketKeyIdentifiers[Input::Code::KC_APOSTROPHE] = Rocket::Core::Input::KI_OEM_7;
        mRocketKeyIdentifiers[Input::Code::KC_GRAVE] = Rocket::Core::Input::KI_OEM_3;
        mRocketKeyIdentifiers[Input::Code::KC_LSHIFT] = Rocket::Core::Input::KI_LSHIFT;
        mRocketKeyIdentifiers[Input::Code::KC_BACKSLASH] = Rocket::Core::Input::KI_OEM_5;
        mRocketKeyIdentifiers[Input::Code::KC_Z] = Rocket::Core::Input::KI_Z;
        mRocketKeyIdentifiers[Input::Code::KC_X] = Rocket::Core::Input::KI_X;
        mRocketKeyIdentifiers[Input::Code::KC_C] = Rocket::Core::Input::KI_C;
        mRocketKeyIdentifiers[Input::Code::KC_V] = Rocket::Core::Input::KI_V;
        mRocketKeyIdentifiers[Input::Code::KC_B] = Rocket::Core::Input::KI_B;
        mRocketKeyIdentifiers[Input::Code::KC_N] = Rocket::Core::Input::KI_N;
        mRocketKeyIdentifiers[Input::Code::KC_M] = Rocket::Core::Input::KI_M;
        mRocketKeyIdentifiers[Input::Code::KC_COMMA] = Rocket::Core::Input::KI_OEM_COMMA;
        mRocketKeyIdentifiers[Input::Code::KC_PERIOD] = Rocket::Core::Input::KI_OEM_PERIOD;
        mRocketKeyIdentifiers[Input::Code::KC_SLASH] = Rocket::Core::Input::KI_OEM_2;
        mRocketKeyIdentifiers[Input::Code::KC_RSHIFT] = Rocket::Core::Input::KI_RSHIFT;
        mRocketKeyIdentifiers[Input::Code::KC_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
        mRocketKeyIdentifiers[Input::Code::KC_LMENU] = Rocket::Core::Input::KI_LMENU;
        mRocketKeyIdentifiers[Input::Code::KC_SPACE] = Rocket::Core::Input::KI_SPACE;
        mRocketKeyIdentifiers[Input::Code::KC_CAPITAL] = Rocket::Core::Input::KI_CAPITAL;
        mRocketKeyIdentifiers[Input::Code::KC_F1] = Rocket::Core::Input::KI_F1;
        mRocketKeyIdentifiers[Input::Code::KC_F2] = Rocket::Core::Input::KI_F2;
        mRocketKeyIdentifiers[Input::Code::KC_F3] = Rocket::Core::Input::KI_F3;
        mRocketKeyIdentifiers[Input::Code::KC_F4] = Rocket::Core::Input::KI_F4;
        mRocketKeyIdentifiers[Input::Code::KC_F5] = Rocket::Core::Input::KI_F5;
        mRocketKeyIdentifiers[Input::Code::KC_F6] = Rocket::Core::Input::KI_F6;
        mRocketKeyIdentifiers[Input::Code::KC_F7] = Rocket::Core::Input::KI_F7;
        mRocketKeyIdentifiers[Input::Code::KC_F8] = Rocket::Core::Input::KI_F8;
        mRocketKeyIdentifiers[Input::Code::KC_F9] = Rocket::Core::Input::KI_F9;
        mRocketKeyIdentifiers[Input::Code::KC_F10] = Rocket::Core::Input::KI_F10;
        mRocketKeyIdentifiers[Input::Code::KC_NUMLOCK] = Rocket::Core::Input::KI_NUMLOCK;
        mRocketKeyIdentifiers[Input::Code::KC_SCROLL] = Rocket::Core::Input::KI_SCROLL;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD7] = Rocket::Core::Input::KI_7;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD8] = Rocket::Core::Input::KI_8;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD9] = Rocket::Core::Input::KI_9;
        mRocketKeyIdentifiers[Input::Code::KC_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD4] = Rocket::Core::Input::KI_4;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD5] = Rocket::Core::Input::KI_5;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD6] = Rocket::Core::Input::KI_6;
        mRocketKeyIdentifiers[Input::Code::KC_ADD] = Rocket::Core::Input::KI_ADD;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD1] = Rocket::Core::Input::KI_1;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD2] = Rocket::Core::Input::KI_2;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD3] = Rocket::Core::Input::KI_3;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPAD0] = Rocket::Core::Input::KI_0;
        mRocketKeyIdentifiers[Input::Code::KC_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
        mRocketKeyIdentifiers[Input::Code::KC_OEM_102] = Rocket::Core::Input::KI_OEM_102;
        mRocketKeyIdentifiers[Input::Code::KC_F11] = Rocket::Core::Input::KI_F11;
        mRocketKeyIdentifiers[Input::Code::KC_F12] = Rocket::Core::Input::KI_F12;
        mRocketKeyIdentifiers[Input::Code::KC_F13] = Rocket::Core::Input::KI_F13;
        mRocketKeyIdentifiers[Input::Code::KC_F14] = Rocket::Core::Input::KI_F14;
        mRocketKeyIdentifiers[Input::Code::KC_F15] = Rocket::Core::Input::KI_F15;
        mRocketKeyIdentifiers[Input::Code::KC_KANA] = Rocket::Core::Input::KI_KANA;
        mRocketKeyIdentifiers[Input::Code::KC_ABNT_C1] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_CONVERT] = Rocket::Core::Input::KI_CONVERT;
        mRocketKeyIdentifiers[Input::Code::KC_NOCONVERT] = Rocket::Core::Input::KI_NONCONVERT;
        mRocketKeyIdentifiers[Input::Code::KC_YEN] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_ABNT_C2] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPADEQUALS] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;
        mRocketKeyIdentifiers[Input::Code::KC_PREVTRACK] = Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
        mRocketKeyIdentifiers[Input::Code::KC_AT] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_COLON] = Rocket::Core::Input::KI_OEM_1;
        mRocketKeyIdentifiers[Input::Code::KC_UNDERLINE] = Rocket::Core::Input::KI_OEM_MINUS;
        mRocketKeyIdentifiers[Input::Code::KC_KANJI] = Rocket::Core::Input::KI_KANJI;
        mRocketKeyIdentifiers[Input::Code::KC_STOP] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_AX] = Rocket::Core::Input::KI_OEM_AX;
        mRocketKeyIdentifiers[Input::Code::KC_UNLABELED] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_NEXTTRACK] = Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPADENTER] = Rocket::Core::Input::KI_NUMPADENTER;
        mRocketKeyIdentifiers[Input::Code::KC_RCONTROL] = Rocket::Core::Input::KI_RCONTROL;
        mRocketKeyIdentifiers[Input::Code::KC_MUTE] = Rocket::Core::Input::KI_VOLUME_MUTE;
        mRocketKeyIdentifiers[Input::Code::KC_CALCULATOR] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_PLAYPAUSE] = Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
        mRocketKeyIdentifiers[Input::Code::KC_MEDIASTOP] = Rocket::Core::Input::KI_MEDIA_STOP;
        mRocketKeyIdentifiers[Input::Code::KC_VOLUMEDOWN] = Rocket::Core::Input::KI_VOLUME_DOWN;
        mRocketKeyIdentifiers[Input::Code::KC_VOLUMEUP] = Rocket::Core::Input::KI_VOLUME_UP;
        mRocketKeyIdentifiers[Input::Code::KC_WEBHOME] = Rocket::Core::Input::KI_BROWSER_HOME;
        mRocketKeyIdentifiers[Input::Code::KC_NUMPADCOMMA] = Rocket::Core::Input::KI_SEPARATOR;
        mRocketKeyIdentifiers[Input::Code::KC_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
        mRocketKeyIdentifiers[Input::Code::KC_SCREENSHOT] = Rocket::Core::Input::KI_SNAPSHOT;
        mRocketKeyIdentifiers[Input::Code::KC_RMENU] = Rocket::Core::Input::KI_RMENU;
        mRocketKeyIdentifiers[Input::Code::KC_PAUSE] = Rocket::Core::Input::KI_PAUSE;
        mRocketKeyIdentifiers[Input::Code::KC_HOME] = Rocket::Core::Input::KI_HOME;
        mRocketKeyIdentifiers[Input::Code::KC_UP] = Rocket::Core::Input::KI_UP;
        mRocketKeyIdentifiers[Input::Code::KC_PGUP] = Rocket::Core::Input::KI_PRIOR;
        mRocketKeyIdentifiers[Input::Code::KC_LEFT] = Rocket::Core::Input::KI_LEFT;
        mRocketKeyIdentifiers[Input::Code::KC_RIGHT] = Rocket::Core::Input::KI_RIGHT;
        mRocketKeyIdentifiers[Input::Code::KC_END] = Rocket::Core::Input::KI_END;
        mRocketKeyIdentifiers[Input::Code::KC_DOWN] = Rocket::Core::Input::KI_DOWN;
        mRocketKeyIdentifiers[Input::Code::KC_PGDOWN] = Rocket::Core::Input::KI_NEXT;
        mRocketKeyIdentifiers[Input::Code::KC_INSERT] = Rocket::Core::Input::KI_INSERT;
        mRocketKeyIdentifiers[Input::Code::KC_DELETE] = Rocket::Core::Input::KI_DELETE;
        mRocketKeyIdentifiers[Input::Code::KC_LWIN] = Rocket::Core::Input::KI_LWIN;
        mRocketKeyIdentifiers[Input::Code::KC_RWIN] = Rocket::Core::Input::KI_RWIN;
        mRocketKeyIdentifiers[Input::Code::KC_APPS] = Rocket::Core::Input::KI_APPS;
        mRocketKeyIdentifiers[Input::Code::KC_POWER] = Rocket::Core::Input::KI_POWER;
        mRocketKeyIdentifiers[Input::Code::KC_SLEEP] = Rocket::Core::Input::KI_SLEEP;
        mRocketKeyIdentifiers[Input::Code::KC_WAKE] = Rocket::Core::Input::KI_WAKE;
        mRocketKeyIdentifiers[Input::Code::KC_WEBSEARCH] = Rocket::Core::Input::KI_BROWSER_SEARCH;
        mRocketKeyIdentifiers[Input::Code::KC_WEBFAVORITES] = Rocket::Core::Input::KI_BROWSER_FAVORITES;
        mRocketKeyIdentifiers[Input::Code::KC_WEBREFRESH] = Rocket::Core::Input::KI_BROWSER_REFRESH;
        mRocketKeyIdentifiers[Input::Code::KC_WEBSTOP] = Rocket::Core::Input::KI_BROWSER_STOP;
        mRocketKeyIdentifiers[Input::Code::KC_WEBFORWARD] = Rocket::Core::Input::KI_BROWSER_FORWARD;
        mRocketKeyIdentifiers[Input::Code::KC_WEBBACK] = Rocket::Core::Input::KI_BROWSER_BACK;
        mRocketKeyIdentifiers[Input::Code::KC_MYCOMPUTER] = Rocket::Core::Input::KI_UNKNOWN;
        mRocketKeyIdentifiers[Input::Code::KC_MAIL] = Rocket::Core::Input::KI_LAUNCH_MAIL;
        mRocketKeyIdentifiers[Input::Code::KC_MEDIASELECT] = Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;

        // mouseMoved
        //MB_Left = 0, MB_Right, MB_Middle,
        //MB_Button3, MB_Button4, MB_Button5, MB_Button6, MB_Button7
        mRocketMouseIdentifiers[Input::Code::MC_LEFT] = 0;
        mRocketMouseIdentifiers[Input::Code::MC_RIGHT] = 1;
        mRocketMouseIdentifiers[Input::Code::MC_MIDDLE] = 2;
        mRocketMouseIdentifiers[Input::Code::MC_BUTTON3] = 3;
        mRocketMouseIdentifiers[Input::Code::MC_BUTTON4] = 4;
        mRocketMouseIdentifiers[Input::Code::MC_BUTTON5] = 5;
        mRocketMouseIdentifiers[Input::Code::MC_BUTTON6] = 6;
        mRocketMouseIdentifiers[Input::Code::MC_BUTTON7] = 7;
        
        // MyGUI stuff
#define MAP_CODE(STEEL_CODE, MYGUI_CODE) mMyGUIData.keyMap[STEEL_CODE] = MYGUI_CODE
        MAP_CODE(Input::Code::KC_UNKONWN, MyGUI::KeyCode::None);
        MAP_CODE(Input::Code::KC_ESCAPE, MyGUI::KeyCode::Escape);
        MAP_CODE(Input::Code::KC_1, MyGUI::KeyCode::One);
        MAP_CODE(Input::Code::KC_2, MyGUI::KeyCode::Two);
        MAP_CODE(Input::Code::KC_3, MyGUI::KeyCode::Three);
        MAP_CODE(Input::Code::KC_4, MyGUI::KeyCode::Four);
        MAP_CODE(Input::Code::KC_5, MyGUI::KeyCode::Five);
        MAP_CODE(Input::Code::KC_6, MyGUI::KeyCode::Six);
        MAP_CODE(Input::Code::KC_7, MyGUI::KeyCode::Seven);
        MAP_CODE(Input::Code::KC_8, MyGUI::KeyCode::Eight);
        MAP_CODE(Input::Code::KC_9, MyGUI::KeyCode::Nine);
        MAP_CODE(Input::Code::KC_0, MyGUI::KeyCode::Zero);
        MAP_CODE(Input::Code::KC_MINUS, MyGUI::KeyCode::Minus);/* - on main keyboard */
        MAP_CODE(Input::Code::KC_EQUALS, MyGUI::KeyCode::Equals);
        MAP_CODE(Input::Code::KC_BACKSPACE, MyGUI::KeyCode::Backspace);
        MAP_CODE(Input::Code::KC_TAB, MyGUI::KeyCode::Tab);
        MAP_CODE(Input::Code::KC_A, MyGUI::KeyCode::A);
        MAP_CODE(Input::Code::KC_B, MyGUI::KeyCode::B);
        MAP_CODE(Input::Code::KC_C, MyGUI::KeyCode::C);
        MAP_CODE(Input::Code::KC_D, MyGUI::KeyCode::D);
        MAP_CODE(Input::Code::KC_E, MyGUI::KeyCode::E);
        MAP_CODE(Input::Code::KC_F, MyGUI::KeyCode::F);
        MAP_CODE(Input::Code::KC_G, MyGUI::KeyCode::G);
        MAP_CODE(Input::Code::KC_H, MyGUI::KeyCode::H);
        MAP_CODE(Input::Code::KC_I, MyGUI::KeyCode::I);
        MAP_CODE(Input::Code::KC_J, MyGUI::KeyCode::J);
        MAP_CODE(Input::Code::KC_K, MyGUI::KeyCode::K);
        MAP_CODE(Input::Code::KC_L, MyGUI::KeyCode::L);
        MAP_CODE(Input::Code::KC_M, MyGUI::KeyCode::M);
        MAP_CODE(Input::Code::KC_N, MyGUI::KeyCode::N);
        MAP_CODE(Input::Code::KC_O, MyGUI::KeyCode::O);
        MAP_CODE(Input::Code::KC_P, MyGUI::KeyCode::P);
        MAP_CODE(Input::Code::KC_Q, MyGUI::KeyCode::Q);
        MAP_CODE(Input::Code::KC_R, MyGUI::KeyCode::R);
        MAP_CODE(Input::Code::KC_S, MyGUI::KeyCode::S);
        MAP_CODE(Input::Code::KC_T, MyGUI::KeyCode::T);
        MAP_CODE(Input::Code::KC_U, MyGUI::KeyCode::U);
        MAP_CODE(Input::Code::KC_V, MyGUI::KeyCode::V);
        MAP_CODE(Input::Code::KC_W, MyGUI::KeyCode::W);
        MAP_CODE(Input::Code::KC_X, MyGUI::KeyCode::X);
        MAP_CODE(Input::Code::KC_Y, MyGUI::KeyCode::Y);
        MAP_CODE(Input::Code::KC_Z, MyGUI::KeyCode::Z);
        MAP_CODE(Input::Code::KC_LBRACKET, MyGUI::KeyCode::LeftBracket);
        MAP_CODE(Input::Code::KC_RBRACKET, MyGUI::KeyCode::RightBracket);
        MAP_CODE(Input::Code::KC_RETURN, MyGUI::KeyCode::Return);
        MAP_CODE(Input::Code::KC_LCONTROL, MyGUI::KeyCode::LeftControl);
        MAP_CODE(Input::Code::KC_SEMICOLON, MyGUI::KeyCode::Semicolon);
        MAP_CODE(Input::Code::KC_APOSTROPHE, MyGUI::KeyCode::Apostrophe);
        MAP_CODE(Input::Code::KC_GRAVE, MyGUI::KeyCode::Grave);
        MAP_CODE(Input::Code::KC_LSHIFT, MyGUI::KeyCode::LeftShift);
        MAP_CODE(Input::Code::KC_BACKSLASH, MyGUI::KeyCode::Backslash);
        MAP_CODE(Input::Code::KC_COMMA, MyGUI::KeyCode::Comma);
        MAP_CODE(Input::Code::KC_PERIOD, MyGUI::KeyCode::Period);
        MAP_CODE(Input::Code::KC_SLASH, MyGUI::KeyCode::Slash);
        MAP_CODE(Input::Code::KC_RSHIFT, MyGUI::KeyCode::RightShift);
        MAP_CODE(Input::Code::KC_LALT, MyGUI::KeyCode::LeftAlt);
        MAP_CODE(Input::Code::KC_SPACE, MyGUI::KeyCode::Space);
        MAP_CODE(Input::Code::KC_CAPITAL, MyGUI::KeyCode::Capital);
        MAP_CODE(Input::Code::KC_F1, MyGUI::KeyCode::F1);
        MAP_CODE(Input::Code::KC_F2, MyGUI::KeyCode::F2);
        MAP_CODE(Input::Code::KC_F3, MyGUI::KeyCode::F3);
        MAP_CODE(Input::Code::KC_F4, MyGUI::KeyCode::F4);
        MAP_CODE(Input::Code::KC_F5, MyGUI::KeyCode::F5);
        MAP_CODE(Input::Code::KC_F6, MyGUI::KeyCode::F6);
        MAP_CODE(Input::Code::KC_F7, MyGUI::KeyCode::F7);
        MAP_CODE(Input::Code::KC_F8, MyGUI::KeyCode::F8);
        MAP_CODE(Input::Code::KC_F9, MyGUI::KeyCode::F9);
        MAP_CODE(Input::Code::KC_F10, MyGUI::KeyCode::F10);
        MAP_CODE(Input::Code::KC_F11, MyGUI::KeyCode::F11);
        MAP_CODE(Input::Code::KC_F12, MyGUI::KeyCode::F12);
        MAP_CODE(Input::Code::KC_F13, MyGUI::KeyCode::F13);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_F14, MyGUI::KeyCode::F14);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_F15, MyGUI::KeyCode::F15);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_NUMLOCK, MyGUI::KeyCode::NumLock);
        MAP_CODE(Input::Code::KC_SCROLL, MyGUI::KeyCode::ScrollLock);
        MAP_CODE(Input::Code::KC_NUMPAD1, MyGUI::KeyCode::Numpad1);
        MAP_CODE(Input::Code::KC_NUMPAD2, MyGUI::KeyCode::Numpad2);
        MAP_CODE(Input::Code::KC_NUMPAD3, MyGUI::KeyCode::Numpad3);
        MAP_CODE(Input::Code::KC_NUMPAD4, MyGUI::KeyCode::Numpad4);
        MAP_CODE(Input::Code::KC_NUMPAD5, MyGUI::KeyCode::Numpad5);
        MAP_CODE(Input::Code::KC_NUMPAD6, MyGUI::KeyCode::Numpad6);
        MAP_CODE(Input::Code::KC_NUMPAD7, MyGUI::KeyCode::Numpad7);
        MAP_CODE(Input::Code::KC_NUMPAD8, MyGUI::KeyCode::Numpad8);
        MAP_CODE(Input::Code::KC_NUMPAD9, MyGUI::KeyCode::Numpad9);
        MAP_CODE(Input::Code::KC_NUMPAD0, MyGUI::KeyCode::Numpad0);
        MAP_CODE(Input::Code::KC_NUMPADENTER, MyGUI::KeyCode::NumpadEnter);
        MAP_CODE(Input::Code::KC_NUMPADCOMMA, MyGUI::KeyCode::NumpadComma);/* , on numeric keypad (NEC PC98) */
        MAP_CODE(Input::Code::KC_NUMPADEQUALS, MyGUI::KeyCode::NumpadEquals);/* = on numeric keypad (NEC PC98) */
        MAP_CODE(Input::Code::KC_DECIMAL, MyGUI::KeyCode::Decimal);/* . on numeric keypad */
        MAP_CODE(Input::Code::KC_OEM_102, MyGUI::KeyCode::OEM_102);/* < > | on UK/Germany keyboards */
        MAP_CODE(Input::Code::KC_KANA, MyGUI::KeyCode::Kana);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_ABNT_C1, MyGUI::KeyCode::ABNT_C1);/* / ? on Portugese (Brazilian) keyboards */
        MAP_CODE(Input::Code::KC_ABNT_C2, MyGUI::KeyCode::ABNT_C2);/* Numpad . on Portugese (Brazilian) keyboards */
        MAP_CODE(Input::Code::KC_CONVERT, MyGUI::KeyCode::Convert);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_NOCONVERT, MyGUI::KeyCode::NoConvert);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_YEN, MyGUI::KeyCode::Yen);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_PREVTRACK, MyGUI::KeyCode::PrevTrack);/* Previous Track (KC_CIRCUMFLEX on Japanese keyboard) */
        MAP_CODE(Input::Code::KC_AT, MyGUI::KeyCode::At);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_COLON, MyGUI::KeyCode::Colon);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_UNDERLINE, MyGUI::KeyCode::Underline);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_KANJI, MyGUI::KeyCode::Kanji);/* (Japanese keyboard)            */
        MAP_CODE(Input::Code::KC_STOP, MyGUI::KeyCode::Stop);/*                     (NEC PC98) */
        MAP_CODE(Input::Code::KC_AX, MyGUI::KeyCode::AX);/*                     (Japan AX) */
        MAP_CODE(Input::Code::KC_UNLABELED, MyGUI::KeyCode::Unlabeled);/*                        (J3100) */
        MAP_CODE(Input::Code::KC_NEXTTRACK, MyGUI::KeyCode::NextTrack);
        MAP_CODE(Input::Code::KC_RCONTROL, MyGUI::KeyCode::RightControl);
        MAP_CODE(Input::Code::KC_MUTE, MyGUI::KeyCode::Mute);
        MAP_CODE(Input::Code::KC_CALCULATOR, MyGUI::KeyCode::Calculator);
        MAP_CODE(Input::Code::KC_PLAYPAUSE, MyGUI::KeyCode::PlayPause);
        MAP_CODE(Input::Code::KC_MEDIASTOP, MyGUI::KeyCode::MediaStop);
        MAP_CODE(Input::Code::KC_VOLUMEDOWN, MyGUI::KeyCode::VolumeDown);
        MAP_CODE(Input::Code::KC_VOLUMEUP, MyGUI::KeyCode::VolumeUp);
        MAP_CODE(Input::Code::KC_WEBHOME, MyGUI::KeyCode::WebHome);
        MAP_CODE(Input::Code::KC_ADD, MyGUI::KeyCode::Add);/* + on numeric keypad */
        MAP_CODE(Input::Code::KC_SUBTRACT, MyGUI::KeyCode::Subtract);/* - on numeric keypad */
        MAP_CODE(Input::Code::KC_MULTIPLY, MyGUI::KeyCode::Multiply);/* * on numeric keypad */
        MAP_CODE(Input::Code::KC_DIVIDE, MyGUI::KeyCode::Divide);/* / on numeric keypad */
        MAP_CODE(Input::Code::KC_SCREENSHOT, MyGUI::KeyCode::SysRq);
//         MAP_CODE(Input::Code::KC_, MyGUI::KeyCode::RightAlt);
        MAP_CODE(Input::Code::KC_PAUSE, MyGUI::KeyCode::Pause);
        MAP_CODE(Input::Code::KC_HOME, MyGUI::KeyCode::Home);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_END, MyGUI::KeyCode::End);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_UP, MyGUI::KeyCode::ArrowUp);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_LEFT, MyGUI::KeyCode::ArrowLeft);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_RIGHT, MyGUI::KeyCode::ArrowRight);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_DOWN, MyGUI::KeyCode::ArrowDown);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_PGUP, MyGUI::KeyCode::PageUp);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_PGDOWN, MyGUI::KeyCode::PageDown);/* on arrow keypad */
        MAP_CODE(Input::Code::KC_INSERT, MyGUI::KeyCode::Insert);
        MAP_CODE(Input::Code::KC_DELETE, MyGUI::KeyCode::Delete);
        MAP_CODE(Input::Code::KC_LWIN, MyGUI::KeyCode::LeftWindows);
        MAP_CODE(Input::Code::KC_RWIN, MyGUI::KeyCode::RightWindows);
        MAP_CODE(Input::Code::KC_APPS, MyGUI::KeyCode::AppMenu);
        MAP_CODE(Input::Code::KC_POWER, MyGUI::KeyCode::Power);
        MAP_CODE(Input::Code::KC_SLEEP, MyGUI::KeyCode::Sleep);
        MAP_CODE(Input::Code::KC_WAKE, MyGUI::KeyCode::Wake);
        MAP_CODE(Input::Code::KC_WEBSEARCH, MyGUI::KeyCode::WebSearch);
        MAP_CODE(Input::Code::KC_WEBFAVORITES, MyGUI::KeyCode::WebFavorites);
        MAP_CODE(Input::Code::KC_WEBREFRESH, MyGUI::KeyCode::WebRefresh);
        MAP_CODE(Input::Code::KC_WEBSTOP, MyGUI::KeyCode::WebStop);
        MAP_CODE(Input::Code::KC_WEBFORWARD, MyGUI::KeyCode::WebForward);
        MAP_CODE(Input::Code::KC_WEBBACK, MyGUI::KeyCode::WebBack);
        MAP_CODE(Input::Code::KC_MYCOMPUTER, MyGUI::KeyCode::MyComputer);
        MAP_CODE(Input::Code::KC_MAIL, MyGUI::KeyCode::Mail);
        MAP_CODE(Input::Code::KC_MEDIASELECT, MyGUI::KeyCode::MediaSelect);
#undef MAP_CODE
    }

    int UI::getRocketKeyModifierState()
    {
        int modifier_state = 0;

        if(mInputMan->isKeyDown(Input::Code::KC_LCONTROL))
            modifier_state |= Rocket::Core::Input::KM_CTRL;

        if(mInputMan->isKeyDown(Input::Code::KC_LSHIFT))
            modifier_state |= Rocket::Core::Input::KM_SHIFT;

        if(mInputMan->isKeyDown(Input::Code::KC_ALT))
            modifier_state |= Rocket::Core::Input::KM_ALT;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

        if(GetKeyState(VK_CAPITAL) > 0)
            modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;

        if(GetKeyState(VK_NUMLOCK) > 0)
            modifier_state |= Rocket::Core::Input::KM_NUMLOCK;

        if(GetKeyState(VK_SCROLL) > 0)
            modifier_state |= Rocket::Core::Input::KM_SCROLLLOCK;

#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE

        UInt32 key_modifiers = GetCurrentEventKeyModifiers();

        if(key_modifiers & (1 << alphaLockBit))
            modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;

#elif OGRE_PLATFORM == OGRE_PLATFORM_LINUX

        //TODO: find if this has to be platform specific
        /*
        XKeyboardState keyboard_state;
        XGetKeyboardControl(DISPLAY!, &keyboard_state);

        if (keyboard_state.led_mask & (1 << 0))
         modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;
        if (keyboard_state.led_mask & (1 << 1))
         modifier_state |= Rocket::Core::Input::KM_NUMLOCK;
        if (keyboard_state.led_mask & (1 << 2))
         modifier_state |= Rocket::Core::Input::KM_SCROLLLOCK;
        */

#endif

        return modifier_state;
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

