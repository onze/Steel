
#include <Rocket/Controls.h>
#include <Rocket/Debugger.h>
#include <Ogre.h>

#include "UI/UI.h"
#include <UI/RenderInterfaceOgre3D.h>
#include "Debug.h"
#include "InputManager.h"
#include <tools/OgreUtils.h>
#include <Level.h>
#include <Engine.h>
#include "UI/RenderInterfaceOgre3D.h"

namespace Steel
{
    UI::UI(): Rocket::Core::SystemInterface(), Ogre::RenderQueueListener(), EngineEventListener(), InputEventListener(),
        mInputMan(nullptr), mWindow(nullptr),
        mRocketRenderInterface(nullptr), mMainContext(nullptr),
        mKeyIdentifiers(KeyIdentifierMap()), mEditor(), mHUD(), mUIDataDir(), mEditMode(false)
    {
        mTimer = Ogre::Timer();
        buildCodeMaps();
    }

    UI::UI(const UI &other)
    {
        mInputMan = other.mInputMan;
        mTimer = other.mTimer;
        mRocketRenderInterface = other.mRocketRenderInterface;
        mMainContext = other.mMainContext;
        mEditor = other.mEditor;
        mHUD = other.mHUD;
        mKeyIdentifiers = other.mKeyIdentifiers;
        mEditMode = other.mEditMode;
        //TODO forbif copy (use save/load instead)
    }

    UI::~UI()
    {
        shutdown();
    }

    UI &UI::operator=(const UI &other)
    {
        if(mMainContext != nullptr && mMainContext != other.mMainContext)
        {
            mMainContext->RemoveReference();
            mMainContext = other.mMainContext;
            mMainContext->AddReference();
        }

        mEditor = other.mEditor;
        mHUD = other.mHUD;
        mKeyIdentifiers = other.mKeyIdentifiers;
        mEditMode = other.mEditMode;
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

        if(mMainContext != nullptr)
        {
            mMainContext->UnloadAllMouseCursors();
            mMainContext->UnloadAllDocuments();
            mMainContext->RemoveReference();
            mMainContext = nullptr;
        }

        if(mInputMan != nullptr)
            mInputMan = nullptr;

        Rocket::Core::Shutdown();

        if(mRocketRenderInterface != nullptr)
        {
            delete mRocketRenderInterface;
            mRocketRenderInterface = nullptr;
        }

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

        //rocket init
        auto orm = Ogre::ResourceGroupManager::getSingletonPtr();
        orm->addResourceLocation(mUIDataDir.fullPath(), "FileSystem", "UI", true);
        bool firstInit = mRocketRenderInterface == nullptr;

        mRocketRenderInterface = new RenderInterfaceOgre3D(mWindow->getWidth(), mWindow->getHeight(), mEngine);
        Rocket::Core::SetRenderInterface(mRocketRenderInterface);


        Rocket::Core::SetSystemInterface(this);
        Rocket::Core::Initialise();

        Rocket::Controls::Initialise();
        Rocket::Core::FontDatabase::LoadFontFace(mUIDataDir.subfile("fonts").subfile("tahoma.ttf").fullPath().c_str());

        mMainContext = Rocket::Core::CreateContext("UI-main", Rocket::Core::Vector2i(mWindow->getWidth(), mWindow->getHeight()));
//         Rocket::Core::ElementDocument* cursor = mMainContext->LoadMouseCursor(mUIDataDir.subfile("current/cursor.rml").fullPath().c_str());

        if(firstInit)
            Rocket::Debugger::Initialise(mMainContext);
        else
            Rocket::Debugger::SetContext(mMainContext);

        if(!Rocket::Debugger::IsVisible())
            Rocket::Debugger::SetVisible(true);

        //UI init
        mEditor.init(mWindow->getWidth(), mWindow->getHeight(), mEngine, this);
        mHUD.init(mWindow->getWidth(), mWindow->getHeight(), mEngine, this);

        orm->initialiseResourceGroup("UI");
        orm->loadResourceGroup("UI");

        OgreUtils::resourceGroupsInfos();
    }

