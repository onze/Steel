#include <iostream>

#include "tools/ConfigFile.h"
#include "tools/StringUtils.h"
#include "tools/JsonUtils.h"
#include "Debug.h"
#include "tests/UnitTestManager.h"

namespace Steel
{
//    const Ogre::String ConfigFile::VERSION="1.0";
//    const Ogre::String ConfigFile::VERSION_ATTRIBUTE_NAME="_version";

    ConfigFile::ConfigFile(File file)
        : mFile(file), mSettings(Json::Value())
    {
        mSettings = Json::objectValue;
        load();
    }

    ConfigFile::ConfigFile(const ConfigFile &o): mFile(o.mFile), mSettings(o.mSettings)
    {
        load();
    }

    ConfigFile::~ConfigFile()
    {
    }

    ConfigFile &ConfigFile::operator=(const ConfigFile &o)
    {
        if(&o == this)
            return *this;

        mFile = o.mFile;
        mSettings = o.mSettings;
        return *this;
    }

    bool ConfigFile::operator!=(const ConfigFile &o) const
    {
        return !((*this) == o);
    }

    bool ConfigFile::operator==(const ConfigFile &o) const
    {
        bool equals = true;

        equals &= mFile == o.mFile;
        equals &= mSettings == o.mSettings;

        return equals;
    }

    void ConfigFile::load()
    {
        if(!mFile.exists())
            return;

        Json::Reader reader;
        Json::Value settings;

        if(!reader.parse(mFile.read(), settings, false))
        {
            // can't debug
            if(Debug::isInit)
                Debug::error(STEEL_METH_INTRO, "can't parse configFile ", mFile, ". Errors: ", reader.getFormattedErrorMessages()).endl();
            else
            {
                std::cerr << STEEL_METH_INTRO
                          <<"can't parse configFile " << mFile.fullPath().c_str()
                          << ". Errors: " << reader.getFormattedErrorMessages()
                          << std::endl;
            }

            return;
        }

        // version control: abort if versions differ
//        Json::Value version=settings[ConfigFile::VERSION_ATTRIBUTE_NAME];
//        bool validVersion=false;
//        if(version.isNull())
//        {
//            if(Debug::isInit)
//                Debug::warning(*this)("::load(): no "+ConfigFile::VERSION_ATTRIBUTE_NAME+" specified, assuming "+ConfigFile::VERSION).endl();
//            validVersion=true;
//        }
//        else if(version.asCString()==ConfigFile::VERSION)
//        {
//            validVersion=true;
//        }
//
//        if(validVersion)
//        {
        mSettings = settings;
//        }
//        else
//        {
//            Ogre::String msg="::load(): wrong "+ConfigFile::VERSION_ATTRIBUTE_NAME+" \""+version.asString()+"\", expecting \""+ConfigFile::VERSION+"\". ";
//            if(Debug::isInit)
//            {
//                Debug::log(*this)(msg);
//                Debug::log("Loading cancelled.").endl();
//            }
//            else
//            {
//                std::cerr<<(Ogre::String(*this).c_str())<<msg;
//                std::cerr<<"Loading cancelled."<<std::endl;
//            }
//        }
    }

    void ConfigFile::save()
    {
        // little backup
        File backup(mFile.fullPath() + ".back");

        if(mFile.exists())
            backup.write(mFile.read(), File::OM_OVERWRITE);

        // write settings
        mFile.write(mSettings.toStyledString(), File::OM_OVERWRITE);
    }

    ConfigFile &ConfigFile::setSetting(Ogre::String key, Ogre::String const &value)
    {
        mSettings[key] = value;
        return *this;
    }

    ConfigFile &ConfigFile::setSetting(Ogre::String key, Json::Value const &value)
    {
        mSettings[key] = value;
        return *this;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, bool &dst, bool defaultValue) const
    {
        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
            {
                dst = JsonUtils::asBool(mSettings[key], defaultValue);
                return true;
            }
        }

        dst = defaultValue;
        return false;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, s32 &dst, s32 defaultValue) const
    {
        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
            {
                dst = JsonUtils::asInt(mSettings[key], defaultValue);
                return true;
            }
        }

        dst = defaultValue;
        return false;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, u32 &dst, u32 defaultValue) const
    {
        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
            {
                dst = (u32)JsonUtils::asUnsignedLong(mSettings[key], (u64)defaultValue);
                return true;
            }
        }

        dst = defaultValue;
        return false;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, u64 &dst, u64 defaultValue) const
    {
        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
            {
                dst = JsonUtils::asUnsignedLong(mSettings[key], defaultValue);
                return true;
            }
        }

        dst = defaultValue;
        return false;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, f32 &dst, f32 defaultValue) const
    {
        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
            {
                dst = JsonUtils::asFloat(mSettings[key], defaultValue);
                return true;
            }
        }

        dst = defaultValue;
        return false;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, Ogre::String &dst, const Ogre::String &defaultValue) const
    {
        if(mSettings.isMember(key))
        {
            Json::Value value = mSettings[key];

            if(value.isString())
                dst = value.asString();
            else
                dst = value.toStyledString(); // TODO: strip quotes ?

            return true;
        }

        dst = defaultValue;
        return false;
    }

    bool ConfigFile::getSetting(const Ogre::String &key, Ogre::Vector3 &dst, Ogre::Vector3 const &defaultValue) const
    {
        if(mSettings.isMember(key))
        {
            Json::Value value = mSettings[key];

            if(value.isString())
                dst = Ogre::StringConverter::parseVector3(value.asString(), defaultValue);
            else
                dst = Ogre::StringConverter::parseVector3(value.toStyledString(), defaultValue); // TODO: strip quotes ?

            return true;
        }

        dst = defaultValue;
        return false;
    }

    bool utest_ConfigFile(UnitTestExecutionContext const *context)
    {
        const Ogre::String path = "/tmp/test.cfg";
        const Ogre::String path2 = "/tmp/test.cfg2";
#define CLEANUP File(path).rm();File(path2).rm();
        const int intValue = 8; // chosen randomly
        const Ogre::String key = "myKey";
        const Ogre::String stringValue = "myStringValue"; // chosen randomly too

        {
            CLEANUP;
            ConfigFile cf(path);
            Ogre::String dst;
            cf.getSetting(key, dst);
            STEEL_UT_ASSERT(dst == StringUtils::BLANK, "[UT001] loading  a value from a new file should return a blank value");
        }

        //TODO: all supported types
        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, stringValue);
            Ogre::String dst;
            cf.getSetting(key, dst);
            STEEL_UT_ASSERT(dst == stringValue, "[UT002] failed set/get string setting");
        }

        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, stringValue);

            ConfigFile copied(cf);
            STEEL_UT_ASSERT(copied == cf, "[UT003] config files do not equal after copy");

            Ogre::String dst;
            copied.getSetting(key, dst);
            STEEL_UT_ASSERT(dst == stringValue, "[UT004] failed at copying value through copy ctor");
        }

        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, stringValue);
            ConfigFile equalled(path2);
            equalled = cf;
            STEEL_UT_ASSERT(equalled == cf, "[UT005] config files do not equal after assignation");

            Ogre::String dst;
            equalled.getSetting(key, dst);
            STEEL_UT_ASSERT(dst == stringValue, "[UT006] failed at copying value through assignation");
        }

        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, intValue);
            cf.save();
            ConfigFile cf2(path);
            int ret = intValue + 1;
            cf2.getSetting(key, ret);
            STEEL_UT_ASSERT(ret == intValue, "[UT007] failed persistency test");
        }

        CLEANUP;
#undef CLEANUP
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

