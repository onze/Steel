#include "tools/ConfigFile.h"
#include <tools/StringUtils.h>

namespace Steel
{
    const Ogre::String ConfigFile::VERSION="1.0";
    const Ogre::String ConfigFile::VERSION_ATTRIBUTE_NAME="_version";

    ConfigFile::ConfigFile(File file,bool autoLoad):mFile(file),mSettings(Json::Value())
    {
        if(autoLoad)
            load();
    }

    ConfigFile::ConfigFile(const ConfigFile& o):mFile(o.mFile),mSettings(o.mSettings)
    {
        load();
    }

    ConfigFile::~ConfigFile()
    {

    }

    ConfigFile& ConfigFile::operator=(const ConfigFile& o)
    {
        if(&o==this)
            return *this;
        mFile=o.mFile;
        mSettings=o.mSettings;
        return *this;
    }

    bool ConfigFile::operator==(const ConfigFile& o) const
    {
        bool equals=true;

        equals&=mFile==o.mFile;
        equals&=mSettings==o.mSettings;

        return equals;
    }

    void ConfigFile::load()
    {
        if(!mFile.exists())
            return;

        Json::Reader reader;
        Json::Value settings;
        if(reader.parse(mFile.read(),settings,true))
        {
            // can't debug
            //Debug::error("in ConfigFile::load(): can't parse configFile ")(mFile).endl();
            return;
        }

        // version control: abort if versions differ
        Json::Value version=settings[ConfigFile::VERSION_ATTRIBUTE_NAME];
        bool validVersion=false;
        if(version.isNull())
        {
            if(Debug::isInit)
                Debug::warning(*this)("::load(): no "+ConfigFile::VERSION_ATTRIBUTE_NAME+" specified, assuming "+ConfigFile::VERSION).endl();
        }
        else if(version.asCString()==ConfigFile::VERSION)
        {
            validVersion=true;
        }

        if(validVersion)
        {
            mSettings=settings;
        }
        else
        {
            if(Debug::isInit)
            {
                Debug::log(*this)("::load(): wrong "+ConfigFile::VERSION_ATTRIBUTE_NAME+" \""+version.asString()+"\", expecting \""+ConfigFile::VERSION+"\". ");
                Debug::log("Loading cancelled.").endl();
            }
        }
    }

    void ConfigFile::save()
    {
        // little backup
        File backup(mFile.fullPath()+".back");
        if(mFile.exists())
            backup.write(mFile.read(),File::OM_OVERWRITE);

        // write settings
        mFile.write(mSettings.toStyledString(),File::OM_OVERWRITE);
    }

    ConfigFile& ConfigFile::setSetting(Ogre::String key, Ogre::String const &value)
    {
        mSettings[key]=value;
        return *this;
    }

    ConfigFile& ConfigFile::setSetting(Ogre::String key, Json::Value const &value)
    {
        mSettings[key]=value;
        return *this;
    }

    Ogre::String ConfigFile::getSetting(Ogre::String key)
    {
        if(mSettings.isMember(key))
        {
            return mSettings[key].asString();
        }
        return "";
    }

}
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
