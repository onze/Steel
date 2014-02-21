#include <iostream>

#include "tools/ConfigFile.h"
#include <tools/StringUtils.h>
#include <tools/JsonUtils.h>
#include <Debug.h>
#include <tests/UnitTestManager.h>

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

        if(!reader.parse(mFile.read(), settings, true))
        {
            // can't debug
            if(Debug::isInit)
                Debug::error("in ConfigFile::load(): can't parse configFile ")(mFile).endl();
            else
            {
                std::cerr << "in ConfigFile::load(): can't parse configFile " << mFile.fullPath().c_str() << std::endl;
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

    int ConfigFile::getSettingAsBool(const Ogre::String &key, bool defaultValue) const
    {
        bool returned = defaultValue;

        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
                returned = JsonUtils::asBool(mSettings[key], defaultValue);
        }

        return returned;
    }

    int ConfigFile::getSettingAsInt(const Ogre::String &key, int defaultValue) const
    {
        int returned = defaultValue;

        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
                returned = JsonUtils::asInt(mSettings[key], defaultValue);
        }

        return returned;
    }

    float ConfigFile::getSettingAsFloat(const Ogre::String &key, float defaultValue) const
    {
        float returned = defaultValue;

        if(mSettings.size() != 0)
        {

            if(mSettings.isMember(key))
                returned = JsonUtils::asFloat(mSettings[key], defaultValue);
        }

        return returned;
    }

    unsigned long ConfigFile::getSettingAsUnsignedLong(const Ogre::String &key, long unsigned int defaultValue) const
    {
        unsigned long returned = defaultValue;

        if(mSettings.size() != 0)
        {
            if(mSettings.isMember(key))
                returned = JsonUtils::asUnsignedLong(mSettings[key], defaultValue);
        }

        return returned;
    }

    Ogre::String ConfigFile::getSetting(const Ogre::String &key, const Ogre::String &defaultValue) const
    {
        Ogre::String returned = defaultValue;

        if(mSettings.isMember(key))
        {
            Json::Value value = mSettings[key];

            if(value.isString())
                returned = value.asString();
            else
                returned = value.toStyledString();
        }

        return returned;
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
            STEEL_UT_ASSERT(cf.getSetting(key) == Ogre::StringUtil::BLANK, "[UT001] loading from a new file should return a blank value");
        }

        //TODO: all supported types
        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, stringValue);
            STEEL_UT_ASSERT(cf.getSetting(key) == stringValue, "[UT002] failed set/get string setting");
        }

        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, stringValue);

            ConfigFile copied(cf);
            STEEL_UT_ASSERT(copied == cf, "[UT003] config files do not equal after copy");
            STEEL_UT_ASSERT(copied.getSetting(key) == stringValue, "[UT004] failed at copying value through copy ctor");
        }

        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, stringValue);
            ConfigFile equalled(path2);
            equalled = cf;
            STEEL_UT_ASSERT(equalled == cf, "[UT005] config files do not equal after assignation");
            STEEL_UT_ASSERT(equalled.getSetting(key) == stringValue, "[UT006] failed at copying value through assignation");
        }

        {
            CLEANUP;
            ConfigFile cf(path);
            cf.setSetting(key, intValue);
            cf.save();
            ConfigFile cf2(path);
            int ret = cf2.getSettingAsInt(key);
            STEEL_UT_ASSERT(ret == intValue, "[UT007] failed persistency test");
        }

        CLEANUP;
#undef CLEANUP
        return true;
    }
}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 

