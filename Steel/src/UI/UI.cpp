
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
    UI::UI():Rocket::Core::SystemInterface(),Ogre::RenderQueueListener(),EngineEventListener(),
        mInputMan(NULL),mWindow(NULL),mWidth(0),mHeight(0),
        mRocketRenderInterface(NULL),mMainContext(NULL),
        mKeyIdentifiers(KeyIdentifierMap()),mEditor(),mHUD(),mUIDataDir(),mEditMode(false)
    {
        mTimer=Ogre::Timer();
        buildKeyMaps();
    }

    UI::UI(const UI& other)
    {
        mInputMan=other.mInputMan;
        mTimer=other.mTimer;
        mRocketRenderInterface=other.mRocketRenderInterface;
        mMainContext=other.mMainContext;
        mEditor=other.mEditor;
        mHUD=other.mHUD;
        mWidth=other.mWidth;
        mHeight=other.mHeight;
        mKeyIdentifiers=other.mKeyIdentifiers;
        mEditMode=other.mEditMode;
    }

    UI::~UI()
    {
        shutdown();
    }

    UI& UI::operator=(const UI& other)
    {
        if(mMainContext!=NULL && mMainContext!=other.mMainContext)
        {
            mMainContext->RemoveReference();
            mMainContext=other.mMainContext;
            mMainContext->AddReference();
        }
        mEditor=other.mEditor;
        mHUD=other.mHUD;
        mWidth=other.mWidth;
        mHeight=other.mHeight;
        mKeyIdentifiers=other.mKeyIdentifiers;
        mEditMode=other.mEditMode;
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

    float UI::GetElapsedTime()
    {
        return mTimer.getMilliseconds() * 0.001f;
    }

    bool UI::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
    {
        Ogre::String intro="[Rocket] ";
        switch (type)
        {
            case Rocket::Core::Log::LT_ERROR:
            case Rocket::Core::Log::LT_ASSERT:
                Debug::error(intro+message.CString()).endl();
                break;

            case Rocket::Core::Log::LT_WARNING:
                Debug::warning(intro+message.CString()).endl();
                break;

            case Rocket::Core::Log::LT_ALWAYS:
            default:
                Debug::log(intro+message.CString()).endl();
                break;
        }
        return false;
    }

    void UI::shutdown()
    {
        stopEditMode();
        mHUD.shutdown();
        mEditor.shutdown();
        if(mMainContext!=NULL)
        {
            mMainContext->UnloadAllMouseCursors();
            mMainContext->UnloadAllDocuments();
            mMainContext->RemoveReference();
            mMainContext=NULL;
        }
        if(mInputMan!=NULL)
            mInputMan=NULL;
        Rocket::Core::Shutdown();
        if(mRocketRenderInterface!=NULL)
        {
            delete mRocketRenderInterface;
            mRocketRenderInterface=NULL;
        }
        mEngine->removeEngineEventListener(this);
    }

    void UI::init(unsigned int width,
                  unsigned int height,
                  File UIDataDir,
                  InputManager *inputMan,
                  Ogre::RenderWindow *window,
                  Engine *engine)
    {
        Debug::log("UI::init()").endl();
        mWidth=width;
        mHeight=height;
        mUIDataDir=UIDataDir.subfile("current");
        mInputMan=inputMan;
        mWindow=window;
        mEngine=engine;

        mEditMode=false;
        mEngine->addEngineEventListener(this);
        
        //rocket init
        auto orm=Ogre::ResourceGroupManager::getSingletonPtr();
        orm->addResourceLocation(mUIDataDir.fullPath(), "FileSystem", "UI",true);
        bool firstInit=mRocketRenderInterface==NULL;

        mRocketRenderInterface=new RenderInterfaceOgre3D(mWidth,mHeight,mEngine);
        Rocket::Core::SetRenderInterface(mRocketRenderInterface);


        Rocket::Core::SetSystemInterface(this);
        Rocket::Core::Initialise();

        Rocket::Controls::Initialise();
        Rocket::Core::FontDatabase::LoadFontFace(mUIDataDir.subfile("fonts").subfile("tahoma.ttf").fullPath().c_str());

        mMainContext = Rocket::Core::CreateContext("UI-main", Rocket::Core::Vector2i(mWidth, mHeight));
//         Rocket::Core::ElementDocument* cursor = mMainContext->LoadMouseCursor(mUIDataDir.subfile("current/cursor.rml").fullPath().c_str());

        if(firstInit)
            Rocket::Debugger::Initialise(mMainContext);
        else
            Rocket::Debugger::SetContext(mMainContext);

        if(!Rocket::Debugger::IsVisible())
            Rocket::Debugger::SetVisible(true);

        //UI init
        mEditor.init(mWidth, mHeight, mEngine, this, mInputMan);
        mHUD.init(mWidth, mHeight, mEngine, this);

        orm->initialiseResourceGroup("UI");
        orm->loadResourceGroup("UI");

        OgreUtils::resourceGroupsInfos();
    }

    void UI::startEditMode()
    {
        mEditMode=true;
        mEditor.show();
    }

    void UI::stopEditMode()
    {
        mEditMode=false;
        mEditor.hide();
    }

    // Called from Ogre before a queue group is rendered.
    void UI::renderQueueStarted(Ogre::uint8 queueGroupId,
                                const Ogre::String& invocation,
                                bool& ROCKET_UNUSED(skipThisInvocation))
    {
        if (queueGroupId == Ogre::RENDER_QUEUE_OVERLAY && Ogre::Root::getSingleton().getRenderSystem()->_getViewport()->getOverlaysEnabled())
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
                              const Ogre::String& ROCKET_UNUSED(invocation),
                              bool& ROCKET_UNUSED(repeatThisInvocation))
    {
    }

    // Configures Ogre's rendering system for rendering RocketUI.
    void UI::configureRenderSystem()
    {
        Ogre::RenderSystem* render_system = Ogre::Root::getSingleton().getRenderSystem();
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
    void UI::buildProjectionMatrix(Ogre::Matrix4& projection_matrix)
    {
        float z_near = -1;
        float z_far = 1;

        projection_matrix = Ogre::Matrix4::ZERO;

        // Set up matrices.
        projection_matrix[0][0] = 2.0f / mWindow->getWidth();
        projection_matrix[0][3]= -1.0000000f;
        projection_matrix[1][1]= -2.0f / mWindow->getHeight();
        projection_matrix[1][3]= 1.0000000f;
        projection_matrix[2][2]= -2.0f / (z_far - z_near);
        projection_matrix[3][3]= 1.0000000f;
    }

    bool UI::keyPressed(const OIS::KeyEvent& evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mKeyIdentifiers[evt.key];
        mMainContext->ProcessKeyDown(keyIdentifier ,getKeyModifierState());
        
        if (evt.text >= 32)
            mMainContext->ProcessTextInput((Rocket::Core::word) evt.text);
        else if (keyIdentifier == Rocket::Core::Input::KI_RETURN)
            mMainContext->ProcessTextInput((Rocket::Core::word) '\n');
        
        if(mEditMode)
            mEditor.keyPressed(evt);
        
        return true;
    }

    bool UI::keyReleased(const OIS::KeyEvent& evt)
    {
        Rocket::Core::Input::KeyIdentifier keyIdentifier = mKeyIdentifiers[evt.key];
        int keyModifierState=getKeyModifierState();
        
        mMainContext->ProcessKeyUp(keyIdentifier ,keyModifierState);
        
        if(mEditMode)
            mEditor.keyReleased(evt);
        
        return true;
    }

    bool UI::mouseMoved(const OIS::MouseEvent& evt)
    {
        int key_modifier_state = getKeyModifierState();
        mMainContext->ProcessMouseMove(evt.state.X.abs, evt.state.Y.abs, key_modifier_state);
        if(mEditMode)
        {
            mEditor.context()->ProcessMouseMove(evt.state.X.abs, evt.state.Y.abs, key_modifier_state);
            mEditor.mouseMoved(evt);
        }
        if (evt.state.Z.rel != 0)
        {
            mMainContext->ProcessMouseWheel(evt.state.Z.rel / -120, key_modifier_state);
            if(mEditMode)
                mEditor.context()->ProcessMouseWheel(evt.state.Z.rel / -120, key_modifier_state);
        }
        return true;
    }

    bool UI::mousePressed(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
//         Debug::log("mousePressed at ")(evt.state.X.abs)(" ")(evt.state.Y.abs).endl();
        mMainContext->ProcessMouseButtonDown((int) id, getKeyModifierState());
        if(mEditMode)
        {
            mEditor.context()->ProcessMouseButtonDown((int) id, getKeyModifierState());
            mEditor.mousePressed(evt,id);
        }
        return true;
    }

    bool UI::mouseReleased(const OIS::MouseEvent& evt, OIS::MouseButtonID id)
    {
//         Debug::log("mouseReleased at ")(evt.state.X.abs)(" ")(evt.state.Y.abs).endl();
        mMainContext->ProcessMouseButtonUp((int) id, getKeyModifierState());
        if(mEditMode)
        {
            mEditor.context()->ProcessMouseButtonUp((int) id, getKeyModifierState());
            mEditor.mouseReleased(evt,id);
        }
        return true;
    }
    
    void UI::onLevelUnset(Level *level)
    {
        // stop rendering as long as there is no level->scenemanager to render from
        level->sceneManager()->removeRenderQueueListener(this);
        mHUD.hide();
        mEditor.hide();
    }
    
    void UI::onLevelSet(Level *level)
    {
        level->sceneManager()->addRenderQueueListener(this);
        mHUD.show();
        mEditor.show();
    }
    
    void UI::buildKeyMaps()
    {
        mKeyIdentifiers[OIS::KC_UNASSIGNED] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_ESCAPE] = Rocket::Core::Input::KI_ESCAPE;
        mKeyIdentifiers[OIS::KC_1] = Rocket::Core::Input::KI_1;
        mKeyIdentifiers[OIS::KC_2] = Rocket::Core::Input::KI_2;
        mKeyIdentifiers[OIS::KC_3] = Rocket::Core::Input::KI_3;
        mKeyIdentifiers[OIS::KC_4] = Rocket::Core::Input::KI_4;
        mKeyIdentifiers[OIS::KC_5] = Rocket::Core::Input::KI_5;
        mKeyIdentifiers[OIS::KC_6] = Rocket::Core::Input::KI_6;
        mKeyIdentifiers[OIS::KC_7] = Rocket::Core::Input::KI_7;
        mKeyIdentifiers[OIS::KC_8] = Rocket::Core::Input::KI_8;
        mKeyIdentifiers[OIS::KC_9] = Rocket::Core::Input::KI_9;
        mKeyIdentifiers[OIS::KC_0] = Rocket::Core::Input::KI_0;
        mKeyIdentifiers[OIS::KC_MINUS] = Rocket::Core::Input::KI_OEM_MINUS;
        mKeyIdentifiers[OIS::KC_EQUALS] = Rocket::Core::Input::KI_OEM_PLUS;
        mKeyIdentifiers[OIS::KC_BACK] = Rocket::Core::Input::KI_BACK;
        mKeyIdentifiers[OIS::KC_TAB] = Rocket::Core::Input::KI_TAB;
        mKeyIdentifiers[OIS::KC_Q] = Rocket::Core::Input::KI_Q;
        mKeyIdentifiers[OIS::KC_W] = Rocket::Core::Input::KI_W;
        mKeyIdentifiers[OIS::KC_E] = Rocket::Core::Input::KI_E;
        mKeyIdentifiers[OIS::KC_R] = Rocket::Core::Input::KI_R;
        mKeyIdentifiers[OIS::KC_T] = Rocket::Core::Input::KI_T;
        mKeyIdentifiers[OIS::KC_Y] = Rocket::Core::Input::KI_Y;
        mKeyIdentifiers[OIS::KC_U] = Rocket::Core::Input::KI_U;
        mKeyIdentifiers[OIS::KC_I] = Rocket::Core::Input::KI_I;
        mKeyIdentifiers[OIS::KC_O] = Rocket::Core::Input::KI_O;
        mKeyIdentifiers[OIS::KC_P] = Rocket::Core::Input::KI_P;
        mKeyIdentifiers[OIS::KC_LBRACKET] = Rocket::Core::Input::KI_OEM_4;
        mKeyIdentifiers[OIS::KC_RBRACKET] = Rocket::Core::Input::KI_OEM_6;
        mKeyIdentifiers[OIS::KC_RETURN] = Rocket::Core::Input::KI_RETURN;
        mKeyIdentifiers[OIS::KC_LCONTROL] = Rocket::Core::Input::KI_LCONTROL;
        mKeyIdentifiers[OIS::KC_A] = Rocket::Core::Input::KI_A;
        mKeyIdentifiers[OIS::KC_S] = Rocket::Core::Input::KI_S;
        mKeyIdentifiers[OIS::KC_D] = Rocket::Core::Input::KI_D;
        mKeyIdentifiers[OIS::KC_F] = Rocket::Core::Input::KI_F;
        mKeyIdentifiers[OIS::KC_G] = Rocket::Core::Input::KI_G;
        mKeyIdentifiers[OIS::KC_H] = Rocket::Core::Input::KI_H;
        mKeyIdentifiers[OIS::KC_J] = Rocket::Core::Input::KI_J;
        mKeyIdentifiers[OIS::KC_K] = Rocket::Core::Input::KI_K;
        mKeyIdentifiers[OIS::KC_L] = Rocket::Core::Input::KI_L;
        mKeyIdentifiers[OIS::KC_SEMICOLON] = Rocket::Core::Input::KI_OEM_1;
        mKeyIdentifiers[OIS::KC_APOSTROPHE] = Rocket::Core::Input::KI_OEM_7;
        mKeyIdentifiers[OIS::KC_GRAVE] = Rocket::Core::Input::KI_OEM_3;
        mKeyIdentifiers[OIS::KC_LSHIFT] = Rocket::Core::Input::KI_LSHIFT;
        mKeyIdentifiers[OIS::KC_BACKSLASH] = Rocket::Core::Input::KI_OEM_5;
        mKeyIdentifiers[OIS::KC_Z] = Rocket::Core::Input::KI_Z;
        mKeyIdentifiers[OIS::KC_X] = Rocket::Core::Input::KI_X;
        mKeyIdentifiers[OIS::KC_C] = Rocket::Core::Input::KI_C;
        mKeyIdentifiers[OIS::KC_V] = Rocket::Core::Input::KI_V;
        mKeyIdentifiers[OIS::KC_B] = Rocket::Core::Input::KI_B;
        mKeyIdentifiers[OIS::KC_N] = Rocket::Core::Input::KI_N;
        mKeyIdentifiers[OIS::KC_M] = Rocket::Core::Input::KI_M;
        mKeyIdentifiers[OIS::KC_COMMA] = Rocket::Core::Input::KI_OEM_COMMA;
        mKeyIdentifiers[OIS::KC_PERIOD] = Rocket::Core::Input::KI_OEM_PERIOD;
        mKeyIdentifiers[OIS::KC_SLASH] = Rocket::Core::Input::KI_OEM_2;
        mKeyIdentifiers[OIS::KC_RSHIFT] = Rocket::Core::Input::KI_RSHIFT;
        mKeyIdentifiers[OIS::KC_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
        mKeyIdentifiers[OIS::KC_LMENU] = Rocket::Core::Input::KI_LMENU;
        mKeyIdentifiers[OIS::KC_SPACE] = Rocket::Core::Input::KI_SPACE;
        mKeyIdentifiers[OIS::KC_CAPITAL] = Rocket::Core::Input::KI_CAPITAL;
        mKeyIdentifiers[OIS::KC_F1] = Rocket::Core::Input::KI_F1;
        mKeyIdentifiers[OIS::KC_F2] = Rocket::Core::Input::KI_F2;
        mKeyIdentifiers[OIS::KC_F3] = Rocket::Core::Input::KI_F3;
        mKeyIdentifiers[OIS::KC_F4] = Rocket::Core::Input::KI_F4;
        mKeyIdentifiers[OIS::KC_F5] = Rocket::Core::Input::KI_F5;
        mKeyIdentifiers[OIS::KC_F6] = Rocket::Core::Input::KI_F6;
        mKeyIdentifiers[OIS::KC_F7] = Rocket::Core::Input::KI_F7;
        mKeyIdentifiers[OIS::KC_F8] = Rocket::Core::Input::KI_F8;
        mKeyIdentifiers[OIS::KC_F9] = Rocket::Core::Input::KI_F9;
        mKeyIdentifiers[OIS::KC_F10] = Rocket::Core::Input::KI_F10;
        mKeyIdentifiers[OIS::KC_NUMLOCK] = Rocket::Core::Input::KI_NUMLOCK;
        mKeyIdentifiers[OIS::KC_SCROLL] = Rocket::Core::Input::KI_SCROLL;
        mKeyIdentifiers[OIS::KC_NUMPAD7] = Rocket::Core::Input::KI_7;
        mKeyIdentifiers[OIS::KC_NUMPAD8] = Rocket::Core::Input::KI_8;
        mKeyIdentifiers[OIS::KC_NUMPAD9] = Rocket::Core::Input::KI_9;
        mKeyIdentifiers[OIS::KC_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
        mKeyIdentifiers[OIS::KC_NUMPAD4] = Rocket::Core::Input::KI_4;
        mKeyIdentifiers[OIS::KC_NUMPAD5] = Rocket::Core::Input::KI_5;
        mKeyIdentifiers[OIS::KC_NUMPAD6] = Rocket::Core::Input::KI_6;
        mKeyIdentifiers[OIS::KC_ADD] = Rocket::Core::Input::KI_ADD;
        mKeyIdentifiers[OIS::KC_NUMPAD1] = Rocket::Core::Input::KI_1;
        mKeyIdentifiers[OIS::KC_NUMPAD2] = Rocket::Core::Input::KI_2;
        mKeyIdentifiers[OIS::KC_NUMPAD3] = Rocket::Core::Input::KI_3;
        mKeyIdentifiers[OIS::KC_NUMPAD0] = Rocket::Core::Input::KI_0;
        mKeyIdentifiers[OIS::KC_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
        mKeyIdentifiers[OIS::KC_OEM_102] = Rocket::Core::Input::KI_OEM_102;
        mKeyIdentifiers[OIS::KC_F11] = Rocket::Core::Input::KI_F11;
        mKeyIdentifiers[OIS::KC_F12] = Rocket::Core::Input::KI_F12;
        mKeyIdentifiers[OIS::KC_F13] = Rocket::Core::Input::KI_F13;
        mKeyIdentifiers[OIS::KC_F14] = Rocket::Core::Input::KI_F14;
        mKeyIdentifiers[OIS::KC_F15] = Rocket::Core::Input::KI_F15;
        mKeyIdentifiers[OIS::KC_KANA] = Rocket::Core::Input::KI_KANA;
        mKeyIdentifiers[OIS::KC_ABNT_C1] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_CONVERT] = Rocket::Core::Input::KI_CONVERT;
        mKeyIdentifiers[OIS::KC_NOCONVERT] = Rocket::Core::Input::KI_NONCONVERT;
        mKeyIdentifiers[OIS::KC_YEN] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_ABNT_C2] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_NUMPADEQUALS] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;
        mKeyIdentifiers[OIS::KC_PREVTRACK] = Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
        mKeyIdentifiers[OIS::KC_AT] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_COLON] = Rocket::Core::Input::KI_OEM_1;
        mKeyIdentifiers[OIS::KC_UNDERLINE] = Rocket::Core::Input::KI_OEM_MINUS;
        mKeyIdentifiers[OIS::KC_KANJI] = Rocket::Core::Input::KI_KANJI;
        mKeyIdentifiers[OIS::KC_STOP] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_AX] = Rocket::Core::Input::KI_OEM_AX;
        mKeyIdentifiers[OIS::KC_UNLABELED] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_NEXTTRACK] = Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
        mKeyIdentifiers[OIS::KC_NUMPADENTER] = Rocket::Core::Input::KI_NUMPADENTER;
        mKeyIdentifiers[OIS::KC_RCONTROL] = Rocket::Core::Input::KI_RCONTROL;
        mKeyIdentifiers[OIS::KC_MUTE] = Rocket::Core::Input::KI_VOLUME_MUTE;
        mKeyIdentifiers[OIS::KC_CALCULATOR] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_PLAYPAUSE] = Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
        mKeyIdentifiers[OIS::KC_MEDIASTOP] = Rocket::Core::Input::KI_MEDIA_STOP;
        mKeyIdentifiers[OIS::KC_VOLUMEDOWN] = Rocket::Core::Input::KI_VOLUME_DOWN;
        mKeyIdentifiers[OIS::KC_VOLUMEUP] = Rocket::Core::Input::KI_VOLUME_UP;
        mKeyIdentifiers[OIS::KC_WEBHOME] = Rocket::Core::Input::KI_BROWSER_HOME;
        mKeyIdentifiers[OIS::KC_NUMPADCOMMA] = Rocket::Core::Input::KI_SEPARATOR;
        mKeyIdentifiers[OIS::KC_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
        mKeyIdentifiers[OIS::KC_SYSRQ] = Rocket::Core::Input::KI_SNAPSHOT;
        mKeyIdentifiers[OIS::KC_RMENU] = Rocket::Core::Input::KI_RMENU;
        mKeyIdentifiers[OIS::KC_PAUSE] = Rocket::Core::Input::KI_PAUSE;
        mKeyIdentifiers[OIS::KC_HOME] = Rocket::Core::Input::KI_HOME;
        mKeyIdentifiers[OIS::KC_UP] = Rocket::Core::Input::KI_UP;
        mKeyIdentifiers[OIS::KC_PGUP] = Rocket::Core::Input::KI_PRIOR;
        mKeyIdentifiers[OIS::KC_LEFT] = Rocket::Core::Input::KI_LEFT;
        mKeyIdentifiers[OIS::KC_RIGHT] = Rocket::Core::Input::KI_RIGHT;
        mKeyIdentifiers[OIS::KC_END] = Rocket::Core::Input::KI_END;
        mKeyIdentifiers[OIS::KC_DOWN] = Rocket::Core::Input::KI_DOWN;
        mKeyIdentifiers[OIS::KC_PGDOWN] = Rocket::Core::Input::KI_NEXT;
        mKeyIdentifiers[OIS::KC_INSERT] = Rocket::Core::Input::KI_INSERT;
        mKeyIdentifiers[OIS::KC_DELETE] = Rocket::Core::Input::KI_DELETE;
        mKeyIdentifiers[OIS::KC_LWIN] = Rocket::Core::Input::KI_LWIN;
        mKeyIdentifiers[OIS::KC_RWIN] = Rocket::Core::Input::KI_RWIN;
        mKeyIdentifiers[OIS::KC_APPS] = Rocket::Core::Input::KI_APPS;
        mKeyIdentifiers[OIS::KC_POWER] = Rocket::Core::Input::KI_POWER;
        mKeyIdentifiers[OIS::KC_SLEEP] = Rocket::Core::Input::KI_SLEEP;
        mKeyIdentifiers[OIS::KC_WAKE] = Rocket::Core::Input::KI_WAKE;
        mKeyIdentifiers[OIS::KC_WEBSEARCH] = Rocket::Core::Input::KI_BROWSER_SEARCH;
        mKeyIdentifiers[OIS::KC_WEBFAVORITES] = Rocket::Core::Input::KI_BROWSER_FAVORITES;
        mKeyIdentifiers[OIS::KC_WEBREFRESH] = Rocket::Core::Input::KI_BROWSER_REFRESH;
        mKeyIdentifiers[OIS::KC_WEBSTOP] = Rocket::Core::Input::KI_BROWSER_STOP;
        mKeyIdentifiers[OIS::KC_WEBFORWARD] = Rocket::Core::Input::KI_BROWSER_FORWARD;
        mKeyIdentifiers[OIS::KC_WEBBACK] = Rocket::Core::Input::KI_BROWSER_BACK;
        mKeyIdentifiers[OIS::KC_MYCOMPUTER] = Rocket::Core::Input::KI_UNKNOWN;
        mKeyIdentifiers[OIS::KC_MAIL] = Rocket::Core::Input::KI_LAUNCH_MAIL;
        mKeyIdentifiers[OIS::KC_MEDIASELECT] = Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;
    }

    int UI::getKeyModifierState()
    {
        int modifier_state = 0;

        if (mInputMan->isModifierDown(OIS::Keyboard::Ctrl))
            modifier_state |= Rocket::Core::Input::KM_CTRL;
        if (mInputMan->isModifierDown(OIS::Keyboard::Shift))
            modifier_state |= Rocket::Core::Input::KM_SHIFT;
        if (mInputMan->isModifierDown(OIS::Keyboard::Alt))
            modifier_state |= Rocket::Core::Input::KM_ALT;

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32

        if (GetKeyState(VK_CAPITAL) > 0)
            modifier_state |= Rocket::Core::Input::KM_CAPSLOCK;
        if (GetKeyState(VK_NUMLOCK) > 0)
            modifier_state |= Rocket::Core::Input::KM_NUMLOCK;
        if (GetKeyState(VK_SCROLL) > 0)
            modifier_state |= Rocket::Core::Input::KM_SCROLLLOCK;

#elif OGRE_PLATFORM == OGRE_PLATFORM_APPLE

        UInt32 key_modifiers = GetCurrentEventKeyModifiers();
        if (key_modifiers & (1 << alphaLockBit))
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