    void UI::startEditMode()
    {
        if(!mEditMode)
        {
            mEditMode = true;
            mEditor.show();
        }
    }

    void UI::stopEditMode()
    {
        if(mEditMode)
        {
            mEditMode = false;
            mEditor.hide();
        }
    }

    // Called from Ogre before a queue group is rendered.
    void UI::renderQueueStarted(Ogre::uint8 queueGroupId,
                                const Ogre::String &invocation,
                                bool &ROCKET_UNUSED(skipThisInvocation))
    {
        if(queueGroupId == Ogre::RENDER_QUEUE_OVERLAY && Ogre::Root::getSingleton().getRenderSystem()->_getViewport()->getOverlaysEnabled())
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
    void UI::renderQueueEnded(Ogre::uint8 ROCKET_UNUSED(queueGroupId),
                              const Ogre::String &ROCKET_UNUSED(invocation),
                              bool &ROCKET_UNUSED(repeatThisInvocation))
    {
    }

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
        Rocket::Core::Input::KeyIdentifier keyIdentifier = getKeyIdentifier(key);

        if(evt.text >= 32)
            mMainContext->ProcessTextInput((Rocket::Core::word) evt.text);
        else if(keyIdentifier == Rocket::Core::Input::KI_RETURN)
            mMainContext->ProcessTextInput((Rocket::Core::word) '\n');
        else
            mMainContext->ProcessKeyDown(keyIdentifier , getKeyModifierState());

        if(mEditMode)
            mEditor.keyPressed(key, evt);

        return true;
    }

    bool UI::keyReleased(Input::Code key, Input::Event const &evt)
    {
        mMainContext->ProcessKeyUp(getKeyIdentifier(key), getKeyModifierState());

        if(mEditMode)
            mEditor.keyReleased(key, evt);

        return true;
    }

    bool UI::mouseMoved(Ogre::Vector2 const &position, Input::Event const &evt)
    {
        mMainContext->ProcessMouseMove(evt.position.x, evt.position.y, getKeyModifierState());

        if(mEditMode)
            mEditor.mouseMoved(position, evt);

        return true;
    }

    Rocket::Core::Input::KeyIdentifier UI::getKeyIdentifier(Input::Code key) const
    {
        auto it = mKeyIdentifiers.find(key);

        if(mKeyIdentifiers.end() == it)
            return Rocket::Core::Input::KeyIdentifier::KI_UNKNOWN;

        return it->second;
    }

    int UI::getMouseIdentifier(Input::Code button) const
    {
        auto it = mMouseIdentifiers.find(button);

        if(mMouseIdentifiers.end() == it)
            return 0; // left btn

        return it->second;
    }

    bool UI::mousePressed(Input::Code button, Input::Event const &evt)
    {
        mMainContext->ProcessMouseButtonDown(getMouseIdentifier(button), getKeyModifierState());

        if(mEditMode)
            mEditor.mousePressed(button, evt);

        return true;
    }

    bool UI::mouseReleased(Input::Code button, Input::Event const &evt)
    {
        mMainContext->ProcessMouseButtonUp(getMouseIdentifier(button), getKeyModifierState());

        if(mEditMode)
            mEditor.mouseReleased(button, evt);

        return true;
    }

    bool UI::mouseWheeled(int delta, Input::Event const &evt)
    {
        mMainContext->ProcessMouseWheel(delta / -120, getKeyModifierState());

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
        level->sceneManager()->addRenderQueueListener(this);
    }

