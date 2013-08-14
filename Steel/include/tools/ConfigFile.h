#ifndef STEEL_CONFIGFILE_H
#define STEEL_CONFIGFILE_H

#include <json/json.h>
#include "File.h"

namespace Steel
{

    class ConfigFile
    {
            static const Ogre::String VERSION_ATTRIBUTE_NAME;
            ConfigFile()
            {
            }
            ;
        public:
            /// Protocol version number.
            static const Ogre::String VERSION;

            ConfigFile(Steel::File file);
            ConfigFile(const ConfigFile& other);
            virtual ~ConfigFile();
            virtual ConfigFile& operator=(const ConfigFile& other);
            virtual bool operator==(const ConfigFile& other) const;

            Ogre::String getSetting(Ogre::String key) const;
            int getSettingAsBool(Ogre::String key, bool defaultValue) const;
            int getSettingAsInt(Ogre::String key, int defaultValue) const;
            float getSettingAsFloat(Ogre::String key, float defaultValue) const;
            unsigned long getSettingAsUnsignedLong(Ogre::String key, unsigned long defaultValue) const;

            ConfigFile &setSetting(Ogre::String key, Ogre::String const &value);
            ConfigFile &setSetting(Ogre::String key, Json::Value const &value);

            void load();
            void save();

            inline File& file()
            {
                return mFile;
            }

            operator Ogre::String()
            {
                return "<ConfigFile file=" + Ogre::String(mFile) + ">";
            }

        protected:
            Json::Value &settings();
            File mFile;
            Json::Value mSettings;
    };
}
#endif
// kate: indent-mode cstyle; indent-width 4; replace-tabs on; 
