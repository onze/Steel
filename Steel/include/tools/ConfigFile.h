#ifndef STEEL_CONFIGFILE_H
#define STEEL_CONFIGFILE_H

#include <json/json.h>

#include "steeltypes.h"
#include "File.h"
#include "StringUtils.h"

namespace Steel
{
    class UnitTestExecutionContext;
    
    class ConfigFile
    {
        static const Ogre::String VERSION_ATTRIBUTE_NAME;
        ConfigFile(){}
    public:
        /// Protocol version number.
        static const Ogre::String VERSION;

        ConfigFile(Steel::File file);
        ConfigFile(const ConfigFile &o);
        virtual ~ConfigFile();
        virtual ConfigFile &operator=(const ConfigFile &other);
        virtual bool operator==(const ConfigFile &other) const;
        virtual bool operator!=(const ConfigFile &other) const;

        bool getSetting(Ogre::String const &key, Ogre::String &dst, Ogre::String const &defaultValue = StringUtils::BLANK) const;
        bool getSetting(Ogre::String const &key, bool &dst, bool defaultValue = false) const;
        bool getSetting(Ogre::String const &key, s32 &dst, s32 defaultValue = 0) const;
        bool getSetting(Ogre::String const &key, u32 &dst, u32 defaultValue = 0U) const;
        bool getSetting(Ogre::String const &key, u64 &dst, u64 defaultValue = 0UL) const;
        bool getSetting(Ogre::String const &key, f32 &dst, f32 defaultValue = .0f) const;
        bool getSetting(Ogre::String const &key, Ogre::Vector2 &dst, Ogre::Vector2 const& defaultValue = Ogre::Vector2::ZERO) const;
        bool getSetting(Ogre::String const &key, Ogre::Vector3 &dst, Ogre::Vector3 const& defaultValue = Ogre::Vector3::ZERO) const;

        ConfigFile &setSetting(Ogre::String key, Ogre::String const &value);
        ConfigFile &setSetting(Ogre::String key, Json::Value const &value);

        void load();
        void save();

        inline File &file()
        {
            return mFile;
        }

        operator Ogre::String()
        {
            return "<ConfigFile file=" + Ogre::String(mFile) + ">";
        }

    protected:
        Json::Value &settings();
        // owned
        File mFile;
        Json::Value mSettings;
    };
    
    bool utest_ConfigFile(UnitTestExecutionContext const* context);
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