    void UI::buildCodeMaps()
    {
        mKeyIdentifiers[Input::Code::UNASSIGNED] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_ESCAPE] = Rocket::Core::Input::KI_ESCAPE;
        mKeyIdentifiers[Input::Code::KC_1] = Rocket::Core::Input::KI_1;
        mKeyIdentifiers[Input::Code::KC_2] = Rocket::Core::Input::KI_2;
        mKeyIdentifiers[Input::Code::KC_3] = Rocket::Core::Input::KI_3;
        mKeyIdentifiers[Input::Code::KC_4] = Rocket::Core::Input::KI_4;
        mKeyIdentifiers[Input::Code::KC_5] = Rocket::Core::Input::KI_5;
        mKeyIdentifiers[Input::Code::KC_6] = Rocket::Core::Input::KI_6;
        mKeyIdentifiers[Input::Code::KC_7] = Rocket::Core::Input::KI_7;
        mKeyIdentifiers[Input::Code::KC_8] = Rocket::Core::Input::KI_8;
        mKeyIdentifiers[Input::Code::KC_9] = Rocket::Core::Input::KI_9;
        mKeyIdentifiers[Input::Code::KC_0] = Rocket::Core::Input::KI_0;
        mKeyIdentifiers[Input::Code::KC_MINUS] = Rocket::Core::Input::KI_OEM_MINUS;
        mKeyIdentifiers[Input::Code::KC_EQUALS] = Rocket::Core::Input::KI_OEM_PLUS;
        mKeyIdentifiers[Input::Code::KC_BACK] = Rocket::Core::Input::KI_BACK;
        mKeyIdentifiers[Input::Code::KC_TAB] = Rocket::Core::Input::KI_TAB;
        mKeyIdentifiers[Input::Code::KC_Q] = Rocket::Core::Input::KI_Q;
        mKeyIdentifiers[Input::Code::KC_W] = Rocket::Core::Input::KI_W;
        mKeyIdentifiers[Input::Code::KC_E] = Rocket::Core::Input::KI_E;
        mKeyIdentifiers[Input::Code::KC_R] = Rocket::Core::Input::KI_R;
        mKeyIdentifiers[Input::Code::KC_T] = Rocket::Core::Input::KI_T;
        mKeyIdentifiers[Input::Code::KC_Y] = Rocket::Core::Input::KI_Y;
        mKeyIdentifiers[Input::Code::KC_U] = Rocket::Core::Input::KI_U;
        mKeyIdentifiers[Input::Code::KC_I] = Rocket::Core::Input::KI_I;
        mKeyIdentifiers[Input::Code::KC_O] = Rocket::Core::Input::KI_O;
        mKeyIdentifiers[Input::Code::KC_P] = Rocket::Core::Input::KI_P;
        mKeyIdentifiers[Input::Code::KC_LBRACKET] = Rocket::Core::Input::KI_OEM_4;
        mKeyIdentifiers[Input::Code::KC_RBRACKET] = Rocket::Core::Input::KI_OEM_6;
        mKeyIdentifiers[Input::Code::KC_RETURN] = Rocket::Core::Input::KI_RETURN;
        mKeyIdentifiers[Input::Code::KC_LCONTROL] = Rocket::Core::Input::KI_LCONTROL;
        mKeyIdentifiers[Input::Code::KC_A] = Rocket::Core::Input::KI_A;
        mKeyIdentifiers[Input::Code::KC_S] = Rocket::Core::Input::KI_S;
        mKeyIdentifiers[Input::Code::KC_D] = Rocket::Core::Input::KI_D;
        mKeyIdentifiers[Input::Code::KC_F] = Rocket::Core::Input::KI_F;
        mKeyIdentifiers[Input::Code::KC_G] = Rocket::Core::Input::KI_G;
        mKeyIdentifiers[Input::Code::KC_H] = Rocket::Core::Input::KI_H;
        mKeyIdentifiers[Input::Code::KC_J] = Rocket::Core::Input::KI_J;
        mKeyIdentifiers[Input::Code::KC_K] = Rocket::Core::Input::KI_K;
        mKeyIdentifiers[Input::Code::KC_L] = Rocket::Core::Input::KI_L;
        mKeyIdentifiers[Input::Code::KC_SEMICOLON] = Rocket::Core::Input::KI_OEM_1;
        mKeyIdentifiers[Input::Code::KC_APOSTROPHE] = Rocket::Core::Input::KI_OEM_7;
        mKeyIdentifiers[Input::Code::KC_GRAVE] = Rocket::Core::Input::KI_OEM_3;
        mKeyIdentifiers[Input::Code::KC_LSHIFT] = Rocket::Core::Input::KI_LSHIFT;
        mKeyIdentifiers[Input::Code::KC_BACKSLASH] = Rocket::Core::Input::KI_OEM_5;
        mKeyIdentifiers[Input::Code::KC_Z] = Rocket::Core::Input::KI_Z;
        mKeyIdentifiers[Input::Code::KC_X] = Rocket::Core::Input::KI_X;
        mKeyIdentifiers[Input::Code::KC_C] = Rocket::Core::Input::KI_C;
        mKeyIdentifiers[Input::Code::KC_V] = Rocket::Core::Input::KI_V;
        mKeyIdentifiers[Input::Code::KC_B] = Rocket::Core::Input::KI_B;
        mKeyIdentifiers[Input::Code::KC_N] = Rocket::Core::Input::KI_N;
        mKeyIdentifiers[Input::Code::KC_M] = Rocket::Core::Input::KI_M;
        mKeyIdentifiers[Input::Code::KC_COMMA] = Rocket::Core::Input::KI_OEM_COMMA;
        mKeyIdentifiers[Input::Code::KC_PERIOD] = Rocket::Core::Input::KI_OEM_PERIOD;
        mKeyIdentifiers[Input::Code::KC_SLASH] = Rocket::Core::Input::KI_OEM_2;
        mKeyIdentifiers[Input::Code::KC_RSHIFT] = Rocket::Core::Input::KI_RSHIFT;
        mKeyIdentifiers[Input::Code::KC_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
        mKeyIdentifiers[Input::Code::KC_LMENU] = Rocket::Core::Input::KI_LMENU;
        mKeyIdentifiers[Input::Code::KC_SPACE] = Rocket::Core::Input::KI_SPACE;
        mKeyIdentifiers[Input::Code::KC_CAPITAL] = Rocket::Core::Input::KI_CAPITAL;
        mKeyIdentifiers[Input::Code::KC_F1] = Rocket::Core::Input::KI_F1;
        mKeyIdentifiers[Input::Code::KC_F2] = Rocket::Core::Input::KI_F2;
        mKeyIdentifiers[Input::Code::KC_F3] = Rocket::Core::Input::KI_F3;
        mKeyIdentifiers[Input::Code::KC_F4] = Rocket::Core::Input::KI_F4;
        mKeyIdentifiers[Input::Code::KC_F5] = Rocket::Core::Input::KI_F5;
        mKeyIdentifiers[Input::Code::KC_F6] = Rocket::Core::Input::KI_F6;
        mKeyIdentifiers[Input::Code::KC_F7] = Rocket::Core::Input::KI_F7;
        mKeyIdentifiers[Input::Code::KC_F8] = Rocket::Core::Input::KI_F8;
        mKeyIdentifiers[Input::Code::KC_F9] = Rocket::Core::Input::KI_F9;
        mKeyIdentifiers[Input::Code::KC_F10] = Rocket::Core::Input::KI_F10;
        mKeyIdentifiers[Input::Code::KC_NUMLOCK] = Rocket::Core::Input::KI_NUMLOCK;
        mKeyIdentifiers[Input::Code::KC_SCROLL] = Rocket::Core::Input::KI_SCROLL;
        mKeyIdentifiers[Input::Code::KC_NUMPAD7] = Rocket::Core::Input::KI_7;
        mKeyIdentifiers[Input::Code::KC_NUMPAD8] = Rocket::Core::Input::KI_8;
        mKeyIdentifiers[Input::Code::KC_NUMPAD9] = Rocket::Core::Input::KI_9;
        mKeyIdentifiers[Input::Code::KC_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
        mKeyIdentifiers[Input::Code::KC_NUMPAD4] = Rocket::Core::Input::KI_4;
        mKeyIdentifiers[Input::Code::KC_NUMPAD5] = Rocket::Core::Input::KI_5;
        mKeyIdentifiers[Input::Code::KC_NUMPAD6] = Rocket::Core::Input::KI_6;
        mKeyIdentifiers[Input::Code::KC_ADD] = Rocket::Core::Input::KI_ADD;
        mKeyIdentifiers[Input::Code::KC_NUMPAD1] = Rocket::Core::Input::KI_1;
        mKeyIdentifiers[Input::Code::KC_NUMPAD2] = Rocket::Core::Input::KI_2;
        mKeyIdentifiers[Input::Code::KC_NUMPAD3] = Rocket::Core::Input::KI_3;
        mKeyIdentifiers[Input::Code::KC_NUMPAD0] = Rocket::Core::Input::KI_0;
        mKeyIdentifiers[Input::Code::KC_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
        mKeyIdentifiers[Input::Code::KC_OEM_102] = Rocket::Core::Input::KI_OEM_102;
        mKeyIdentifiers[Input::Code::KC_F11] = Rocket::Core::Input::KI_F11;
        mKeyIdentifiers[Input::Code::KC_F12] = Rocket::Core::Input::KI_F12;
        mKeyIdentifiers[Input::Code::KC_F13] = Rocket::Core::Input::KI_F13;
        mKeyIdentifiers[Input::Code::KC_F14] = Rocket::Core::Input::KI_F14;
        mKeyIdentifiers[Input::Code::KC_F15] = Rocket::Core::Input::KI_F15;
        mKeyIdentifiers[Input::Code::KC_KANA] = Rocket::Core::Input::KI_KANA;
        mKeyIdentifiers[Input::Code::KC_ABNT_C1] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_CONVERT] = Rocket::Core::Input::KI_CONVERT;
        mKeyIdentifiers[Input::Code::KC_NOCONVERT] = Rocket::Core::Input::KI_NONCONVERT;
        mKeyIdentifiers[Input::Code::KC_YEN] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_ABNT_C2] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_NUMPADEQUALS] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;
        mKeyIdentifiers[Input::Code::KC_PREVTRACK] = Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
        mKeyIdentifiers[Input::Code::KC_AT] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_COLON] = Rocket::Core::Input::KI_OEM_1;
        mKeyIdentifiers[Input::Code::KC_UNDERLINE] = Rocket::Core::Input::KI_OEM_MINUS;
        mKeyIdentifiers[Input::Code::KC_KANJI] = Rocket::Core::Input::KI_KANJI;
        mKeyIdentifiers[Input::Code::KC_STOP] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_AX] = Rocket::Core::Input::KI_OEM_AX;
        mKeyIdentifiers[Input::Code::KC_UNLABELED] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_NEXTTRACK] = Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
        mKeyIdentifiers[Input::Code::KC_NUMPADENTER] = Rocket::Core::Input::KI_NUMPADENTER;
        mKeyIdentifiers[Input::Code::KC_RCONTROL] = Rocket::Core::Input::KI_RCONTROL;
        mKeyIdentifiers[Input::Code::KC_MUTE] = Rocket::Core::Input::KI_VOLUME_MUTE;
        mKeyIdentifiers[Input::Code::KC_CALCULATOR] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_PLAYPAUSE] = Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
        mKeyIdentifiers[Input::Code::KC_MEDIASTOP] = Rocket::Core::Input::KI_MEDIA_STOP;
        mKeyIdentifiers[Input::Code::KC_VOLUMEDOWN] = Rocket::Core::Input::KI_VOLUME_DOWN;
        mKeyIdentifiers[Input::Code::KC_VOLUMEUP] = Rocket::Core::Input::KI_VOLUME_UP;
        mKeyIdentifiers[Input::Code::KC_WEBHOME] = Rocket::Core::Input::KI_BROWSER_HOME;
        mKeyIdentifiers[Input::Code::KC_NUMPADCOMMA] = Rocket::Core::Input::KI_SEPARATOR;
        mKeyIdentifiers[Input::Code::KC_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
        mKeyIdentifiers[Input::Code::KC_SCREENSHOT] = Rocket::Core::Input::KI_SNAPSHOT;
        mKeyIdentifiers[Input::Code::KC_RMENU] = Rocket::Core::Input::KI_RMENU;
        mKeyIdentifiers[Input::Code::KC_PAUSE] = Rocket::Core::Input::KI_PAUSE;
        mKeyIdentifiers[Input::Code::KC_HOME] = Rocket::Core::Input::KI_HOME;
        mKeyIdentifiers[Input::Code::KC_UP] = Rocket::Core::Input::KI_UP;
        mKeyIdentifiers[Input::Code::KC_PGUP] = Rocket::Core::Input::KI_PRIOR;
        mKeyIdentifiers[Input::Code::KC_LEFT] = Rocket::Core::Input::KI_LEFT;
        mKeyIdentifiers[Input::Code::KC_RIGHT] = Rocket::Core::Input::KI_RIGHT;
        mKeyIdentifiers[Input::Code::KC_END] = Rocket::Core::Input::KI_END;
        mKeyIdentifiers[Input::Code::KC_DOWN] = Rocket::Core::Input::KI_DOWN;
        mKeyIdentifiers[Input::Code::KC_PGDOWN] = Rocket::Core::Input::KI_NEXT;
        mKeyIdentifiers[Input::Code::KC_INSERT] = Rocket::Core::Input::KI_INSERT;
        mKeyIdentifiers[Input::Code::KC_DELETE] = Rocket::Core::Input::KI_DELETE;
        mKeyIdentifiers[Input::Code::KC_LWIN] = Rocket::Core::Input::KI_LWIN;
        mKeyIdentifiers[Input::Code::KC_RWIN] = Rocket::Core::Input::KI_RWIN;
        mKeyIdentifiers[Input::Code::KC_APPS] = Rocket::Core::Input::KI_APPS;
        mKeyIdentifiers[Input::Code::KC_POWER] = Rocket::Core::Input::KI_POWER;
        mKeyIdentifiers[Input::Code::KC_SLEEP] = Rocket::Core::Input::KI_SLEEP;
        mKeyIdentifiers[Input::Code::KC_WAKE] = Rocket::Core::Input::KI_WAKE;
        mKeyIdentifiers[Input::Code::KC_WEBSEARCH] = Rocket::Core::Input::KI_BROWSER_SEARCH;
        mKeyIdentifiers[Input::Code::KC_WEBFAVORITES] = Rocket::Core::Input::KI_BROWSER_FAVORITES;
        mKeyIdentifiers[Input::Code::KC_WEBREFRESH] = Rocket::Core::Input::KI_BROWSER_REFRESH;
        mKeyIdentifiers[Input::Code::KC_WEBSTOP] = Rocket::Core::Input::KI_BROWSER_STOP;
        mKeyIdentifiers[Input::Code::KC_WEBFORWARD] = Rocket::Core::Input::KI_BROWSER_FORWARD;
        mKeyIdentifiers[Input::Code::KC_WEBBACK] = Rocket::Core::Input::KI_BROWSER_BACK;
        mKeyIdentifiers[Input::Code::KC_MYCOMPUTER] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[Input::Code::KC_MAIL] = Rocket::Core::Input::KI_LAUNCH_MAIL;
        mKeyIdentifiers[Input::Code::KC_MEDIASELECT] = Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;

        // mouseMoved
        //MB_Left = 0, MB_Right, MB_Middle,
        //MB_Button3, MB_Button4, MB_Button5, MB_Button6, MB_Button7
        mMouseIdentifiers[Input::Code::MC_LEFT] = 0;
        mMouseIdentifiers[Input::Code::MC_RIGHT] = 1;
        mMouseIdentifiers[Input::Code::MC_MIDDLE] = 2;
        mMouseIdentifiers[Input::Code::MC_BUTTON3] = 3;
        mMouseIdentifiers[Input::Code::MC_BUTTON4] = 4;
        mMouseIdentifiers[Input::Code::MC_BUTTON5] = 5;
        mMouseIdentifiers[Input::Code::MC_BUTTON6] = 6;
        mMouseIdentifiers[Input::Code::MC_BUTTON7] = 7;
    }

    int UI::getKeyModifierState()
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

